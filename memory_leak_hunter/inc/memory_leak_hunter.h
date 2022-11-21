#ifndef __MEMORY_LEAK_HUNTER_H__
#define __MEMORY_LEAK_HUNTER_H__
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif
__attribute((constructor)) void memory_leak_hunter_init();

__attribute((destructor)) void memory_leak_hunter_fini();

void* malloc(size_t uSize);

void* calloc(size_t nBlockSize, size_t nBlockNum);

void* realloc(void *ptr, size_t uSize);

void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
