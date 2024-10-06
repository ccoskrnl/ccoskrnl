#include "../../include/libk/bitmap.h"
#include "../../include/libk/stdlib.h"

void _bitmap_fini(bitmap_t* bitmap)
{
    assert(bitmap->bits != NULL);
    free(bitmap->bits);
}