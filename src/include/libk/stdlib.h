#ifndef __LIBK_STDLIB_H__
#define __LIBK_STDLIB_H__

#include "../types.h"

int stoi(const char *str, int base);
long stol(const char *str, int base);
long strtol(const char *str, char **endptr, int base);
int64_t rand();

void assertion_failure(char *exp, char *file, char *base, int line);

#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
        assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

void krnl_panic(wch_t * wstr);

uint32_t swap_endian_32(uint32_t value);
uint16_t swap_endian_16(uint16_t value);
uint64_t swap_endian_64(uint64_t value);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++ The implementation of the following functions is provided by arch/lib. ++++

void* calloc(size_t size);
void* malloc(size_t size);
void free(void* addr);

void* memsety(void* bufptr, uint256_t* value, size_t size_of_dest_in_ywords);
void* memseto(void* bufptr, uint128_t* value, size_t size_of_dest_in_owords);
void* memsetq(uint64_t* bufptr, uint64_t value, size_t size_of_dest_in_qwords);
void* memsetd(uint32_t* dstptr, uint32_t value, size_t size_of_dst_in_dword);
void* memset(void* bufptr, int value, size_t size);

void* memcpyb(void* dstptr, const uint8_t* srcptr, uint64_t size);
void* memcpy(void* dstptr, const void* srcptr, size_t size);

void memzero(void* dst, uint64_t size);

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



int memcmp(const void* aptr, const void* bptr, size_t size);
void* memmove(void* dstptr, const void* srcptr, size_t size);

#endif
