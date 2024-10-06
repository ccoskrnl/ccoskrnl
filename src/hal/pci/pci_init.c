#include "../../include/types.h"
#include "../../include/libk/string.h"
#include "../../include/go/go.h"
#include "../../include/machine_info.h"
#include "../../include/arch/io.h"
#include "../../include/arch/mm.h"
#include "pci.h"


extern uint64_t fixup_acpi_table_addr(uint64_t phys_addr);

MCFG_TABLE* MCFG;
MCFG_ALLOCATION* mcfg_allocations;
uint64_t pcie_conf_space_addr;

void pci_init()
{
    MCFG = (MCFG_TABLE*)fixup_acpi_table_addr((uint64_t)_current_machine_info->acpi_info.mcfg);
    mcfg_allocations = (MCFG_ALLOCATION*)(MCFG + 1);
    pcie_conf_space_addr = mm_set_mmio(mcfg_allocations->BaseAddress, 1);
}