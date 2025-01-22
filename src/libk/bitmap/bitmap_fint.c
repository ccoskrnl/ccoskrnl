#include "../../include/libk/bitmap.h"
#include "../../include/libk/stdlib.h"

void _bitmap_fint(bitmap_t* bitmap)
{
    assert(bitmap->bits != NULL);
    free(bitmap->bits);
}