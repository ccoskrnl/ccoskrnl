#include "../../../include/types.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/hal/acpi.h"

#include "../mm/mm_arch.h"
#include "apic.h"



extern intr_ctr_struct_head_t intr_ctr_structs[MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES];


lapic_ver_t lapic_ver;

/**
 * @brief Local APIC Address for single processor system.

 * @note CPU core in a modern multiprocessor system typically has its own LAPIC. 
 * The LAPIC is responsible for handling interrupts that are specific to the CPU 
 * core it is associated with.

*/
volatile uint64_t local_apic_addr;

/**
 * @brief I/O APIC address for single processor system.

 * @note IOAPIC is responsible for handling interrupts from peripheral devices and 
 * routing them to the appropriate CPU cores. A system can have multiple IOAPICs, 
 * especially in configurations with multiple peripheral buses. Each additional 
 * I/O APIC can manage another set of interrupt lines, allowing your system to 
 * support more devices.

*/  
volatile uint32_t *volatile ioapics[IOAPIC_ADDRESSES_MAX] = { 0 };






/**
 * This function writes a value to a specified APIC register.
 *
 * @param[in]           reg             Which register to write.
 * @param[in]           value           The value will be written to specific register.
 *
 * @retval              none
 */
void write_lapic_register(uint32_t reg, uint32_t value)
{
    *(uint32_t *)(local_apic_addr + reg) = value;
}

/**
 * This function reads a value from a specified APIC register.
 *
 * @param[in]           reg             Which register to read.
 *
 * @retval                              The value of specific register.
 */
uint32_t read_lapic_register(uint32_t reg)
{
    return *(uint32_t *)(local_apic_addr + reg);
}

/**
 * This function writes a value to a specified I/O APIC register.
 *
 * @param[in]           ioapic_addr     I/O APIC Address.
 * @param[in]           reg             Which register to write.
 * @param[in]           value           The value will be written to specific register.
 *
 * @retval              none
 */
void write_ioapic_register(volatile uint32_t* ioapic_addr, uint32_t reg, uint32_t value)
{
    volatile uint32_t *ioapic = (volatile uint32_t*)ioapic_addr;
    ioapic[0] = reg;
    ioapic[4] = value;
}

/**
 * This function reads a value from a specified I/O APIC register.
 *
 * @param[in]           ioapic_addr     I/O APIC Address.
 * @param[in]           reg             Which register to read.
 *
 * @retval                              The value of specific register.
 */
uint32_t read_ioapic_register(volatile uint32_t* ioapic_addr, uint32_t reg)
{
    volatile uint32_t *ioapic = (volatile uint32_t*)ioapic_addr;
    ioapic[0] = reg;

    return ioapic[4];
}

/**
 * The routine to notice the servicing of the current interrupt is complete.
 **/
void send_eoi()
{
    // Send EOI to local APIC
    write_lapic_register(LOCAL_APIC_EOI_REG, 0);
}

// Check Delivery Status
boolean _lapic_check_delivery_status()
{
    uint32_t low = read_lapic_register(LOCAL_APIC_ICR_LOW);
    return ((low >> 12) & 0x1) == 0;
}

uint32_t _lapic_get_apic_id()
{
    uint32_t reg = read_lapic_register(LOCAL_APIC_ID_REG);
    return reg >> 24;
}


// Helper functions
void get_ioapic_ver(volatile uint32_t* ioapic, ioapic_version_t* ioapic_version)
{
    ioapic_version->ioapic_identification = (read_ioapic_register(ioapic, IOAPIC_ID_REG) >> 24) & 0xF;
    uint32_t reg_value = read_ioapic_register(ioapic, IOAPIC_VER_REG);
    ioapic_version->apic_version = reg_value & 0xFF;
    ioapic_version->max_redirection_entry = (reg_value >> 16) & 0xFF;
}


// Is 82489DX discrete APIC?
boolean _lapic_is_82489DX(uint8_t version)
{
    return (version <= 0x10);
}

void get_lapic_ver(lapic_ver_t* lapic_version)
{
    uint32_t reg = read_lapic_register(LOCAL_APIC_VERSION_REG);
    lapic_version->version = reg & 0xFF;
    lapic_version->maxlvt = (reg >> 16) & 0xFF;
    lapic_version->eoi_broatcast = (reg >> 24) & 0x1;
}


void map_lapic_and_ioapic(
        _in_ uint64_t lapic_phys_addr, 
        _in_ intr_ctr_struct_head_t* ioapic_intr_ctr_structure
        )
{

    local_apic_addr = mm_set_mmio(lapic_phys_addr, 1);

    list_node_t *head = &(intr_ctr_structs+IOAPIC)->head;
    list_node_t *node = head->flink;
    for (size_t i = 0; i < (intr_ctr_structs+IOAPIC)->total; i++, node = node->flink)
    {
        io_apic_t *ioapic = struct_base(io_apic_t, node, node);
        ioapics[i] = (volatile uint32_t*)(mm_set_mmio(ioapic->ioapic_addr, 1));
    }

    get_lapic_ver(&lapic_ver);

}
