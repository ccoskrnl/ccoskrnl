#include "../../include/libk/stdlib.h"
#include "../../include/libk/limits.h"
#include "../../include/libk/ctype.h"

long strtol(const char *str, char **endptr, int base)
{
    const char *s = str;
    long result = 0;
    int sign = 1;
    int overflow = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*s))
    {
        s++;
    }

    // Handle optional sign
    if (*s == '-')
    {
        sign = -1;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }

    // Handle optional base prefix
    if ((base == 0 || base == 16) && *s == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        s += 2;
        base = 16;
    }
    else if ((base == 0 || base == 8) && *s == '0' && (s[1] == 'o' || s[1] == 'O'))
    {
        s += 2;
        base = 8;
    }
    else if (base == 0)
    {
        base = 10;
    }

    // Convert the number
    while (*s)
    {
        int digit;
        if (isdigit((unsigned char)*s))
        {
            digit = *s - '0';
        }
        else if (isalpha((unsigned char)*s))
        {
            digit = toupper((unsigned char)*s) - 'A' + 10;
        }
        else
        {
            break;
        }

        if (digit >= base)
        {
            break;
        }

        if (result > (LONG_MAX - digit) / base)
        {
            overflow = 1;
            break;
        }

        result = result * base + digit;
        s++;

    }

    if (overflow)
    {
        result = (sign == 1) ? LONG_MAX : LONG_MIN;
    }
    else
    {
        result *= sign;
    }

    if (endptr)
    {
        *endptr = (char*)(overflow ? str : s);
    }

    return result;
}
