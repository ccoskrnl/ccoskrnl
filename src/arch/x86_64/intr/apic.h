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
    list_node_t head;

} intr_ctr_struct_head_t;

/**
 *
 * The IOAPIC has a message unit for sending and receiving APIC messages over
 * the APIC bus. I/O devices inject interrupts into the system by asserting one
 * of the interrupt lines to the IOAPIC. The IOAPIC selects the corresponding
 * entry in the Redirection Table and uses the information in that entry to format
 * an interrupt request message.
 *
 **/
typedef struct _ioapic_rte
{
    // Interrupt vector. Allowed values are from 0x10 to 0xFE.
    uint64_t intr_vector : 8;

    // Type of delivery mode. 0 = Normal, 1 = Low priority, 
    // 2 = System management interrupt, 4 = Non maskable interrupt, 
    // 5 = INIT, 7 = External. All others are reserved.
    uint64_t delivery_mode : 3;

    // Destination mode. Affects how the destination field is read, 
    // 0 is physical mode, 1 is logical. If the Destination Mode 
    // of this entry is Physical Mode, bits 56-59 contain an APIC ID.
    uint64_t dest_mode : 1;

    // Set if this interrupt is going to be sent, but the APIC is busy.
    // Read only.
    uint64_t delivery_status : 1;


    // Polarity of the interrupt. 0 = High is active, 1 = Low is active.
    uint64_t intr_polarity: 1;

    // Used for level triggered interrupts only to show if a local 
    // APIC has received the interrupt (= 1), or has sent an EOI (= 0). 
    // Read only.
    uint64_t remote_irr : 1;

    // Trigger mode. 0 = Edge sensitive, 1 = Level sensitive.
    uint64_t trigger_mod : 1;

    // Interrupt mask. Stops the interrupt from reaching the processor if set.
    uint64_t intr_mask : 1;

    uint64_t reserved : 39;

    // Destination field. If the destination mode bit was clear, then the lower 
    // 4 bits contain the bit APIC ID to sent the interrupt to. If the bit was set, 
    // the upper 4 bits also contain a set of processors.
    uint64_t dest_field : 8;

} __attribute__((packed))  ioapic_rte_t;




// Actually, we should not assume the value of any IRQ.
// To handle IRQ, we need to check ISO structures at first.

// Timer IRQ
#define IRQ_TIMER                                   0
// Keyboard IRQ
#define IRQ_KEYBOARD                                1



/**
 * To calculate the offset in I/OAPIC address based on specific IRQ
 **/
#define OFFSET_OF_RTE(IRQ)                          (10 + (IRQ) << 1)


/**
 * This function writes a value to a specified APIC register.
 *
 * @param[in]           reg             Which register to write.
 * @param[in]           value           The value will be written to specific register.
 *
 * @retval              none
 */
void write_lapic_register(uint32_t reg, uint32_t value);

/**
 * This function reads a value from a specified APIC register.
 *
 * @param[in]           reg             Which register to read.
 *
 * @retval                              The value of specific register.
 */
uint32_t read_lapic_register(uint32_t reg);


/**
 * This function writes a value to a specified I/O APIC register.
 *
 * @param[in]           reg             Which register to write.
 * @param[in]           value           The value will be written to specific register.
 *
 * @retval              none
 */
void write_ioapic_register(uint32_t reg, uint32_t value);

/**
 * This function reads a value from a specified I/O APIC register.
 *
 * @param[in]           reg             Which register to read.
 *
 * @retval                              The value of specific register.
 */
uint32_t read_ioapic_register(uint32_t reg);


void map_lapic_and_ioapic(
    _in_ uint64_t lapic_phys_addr, 
    _in_ intr_ctr_struct_head_t* ioapic_intr_ctr_structure
);


/**
 * @brief Local APIC Address for single processor system.

 * @note CPU core in a modern multiprocessor system typically has its own LAPIC. 
 * The LAPIC is responsible for handling interrupts that are specific to the CPU 
 * core it is associated with.

 */
extern volatile uint64_t local_apic_addr;

/**
 * @brief I/O APIC address for single processor system.

 * @note IOAPIC is responsible for handling interrupts from peripheral devices and 
 * routing them to the appropriate CPU cores. A system can have multiple IOAPICs, 
 * especially in configurations with multiple peripheral buses. 

 */  
extern volatile uint64_t io_apic_addr;


extern uint32_t apic_version;

typedef struct _ioapic_version
{
    // This 4 bit field contains the IOAPIC identification.
    uint32_t ioapic_identification : 4;

    // This member identifies the implementation version.
    uint8_t apic_version;

    // This field contains the entry number of the highest
    // entry in the I/O Rediection Table.
    uint8_t max_redirection_entry;

} ioapic_version_t;

extern ioapic_version_t ioapic_version;

#endif
