#include "../../include/types.h"
#include "../../include/libk/string.h"

char * strrchr(const char * str,int ch)
{
    char * start = (char *)str;
    while(*str++)/*get the end of the string*/
        ;
    while(--str != start && *str != (char)ch)
        ;
    if(*str == (char)ch)
        return((char *)str);
    return NULL;

}
