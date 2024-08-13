#include "../../include/libk/math.h"

long powdu(int base, unsigned exponent)
{
    long result = 1;
    if (exponent == 0)
    {
        return result;
    }
    for (unsigned i = 0; i < exponent; i++)
    {
        result *= base;
    }
    
    return result;
}

double powfu(double base, unsigned exponent)
{
    union {
        uint64_t u;
        double d;
    } ud0;

    double result = 1.0;
    ud0.d = base;

    if ((ud0.u & 0x7000000000000000) == 0 || (ud0.u & 0x7FF0000000000000) == 0x7FF0000000000000)
    {
        return 0.0;
    }

    if (exponent == 0)
    {
        return result;
    }
    
    for (unsigned int i = 0; i < exponent; i++)
    {
        result *= base;
    }

    return result;
}

unsigned int log2_int(unsigned int n) {
    unsigned int result = 0;
    while (n >>= 1) {
        result++;
    }
    return result;
}