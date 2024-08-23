#include "../../include/types.h"
#include "../../include/libk/string.h"

size_t wstrlen(const wch_t* wstr)
{
    size_t len = 0;
    while (wstr[len] != 0) {
        len++;
    }
    return len;
}