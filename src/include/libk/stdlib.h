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

uint32_t swap_endian(uint32_t value);
uint16_t swap_endian_16(uint16_t value);
uint64_t swap_endian_64(uint64_t value);

#endif