//#define _GNU_SOURCE
#include "memory_leak_hunter.h"
#include <dlfcn.h>
#include <stdio.h>
#define UNW_LOCAL_ONLY
extern "C"
{
#include <libunwind.h>
}
#include "Type.h"
#include "shared_memory_wrapper.h"

using preal_calloc = void *(*)(size_t nmemb, size_t size);

using preal_malloc = void *(*)(size_t size);

using preal_free = void (*)(void *ptr);

using preal_realloc = void *(*)(void *ptr, size_t size);

static preal_calloc s_preal_callocfun = NULL;
static preal_malloc s_preal_mallocfun = NULL;
static preal_free s_preal_freefun = NULL;
static preal_realloc s_preal_reallocfun = NULL;
static pthread_mutex_t s_Mutex = PTHREAD_MUTEX_INITIALIZER;

// 内存映射区域的头
static LP_SHARED_MEMORY_HEADER s_pHeader = NULL;
// 内存映射区域的数据
static LP_SHARED_MEMORY_DATA s_pData = NULL;
// 内存映射对象
static int s_ShmFd = -1;

__attribute((constructor)) void memory_leak_hunter_init()
{
    // 开始打开共享内存
    const char *szName = "memory_leak_hunter";

    int nShmFd = open_shm(szName);
    if (nShmFd < 0)
    {
        fprintf(stderr, "In [%s] call open_shm failed.\n", __FUNCTION__);
        _exit(1);
    }
    s_ShmFd = nShmFd;

    unsigned int uMapSize = sizeof(SHARED_MEMORY_HEADER) + sizeof(SHARED_MEMORY_DATA) * MAX_DATA_LENGTH;

    void *pAddress = map_shm(nShmFd, uMapSize, 0);
    if (NULL == pAddress)
    {
        fprintf(stderr, "In [%s] call map_shm failed.\n", __FUNCTION__);
        close(nShmFd);
        _exit(1);
    }

    // 数据头
    s_pHeader = (LP_SHARED_MEMORY_HEADER)pAddress;

    // 数据区
    s_pData = (LP_SHARED_MEMORY_DATA)(s_pHeader + 1);

    //写入数据
    pthread_mutex_lock(&(s_pHeader->process_Mutex));

    //获取启动时间
    s_pHeader->processStartTime = time(NULL);
    s_pHeader->processPID = getpid();

    pthread_mutex_unlock(&(s_pHeader->process_Mutex));

    s_preal_mallocfun = (preal_malloc)dlsym(RTLD_NEXT, "malloc");
    if (!s_preal_mallocfun)
    {
        fprintf(stderr, "In [%s] get real malloc function failed.\n", __FUNCTION__);
        umap_shm(pAddress, uMapSize);
        close(nShmFd);
        s_pHeader = NULL;
        s_pData = NULL;
        _exit(1);
    }

    s_preal_callocfun = (preal_calloc)dlsym(RTLD_NEXT, "calloc");
    if (!s_preal_callocfun)
    {
        fprintf(stderr, "In [%s] get real calloc function failed.\n", __FUNCTION__);
        umap_shm(pAddress, uMapSize);
        close(nShmFd);
        s_pHeader = NULL;
        s_pData = NULL;
        _exit(1);
    }

    s_preal_reallocfun = (preal_realloc)dlsym(RTLD_NEXT, "realloc");
    if (!s_preal_reallocfun)
    {
        fprintf(stderr, "In [%s] get real realloc function failed.\n", __FUNCTION__);
        umap_shm(pAddress, uMapSize);
        close(nShmFd);
        s_pHeader = NULL;
        s_pData = NULL;
        _exit(1);
    }

    s_preal_freefun = (preal_free)dlsym(RTLD_NEXT, "free");
    if (!s_preal_freefun)
    {
        fprintf(stderr, "In [%s] get real free function failed.\n", __FUNCTION__);
        umap_shm(pAddress, uMapSize);
        close(nShmFd);
        s_pHeader = NULL;
        s_pData = NULL;
        _exit(1);
    }
}

