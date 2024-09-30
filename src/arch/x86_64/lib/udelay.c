#include "../intr/pit.h"
#include "../cpu/cpu_features.h"
#include "../../../include/types.h"

void udelay(int64_t microseconds)
{
    while (microseconds-- > 0) {
        asm("nop");
    }
}