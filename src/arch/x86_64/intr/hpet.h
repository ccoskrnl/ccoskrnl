#ifndef __INTR_HPET_H__
#define __INTR_HPET_H__

#include "../../../include/types.h"
#include "../../../include/hal/acpi/sdt.h"

// HPET Address Structure
typedef struct _hpet_addr_struct{
    uint8_t AddressSpaceID;      // Address space where the HPET registers are located
    uint8_t RegisterBitWidth;    // Width of the registers in bits
    uint8_t RegisterBitOffset;   // Bit offset within the registers
    uint8_t Reserved;            // Reserved, must be zero
    uint64_t Address;            // Base address of the HPET registers
} __attribute__((packed)) hpet_addr_struct_t;


// HPET Table
typedef struct _hpet_table {
    struct _ACPISDTHeader Header;    // HPET table header

    // Hardware ID of Event Timer Block
    uint32_t HardwareRevID : 8;
    uint32_t NumberOfComparatorsIn1stTimerBlock : 5;
    uint32_t COUNT_SIZE_CAP_counter_size : 1;
    uint32_t Reserved : 1;
    uint32_t LegacyReplacementIRQRoutingCapble : 1;
    uint32_t PCIVendorID : 16;

    hpet_addr_struct_t BaseAddressLower32Bit; // Base address of the HPET registers
    uint8_t HPETNumber;          // HPET number
    uint16_t MinimumTick;        // Minimum clock tick in femtoseconds
    uint8_t PageProtection;      // Page protection and OEM attributes
} hpet_table_t;

#endif