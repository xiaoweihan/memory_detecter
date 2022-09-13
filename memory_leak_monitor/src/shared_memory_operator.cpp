#include "shared_memory_operator.h"
#include <pthread.h>
#include <new>
#include "shared_memory_wrapper.h"
#include "shared_memory_env.h"
#include "pthread_mutex_locker.h"
#include "shared_memory_data_manager.h"
#include "Type.h"
#include "logger.h"

CSharedMemoryOperator::CSharedMemoryOperator(CLogger *pLogger, CSharedMemoryEnv *pEnv, CSharedMemoryDataManager *pDataManager) : CSharedMemoryObject(pLogger), m_pEnv(pEnv), m_pDataManager(pDataManager)
{
}

CSharedMemoryOperator::~CSharedMemoryOperator()
{
    m_pEnv = nullptr;
    m_pDataManager = nullptr;
}

// 读取数据
void CSharedMemoryOperator::ReadSharedMemoryData(void)
{
    if (nullptr == m_pEnv || nullptr == m_pDataManager)
    {
        return;
    }
    //获取映射的内存地址
    void *pMapAddress = m_pEnv->GetMapAddress();
    if (nullptr == pMapAddress)
    {
        m_pLogger->GetFileLogger().error("GetMapAddress failed! [%s:%d]", __FILE__, __LINE__);
        return;
    }

    LP_SHARED_MEMORY_HEADER pHeader = (LP_SHARED_MEMORY_HEADER)m_pEnv->GetMapAddress();
    if (nullptr == pHeader)
    {
        m_pLogger->GetFileLogger().error("GetHeader failed! [%s:%d]", __FILE__, __LINE__);
        return;
    }

    // 数据的指针
    LP_SHARED_MEMORY_DATA pDataArray = (LP_SHARED_MEMORY_DATA)(pHeader + 1);
    if (nullptr == pDataArray)
    {
        m_pLogger->GetFileLogger().error("GetData failed! [%s:%d]", __FILE__, __LINE__);
        return;
    }

    {
        CPthreadMutexLocker(&pHeader->process_Mutex);

        // 进程的启动时间
        time_t StartTime = pHeader->processStartTime;

        // 进程的结束时间
        time_t EndTime = pHeader->processEndTime;

        // 进程的Pid
        pid_t nPID = pHeader->processPID;

        //设置头信息
        if (0 != StartTime || 0 != EndTime || 0 != nPID)
        {
            m_pDataManager->SetMemoryDataHeader(MEMORY_EVENT_HEADER(StartTime, EndTime, nPID));
        }

        // 数据区的读索引
        unsigned int uReadIndex = pHeader->nReadIndex;

        // 数据区的写索引
        unsigned int uWriteIndex = pHeader->nWriteIndex;
        //判断数据区的队列是否空，如果为空，则不读取
        if (uReadIndex == uWriteIndex)
        {
            //m_pLogger->GetFileLogger().info("queue is empty! [%s:%d]", __FILE__, __LINE__);
            return;
        }

        //开始读数据
        LP_SHARED_MEMORY_DATA pData = new (std::nothrow) SHARED_MEMORY_DATA();

        if (nullptr == pData)
        {
            m_pLogger->GetFileLogger().error("allocate SHARED_MEMORY_DATA obj failed! [%s:%d]", __FILE__, __LINE__);
            return;
        }

        *pData = pDataArray[uReadIndex];

        pHeader->nReadIndex = (pHeader->nReadIndex + 1) % pHeader->nDataNum;

        // 通知有时间发生
        m_pDataManager->NotifyEvent(pData);
    }
}