__attribute((destructor)) void memory_leak_hunter_fini()
{
    s_preal_callocfun = NULL;
    s_preal_mallocfun = NULL;
    s_preal_freefun = NULL;
    s_preal_reallocfun = NULL;

    // 写入进程退出信息
    pthread_mutex_lock(&(s_pHeader->process_Mutex));
    // 写入进程结束时间
    s_pHeader->processEndTime = time(NULL);
    s_pHeader->processPID = getpid();
    pthread_mutex_unlock(&(s_pHeader->process_Mutex));
    unsigned int uMapSize = sizeof(SHARED_MEMORY_HEADER) + sizeof(SHARED_MEMORY_DATA) * MAX_DATA_LENGTH;
    umap_shm((void *)s_pHeader, uMapSize);
    close(s_ShmFd);
    s_pHeader = NULL;
    s_pData = NULL;
}

void *malloc(size_t uSize)
{

    if (NULL == s_preal_mallocfun)
    {
        return NULL;
    }

    void *pResult = s_preal_mallocfun(uSize);

    pthread_mutex_lock(&s_Mutex);

    SHARED_MEMORY_DATA TmpData;
    TmpData.eventoccureTime = time(NULL);
    TmpData.eumEventType = MEMORY_EVENT::MALLOC_EVENT;
    TmpData.eventProcessPID = getpid();
    TmpData.pEventAddress = pResult;
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    unsigned int nLevels = 0;
    while (unw_step(&cursor) > 0 && nLevels < MAX_CALL_STACK_LEVEL)
    {
        unw_word_t offset = 0;
        unw_word_t pc = 0;
        // char fname[64] = {0};
        TmpData.CallStackArray[nLevels].szFrameName[0] = '\0';
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        (void)unw_get_proc_name(&cursor, TmpData.CallStackArray[nLevels].szFrameName, sizeof(TmpData.CallStackArray[nLevels].szFrameName), &offset);
        TmpData.CallStackArray[nLevels].uPC = pc;
        TmpData.CallStackArray[nLevels].uOffset = offset;
        ++nLevels;
        // fprintf(stderr,"%p : (%s+0x%x) [%p]\n", pc, fname, offset, pc);
    }

    if (NULL != pResult)
    {
        // 写入进程退出信息
        pthread_mutex_lock(&(s_pHeader->process_Mutex));

        // 判断队列是否已满
        if ((s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum == s_pHeader->nReadIndex)
        {
            fprintf(stderr, "In [%s] the queue is full!\n", __FUNCTION__);
        }
        else
        {
            s_pData[s_pHeader->nWriteIndex] = TmpData;
            s_pHeader->nWriteIndex = (s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum;
        }
        pthread_mutex_unlock(&(s_pHeader->process_Mutex));
    }

    pthread_mutex_unlock(&s_Mutex);

    return pResult;
}

void *calloc(size_t nBlockSize, size_t nBlockNum)
{
    if (NULL == s_preal_callocfun)
    {
        return NULL;
    }
    void *pResult = s_preal_callocfun(nBlockSize, nBlockNum);

    pthread_mutex_lock(&s_Mutex);
    SHARED_MEMORY_DATA TmpData;
    TmpData.eventoccureTime = time(NULL);
    TmpData.eumEventType = MEMORY_EVENT::CALLOC_EVENT;
    TmpData.eventProcessPID = getpid();
    TmpData.pEventAddress = pResult;
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    unsigned int nLevels = 0;
    while (unw_step(&cursor) > 0 && nLevels < MAX_CALL_STACK_LEVEL)
    {
        unw_word_t offset = 0;
        unw_word_t pc = 0;
        // char fname[64] = {0};
        // memset(TmpData.CallStackArray[nLevels].szFrameName,0,sizeof(TmpData.CallStackArray[nLevels].szFrameName));
        TmpData.CallStackArray[nLevels].szFrameName[0] = '\0';
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        (void)unw_get_proc_name(&cursor, TmpData.CallStackArray[nLevels].szFrameName, sizeof(TmpData.CallStackArray[nLevels].szFrameName), &offset);
        TmpData.CallStackArray[nLevels].uPC = pc;
        TmpData.CallStackArray[nLevels].uOffset = offset;
        ++nLevels;
        // fprintf(stderr,"%p : (%s+0x%x) [%p]\n", pc, fname, offset, pc);
    }

    if (NULL != pResult)
    {
        // 写入进程退出信息
        pthread_mutex_lock(&(s_pHeader->process_Mutex));

        // 判断队列是否已满
        if ((s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum == s_pHeader->nReadIndex)
        {
            fprintf(stderr, "In [%s] the queue is full!\n", __FUNCTION__);
        }
        else
        {
            s_pData[s_pHeader->nWriteIndex] = TmpData;
            s_pHeader->nWriteIndex = (s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum;
        }
        pthread_mutex_unlock(&(s_pHeader->process_Mutex));
    }

    pthread_mutex_unlock(&s_Mutex);
    return pResult;
}

void *realloc(void *ptr, size_t uSize)
{
    if (NULL == s_preal_reallocfun)
    {
        return NULL;
    }

    void *pResult = s_preal_reallocfun(ptr, uSize);

    pthread_mutex_lock(&s_Mutex);
    SHARED_MEMORY_DATA TmpData;
    TmpData.eventoccureTime = time(NULL);
    TmpData.eumEventType = MEMORY_EVENT::REALLOC_EVENT;
    TmpData.eventProcessPID = getpid();
    TmpData.pEventAddress = pResult;
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    unsigned int nLevels = 0;
    while (unw_step(&cursor) > 0 && nLevels < MAX_CALL_STACK_LEVEL)
    {
        unw_word_t offset = 0;
        unw_word_t pc = 0;
        // char fname[64] = {0};
        TmpData.CallStackArray[nLevels].szFrameName[0] = '\0';
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        (void)unw_get_proc_name(&cursor, TmpData.CallStackArray[nLevels].szFrameName, sizeof(TmpData.CallStackArray[nLevels].szFrameName), &offset);
        TmpData.CallStackArray[nLevels].uPC = pc;
        TmpData.CallStackArray[nLevels].uOffset = offset;
        ++nLevels;
        // fprintf(stderr,"%p : (%s+0x%x) [%p]\n", pc, fname, offset, pc);
    }

    if (NULL != pResult)
    {
        // 写入进程退出信息
        pthread_mutex_lock(&(s_pHeader->process_Mutex));

        // 判断队列是否已满
        if ((s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum == s_pHeader->nReadIndex)
        {
            fprintf(stderr, "In [%s] the queue is full!\n", __FUNCTION__);
        }
        else
        {
            s_pData[s_pHeader->nWriteIndex] = TmpData;
            s_pHeader->nWriteIndex = (s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum;
        }
        pthread_mutex_unlock(&(s_pHeader->process_Mutex));
    }
    pthread_mutex_unlock(&s_Mutex);
    return pResult;
}

void free(void *ptr)
{
    if (NULL == s_preal_freefun || NULL == ptr)
    {
        return;
    }
    pthread_mutex_lock(&s_Mutex);
    SHARED_MEMORY_DATA TmpData;
    TmpData.eventoccureTime = time(NULL);
    TmpData.eumEventType = MEMORY_EVENT::FREE_EVENT;
    TmpData.eventProcessPID = getpid();
    TmpData.pEventAddress = ptr;
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    unsigned int nLevels = 0;
    while (unw_step(&cursor) > 0 && nLevels < MAX_CALL_STACK_LEVEL)
    {
        unw_word_t offset = 0;
        unw_word_t pc = 0;
        // char fname[64] = {0};
        TmpData.CallStackArray[nLevels].szFrameName[0] = '\0';
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        (void)unw_get_proc_name(&cursor, TmpData.CallStackArray[nLevels].szFrameName, sizeof(TmpData.CallStackArray[nLevels].szFrameName), &offset);
        TmpData.CallStackArray[nLevels].uPC = pc;
        TmpData.CallStackArray[nLevels].uOffset = offset;
        ++nLevels;
        // fprintf(stderr,"%p : (%s+0x%x) [%p]\n", pc, fname, offset, pc);
    }

    {
        // 写入进程退出信息
        pthread_mutex_lock(&(s_pHeader->process_Mutex));
        // 判断队列是否已满
        if ((s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum == s_pHeader->nReadIndex)
        {
            fprintf(stderr, "In [%s] the queue is full!\n", __FUNCTION__);
        }
        else
        {
            s_pData[s_pHeader->nWriteIndex] = TmpData;
            s_pHeader->nWriteIndex = (s_pHeader->nWriteIndex + 1) % s_pHeader->nDataNum;
        }
        pthread_mutex_unlock(&(s_pHeader->process_Mutex));
    }
    pthread_mutex_unlock(&s_Mutex);
    s_preal_freefun(ptr);
}
