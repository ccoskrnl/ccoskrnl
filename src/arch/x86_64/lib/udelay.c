#include "../intr/pit.h"
#include "../cpu/cpu_features.h"
#include "../../../include/types.h"

void udelay(uint64_t microseconds)
{
    pit_prepare_sleep(microseconds);
    pit_perform_sleep();
}