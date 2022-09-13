#ifndef __SHARED__MEMORY_ENV_H__
#define __SHARED__MEMORY_ENV_H__
#include <string>
#include "shared_memory_object.h"
class CSharedMemoryEnv : public CSharedMemoryObject
{
public:
    CSharedMemoryEnv(CLogger* pLogger,const std::string& strShmFilePath,unsigned int uMapSize);
    ~CSharedMemoryEnv();

    unsigned int GetMapSize(void) const
    {
        return m_uMapLength;
    }

    void* GetMapAddress(void) const
    {
        return m_pMapAddress;
    }

    std::string GetShmFilePath(void) const
    {
        return m_strShmFilePath;
    }

    bool InitSharedMemoryHeader(void);

private:
    // 内存共享对象句柄
    int m_nShmFd{-1};
    // 内存共享对象所在的文件路径
    std::string m_strShmFilePath;
    //映射的内存地址
    void* m_pMapAddress{nullptr};
    // 映射的长度
    unsigned int m_uMapLength{0};

};







#endif   //__SHARED__MEMORY_ENV_H__
