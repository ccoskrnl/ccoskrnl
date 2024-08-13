#include "../../include/libk/stdlib.h"

void assert(boolean result)
{
    if (!result)
    {
        krnl_panic();
    }
    
}