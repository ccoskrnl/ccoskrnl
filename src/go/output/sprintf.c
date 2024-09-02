#ifdef _X86

#include "../../include/go/go.h"
#include "../../include/libk/stdarg.h"

int64_t sprintf(char* buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int64_t i = vsprintf(buf, fmt, args);

    va_end(args);
    return i;
}

#endif 