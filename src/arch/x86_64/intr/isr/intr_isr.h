#ifndef __INTR_ISR_H__
#define __INTR_ISR_H__

#include "../../../../include/types.h"
#include "../../../../include/libk/list.h"

#define INTR_ISR_TIMER_INT                          0x20             
#define INTR_ISR_KBD_INT                            0x21


/**
 * The routines installs timer interrupt on IDT[0x20].
 * By default, We use apic timer to emit timer interrupt.
 **/
void enable_timer_intr();

#endif
