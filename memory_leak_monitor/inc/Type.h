#ifndef __MEMORY_LEAK_HUNTER_TYPE_H__
#define __MEMORY_LEAK_HUNTER_TYPE_H__

#include <pthread.h> 
#include <time.h>
#include <unistd.h>
#include <string.h>

//调用堆栈每次的字符串长度
constexpr unsigned int MAX_BUFFER_LENGTH = 100;
// 调用堆栈最多的打印层次
constexpr unsigned int MAX_CALL_STACK_LEVEL = 5;
// 数据元素的最大个数
constexpr unsigned int MAX_DATA_LENGTH = 1000;
// Frame的最大长度
constexpr unsigned int MAX_FRAME_NAME_LENGTH = 64;
// 事件类型
enum class MEMORY_EVENT
{
    EVENT_MIN = 0,

    MALLOC_EVENT,
    CALLOC_EVENT,
    REALLOC_EVENT,
    FREE_EVENT,
    
    EVENT_MAX,

    MEMORY_EVENT_MIN = EVENT_MIN,
    MEMORY_EVENT_MAX = EVENT_MAX

};

//共享内存去索引
struct _shared_memory_header
{
    // 用于进程间同步
    pthread_mutex_t process_Mutex;
    // 后面数据区的数据个数
    unsigned int nDataNum;
    // 进程的启动时间
    time_t processStartTime;
    // 进程的退出时间
    time_t processEndTime;
    // 进程的pid
    pid_t processPID;
    // 数据区的读索引
    unsigned int nReadIndex;
    // 数据区的写索引
    unsigned int nWriteIndex;

    _shared_memory_header():nDataNum(MAX_DATA_LENGTH),processStartTime(0),processEndTime(0),processPID(0),nReadIndex(0),nWriteIndex(0)
    {

    }

    _shared_memory_header(const _shared_memory_header& CopyValue) = delete;
    _shared_memory_header& operator=(const _shared_memory_header& CopyValue) = delete;

};

using SHARED_MEMORY_HEADER = _shared_memory_header;
using LP_SHARED_MEMORY_HEADER = _shared_memory_header*;

struct _call_stack_element
{
    char szFrameName[MAX_FRAME_NAME_LENGTH];
    // PC位置
    uint64_t uPC;
    // offset
    uint64_t uOffset;

    _call_stack_element():uPC(0),uOffset(0)
    {
        memset(szFrameName,0,sizeof(szFrameName));
    }
};


using CALL_STACK_ELEMENT = _call_stack_element;
using LP_CALL_STACK_ELEMENT = _call_stack_element*;

struct _shared_memory_data
{
    // 事件发生的时间
    time_t eventoccureTime;
    // 事件类型
    MEMORY_EVENT eumEventType;
    // 发生事件的进程PID
    pid_t eventProcessPID;
    // 事件的地址
    void* pEventAddress;
    // 事件的调用堆栈
    CALL_STACK_ELEMENT CallStackArray[MAX_CALL_STACK_LEVEL];
};

using SHARED_MEMORY_DATA = _shared_memory_data;
using LP_SHARED_MEMORY_DATA = _shared_memory_data*;


#endif //__MEMORY_LEAK_HUNTER_TYPE_H__


