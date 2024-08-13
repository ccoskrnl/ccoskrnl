#ifndef __LIBK_STRING_H__
#define __LIBK_STRING_H__

#include "../types.h"


void* memset(void* bufptr, int value, size_t size);
void* memcpy(void* dstptr, const void* srcptr, size_t size);
void memzero(void* dst, uint64_t size);
int memcmp(const void* aptr, const void* bptr, size_t size);
size_t strlen(const char* str);
void* memmove(void* dstptr, const void* srcptr, size_t size);

#endif