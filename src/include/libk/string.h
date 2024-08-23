#ifndef __LIBK_STRING_H__
#define __LIBK_STRING_H__

#include "../types.h"


size_t wstrlen(const wch_t* wstr);
void str2wstr(const char* str, wch_t* wstr, const size_t length);
size_t strlen(const char* str);

#endif
