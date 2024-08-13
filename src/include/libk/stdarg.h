#ifndef __LIBK_STDARG_H__
#define __LIBK_STDARG_H__

#include "../types.h"

#ifdef _X86

typedef char* va_list;

#define stack_size(t) \
    (sizeof(t) <= sizeof(char*) ? sizeof(char*) : sizeof(t))

#define va_start(ap, v) \
    (ap = (va_list)&v + sizeof(char*))

#define va_arg(ap, t) \
    (*(t*)((ap += stack_size(t)) - stack_size(t)))

#define va_end(ap) \
    (ap = (va_list)0)
#endif 

#endif 