#include "../../include/libk/stdio.h"
#include "../../include/libk/string.h"
#include "../../include/libk/math.h"
#include "../../include/libk/limits.h"

#define MAXPUTBUF                   0x80

#define FLOAT_BINARY_64             64

// b = the radix, 2 or 10

// p = the number f digits in the significand (precision)
#define FLOAT_BINARY_64_P_BIT           53

// s is 0 or 1.
#define FLOAT_BINARY_64_S_BIT           1
#define FLOAT_BINARY_64_T_BIT           (FLOAT_BINARY_64_P_BIT - 1)

// In normailzed values, The exponent field is interpreted as representing a
// signed integer in biased form.
#define FLOAT_BINARY_64_W_BIT           \
    (FLOAT_BINARY_64 - FLOAT_BINARY_64_S_BIT - (FLOAT_BINARY_64_P_BIT - 1))

// emax = the maximum exponent e
#define FLOAT_BINARY_64_EMAX        1023  

// emin = the minimum exponent e
// emin shall be 1 - emax for all formats.
#define FLOAT_BINARY_64_EMIM        (1 - FLOAT_BINARY_64_EMAX)  

#define FLOAT_BINARY_64_BIAS        (powdu(2,FLOAT_BINARY_64_W_BIT - 1) - 1)    // 1023

/**
 * 
 * q is any integer emin <= q + p - 1 <= emax
 * c is a number represented by a digit string of the form
*/

#define __get_sign_of_double(f)     (f & (0x8000000000000000))

static char putbuf[MAXPUTBUF];


static go_blt_pixel_t get_color(_in_ PREDEFINED_COLOR color_val)
{
    go_blt_pixel_t color;
    switch (color_val) {
        case CYAN3:
            color.Red = 0;
            color.Blue = 205;
            color.Green = 205;
            break;
        case FIRE_BRICK2:
            color.Red = 238;
            color.Green = 44;
            color.Blue = 44;
            break;
        case SPRINT_GREEN2:
            color.Red = 0;
            color.Green = 238;
            color.Blue = 118;
            break;
        default:
            color.Blue = 255;
            color.Green = 255;
            color.Red = 255;
            break;

    }
    return color;
}

static int calculate_bits_for_decimal_digit(int d) {
    double power_of_ten = pow(10, d - 1);
    double log_base_2 = log2(power_of_ten);
    int bits_required = (int)(log_base_2) + 1;
    return bits_required;
}

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
    boolean is_negative = false;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return 1;
    }

    if (num < 0)
    {
        num = -num;
        str[i++] = '-';
        is_negative = true;
    }
        

    while (num)
    {
        str[i++] = (num % base) + '0';
        num = num / base;
    }

    while (i <= min_digtis)
    {
        str[i++] = '0';
    }

    str[i] = '\0'; 
    if (is_negative)
    {
        reverse(str+1, i-1);
    }
    else
    {
        reverse(str, i);
    }

    return i;

}

void uint64_to_hexstr(uint64_t value, char* buffer) {
    const char hex_digits[] = "0123456789ABCDEF";
    int i;
    for (i = 0; i < 16; i++) {
        buffer[15 - i] = hex_digits[value & 0xF];
        value >>= 4;
    }
    buffer[16] = '\0'; // Null-terminate the string
}


/*  Debug Print */
void put_check(boolean cond, const wch_t *ws)
{
    putwc('[');
    if (cond) 
        putwsc(L" Successful ", SPRINT_GREEN2);
    else
        putwsc(L" Failed ", FIRE_BRICK2);
    putws(L"]  ");
    putws(ws);
}

/*  Put a wstring with specific predefined color */
void putwsc(const wch_t *ws, PREDEFINED_COLOR color_val)
{
    go_blt_pixel_t color = get_color(color_val);
    int len = wstrlen(ws);
    for (int i = 0; i < len; i++) {
        putwcc(ws[i], color);
    }
}

/*  Put a wstring */
void putws(const wch_t *ws)
{
    int len = wstrlen(ws);
    for (int i = 0; i < len; i++)
    {
        putwc(ws[i]);
    }
}

/*  Put a string */
void puts(const char *s)
{
    int len = strlen(s);
    for (int i = 0; i < len; i++)
    {
        putc(s[i]);
    }
}






/*  Put double string */
void putss(const char *s1, const char *s2)
{
    if (s1 != NULL)
    {
        puts(s1);
    }
    if (s2 != NULL)
    {
        puts(s2);
    }
    
}

/*  Put string and digit */
void putsd(const char *s, int64_t d)
{
    if (s != NULL)
    {
        puts(s);
    }
    long2str(d, putbuf, INT64_MIN, 10);
    puts(putbuf);
    
}


/*  Put string and hex */
void putsx(const char *s, uint64_t x)
{
    if (s != NULL)
    {
        puts(s);
    }
    putbuf[0] = '0';
    putbuf[1] = 'x';
    uint64_to_hexstr(x, putbuf+2);
    puts(putbuf);
}


/*  Put string and float */
void putsf(const char *s, double f)
{
    char* str;

    uint64_t exp;
    union 
    {
        uint64_t d;
        double f;
    } uf;

    uint64_t u;
    uint64_t m;

    if (s != NULL)
    {
        puts(s);
    }
    
    uf.f = f;
    str = putbuf;

    exp = (uf.d & 0x7FF0000000000000) >> 52;

    if (exp == 0x7ff && (uf.d & 0xfffffffffffff) != 0) 
    {
        str[0] = 'n';
        str[1] = 'a';
        str[2] = 'n';
        str[3] = '\0';
        goto __put_str;
    }

    if (uf.d & 0x8000000000000000)
    {
        str[0] = '-';     
        str++;
    }

     
    if (exp == 0x0)
    {
        str[0] = '0';
        str[1] = '\0';
    }
    else if (exp == 0x7ff)
    {
        str[0] = 'i';
        str[1] = 'n';
        str[2] = 'f';
        str[3] = '\0';
    }
    /**
     * If 1 <= E <= 2^W - 2
     * 
    */
    else
    {
        uf.d &= 0x7FFFFFFFFFFFFFFF;               
        u = (uint64_t)uf.f;
        m = (uint64_t)((uf.f - u) * 1000000);

        long2str(u, str, 0, 10);

        str += strlen(str);
        *str = '.';
        str++;
        long2str(m, str, 0, 10);
    }

__put_str:

    puts(putbuf);

}





/* Put string, digit and string. */
void putsds(const char *s, int64_t d, const char *s1)
{
    putsd(s, d);
    if (s1 != NULL)
    {
        puts(s1);
    }
    
}

/* Put string, hex and string. */
void putsxs(const char *s, uint64_t x, const char *s1)
{
    putsx(s, x);
    if (s1 != NULL)
    {
        puts(s1);
    }

}

/* Put string, float and string. */
void putsfs(const char *s, double f, const char *s1)
{
    putsf(s, f);
    if (s1 != NULL)
    {
        puts(s1);
    }

}
