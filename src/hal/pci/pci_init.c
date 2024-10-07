#include "../../include/types.h"
#include "../../include/libk/string.h"
#include "../../include/go/go.h"
#include "../../include/machine_info.h"
#include "../../include/arch/io.h"
#include "pci.h"


extern uint64_t fixup_acpi_table_addr(uint64_t phys_addr);

MCFG_TABLE* MCFG;
MCFG_ALLOCATION* mcfg_allocations;
uint64_t pcie_conf_space_addr;


/* Each item point a buses. */
Pci_seg_grp_t pci_seg_grp[MAX_SEG_GRPS];



/*
 * Use the following formula to determine where the (4096-byte) area for 
 * a function's PCI configuration space is: Physical_Address = 
 * MMIO_Starting_Physical_Address + ((Bus) << 20 | Device << 15 | Function << 12).
 * Here, the MMIO_Starting_Physical_Address is the base address of the ECAM region 
 * from the MCFG table, and is always relative to bus number zero (ie. the 
 * configuration space for all bus 0 functions resides starting at the 
 * address in the table itself). You must start enumerating at the start bus number 
 * decoded by the host bridge.
 */

/*
 * For absolute maximums (with 65536 PCI segment groups and 256 bus segments per 
 * segment group), the amount of physical address space consumed by memory mapped 
 * PCI configuration space ranges would be (up to) 16 TiB (or 244 bytes); and 
 * ACPI's "MCFG" table may (in theory) be slightly larger than 256 MiB (a 16-byte 
 * entry for each individual PCI bus within each PCI segment group plus the 36-byte 
 * table header).
 */



void pci_init()
{
    MCFG = (MCFG_TABLE*)fixup_acpi_table_addr((uint64_t)_current_machine_info->acpi_info.mcfg);
    mcfg_allocations = (MCFG_ALLOCATION*)(MCFG + 1);
    // pcie_conf_space_addr = mm_set_mmio(mcfg_allocations->BaseAddress, 1);

    pci_enumerate_seg_grp(mcfg_allocations);


}