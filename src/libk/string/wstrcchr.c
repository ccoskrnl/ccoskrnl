#include "../../include/types.h"
#include "../../include/libk/string.h"

wch_t * wstrrchr(const wch_t * wstr, wch_t ch)
{
    wch_t * start = (wch_t *)wstr;
    while(*wstr++)/*get the end of the string*/
        ;
    while(--wstr != start && *wstr != ch)
        ;
    if(*wstr == ch)
        return((wch_t *)wstr);

    return NULL;
 
}