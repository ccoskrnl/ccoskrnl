#include "../../../include/types.h"

void* memsetd(uint32_t* dstptr, uint32_t value, size_t size_of_dst_in_dword)
{
    for (size_t i = 0; i < size_of_dst_in_dword; i++) 
        dstptr[i] = value; 

    return dstptr;
}
