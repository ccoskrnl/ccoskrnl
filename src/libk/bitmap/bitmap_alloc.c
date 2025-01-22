#include "../../include/libk/bitmap.h"

uint64_t _bitmap_alloc(bitmap_t* bitmap, uint64_t size)
{
    // The starting position of this search.
    uint64_t starting_position_in_byte;
    uint64_t starting_position_in_bit;
    uint64_t offset_in_byte;
    uint64_t start = -1;
    uint64_t free_bits;

    // The first search - range: offset ~ bitmap_end
    free_bits = 0;
    starting_position_in_byte = bitmap->offset >> ITEM_SHIFT;
    for (; starting_position_in_byte < (bitmap->size >> ITEM_SHIFT); starting_position_in_byte++) 
    {
        for (offset_in_byte = 0; offset_in_byte < ITEM_BITS; offset_in_byte++)
        {
            if (bitmap->bits[starting_position_in_byte] & 
                (ITEM)(1UL << (ITEM_BITS - offset_in_byte - 1)))
            {
                free_bits = 0;
            }
            else
            {
                free_bits++;
                if (free_bits == size)
                {
                    start = ((starting_position_in_byte << ITEM_SHIFT) + offset_in_byte) - (size - 1);
                    bitmap->offset = start + size;
                    for (int i = 0; i < size; i++)
                        _bitmap_set(bitmap, start+i, 1);

                    goto _allocated;
                }
                
            }
        }
    }

    if (start != -1)
        goto _allocated;

    // The second search - range: 0 ~ offset
    free_bits = 0;
    starting_position_in_byte = 0;
    for (; starting_position_in_byte < (bitmap->offset >> ITEM_SHIFT); starting_position_in_byte++) 
    {
        for (offset_in_byte = 0; offset_in_byte < ITEM_BITS; offset_in_byte++)
        {
            if ((bitmap->bits[starting_position_in_byte] & 
                (ITEM)(1UL << (ITEM_BITS - offset_in_byte - 1))))
            {
                free_bits = 0;
            }
            else
            {
                free_bits++;
                if (free_bits == size)
                {
                    start = ((starting_position_in_byte << ITEM_SHIFT) + offset_in_byte) - (size - 1);
                    bitmap->offset = start + size;
                    for (int i = 0; i < size; i++)
                        _bitmap_set(bitmap, start+i, 1);

                    goto _allocated;
                }
                
            }
        }
    }

    
_allocated:

    return start;
}