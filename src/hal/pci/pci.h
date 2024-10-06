#ifndef __HAL_PCI_H__
#define __HAL_PCI_H__

#include "../../include/types.h"
#include "../acpi/sdt.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// MCFG Table Structure
typedef struct {
    struct _ACPISDTHeader Header; // ACPI table header
    uint64_t Reserved;        // Reserved, must be zero
} __attribute__((packed)) MCFG_TABLE;

// MCFG Configuration Space Base Address Allocation Structure
typedef struct {
    uint64_t BaseAddress;     // Base address of the configuration space
    uint16_t PCIeSegmentGroup;// PCIe segment group number
    uint8_t  StartBusNumber;  // Start bus number
    uint8_t  EndBusNumber;    // End bus number
    uint32_t Reserved;        // Reserved, must be zero
} MCFG_ALLOCATION;

#endif