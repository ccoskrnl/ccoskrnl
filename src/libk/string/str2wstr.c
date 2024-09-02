#include "../../include/libk/string.h"

void str2wstr(const char* str, wch_t* wstr, const size_t length)
{
    size_t i;
    for (i = 0; i < length; i++) {
        wstr[i] = str[i];
    }
    wstr[i] = 0;
}