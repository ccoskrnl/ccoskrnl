#include "../intr/pit.h"
#include "../cpu/cpu_features.h"
#include "../../../include/types.h"

void udelay(int64_t microseconds)
{
    int64_t us = microseconds * 1000;
    // int64_t us = microseconds;
    while (us-- > 0) {
        asm("nop");
    }
}