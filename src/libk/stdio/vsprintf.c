#include "../../include/types.h"

#ifdef _X86

#include "../../include/libk/stdio.h"
#include "../../include/libk/ctype.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/string.h"
#include "../../include/libk/stdarg.h"
#include "../../include/libk/math.h"

#define ZEROPAD             0x01        //  fill with 0
#define SIGN                0x02        //  unsigned / signed 
#define PLUS                0x04        //  print '+'
#define SPACE               0x08        //  put a space
#define LEFT                0x10        //  Align to the left
#define SPECIAL             0x20        //  hex prefix
#define SMALL               0x40        //  smaller alpha
#define DOUBLE              0x80        //  float number


// Function to reverse a string
static void reverse(char *str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static long long2str(long num, char* str, long min_digtis, int base)
{
    long i = 0;

    while (num)
    {
        str[i++] = (num % base) + '0';
        num = num / base;
    }

    while (i < min_digtis)
    {
        str[i++] = '0';
    }

    str[i] = '\0'; 
    reverse(str, i);

    return i;

}

static void float2str(double num, char* str, int after_point)
{
    int i = 0;
    int is_negative = 0;

    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    // Extract integer part
    long long_part = (long)num;

    // Extract floating part
    double float_part = num - (double)long_part;

    // Convert integer part to string
    i = long2str(long_part, str, 0, 10);

    // Add negative sign if needed 
    if (is_negative)
    {
        for (int j = i; j < 0; j--)
        {
            str[j] = str[j - 1];
        }

        str[0] = '-';
        i++;
    }

    // Add decimal point if needed
    if (after_point != 0)
    {
        str[i] = '.';
        float_part = float_part * pow(10, after_point);

        long2str((long)float_part, str + i + 1, after_point, 10);
    }
    
}

static char* number(char *str, uint64_t *num, int base, long size, uint64_t precision, uint32_t flags)
{
    char pad, sign, tmp[36];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    long i;
    long index;
    char *ptr = str;

    if (flags & SMALL)
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  
    

    if (flags & SMALL)
    {
        /* code */
    }

    if (flags & LEFT)
    {
        flags &= ~ZEROPAD;
    }

    // the routine can only handle the range of base 2 ~ base 16
    if (base < 2 || base > 16)
    {
        return 0;
    }

    pad = (flags & ZEROPAD) ? '0' : ' ';

    if (flags & DOUBLE && (*(double*)(num)) < 0)
    {
        sign = '-';
        *(double*)(num) = -(*(double*)(num));
    }
    else if (flags & SIGN && !(flags & DOUBLE) && ((long)(*num)) < 0) 
    {
        sign = '-';
        (*num) = -(long)(*num);
    }
    else
        sign = (flags & PLUS) ? '+' : (flags & SPACE) ? ' ' : 0;
    
    if (sign)
    {
        size--;
    }
    
    if (flags & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        
        else if (base == 8)
            size--;
    }

    i = 0;

    if (flags & DOUBLE) 
    {
        float2str(*num, tmp, 1);
    } 
    else
    {
        long2str(*num, tmp, 0, 10);
    }

    if (i > precision)
    {
        precision = i;
    }

    size -= precision;

    // construct converted result

    if (!(flags & (ZEROPAD + LEFT)))
    {
        while (size-- > 0)
        {
            *str++ = ' ';
        }
    }

    if (sign)
    {
        *str++ = sign;
    }

    if (flags & SPECIAL)
    {
        if (base == 8)
        {
            *str++ = '0';
        }
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = digits[33];
        }
    }
    

    if (!(flags & LEFT))
    {
        while (size-- > 0)
        {
            *str++ = pad;
        }
        
    }

    while (i < precision--)
    {
        *str++ = '0';
    }

    while (i-- < 0)
    {
        *str++ = tmp[i];
    }

    
    while (size-- > 0)
    {
        *str++ = ' ';
    }
    
    return str;
}

