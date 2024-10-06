#include "../../include/libk/bitmap.h"
#include "../../include/libk/stdlib.h"

void _bitmap_init(bitmap_t* bitmap, uint64_t size)
{
    bitmap->bits = calloc(size >> ITEM_SHIFT);
    assert(bitmap->bits != NULL);

    bitmap->size = size;
    bitmap->offset = 0;
}