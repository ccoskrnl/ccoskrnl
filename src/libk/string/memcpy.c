#include "../../include/libk/string.h"
#include "../../include/arch/cpu_features.h"

extern void memcpy_avx(void* dstptr, const void* srcptr, uint64_t size);
extern void memcpy_sse2(void* dstptr, const void* srcptr, uint64_t size);

void* memcpy(void* dstptr, const void* srcptr, size_t size) {

	uint64_t remainder;
	uint64_t csize;
	unsigned char* rest;

	if (support_avx || support_avx2)
	{
        remainder = size & 0x1F;
        csize = size & 0xFFFFFFFFFFFFFFE0;
        if (csize)
        {
            memcpy_avx(dstptr, srcptr, csize);
        }
        
        if (remainder)
        {
            goto copying_rest_of_size;
        }

        return dstptr;	
	}
	else if (support_sse3 || support_sse41 || support_sse42 || support_ssse3)
	{
        remainder = size & 0xF;
        csize = size & 0xFFFFFFFFFFFFFFF0;

        if (csize)
        {
            memcpy_sse2(dstptr, srcptr, csize);
        }
        
        if (remainder)
        {
            goto copying_rest_of_size;
        }

        return dstptr;	

	}
	else
	{
        remainder = size & 0x8;
        csize = size & 0xFFFFFFFFFFFFFFF8;
        uint64_t* dest = dstptr;
		uint64_t* srce = (uint64_t*)srcptr;
        for (uint64_t i = 0; i < csize; i++)
            dest[i] = srce[i];
        
        if (remainder)
        {
            goto copying_rest_of_size;
        }

        return dstptr;	
	}
	

copying_rest_of_size:
	rest = (unsigned char*)(dstptr + csize);
	const unsigned char* src = (const unsigned char*)(srcptr + csize);
	for (size_t i = 0; i < remainder; i++)
		rest[i] = src[i];
	return dstptr;
}