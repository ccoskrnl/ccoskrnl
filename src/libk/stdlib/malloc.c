#include "../../include/libk/stdlib.h"

extern void* _mm_kmalloc(uint64_t size);
extern void _mm_kfree(void* addr);

inline void* malloc(size_t size)
{
    return _mm_kmalloc(size);
}
inline void free(void* addr)
{
    return _mm_kfree(addr);
}
