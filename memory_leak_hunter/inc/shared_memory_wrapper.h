#ifndef __SHARED_MEMORY_WRAPPER_H__
#define __SHARED_MEMORY_WRAPPER_H__

#ifdef __cplusplus
extern "C"
{
#endif

    int open_shm(const char* szName);

    int create_shm(const char* szName,unsigned int nSize);

    void* map_shm(int nshmFd,unsigned int nSize,unsigned int nOffset);

    int umap_shm(void* pAddress,unsigned int nSize);

    int close_shm(int nShmFd);

    int del_shm(const char* szName);

#ifdef __cplusplus
}
#endif





#endif  // __SHARED_MEMORY_WRAPPER_H__