int64_t vsprintf(char* buf, const char *fmt, va_list args)
{
    int64_t len;
    int64_t i;

    char* str;
    char* s;

    uint64_t flags;

    int field_width;
    int precision;
    int qualifier;
    uint64_t num;
    uint8_t *ptr;

    for (str = buf; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }

        flags = 0;

    repeat:

        // ignore '%'
        ++fmt;        

        switch (*fmt)
        {
        case '-':
            flags |= LEFT;
            goto repeat; 
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        field_width = -1;
        if (isdigit(*fmt))
        {
            field_width = stoi(fmt, 0);

            // ignore number
            while (isdigit(*fmt))
            {
                fmt++;
            }
        }

        else if (*fmt == '*')
        {
            ++fmt;

            // get the size of the output width
            field_width = va_arg(args, int);

            // check if field_width is not equal to -1.
            // otherwise, align the output to left
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        // get precision

        precision = -1;

        // (ditto)
        if (*fmt == '.')
        {
            ++fmt;
            if (isdigit(*fmt))
            {
                precision = stoi(fmt, 0);

                while (isdigit(*fmt))
                {
                    fmt++;
                }
                
            }
            else if (*fmt == '*')
            {
                precision = va_arg(args, int);
            }

            if (precision < 0)
            {
                precision = 0;
            }
            
        }

        qualifier = -1;
        if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
        {
            qualifier = *fmt;
            ++fmt;
        }

        // handle conversion indicator
        switch (*fmt)
        {
        // character
        case 'c':

            // if flags not contains LEFT flag, we need 
            // to fill the front of output field with whilespace 
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    *str++ = ' ';
                }
            }
            // copy char into buffer 
            *str++ = (unsigned char)va_arg(args, long);

            // if flags contains LEFT flag, then fill temporary string with whitespace
            while (--field_width > 0)
            {
                *str++ = ' ';
            }

            break;

        // string
        case 's':       

            s = va_arg(args, char*);

            // get the length of the passed string
            len = strlen(s);

            if (precision < 0)
            {
                precision = len;
            }

            // expand the precision to length of the caller passed string
            else if (len > precision)
                len = precision; 

            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    *str++ = ' ';
                }
            }

            // copy caller passed string into formated buffer
            for (i = 0; i < len; i++)
            {
                *str++ = *s++;
            }
            
            while (len < field_width--)
            {
                *str++ = ' ';
            }
            break; 
        
        case 'o':
            num = va_arg(args, unsigned long);    
            str = number(str, &num, 8, field_width, precision, flags);
            break;

        case 'p':
            if (field_width == -1)
            {
                field_width = 16;
                flags |= ZEROPAD;
            }

            num = va_arg(args, unsigned long);
            str = number(str, &num, 16, field_width, precision, flags);

        case 'x':
            flags |= SMALL;
        case 'X':
            num = va_arg(args, unsigned long);
            str = number(str, &num, 16, field_width, precision, flags);
            break;

        // if convertion indicator is 'd' or 'i', so that corrresponding arg is signed integer
        case 'd':
        case 'i':
            flags |= SIGN;    

        // and if it is 'u',  the arg is unsigend interger;
        case 'u':

            num = va_arg(args, unsigned long);
            str = number(str, &num, 10, field_width, precision, flags);

            break;

        case 'f':
            flags |= SIGN;
            flags |= DOUBLE;
            double d = va_arg(args, double); 

            str = number(str, (uint64_t*)&d, 10, field_width, precision, flags);

        case 'b':
            num = va_arg(args, unsigned long);
            str = number(str, &num, 2, field_width, precision, flags);
            break;

        default:
            if (*fmt != '%')
            {
                *str++ = '%';
            }

            if (*fmt)
                *str++ = *fmt; 
            else
                --fmt;
            break;
        }
        
    }

    *str = '\0';

    i = str - buf;

    return i;
}

#endif