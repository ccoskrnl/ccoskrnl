#include "../../include/types.h"
#include "../../include/libk/string.h"

char* strchr(char* str, char c)
{
    for (; *str != 0; ++str) {
        if (*str == c) {
            return str;
        }
    }
    return NULL;
}