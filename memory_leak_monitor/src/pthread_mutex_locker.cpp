#include "pthread_mutex_locker.h"

CPthreadMutexLocker::CPthreadMutexLocker(pthread_mutex_t *pMutex):m_pMutex(pMutex)
{
    pthread_mutex_lock(m_pMutex);
}

CPthreadMutexLocker::~CPthreadMutexLocker()
{
    pthread_mutex_unlock(m_pMutex);
}
