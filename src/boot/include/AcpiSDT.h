#ifndef __ACPI_SDT_H__
#define __ACPI_SDT_H__

#include "types.h"

struct _RSDP
{

  // “RSD PTR ” (Notice that this signature must contain a trailing blank character.)
  char Signature[8];

  // This is the checksum of the fields defined in the ACPI 1.0 specification.
  // This includes only the first 20 bytes of this table, bytes 0 to 19, including
  // the checksum field. These bytes must sum to zero.
  uint8_t Checksum;

  // An OEM-supplied string that identifies the OEM.
  char OEMID[6];

  // The revision of this structure. Larger revision numbers are backward compatible
  // to lower revision numbers. The ACPI version 1.0 revision number of this table
  // is zero. The ACPI version 1.0 RSDP Structure only includes the first 20 bytes of
  // this table, bytes 0 to 19. It does not include the Length field and beyond.
  // The current value for this field is 2.
  uint8_t Revision;

  // 32 bit physical address of the RSDT.
  uint32_t RsdtAddress; // deprecated since version 2.0

  // The length of the table, in bytes, including the header, starting from offset 0.
  // This field is used to record the size of the entire table. This field is not
  // available in the ACPI version 1.0 RSDP Structure.
  uint32_t Length;

  // 64 bit physical address of the XSDT.
  uint64_t XsdtAddress;

  // This is a checksum of the entire table, including both checksum fields.
  uint8_t ExtendedChecksum;

  // Reserved field
  uint8_t reserved[3];
} __attribute__((packed));

struct _ACPISDTHeader
{
  // ‘XSDT’. Signature for the Extended System Description Table.
  char Signature[4]; // Signature identifying the type of table

  uint32_t Length; // Length of the table, in bytes, including the header
  // 1
  uint8_t Revision; // Revision number of the structure
  // Entire table must sum to zero.
  uint8_t Checksum; // Checksum of the entire table
  char OEMID[6];    // OEM ID

  // For the XSDT, the table ID is the manufacture model ID.
  // This field must match the OEM Table ID in the FADT.
  char OEMTableID[8]; // OEM Table ID

  // OEM revision of XSDT table for supplied OEM Table ID.
  uint32_t OEMRevision; // OEM Revision number

  // Vendor ID of utility that created the table. For tables
  // containing Definition Blocks, this is the ID for the ASL Compiler.
  uint32_t CreatorID; // Creator ID

  // Revision of utility that created the table. For tables containing
  // Definition Blocks, this is the revision for the ASL Compiler.
  uint32_t CreatorRevision; // Creator Revision number
} __attribute__((packed));

struct _XSDT
{
  struct _ACPISDTHeader Header; // Standard header

  // An array of 64-bit physical addresses that point to other DESCRIPTION_HEADERs.
  // OSPM assumes at least the DESCRIPTION_HEADER is addressable, and then can
  // further address the table based upon its Length field.

  uint64_t Entry[]; // An array of 64-bit addresses pointing to other ACPI tables
};

struct _GenericAddressStructure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
};

