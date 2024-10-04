#include "../../../include/types.h"
#include "../../../include/libk/stdlib.h"
#include "../mm/mm_pool.h"

void* calloc(size_t size)
{
    void* ptr = _mm_kmalloc(size);
    if (ptr != NULL) 
        memzero(ptr, size); 

    return ptr;
}

void* malloc(size_t size)
{
    return _mm_kmalloc(size);
}
void free(void* addr)
{
    return _mm_kfree(addr);
}
