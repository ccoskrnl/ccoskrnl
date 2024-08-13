#ifndef __LIBK_STDIO_H__
#define __LIBK_STDIO_H__

#include "../types.h"
#include "stdarg.h"


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