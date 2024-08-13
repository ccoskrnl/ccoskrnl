#include "../../include/libk/bitmap.h"

uint64_t _bitmap_alloc(bitmap_t* bitmap, uint64_t size)
{
    uint64_t free_bits = 0;
    uint64_t start_bit = 0;
    uint8_t* bitmap_end = bitmap->bits + (bitmap->size >> 3);
    // uint8_t remainder = bitmap->offset & 0x7;

    // while ()
    // {
    //     /* code */
    // }
    
_allocated:
    return start_bit;
}