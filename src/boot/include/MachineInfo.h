#ifndef __INCLUDE_MACHINE_INFO_H__
#define __INCLUDE_MACHINE_INFO_H__

#include "FrameWork.h"

#include "types.h"

#define MACHINE_INFO_STRUCT_SIZE                            0x10000
#define CCLDR_ROUTINE_SIZE				                    0x1000

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


// Byte-Alignment
#define MM1MB                                               0x100000
#define MM16MB                                              0x1000000
#define MM32MB                                              0x2000000
#define MM64MB                                              0x4000000
#define MM128MB                                             0x8000000
#define MM256MB                                             0x10000000
#define MM512MB                                             0x20000000
#define MM1GB                                               0x40000000
#define MM2GB                                               0x80000000
#define MM4GB                                               0x100000000
#define MM8GB                                               0x200000000
#define MM16GB                                              0x400000000


typedef struct _LOADER_IMAGE_DESCRIPTOR
{
    UINT16 Width;
    UINT16 Height;
    UINT64 Addr;
    UINT32 Size;

}LOADER_IMAGE_DESCRIPTOR;

typedef struct _LOADER_FONT_DESCRIPTOR
{
    UINTN TTF_Addr;
    UINTN TTF_Size;

}LOADER_FONT_DESCRIPTOR;


typedef struct _LOADER_GRAPHICS_OUTPUT_INFORMATION
{
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelsPerScanLine;
    UINT32 Reserved; // for memory aligned 

    UINTN FrameBufferBase;
    UINTN FrameBufferSize;

}LOADER_GRAPHICS_OUTPUT_INFORMATION;

typedef struct _LOADER_ACPI_INFORMATION
{
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp;
    EFI_ACPI_DESCRIPTION_HEADER *Xsdt;
    UINTN EntryCount;
    UINT64* EntryPtr;
    EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* Fadt;
    EFI_PHYSICAL_ADDRESS Dsdt;
    EFI_ACPI_DESCRIPTION_HEADER* Madt;
    EFI_ACPI_DESCRIPTION_HEADER* Hpet;

} LOADER_ACPI_INFORMATION;


typedef struct _LOADER_MEMORY_INFORMATION
{
    UINTN RamSize;
    UINTN HighestPhysicalAddress;
    UINTN EfiMemDescCount;
    EFI_MEMORY_DESCRIPTOR EfiMemoryDesc[];

} LOADER_MEMORY_INFORMATION;

typedef struct _LOADER_MEMORY_SPACE_DESCRIPTOR
{
    UINTN BaseAddress;
    UINTN Size;

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
    LOADER_MEMORY_SPACE_DESCRIPTOR MemorySpaceInformation[4]; 

    // Background Info
    LOADER_IMAGE_DESCRIPTOR Background; 

    // Font[0]: primary font
    // Font[1]: secondary font
    LOADER_FONT_DESCRIPTOR Font[2];

    // The size of the sum of all files in pages
    UINTN SumOfSizeOfFilesInPages;

    LOADER_GRAPHICS_OUTPUT_INFORMATION GraphicsOutputInformation;
    LOADER_ACPI_INFORMATION AcpiInformation;
    LOADER_MEMORY_INFORMATION MemoryInformation;

} LOADER_MACHINE_INFORMATION;

// extern LOADER_MACHINE_INFORMATION* MachineInfo;

#endif
