#include "shared_memory_env.h"
#include <pthread.h>
#include "shared_memory_wrapper.h"
#include "Type.h"
#include "logger.h"

CSharedMemoryEnv::CSharedMemoryEnv(CLogger *pLogger, const std::string &strShmFilePath, unsigned int uMapSize) : CSharedMemoryObject(pLogger), m_nShmFd(-1),
                                                                                                                 m_strShmFilePath(strShmFilePath),
                                                                                                                 m_pMapAddress(nullptr),
                                                                                                                 m_uMapLength(uMapSize)
{
    // 不惯是否有没有先删除
    del_shm(m_strShmFilePath.c_str());

    // 创建共享内存区对象
    m_nShmFd = create_shm(m_strShmFilePath.c_str(), uMapSize);

    if (m_nShmFd < 0)
    {
        if (nullptr != m_pLogger)
        {
            m_pLogger->GetFileLogger().error("call create_shm failed! [%s:%d]", __FILE__, __LINE__);
        }
        return;
    }

    m_pMapAddress = map_shm(m_nShmFd, uMapSize, 0);

    if (nullptr == m_pMapAddress)
    {
        if (nullptr != m_pLogger)
        {
            m_pLogger->GetFileLogger().error("call map_shm failed! [%s:%d]", __FILE__, __LINE__);
        }
        return;
    }
    // 进行初始化
    if (!InitSharedMemoryHeader())
    {
        if (nullptr != m_pLogger)
        {
            m_pLogger->GetFileLogger().error("call InitSharedMemoryHeader failed! [%s:%d]", __FILE__, __LINE__);
        }
    }
}

CSharedMemoryEnv::~CSharedMemoryEnv()
{
    if (nullptr != m_pMapAddress)
    {
        umap_shm(m_pMapAddress,m_uMapLength);
        m_pMapAddress = nullptr;
    }

    if (m_nShmFd >= 0)
    {
        close_shm(m_nShmFd);
        m_nShmFd = -1;
    }

    del_shm(m_strShmFilePath.c_str());
}

bool CSharedMemoryEnv::InitSharedMemoryHeader(void)
{

    if (nullptr == m_pMapAddress || nullptr == m_pLogger)
    {
        return false;
    }

    // 开始初始化头
    LP_SHARED_MEMORY_HEADER pHeader = (LP_SHARED_MEMORY_HEADER)m_pMapAddress;

    if (nullptr == pHeader)
    {
        m_pLogger->GetFileLogger().error("pHeader is nullptr! [%s:%d]", __FILE__, __LINE__);
        return false;
    }

    pHeader->nDataNum = MAX_DATA_LENGTH;
    pHeader->processStartTime = 0;
    pHeader->processEndTime = 0;
    pHeader->processPID = 0;
    pHeader->nReadIndex = 0;
    pHeader->nWriteIndex = 0;

    // 开始初始化进程间同步的Mutex
    pthread_mutexattr_t mutexattr;
    if (0 != pthread_mutexattr_init(&mutexattr))
    {
        m_pLogger->GetFileLogger().error("call pthread_mutexattr_init failed! [%s:%d]", __FILE__, __LINE__);
        return false;
    }

    if (0 != pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED))
    {
        m_pLogger->GetFileLogger().error("call pthread_mutexattr_setpshared failed! [%s:%d]", __FILE__, __LINE__);
        pthread_mutexattr_destroy(&mutexattr);
        return false;
    }

    if (0 != pthread_mutex_init(&pHeader->process_Mutex, &mutexattr))
    {
        m_pLogger->GetFileLogger().error("call pthread_mutex_init failed! [%s:%d]", __FILE__, __LINE__);
        pthread_mutexattr_destroy(&mutexattr);
        return false;
    }
    pthread_mutexattr_destroy(&mutexattr);
    return true;
}
