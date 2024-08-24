#ifndef __LIBK_STDIO_H__
#define __LIBK_STDIO_H__

#include "../types.h"
#include "../hal/op/graphics.h"
#include "../hal/op/font/font_ttf.h"
#include "stdarg.h"


// Helper macros to count the number of arguments
#define COUNT_ARGS(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define EXPAND_ARGS(...) __VA_ARGS__
#define COUNT(...) EXPAND_ARGS(COUNT_ARGS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// Macro to select the appropriate function based on the number of arguments
#define FOL(...) FOL_IMPL(COUNT(__VA_ARGS__), __VA_ARGS__)
#define FOL_IMPL(N, ...) FOL_##N(__VA_ARGS__)

// Define macros for different numbers of arguments
#define FOL_1(_1) _putc(_1)
#define FOL_2(_1, _2) _puts(_1)
#define FOL_3(_1, _2, _3) _putss(_1, _2)

// Main macro to call the appropriate function
#define _put(...) FOL(__VA_ARGS__)


void putwccf(wch_t wch, go_blt_pixel_t color, font_ttf_t* family);
void putwcc(wch_t wch, go_blt_pixel_t color);
void putwc(wch_t wch);
void putws(const wch_t *ws);
void putc(char c);
void puts(const char *s);

void putss(const char *s1, const char *s2);
void putsd(const char *s, int64_t d);
void putsx(const char *s, uint64_t x);
void putsf(const char *s, double f);

void putsds(const char *s, int64_t d, const char *s1);
void putsxs(const char *s, uint64_t x, const char *s1);
void putsfs(const char *s, double f, const char *s1);

#ifdef _X86

int64_t vsprintf(char* buf, const char *fmt, va_list args);
int64_t sprintf(char* buf, const char* fmt, ...);
int64_t printk(const char* fmt, ...);

#endif

#endif