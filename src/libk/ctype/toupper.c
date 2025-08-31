#include "../../include/libk/ctype.h"

inline int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - 32;
    }
    else 
    {
        return c;
    }

}
