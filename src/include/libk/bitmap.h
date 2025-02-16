#ifndef __LIBK_BITMAP_H__
#define __LIBK_BITMAP_H__

#include "../types.h"

#define ITEM_BITS                  0x8
#define ITEM_SHIFT                              0x3

#define ITEM                                    uint8_t
// #define ITEM                                    uint64_t

typedef struct _bitmap {

    ITEM* bits;

    /*  The size of bitmap in bit. */
    uint64_t size;
    /*  The starting position of next search. */
    uint64_t offset;

} bitmap_t;

void _bitmap_init(bitmap_t* bitmap, uint64_t size);
void _bitmap_set(bitmap_t* bitmap, uint64_t offset, uint64_t set);
uint64_t _bitmap_alloc(bitmap_t* bitmap, uint64_t size);
void _bitmap_fini(bitmap_t* bitmap);

#endif