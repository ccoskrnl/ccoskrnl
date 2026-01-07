#include "../../../include/types.h"
#include "./lib.h"
#include "../intr/pit.h"


/**
 * @brief The Routine allows the users to wait for a current thread for a specific time in seconds.
 * 
 * @param[in]       uint32_t                        seconds                     Specific time in seconds.
 *
 * @retval          None.
 **/
void sleep(uint32_t seconds)
{
    uint64_t sec_in_10ms = 100 * seconds;

    for (uint64_t i = 0; i < sec_in_10ms; i++) 
    {
        pit_prepare_sleep(10000);
        pit_perform_sleep();
    }
}

void sleep_ms(uint64_t milliseconds)
{
    if (milliseconds == 0)
        return;
    
    uint64_t microseconds = milliseconds * 1000;
    uint64_t ten_ms_units = milliseconds / 10;

    uint64_t remaining_ms = milliseconds % 10;
    uint64_t remaining_us = remaining_ms * 1000;

    for (uint64_t i = 0; i < ten_ms_units; i++) 
    {
        pit_prepare_sleep(10000);
        pit_perform_sleep();
    }

    if (remaining_us > 0) 
    {
        pit_prepare_sleep(remaining_us);
        pit_perform_sleep();
    }
}