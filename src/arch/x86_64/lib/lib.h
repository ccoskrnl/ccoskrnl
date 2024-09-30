#ifndef __x86_LIB_H__
#define __x86_LIB_H__

#include "../../../include/types.h"

/**
 * @param[in out]    uint64_t volatile              *destination                A pointer to the destination value.
 * @param[in]        uint64_t                       exchange                    The exchange value.
 * @param[in]        uint64_t                       comperand                   The value to compare to Destination.
 *
 * @retval           uint64_t                       The function returns the initial value of the destination parameter.
 **/
uint64_t lock_cmpxchg(uint64_t volatile *destination, uint64_t exchange, uint64_t comperand);

/**
 * @param[in out]    uint128_t volatile             *destination                A pointer to the destination value.
 * @param[in]        uint128_t                      exchange                    The exchange value.
 * @param[in]        uint128_t                      comperand                   The value to compare to Destination.
 *
 * @retval           uint128_t                      The functin returs the initial value of the destination parameter.
 **/
uint128_t lock_cmpxchg16b(uint128_t volatile *destination, uint128_t exchange, uint128_t comperand);

/**
 * @brief The Routine allows the users to wait for a current thread for a specific time in seconds.
 * 
 * @param[in]       uint32_t                        seconds                     Specific time in seconds.
 *
 * @retval          None.
 **/
void sleep(uint32_t seconds);


void _lib_udelay_init();
void udelay(uint64_t microseconds);

/**
 * Reference from intr/apic.c
 * The routine to notice the servicing of the current interrupt is complete.
 **/
extern void send_eoi();

#endif