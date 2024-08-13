#include "../../include/libk/stdlib.h"

void krnl_panic()
{
    __asm__("hlt\n\t");
}