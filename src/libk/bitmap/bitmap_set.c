#include "../../include/libk/bitmap.h"
#include "../../include/libk/stdlib.h"

void _bitmap_set(bitmap_t* bitmap, uint64_t offset, uint64_t set)
{
    // Otherwise, offset out of bounds.
    assert(offset < bitmap->size);  

    uint64_t index = offset >> ITEM_SHIFT;
    uint64_t bit = offset % ITEM_BITS;

    if (set)
    {
        bitmap->bits[index] |= (ITEM)(1UL << ((ITEM_BITS - 1) - bit));
    }
    else 
    {
        bitmap->bits[index] &= ~((ITEM)(1UL << ((ITEM_BITS - 1) - bit)));
    }


}

