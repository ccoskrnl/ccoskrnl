#ifndef __INTR_PIT_H__
#define __INTR_PIT_H__

#include "../../../include/types.h"

// Definitions                  I/O port        Usage
#define PIT_CHANNEL_0           0x40            //Channel 0 data port (read/write)
#define PIT_CHANNEL_1           0x41            //Channel 1 data port (read/write)
#define PIT_CHANNEL_2           0x42            //Channel 2 data port (read/write)
#define PIT_COMMAND             0x43            //Mode/Command register (write only, a read is ignored)

/**
 * Function: sleep specific number of micorseconds.
 * Parameters:
 *      microseconds                            Which holds how many number of microseconds 
 *                                              that need to sleep. But it must greater that
 *                                              0 and smaller that 55,555.
 *                                              55,555 = 1,000,000 / (1,193,180 / 65,535)
 * Return value:
 *      ST_INVALID_PARAMETER                    Passed one or more invalid parameters.
 **/
status_t pit_prepare_sleep(uint32_t microseconds);


// Function: perform for sleep specific number of micorseconds.
void pit_perform_sleep();

#endif