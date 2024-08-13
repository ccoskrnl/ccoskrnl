#include "../../include/libk/bitmap.h"

void _bitmap_set(bitmap_t* bitmap, uint64_t index)
{
    uint64_t result = (index + 1) >> 3;
    uint8_t remainder = (index + 1) & 0x7;

    if (remainder)
    {
        bitmap->bits[result] |= 1 << (remainder - 1);
    }
    else
    {
        bitmap->bits[result] |= 0x80;
    }
    
}

