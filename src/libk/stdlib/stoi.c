#include "../../include/libk/stdlib.h"
#include "../../include/libk/limits.h"

int stoi(const char *str, int base)
{
    char *endptr;
    // errno = 0; // To distinguish success/failure after call
    long val = strtol(str, &endptr, base);

    // Check for various possible errors
    // if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
    // {
    //     perror("strtol");
    //     return 0;
    // }
    if ((val == LONG_MAX || val == LONG_MIN))
    {
        krnl_panic(NULL);
        return 0;
    }


    if (endptr == str)
    {
        krnl_panic(NULL);
        // fprintf(stderr, "No digits were found\n");
        return 0;
    }

    // Ensure the value fits in an int
    if (val > INT_MAX || val < INT_MIN)
    {
        krnl_panic(NULL);
        // fprintf(stderr, "Value out of range for int\n");
        return 0;
    }

    return (int)val;
}
