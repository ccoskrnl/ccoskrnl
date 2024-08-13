#ifndef __INTR_APIC_H__
#define __INTR_APIC_H__

#include "../../../include/types.h"
#include "../../../include/libk/list.h"

// The offset of Local APIC Version Register(Read/Write) from Local APIC address
#define LOCAL_APIC_ID_REG                           0x20

// Local APIC Version Register (Read Only)
#define LOCAL_APIC_VERSION_REG                      0x30

// Task Priority Register (Read/Write)
#define LOCAL_APIC_TASK_PRIORITY_REG                0x80

/**
 * @brief Arbitration Priority Register (Read Only)
 * 
 * The value in the Arbitration Priority field is equal to the highest priority of
 * the Task Priority field of the Task Priority Register, the highest bit set in the
 * In-Service Register vector, or the highest bit set in the Interrupt Request Register
 * vector. The value in the Arbitration Priority Sub-class field is equal to the Task
 * Priority Sub-class if the APR is equal to the TPR, and zero otherwise.
 */
#define LOCAL_APIC_ARIBITRATION_PRIORITY_REG        0x90

// Processor Priority Register (Read Only)
#define LOCAL_APIC_PROCESSOR_PRIORITY_REG           0xA0
// EOI Register
#define LOCAL_APIC_EOI_REG                          0xB0
// Spurious Interrupt Vector Register (Read/Write)
#define LOCAL_APIC_SPURIOUS_INTERRUPT_VEC_REG       0xF0

// LVT Timer Register(Read/Write)
#define LOCAL_APIC_LVT_TIMER_REG                    0x320

// Initial Count Register (for Timer) (Read/Write)
#define LOCAL_APIC_INIT_COUNT_REG                   0x380
// Current Count Register (for Timer) (Read Only)
#define LOCAL_APIC_CURRENT_COUNT_REG                0x390
// Divide Configuration Register for Timer (Read/Write)
#define LOCAL_APIC_DIVIDE_CONF_REG                  0x3E0

/**
 * @brief Accpeting System and IPI Interrupts
 * 
 * If the local APIC accepting the interrupt determines that the message type for the interrupt request
 * indicates SMI, NMI, INIT, STARTUP or ExtINT, it sends the interrupt directly to the CPU core for 
 * handling. If the message type is fixed or lowest priority, the accepting local APIC places the interrupt 
 * into an open slot in either the IRR or ISR registers. If there is no free slot, the interrupt is rejected and 
 * sent back to the sender with a retry request.
 * 
 */
 





typedef struct _intr_ctr_struct_head
{
    uint64_t total;
    list_node head;

} intr_ctr_struct_head_t;

#endif