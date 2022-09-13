#include "shared_memory_wrapper.h"
#include <sys/types.h> 
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int open_shm(const char *szName)
{
    if (NULL == szName)
    {
        return -1;
    }

    return shm_open(szName, O_CREAT | O_RDWR, 0777);
}

int create_shm(const char *szName, unsigned int nSize)
{
    if (NULL == szName || 0 == nSize)
    {
        return -1;
    }

    int nfd = shm_open(szName, O_CREAT | O_EXCL | O_RDWR, 0777);
    if (nfd < 0)
    {
        return -1;
    }

    if (ftruncate(nfd,nSize) < 0)
    {
        return -1;
    }

    return nfd;
}

void *map_shm(int nshmFd, unsigned int nSize, unsigned int nOffset)
{
    return mmap(0, nSize, PROT_READ | PROT_WRITE, MAP_SHARED, nshmFd, 0);
}

int umap_shm(void *pAddress, unsigned int nSize)
{
    if (NULL == pAddress || 0 == nSize)
    {
        return -1;
    }

    return munmap(pAddress,nSize);
}

int del_shm(const char *szName)
{
    if (NULL == szName)
    {
        return -1;
    }

    return shm_unlink(szName);
}

int close_shm(int nShmFd)
{
    if (nShmFd < 0)
    {
        return -1;
    }
    return close(nShmFd);
}