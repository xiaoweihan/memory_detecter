#ifndef __SHARED_MEMORY_OPERATOR_H__
#define __SHARED_MEMORY_OPERATOR_H__
#include "shared_memory_object.h"

class CSharedMemoryEnv;
class CSharedMemoryDataManager;
class CSharedMemoryOperator : public CSharedMemoryObject
{
public:
    CSharedMemoryOperator(CLogger* pLogger,CSharedMemoryEnv* pEnv,CSharedMemoryDataManager* pDataManager);
    ~CSharedMemoryOperator();

    // 读取数据
    void ReadSharedMemoryData(void);

private:
    CSharedMemoryEnv* m_pEnv{nullptr};
    CSharedMemoryDataManager* m_pDataManager{nullptr};
};



#endif  // __SHARED_MEMORY_OPERATOR_H__
