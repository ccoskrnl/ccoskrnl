#include "../../../include/types.h"
#include "../cpu/cpu_features.h"
#include "../../../include/libk/stdlib.h"

extern void memzero_sse2(void* dst, uint64_t size);
extern void memzero_avx(void* dst, uint64_t size);

void memzero(void* dst, uint64_t size)
{
    uint64_t remainder;
    uint64_t csize;
    unsigned char* rest;

    if (size > (32 * 8))
    {
        if (((uint64_t)dst & 0x1F) == 0) 
        {

            if (cpu_feature_support(X86_FEATURE_AVX) || cpu_feature_support(X86_FEATURE_AVX2))
                goto __256bits;
            else
                goto __64bits;
        }


        if (((uint64_t)dst & 0xF) == 0) 
        {

            if (cpu_feature_support(X86_FEATURE_SSE3) 
                || cpu_feature_support(X86_FEATURE_SSSE3)
                || cpu_feature_support(X86_FEATURE_SSE41) 
                || cpu_feature_support(X86_FEATURE_SSE42))
                goto __128bits; 
            else
                goto __64bits;
        }

        if (((uint64_t)dst & 0x7) == 0) 
            goto __64bits; 
        else
        {
            memset(dst, 0, size);
            return;
        }
    }
    else {
        goto __64bits;
    }

__256bits:
    remainder = size & 0x1F;
    csize = size & 0xFFFFFFFFFFFFFFE0;
    if (csize)
    {
        memzero_avx(dst, csize);
    }
    if (remainder)
    {
        memset(((uint8_t*)dst + csize), 0, remainder);
    }

    return;

__128bits:
    remainder = size & 0xF;
    csize = size & 0xFFFFFFFFFFFFFFF0;
    if (csize)
    {
        memzero_sse2(dst, csize);
    }
    if (remainder)
    {
        memset(((uint8_t*)dst + csize), 0, remainder);
    }

    return;

__64bits:
    remainder = size & 0x7;
    csize = size >> 3;
    uint64_t* dest = dst;
    for (uint64_t i = 0; i < csize; i++)
        dest[i] = 0;

    if (remainder)
    {
        memset(((uint8_t*)dst + 8 * csize), 0, remainder);
    }

    return;

}
