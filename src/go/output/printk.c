#ifdef _X86

#include "../../include/go/go.h"
#include "../../include/libk/stdarg.h"

#define MAXLEN              8192

int64_t printk(const char* fmt, ...)
{
    va_list args;
    int64_t len;


    char buf[MAXLEN];
    va_start(args, fmt);

    len = vsprintf(buf, fmt, args);

    va_end(args);

    for (int64_t i = 0; i < len; i++)
    {
        putc(0, buf[i]);
    }

    return len;
}

#endif