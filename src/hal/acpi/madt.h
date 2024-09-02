#ifndef __ACPI_MADT_H__
#define __ACPI_MADT_H__

#include "sdt.h"
#include "../../include/libk/list.h"

#define MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES                   0x18

typedef enum
{
    ProcessorLocalAPIC = 0,
    IOAPIC,
    InterruptSourceOverride,
    NonMaskableInterruptSource,
    LocalAPICNMI,
    LocalAPICAddressOverride,
    IOSAPIC,
    LocalSAPIC,
    PlatformInterruptSources,
    ProcessorLocalx2APIC,
    Localx2APICNMI,
    GICCPUInterface,
    GICDistributor,
    GICMSIFrame,
    GICRedistributor,
    GICInterruptTranslationService,
    MultiprocessorWakeup

} InterruptControllerStructureType;


// APIC structure header
typedef struct _apic_header
{
    uint8_t type;          // Entry Type
    uint8_t length;        // Length of the structure
} __attribute__((packed)) apic_header_t;

// Processor Local APIC structure
typedef struct _processor_local_apic
{

    apic_header_t header;  // Type 0
    uint8_t acpi_processor_id; // ACPI processor ID
    uint8_t apic_id;       // Local APIC ID
    uint32_t flags;        // Flags (e.g., if the processor is enabled)
						   // Flags (bit 0 = Processor Enabled) (bit 1 = Online Capable)
    list_node_t node;
} __attribute__((packed)) processor_local_apic_t;

// I/O APIC structure
typedef struct _io_apic
{
    apic_header_t header;  // Type 1
    uint8_t io_apic_id;    // I/O APIC ID
    uint8_t reserved;      // Reserved, must be zero
    uint32_t io_apic_addr; // I/O APIC Address

    // The global system interrupt number where this I/O APIC’s interrupt 
    // inputs start. The number of interrupt inputs is determined by the 
    // I/O APIC’s Max Redir Entry register.
    uint32_t global_sys_interrupt_base; // Global System Interrupt Base

    list_node_t node;
} __attribute__((packed)) io_apic_t;

// Interrupt Source Override structure
typedef struct _iso
{
    apic_header_t header;  // Type 2
    uint8_t bus;           // Bus where the interrupt source resides
    uint8_t source;        // Bus-relative interrupt source (IRQ)
    uint32_t global_sys_interrupt; // Global System Interrupt this source will signal
    uint16_t flags;        // Flags (e.g., polarity and trigger mode)

    list_node_t node;
} __attribute__((packed)) iso_t;

// Non-maskable Interrupts structure
typedef struct _nmi
{
    apic_header_t header;  // Type 4
    uint8_t acpi_processor_id; // Processor ID
    uint16_t flags;        // Flags
    uint8_t lint;          // Local APIC LINTn pin number

    list_node_t node;
} __attribute__((packed)) nmi_t;

// Local APIC Address Override structure
typedef struct _local_apic_addr_override
{
    apic_header_t header;  // Type 5
    uint16_t reserved;     // Reserved, must be zero
    uint64_t local_apic_addr; // 64-bit physical address of local APIC

    list_node_t node;
} __attribute__((packed)) local_apic_addr_override_t;


struct _MADT {

	struct _ACPISDTHeader h;
	uint32_t local_apic_addr;

	uint32_t flags;			//	Flags (1 = Dual 8259 Legacy PICs Installed)
							// 	If bit 0 in the flags field is set then you 
							//  need to mask all the 8259 PIC's interrupts, 
							//  but you should probably do this anyway.
} __attribute__((packed));

#endif