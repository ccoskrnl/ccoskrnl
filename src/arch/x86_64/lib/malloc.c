#include "../../../include/types.h"
#include "../mm/mm_pool.h"

void* malloc(size_t size)
{
    return _mm_kmalloc(size);
}
void free(void* addr)
{
    return _mm_kfree(addr);
}
