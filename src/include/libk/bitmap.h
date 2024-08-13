#ifndef __LIBK_BITMAP_H__
#define __LIBK_BITMAP_H__

#include "../types.h"

#define NUMBER_OF_BITS_PER_BYTE                 0x8

typedef struct _bitmap {

    /*  The start of bitmap */
    uint8_t* bits;
    /*  The size of bitmap in bit */
    uint64_t size;
    /*  The start of last allocated bits + last allocated size */
    // uint64_t offset;

} bitmap_t;

void _bitmap_init(bitmap_t* bitmap, uint64_t size);
void _bitmap_set(bitmap_t* bitmap, uint64_t index);
// uint64_t _bitmap_alloc(bitmap_t* bitmap, uint64_t size);

#endif