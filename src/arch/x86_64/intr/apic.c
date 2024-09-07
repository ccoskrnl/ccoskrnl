#include "../../../include/types.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/hal/acpi.h"
#include "../mm/mm_arch.h"
#include "apic.h"

// The start address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_start;
// The physical start address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_phys_start;

extern uint64_t _mm_get_sys_pte_next_page();

extern intr_ctr_struct_head_t intr_ctr_structs[MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES];

uint32_t apic_version;
ioapic_version_t ioapic_version;

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
 * especially in configurations with multiple peripheral buses. 

 */  
volatile uint64_t io_apic_addr;

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
 * @param[in]           reg             Which register to write.
 * @param[in]           value           The value will be written to specific register.
 *
 * @retval              none
 */
void write_ioapic_register(uint32_t reg, uint32_t value)
{
    volatile uint32_t *ioapic = (volatile uint32_t*)io_apic_addr;
    ioapic[0] = reg;
    ioapic[4] = value;
}

/**
 * This function reads a value from a specified I/O APIC register.
 *
 * @param[in]           reg             Which register to read.
 *
 * @retval                              The value of specific register.
 */
uint32_t read_ioapic_register(uint32_t reg)
{
    volatile uint32_t *ioapic = (volatile uint32_t*)io_apic_addr;
    ioapic[0] = reg;

    return ioapic[4];
}


void map_lapic_and_ioapic(
    _in_ uint64_t lapic_phys_addr, 
    _in_ intr_ctr_struct_head_t* ioapic_intr_ctr_structure
)
{
    uint64_t new_page;
    mmpte_hardware_t *pde;
    mmpte_hardware_t *pte;
    int pte_index = 0;

    // get hardware pdpt
    mmpte_hardware_t *pdpt =
        (mmpte_hardware_t *)(_mm_pt_pdpt_pool_start + (PML4E_OFFSET_OF_HARDWARE << PAGE_SHIFT));

    // allocate a new page to store pdes
    new_page = _mm_get_sys_pte_next_page();
    pde = (mmpte_hardware_t *)(new_page + _mm_sys_pte_pool_start);

    // Use the last item of Hardware PDPT to store APIC PD
    pdpt[0x1FF].present = 1;
    pdpt[0x1FF].rw = 1;
    pdpt[0x1FF].address = (new_page + _mm_sys_pte_pool_phys_start) >> PAGE_SHIFT;

    // allocate a new page
    new_page = _mm_get_sys_pte_next_page();
    pte = (mmpte_hardware_t *)(new_page + _mm_sys_pte_pool_start);

    // Use the last item of APIC PD to store APIC PT
    pde[0x1FF].present = 1;
    pde[0x1FF].rw = 1;
    pde[0x1FF].address = (new_page + _mm_sys_pte_pool_phys_start) >> PAGE_SHIFT;
    
    pte[pte_index].present = 1;
    pte[pte_index].rw = 1;
    pte[pte_index].pwt = 1;
    pte[pte_index].pcd = 1;
    pte[pte_index].address = (lapic_phys_addr) >> PAGE_SHIFT;
        
    local_apic_addr = 0xFFFF000000000000UL |
                      (PML4E_OFFSET_OF_HARDWARE << 39) |
                      (0x1FFUL << 30) |
                      (0x1FFUL << 21) |
                      (pte_index << 12);
    pte_index++;

    // It is a great pity that we can't handle the situation if the current machine has two or more i/o apic.
    assert((intr_ctr_structs+IOAPIC)->total == 1);

    list_node_t *head = &(intr_ctr_structs+IOAPIC)->head;
    list_node_t *node = head->flink;
    // for (size_t i = 0; i < (intr_ctr_structs+IOAPIC)->total; i++, node = node->flink)
    // {
        io_apic_t *ioapic = struct_base(io_apic_t, node, node);
        pte[pte_index].present = 1;
        pte[pte_index].rw = 1;
        pte[pte_index].pwt = 1;
        pte[pte_index].pcd = 1;
        pte[pte_index].address = (ioapic->ioapic_addr) >> PAGE_SHIFT;

        io_apic_addr = 0xFFFF000000000000UL |
                        (PML4E_OFFSET_OF_HARDWARE << 39) |
                        (0x1FFUL << 30) |
                        (0x1FFUL << 21) |
                        (pte_index << 12);

        // if (i == 0)
            // io_apic_addr = ioapic->ioapic_addr;

        // pte_index++;
    // }

    // Read local apic version registers, record local apic version.
    apic_version = (read_lapic_register(LOCAL_APIC_VERSION_REG)) & 0xFF;
    ioapic_version.ioapic_identification = (read_ioapic_register(0) >> 24) & 0xF;

    uint32_t reg_value = read_ioapic_register(1);
    ioapic_version.apic_version = reg_value & 0xFF;
    ioapic_version.max_redirection_entry = (reg_value >> 16) & 0xFF;

}
