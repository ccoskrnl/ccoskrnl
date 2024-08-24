#ifndef __LIBK_STRING_H__
#define __LIBK_STRING_H__

#include "../types.h"


char * strrchr(const char * str,int ch);
wch_t * wstrrchr(const wch_t * wstr, wch_t ch);
wch_t* wstrchr(wch_t* wstr, wch_t c);
char* strchr(char* str, char c);
size_t wstrlen(const wch_t* wstr);
void str2wstr(const char* str, wch_t* wstr, const size_t length);
size_t strlen(const char* str);

#endif
