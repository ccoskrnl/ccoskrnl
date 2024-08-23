#include "../../include/libk/string.h"

void str2wstr(const char* str, wch_t* wstr, const size_t length)
{
    for (size_t i = 0; i < length; i++) {
        wstr[i] = str[i];
    }
}