#include "../../include/libk/stdlib.h"

#include "../../include/arch/cpu_features.h"

extern void memzero_sse2(void* dst, uint64_t size);
extern void memzero_avx(void* dst, uint64_t size);

void memzero(void* dst, uint64_t size)
{
    uint64_t remainder;
    uint64_t csize;
    unsigned char* rest;

	if (support_avx || support_avx2)
    {
        remainder = size & 0x1F;
        csize = size & 0xFFFFFFFFFFFFFFE0;
        if (csize)
        {
            memzero_avx(dst, csize);
        }
        if (remainder)
        {
            goto zero_rest_of_size;
        }

        return;
    } else if (support_sse3 || support_sse41 || support_sse42 || support_ssse3) {
        remainder = size & 0xF;
        csize = size & 0xFFFFFFFFFFFFFFF0;
        if (csize)
        {
            memzero_sse2(dst, csize);
        }
        if (remainder)
        {
            goto zero_rest_of_size;
        }

        return;
    } else {
        remainder = size & 0x8;
        csize = size & 0xFFFFFFFFFFFFFFF8;
        uint64_t* dest = dst;
        for (uint64_t i = 0; i < csize; i++)
            dest[i] = 0;
        
        if (remainder)
        {
            goto zero_rest_of_size;
        }

        return;
    }

zero_rest_of_size:
    rest = (unsigned char*)(dst + csize);
    for (uint64_t i = 0; i < remainder; i++)
        rest[i] = 0;
}