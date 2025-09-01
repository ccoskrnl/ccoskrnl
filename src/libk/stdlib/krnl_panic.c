#include "../../include/libk/stdlib.h"
#include "../../include/go/go.h"

extern boolean _go_has_been_initialize;

void krnl_panic(wch_t * wstr)
{
    if (_go_has_been_initialize && wstr != NULL) 
    {
        putws(bsp_window, wstr); 
    }
    __asm__("hlt\n\t");

}