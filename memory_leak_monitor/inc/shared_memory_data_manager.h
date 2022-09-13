#ifndef __SHARED_MEMORY_DATA_MANAGER_H__
#define __SHARED_MEMORY_DATA_MANAGER_H__
#include <map>
#include <mutex>
#include <vector>
#include <string>
#include "Type.h"
#include "shared_memory_object.h"

// 数据头定义
struct _memory_event_header
{
    // 进程的启动时间
    time_t ProcessStartTime;
    // 进程的退出时间
    time_t ProcessEndTime;
    // 进程的pid
    pid_t processID;

    _memory_event_header() : ProcessStartTime(0), ProcessEndTime(0), processID(0)
    {
    }

    _memory_event_header(time_t StartTime, time_t EndTime, pid_t nPID) : ProcessStartTime(StartTime),
                                                                         ProcessEndTime(EndTime),
                                                                         processID(nPID)
    {

    }

    bool operator==(const _memory_event_header &CompareValue)
    {
        if (ProcessStartTime == CompareValue.ProcessStartTime && ProcessEndTime == CompareValue.ProcessEndTime &&
            processID == CompareValue.processID)
        {
            return true;
        }

        return false;
    }
};

using MEMORY_EVENT_HEADER = _memory_event_header;
using LP_MEMORY_EVENT_HEADER = _memory_event_header*;

class CSharedMemoryDataManager : public CSharedMemoryObject
{

public:
    CSharedMemoryDataManager(CLogger* pLogger);
    ~CSharedMemoryDataManager();

public:
    // 通知事件到来
    void NotifyEvent(const LP_SHARED_MEMORY_DATA pData);
    // 增加事件
    bool AddMemoryLeakElement(const LP_SHARED_MEMORY_DATA pData);
    // 删除事件
    bool DelMemoryLeakElement(const LP_SHARED_MEMORY_DATA pData);

    void GetMemoryLeakElement(std::vector<LP_SHARED_MEMORY_DATA>& ElementArray);

    void GetExceptionMemoryLeakElement(std::vector<LP_SHARED_MEMORY_DATA>& ElementArray);

    void SetMemoryDataHeader(const MEMORY_EVENT_HEADER& Header);

    void GetMemoryDataHeader(MEMORY_EVENT_HEADER& Header);

private:
    // 内存使用信息头
    LP_MEMORY_EVENT_HEADER m_pMemoryDataHeader = nullptr;
    // 内存使用信息
    std::map<void*,LP_SHARED_MEMORY_DATA> m_MemoryLeakInfo;
    // 内存同步对象
    std::mutex m_NormalLocker;
    // 申请内存出现相同的地址
    std::vector<LP_SHARED_MEMORY_DATA> m_ExceptionLeakInfo;
    std::mutex m_ExceptionMutex;
};



#endif    //__SHARED_MEMORY_DATA_MANAGER_H__
