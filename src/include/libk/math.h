#ifndef __LIBK_MATH_H__
#define __LIBK_MATH_H__

#include "../types.h"

#define abs(integer)            ((integer) > 0 ? (integer) : -(integer))

long powdu(int base, unsigned exponent);
double powfu(double base, unsigned exponent);
// Custom log2 using bit manipulation approach
unsigned int log2_int(unsigned int n);

#endif