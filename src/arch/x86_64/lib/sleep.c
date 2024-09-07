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