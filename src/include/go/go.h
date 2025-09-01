#ifndef __INCLUDE_GO_OUTPUT_H__
#define __INCLUDE_GO_OUTPUT_H__

#include "../types.h"
#include "../../go/graphics.h"

typedef enum _PREDEFINED_COLOR
{
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    CYAN3,
    FIRE_BRICK2,
    SPRINT_GREEN2,
    DARK_GOLDEN_ROD2,

} PREDEFINED_COLOR;

extern int bsp_window;
extern int dbg_window;

/*  Put a wide character with specific color */
void putwcc(int window_index, wch_t wch, go_blt_pixel_t color);
/*  Put a wide character */
void putc(int window_index, wch_t wch);

/*  Debug Print */
void put_check(int window_index, int status, const wch_t *ws);
/*  Put a wstring with specific predefined color */
void putwsc(int window_index, const wch_t *ws, PREDEFINED_COLOR color);
/*  Put a wstring */
void putws(int window_index, const wch_t *ws);
/*  Put a string */
void puts(int window_index, const char *s);


/*  Put double string */
void putss(int window_index, const char *s1, const char *s2);
/*  Put string and digit */
void putsd(int window_index, const char *s, int64_t d);
/*  Put string and hex */
void putsx(int window_index, const char *s, uint64_t x);
/*  Put string and float */
void putsf(int window_index, const char *s, double f);


/*  Put string, digit and string. */
void putsds(int window_index, const char *s, int64_t d, const char *s1);
/*  Put string, hex and string. */
void putsxs(int window_index, const char *s, uint64_t x, const char *s1);
/*  Put string, float and string. */
void putsfs(int window_index, const char *s, double f, const char *s1);

#ifdef _X86

int64_t vsprintf(char* buf, const char *fmt, va_list args);
int64_t sprintf(char* buf, const char* fmt, ...);
int64_t printk(const char* fmt, ...);

#endif

#endif