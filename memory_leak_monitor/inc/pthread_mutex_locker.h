#ifndef __PTHREAD_MUTEX_LOCKER_H__
#define __PTHREAD_MUTEX_LOCKER_H__

#include <pthread.h>

class CPthreadMutexLocker
{
public:
    CPthreadMutexLocker(pthread_mutex_t* pMutex);
    ~CPthreadMutexLocker();

private:
    pthread_mutex_t* m_pMutex{nullptr};
};

#endif   //__PTHREAD_MUTEX_LOCKER_H__