struct _FADT
{
  // ACPI Standard Header
  struct _ACPISDTHeader h; // Common ACPI table header
  uint32_t FirmwareCtrl;   // Physical address of the FIRMWARE_CTRL structure
  uint32_t Dsdt;           // Physical address of the Differentiated System Description Table (DSDT)
  // Field used in ACPI 1.0; no longer in use, for compatibility only
  uint8_t Reserved;
  uint8_t PreferredPowerManagementProfile; // The preferred power profile for the system
  uint16_t SCI_Interrupt;                  // System Control Interrupt number
  uint32_t SMI_CommandPort;                // Port address of the SMI command port
  uint8_t AcpiEnable;                      // Value to write to SMI_CommandPort to enable ACPI
  uint8_t AcpiDisable;                     // Value to write to SMI_CommandPort to disable ACPI
  uint8_t S4BIOS_REQ;                      // Value to write to SMI_CommandPort to enter the S4BIOS state
  uint8_t PSTATE_Control;                  // Processor performance state control
  uint32_t PM1aEventBlock;                 // Port address of the Power Management 1a Event Register Block
  uint32_t PM1bEventBlock;                 // Port address of the Power Management 1b Event Register Block
  uint32_t PM1aControlBlock;               // Port address of the Power Management 1a Control Register Block
  uint32_t PM1bControlBlock;               // Port address of the Power Management 1b Control Register Block
  uint32_t PM2ControlBlock;                // Port address of the Power Management 2 Control Register Block
  uint32_t PMTimerBlock;                   // Port address of the Power Management Timer Control Register Block
  uint32_t GPE0Block;                      // Port address of the General Purpose Event 0 Register Block
  uint32_t GPE1Block;                      // Port address of the General Purpose Event 1 Register Block
  uint8_t PM1EventLength;                  // Byte length of the PM1 Event Register Block
  uint8_t PM1ControlLength;                // Byte length of the PM1 Control Register Block
  uint8_t PM2ControlLength;                // Byte length of the PM2 Control Register Block
  uint8_t PMTimerLength;                   // Byte length of the PM Timer Register Block
  uint8_t GPE0Length;                      // Byte length of the GPE0 Register Block
  uint8_t GPE1Length;                      // Byte length of the GPE1 Register Block
  uint8_t GPE1Base;                        // Offset in the GPE number space where GPE1 events start
  uint8_t CStateControl;                   // Support for C State change control
  uint16_t WorstC2Latency;                 // Worst case latency to enter/exit C2 state
  uint16_t WorstC3Latency;                 // Worst case latency to enter/exit C3 state
  uint16_t FlushSize;                      // Size of the cache flush area
  uint16_t FlushStride;                    // Stride used in flushing the cache
  uint8_t DutyOffset;                      // Offset in the Processor Duty Cycle Register
  uint8_t DutyWidth;                       // Bit width of the Processor Duty Cycle Register
  uint8_t DayAlarm;                        // Index to the day-of-month alarm in RTC
  uint8_t MonthAlarm;                      // Index to the month-of-year alarm in RTC
  uint8_t Century;                         // Index to the century in RTC
  // reserved in ACPI 1.0; used since ACPI 2.0+
  uint16_t BootArchitectureFlags; // Flags indicating supported boot architecture (legacy devices, etc.)
  uint8_t Reserved2;              // Reserved field; must be zero
  uint32_t Flags;                 // Miscellaneous flags
  // 12 byte structure; see below for details
  struct _GenericAddressStructure ResetReg; // Register to reset the system
  uint8_t ResetValue;                       // Value to write to the reset register to reset the system
  uint8_t Reserved3[3];                     // Reserved field; must be zero
  // 64bit pointers - Available on ACPI 2.0+
  uint64_t X_FirmwareControl;                         // 64-bit physical address of the FIRMWARE_CTRL structure
  uint64_t X_Dsdt;                                    // 64-bit physical address of the DSDT
  struct _GenericAddressStructure X_PM1aEventBlock;   // 64-bit address of the PM1a Event Register Block
  struct _GenericAddressStructure X_PM1bEventBlock;   // 64-bit address of the PM1b Event Register Block
  struct _GenericAddressStructure X_PM1aControlBlock; // 64-bit address of the PM1a Control Register Block
  struct _GenericAddressStructure X_PM1bControlBlock; // 64-bit address of the PM1b Control Register Block
  struct _GenericAddressStructure X_PM2ControlBlock;  // 64-bit address of the PM2 Control Register Block
  struct _GenericAddressStructure X_PMTimerBlock;     // 64-bit address of the PM Timer Register Block
  struct _GenericAddressStructure X_GPE0Block;        // 64-bit address of the GPE0 Register Block
  struct _GenericAddressStructure X_GPE1Block;        // 64-bit address of the GPE1 Register Block
};

struct _DSDT{
  struct _ACPISDTHeader Header; // Standard ACPI table header
  // The rest of the table contains AML bytecode, which is not directly represented in C structures.
  // The AML bytecode starts here and continues for the length specified in the header.
  uint8_t AmlCode[1]; // Placeholder for the start of the AML bytecode
};


#endif