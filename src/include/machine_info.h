#ifndef __MACHINE_INFO_H__
#define __MACHINE_INFO_H__

#include "types.h"
#include "hal/acpi.h"
#include "go/go.h"
#include "arch/cpu_features.h"

/*  The size of LOADER_MACHINE_INFORMATION structure in bytes  */
#define MACHINE_INFO_SIZE 					    0x10000
#define CCLDR_ROUTINE_SIZE                      0x1000


/*  The size of start-up routine in bytes  */
#define STARTUP_ROUTINE_SIZE 					0x8000
#define AP_STARTUP_ROUTINE_DEF_ADDR             0x1000


/*
 * ┌───────────────────┐◄────── entry_addr         
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │   Startup Routine │                           
 * │        Code       │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * ├───────────────────┤◄────── entry_addr + 0x800 
 * │    MTRRs state    │                           
 * ├───────────────────┤◄────── entry_addr + 0x900 
 * │   Long-Mode GDT   │                           
 * ├───────────────────┤◄────── entry_addr + 0xA00 
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │       Shared      │                           
 * │        Data       │                           
 * │        Area       │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * │                   │                           
 * ├───────────────────┤◄────── entry_addr + 0x1000
 * │                   │                           
 * │     Temporary     │                           
 * │     Page-Table    │                           
 * │                   │                           
 * └───────────────────┘                           
 */



//******************************************************
// EFI_MEMORY_TYPE
//******************************************************
// These type values are discussed in Memory Type Usage before ExitBootServices()  and  Memory Type Usage after ExitBootServices().
typedef enum
{
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct _EFI_MEMORY_DESCRIPTOR
{
    uint64_t Type;
    uint64_t PhysicalStart;
    uint64_t VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
    uint64_t Reserved;

} __attribute__((packed)) EFI_MEMORY_DESCRIPTOR;

///
/// EFI Time Abstraction:
///  Year:       1900 - 9999
///  Month:      1 - 12
///  Day:        1 - 31
///  Hour:       0 - 23
///  Minute:     0 - 59
///  Second:     0 - 59
///  Nanosecond: 0 - 999,999,999
///  TimeZone:   -1440 to 1440 or 2047
///
typedef struct
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
    uint8_t Pad1;
    uint32_t Nanosecond;
    int16_t TimeZone;
    uint8_t Daylight;
    uint8_t Pad2;
} EFI_TIME;

typedef struct
{
    int8_t Blue;
    int8_t Green;
    int8_t Red;
    int8_t Reserved;

} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

/**

  Definition of EFI_IMAGE_OUTPUT.

  @param Width  Width of the output image.

  @param Height Height of the output image.

  @param Bitmap Points to the output bitmap.

  @param Screen Points to the EFI_GRAPHICS_OUTPUT_PROTOCOL which
                describes the screen on which to draw the
                specified image.

**/

typedef struct _LOADER_IMAGE_DESCRIPTOR
{
    uint16_t width;
    uint16_t height;
    uint64_t addr;
    uint32_t size;

}LOADER_IMAGE_DESCRIPTOR;

typedef struct _LOADER_FONT_DESCRIPTOR
{
    uint64_t ttf_addr;
    uint64_t ttf_size;

} LOADER_FONT_DESCRIPTOR; // 0x10 bytes


typedef struct _LOADER_GRAPHICS_OUTPUT_INFORMATION
{
    uint32_t HorizontalResolution;
    uint32_t VerticalResolution;
    uint32_t PixelsPerScanLine;
    uint32_t reserved; // for memory aligned

    uint64_t FrameBufferBase;
    uint64_t FrameBufferSize;
} LOADER_GRAPHICS_OUTPUT_INFORMATION;


typedef struct _LOADER_ACPI_INFORMATION
{
    struct _RSDP *rsdp;
    struct _XSDT *xsdt;
    uint64_t entry_count;
    uint64_t* entry_ptr;
    struct _FADT *fadt;
    struct _DSDT *dsdt;
    struct _ACPISDTHeader* madt;
    struct _ACPISDTHeader* hpet;
    struct _ACPISDTHeader* mcfg;

} LOADER_ACPI_INFORMATION;

typedef struct _LOADER_MEMORY_INFORMATION
{
    uint64_t ram_size;
    uint64_t highest_physical_addr;
    uint64_t efi_mem_desc_count;
    EFI_MEMORY_DESCRIPTOR efi_mem_desc[];

} LOADER_MEMORY_INFORMATION;

typedef struct _LOADER_MEMORY_SPACE_DESCRIPTOR
{
    uint64_t base_address;
    uint64_t size;

} LOADER_MEMORY_SPACE_DESCRIPTOR;

typedef struct _LOADER_MACHINE_INFORMATION
{
    // Memory Informations
    //
    // 1st: kernel Space
    // 2nd: ccldr image space
    // 3rd: krnl image space
    // 4th: start-up routine
    //
    LOADER_MEMORY_SPACE_DESCRIPTOR memory_space_info[4]; 
    
    // Background Info
    LOADER_IMAGE_DESCRIPTOR bg; 

    // Font[0]: SourceHanSansSCVF
    // Font[1]: AgeFonts0001
    LOADER_FONT_DESCRIPTOR font[2];

    // The sum of size of all files in bytes.
    uint64_t sum_of_size_of_files_in_pages;

    LOADER_GRAPHICS_OUTPUT_INFORMATION graphics_info;
    LOADER_ACPI_INFORMATION acpi_info;
    LOADER_MEMORY_INFORMATION memory_info;

} LOADER_MACHINE_INFORMATION;

extern struct _LOADER_MACHINE_INFORMATION *_current_machine_info;

#endif
