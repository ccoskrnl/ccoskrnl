#include "../../../include/types.h"
#include "../cpu/cpu_features.h"

extern void memcpy_avx(void* dstptr, const void* srcptr, uint64_t size);
extern void memcpy_sse2(void* dstptr, const void* srcptr, uint64_t size);

void* memcpyb(void* dstptr, const uint8_t* srcptr, uint64_t size)
{
    uint8_t* dest = dstptr;
    for (size_t i = 0; i < size; i++)
        dest[i] = srcptr[i];
    return dstptr;
}


void* memcpy(void* dstptr, const void* srcptr, size_t size) {

    uint64_t remainder;
    uint64_t csize;
    unsigned char* rest;

    if (size > (32 * 8)) {

        if (((uint64_t)dstptr & 0x1F) == 0 && ((uint64_t)srcptr & 0x1F) == 0) 
        {
            if (cpu_feature_support(X86_FEATURE_AVX) || cpu_feature_support(X86_FEATURE_AVX2))
                goto __256bits;
            else
                goto __64bits;
        }


        if (((uint64_t)dstptr & 0xF) == 0 && ((uint64_t)srcptr & 0xF) == 0) 
        {
            if (cpu_feature_support(X86_FEATURE_SSE3) 
                || cpu_feature_support(X86_FEATURE_SSSE3)
                || cpu_feature_support(X86_FEATURE_SSE41) 
                || cpu_feature_support(X86_FEATURE_SSE42))
                goto __128bits; 
            else
                goto __64bits;
        }


        if (((uint64_t)dstptr & 0x7) == 0 && ((uint64_t)srcptr & 0x7) == 0) 
            goto __64bits; 
        else
            return memcpyb(dstptr, srcptr, size);

    }
    else {
    
        if (((uint64_t)dstptr & 0x7) == 0 && ((uint64_t)srcptr & 0x7) == 0) 
            goto __64bits; 
        else
            return memcpyb(dstptr, srcptr, size);
    }

__256bits:
    remainder = size & 0x1F;
    csize = size & 0xFFFFFFFFFFFFFFE0;
    if (csize)
    {
        memcpy_avx(dstptr, srcptr, csize);
    }

    if (remainder)
    {
        memcpyb(((uint8_t*)dstptr + csize), ((uint8_t*)srcptr + csize), remainder);
    }

    return dstptr;	

__128bits:
    remainder = size & 0xF;
    csize = size & 0xFFFFFFFFFFFFFFF0;

    if (csize)
    {
        memcpy_sse2(dstptr, srcptr, csize);
    }

    if (remainder)
    {
        memcpyb(((uint8_t*)dstptr + csize), ((uint8_t*)srcptr + csize), remainder);
    }

    return dstptr;	


__64bits:
    remainder = size & 0x7;
    csize = size >> 3;
    uint64_t* dest = dstptr;
    uint64_t* srce = (uint64_t*)srcptr;
    for (uint64_t i = 0; i < csize; i++)
        dest[i] = srce[i];

    if (remainder)
    {
        memcpyb(((uint8_t*)dstptr + 8 * csize), ((uint8_t*)srcptr + 8 * csize), remainder);
    }

    return dstptr;	


}
