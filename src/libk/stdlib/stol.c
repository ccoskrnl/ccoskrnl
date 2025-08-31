#include "../../include/libk/stdlib.h"
#include "../../include/libk/limits.h"

long stol(const char *str, int base) {
    char *endptr;
    long val = strtol(str, &endptr, base);

    // Check for various possible errors
    // if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
    //     perror("strtol");
    //     return 0;
    // }
    if ((val == LONG_MAX || val == LONG_MIN))
    {
        krnl_panic(NULL);
        return 0;
    }


    if (endptr == str) {
        // fprintf(stderr, "No digits were found\n");
        krnl_panic(NULL);
        return 0;
    }

    return val;
}
