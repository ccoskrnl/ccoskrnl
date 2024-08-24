#include "../../include/types.h"
#include "../../include/libk/string.h"

wch_t* wstrchr(wch_t* wstr, wch_t c)
{
    for (; *wstr != 0; ++wstr) {
        if (*wstr == c) {
            return wstr;
        }
    }
    return NULL;
}