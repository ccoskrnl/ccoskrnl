#ifndef __HAL_PCI_H__
#define __HAL_PCI_H__

#include "../../include/types.h"
#include "../../include/libk/list.h"
#include "../acpi/sdt.h"

#define PCI_CFG_DATA                                                        0xCFC
#define PCI_CFG_CTRL                                                        0xCF8

/*
 * Segment Groups:
 *      A system can theoretically have up to 65,536 PCI Segment Groups.
 * Buses per Segment Group:
 *      Each segment group can have up to 256 PCI buses.
 * Devices per Bus:
 *      Each PCI bus can support up to 32 devices.
 * Functions per Device:
 *      Each device can have up to 8 functions.
 */

#define MAX_PCI_SEGMENT_GROUPS                                              65536
#define MAX_PCI_BUSES                                                       256
#define MAX_PCI_DEVICES                                                     32
#define MAX_PCI_FUNCTIONS                                                   8

/*
 * A system can theoretically have up to 65,536 PCI Segment Groups. Actually,
 * for personal pc, which only have one segment group.
 */
#define MAX_SEG_GRPS                                                        32


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


typedef enum _PCIeDeviceFuncType
{
    PCItoPCI_Bridge

}PCIeDeviceFuncType;

typedef struct {
    uint16_t vendor_id;        // Vendor ID
    uint16_t device_id;        // Device ID
    uint16_t command;          // Command register
    uint16_t status;           // Status register
    uint8_t revision_id;       // Revision ID
    uint8_t prog_if;           // Programming Interface
    uint8_t subclass;          // Subclass code
    uint8_t class_code;        // Class code
    uint8_t cache_line_size;   // Cache line size
    uint8_t latency_timer;     // Latency timer
    uint8_t header_type;       // Header type
    uint8_t bist;              // Built-in self-test
    uint32_t bar[6];           // Base Address Registers (BARs)
    uint32_t cardbus_cis_ptr;  // CardBus CIS Pointer
    uint16_t subsystem_vendor_id; // Subsystem Vendor ID
    uint16_t subsystem_id;     // Subsystem ID
    uint32_t expansion_rom_base_addr; // Expansion ROM Base Address
    uint8_t capabilities_ptr;  // Capabilities Pointer
    uint8_t reserved1[3];      // Reserved
    uint32_t reserved2;        // Reserved
    uint8_t interrupt_line;    // Interrupt Line
    uint8_t interrupt_pin;     // Interrupt Pin
    uint8_t min_grant;         // Minimum Grant
    uint8_t max_latency;       // Maximum Latency
} pci_config_space_t;


typedef struct _Pci_seg_grp_node
{
    uint64_t base_addr;
    uint32_t seg_grp;
    uint8_t start_bus_num;
    uint8_t end_bus_num;
    uint8_t bus_count;

    list_node_t buses;
} Pci_seg_grp_t;

typedef struct _Pci_bus_node
{
    uint32_t seg_grp;
    uint8_t bus;
    uint8_t dev_count;
    list_node_t devs;

    list_node_t bus_node;
} Pci_bus_t;

typedef struct _Pci_dev_node
{
    uint8_t dev;
    uint8_t func_count;

    Pci_bus_t* bus;
    list_node_t functions;    

    list_node_t dev_node;
} Pci_dev_t;

typedef struct _Pci_func_node
{
    pci_config_space_t* config_space;
    uint8_t func_code;
    Pci_dev_t* dev;

    PCIeDeviceFuncType type;

    union
    {
        Pci_bus_t *Pci2PciBridge;
    } func;

    list_node_t func_node;
} Pci_func_t;

/* Each item point a buses. */
extern Pci_seg_grp_t pci_seg_grp[MAX_SEG_GRPS];

void pci_enumerate_seg_grp(MCFG_ALLOCATION* mcfg);

void pci_read_config_dword(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned int *val);
void pci_read_config_word(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned short *val);
void pci_read_config_byte(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned char *val);

#endif
