#ifndef __LIBK_STDLIB_H__
#define __LIBK_STDLIB_H__

#include "../types.h"

int stoi(const char *str, int base);
long stol(const char *str, int base);
long strtol(const char *str, char **endptr, int base);
int64_t rand();
void assert(boolean result);
void krnl_panic();

void* malloc(size_t size);
void free(void* addr);

uint32_t swap_endian_32(uint32_t value);
uint16_t swap_endian_16(uint16_t value);
uint64_t swap_endian_64(uint64_t value);

void* memcpyb(void* dstptr, const uint8_t* srcptr, uint64_t size);
void* memsetq(uint64_t* bufptr, uint64_t value, size_t size_of_dest_in_qwords);
void* memset(void* bufptr, int value, size_t size);
void* memcpy(void* dstptr, const void* srcptr, size_t size);
void memzero(void* dst, uint64_t size);
int memcmp(const void* aptr, const void* bptr, size_t size);
void* memmove(void* dstptr, const void* srcptr, size_t size);

#endif
