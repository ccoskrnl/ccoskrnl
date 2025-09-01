/** @file
 * CcLoader
 **/

#include "include/FrameWork.h"
#include "include/AcpiSDT.h"
#include "include/MachineInfo.h"
#include "include/Wrapper.h"
#include "include/ReadFile.h"

#define PAGE_ALIGNED(x)               (((UINTN)x + (EFI_PAGE_SIZE - 1)) & ~(EFI_PAGE_SIZE - 1))
#define KRNL_PATH                     L"EFI\\ChengChengOS\\ccoskrnl"
#define CCLDR_PATH                    L"EFI\\ChengChengOS\\ccldr"
#define AP_PATH                       L"EFI\\ChengChengOS\\ap"
#define BG_PATH                       L"EFI\\ChengChengOS\\rec\\bg\\background"
#define BG_PNG_PATH                   L"EFI\\ChengChengOS\\rec\\bg\\background.png"

#define AgeFonts001_TTF_PATH          L"EFI\\ChengChengOS\\rec\\AgeFonts001.ttf"
#define SourceHanSansSCVF_TTF_PATH    L"EFI\\ChengChengOS\\rec\\SourceHanSansSCVF.ttf"


// Protocols
EFI_SYSTEM_TABLE *gSystemTable;

#ifndef _DEBUG
extern VOID DisplayLoadingLogo();
#endif


extern VOID FindAcpiTable(IN LOADER_MACHINE_INFORMATION *MachineInfo);
extern VOID AdjustGraphicsMode(IN LOADER_MACHINE_INFORMATION *MachineInfo);
extern VOID GetPNGSize(
        IN CHAR16* PngPath,
        IN OUT UINT32* Width,
        IN OUT UINT32* Height
        );


// Function to convert memory type to string
    CONST CHAR16 *
MemoryTypeToStr(
        IN EFI_MEMORY_TYPE MemoryType)
{
    switch (MemoryType)
    {
        case EfiReservedMemoryType:
            return L"Reserved";
        case EfiLoaderCode:
            return L"Loader Code";
        case EfiLoaderData:
            return L"Loader Data";
        case EfiBootServicesCode:
            return L"Boot Services Code";
        case EfiBootServicesData:
            return L"Boot Services Data";
        case EfiRuntimeServicesCode:
            return L"Runtime Services Code";
        case EfiRuntimeServicesData:
            return L"Runtime Services Data";
        case EfiConventionalMemory:
            return L"Conventional Memory";
        case EfiUnusableMemory:
            return L"Unusable Memory";
        case EfiACPIReclaimMemory:
            return L"ACPI Reclaim Memory";
        case EfiACPIMemoryNVS:
            return L"ACPI Memory NVS";
        case EfiMemoryMappedIO:
            return L"Memory Mapped IO";
        case EfiMemoryMappedIOPortSpace:
            return L"Memory Mapped IO Port Space";
        case EfiPalCode:
            return L"Pal Code";
        case EfiPersistentMemory:
            return L"Persistent Memory";
        default:
            return L"Unknown Type";
    }
}




STATIC
    VOID
GetEfiMemMap(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable,
        IN LOADER_MACHINE_INFORMATION *MachineInfo)
{
    EFI_STATUS Status;
    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    UINTN Index = 0;
    UINTN PageCount = 0;
    EFI_PHYSICAL_ADDRESS HighestAddress = 0;
    EFI_MEMORY_DESCRIPTOR *Descriptor;

    Status = gBS->GetMemoryMap(
            &MemoryMapSize,
            MemoryMap,
            &MapKey,
            &DescriptorSize,
            &DescriptorVersion);

    MemoryMap = MachineInfo->MemoryInformation.EfiMemoryDesc;

    Status = gBS->GetMemoryMap(
            &MemoryMapSize,
            MemoryMap,
            &MapKey,
            &DescriptorSize,
            &DescriptorVersion);
    if (EFI_ERROR(Status))
    {
        Print(L"GetMemoryMap failed with error code:%d. \n\r", Status);
        UEFI_PANIC;
    }

#ifdef _DEBUG
    Print(L"Type                    Physical Start    Number of Pages    Attribute\n");
#endif
    for (Index = 0; Index < MemoryMapSize / DescriptorSize; Index++)
    {
        Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + (Index * DescriptorSize));
#ifdef _DEBUG
        Print(L"%-24s %-16llx %-16llx %-16llx\n",
                MemoryTypeToStr(Descriptor->Type),
                Descriptor->PhysicalStart,
                Descriptor->NumberOfPages,
                Descriptor->Attribute);
#endif
    }

    MachineInfo->MemoryInformation.EfiMemDescCount = Index;

    /*
     * Calculate total RAM size and the highest physical address
     */
    for (UINTN i = 0; i < MemoryMapSize / DescriptorSize; i++)
    {
        Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + i * DescriptorSize);
        switch (Descriptor->Type)
        {
            case EfiConventionalMemory:
                PageCount += Descriptor->NumberOfPages;
                break;
            case EfiLoaderCode:
            case EfiLoaderData:
            case EfiBootServicesCode:
            case EfiBootServicesData:
            case EfiRuntimeServicesCode:
            case EfiRuntimeServicesData:
            case EfiACPIReclaimMemory:
            case EfiACPIMemoryNVS:
            case EfiMemoryMappedIO:
            case EfiMemoryMappedIOPortSpace:
            case EfiPalCode:
            case EfiPersistentMemory:
                EFI_PHYSICAL_ADDRESS EndAddress = Descriptor->PhysicalStart + Descriptor->NumberOfPages * EFI_PAGE_SIZE;
                if (EndAddress > HighestAddress)
                {
                    HighestAddress = EndAddress;
                }
                break;

            default:
                break;
        }
    }

    MachineInfo->MemoryInformation.RamSize = PageCount * EFI_PAGE_SIZE;

    /*
     * Subtract page size to obtain the last addressable page.
     */
    MachineInfo->MemoryInformation.HighestPhysicalAddress = HighestAddress - EFI_PAGE_SIZE;

#ifdef _DEBUG
    Print(L"Total RAM: %ld Bytes\nHighest Physical Address: %-16lx\n",
            MachineInfo->MemoryInformation.RamSize,
            MachineInfo->MemoryInformation.HighestPhysicalAddress);
#endif
}



/**
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

 **/
EFI_STATUS
    EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable)
{

    // Status
    EFI_STATUS Status;

    // UINT32 PNGWidth;
    // UINT32 PNGHeight;

    // Current Machine Information
    EFI_PHYSICAL_ADDRESS InfoAddress;
    LOADER_MACHINE_INFORMATION *MachineInfo;

    EFI_PHYSICAL_ADDRESS KrnlImageBase;
    UINTN KernelImageBufferSize = 0;
    UINTN KernelSpaceSize = 0;

    UINTN CcldrBufferSize = 0;
    EFI_PHYSICAL_ADDRESS CcldrBase;
    UINTN CcldrSpaceSize = 0;


    EFI_PHYSICAL_ADDRESS FileBuffer;
    UINTN FileSize;

    // ExitUefiService
    UINTN MemMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemMap = 0;
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DesVersion = 0;

    // Start-up routine
    UINTN StartUpRoutineBufferSize = 0;
    UINTN StartUpRoutineAddress = 0x1000;


    /*
     * Clear screen.
     */
    gSystemTable = SystemTable;
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);


    /*
     * Allocate memory for temporary machine_info structure.
     */
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, MACHINE_INFO_STRUCT_SIZE >> EFI_PAGE_SHIFT, &InfoAddress);
    if (EFI_ERROR(Status))
    {
        Print(L"[ERROR]: Allocate memory for Machine Information Structure failed...\n\r");
        goto ExitUefi;
    }

    MachineInfo = (LOADER_MACHINE_INFORMATION *)(InfoAddress);
    gBS->SetMem((VOID*)MachineInfo, MACHINE_INFO_STRUCT_SIZE, 0);



    /*
     * Parpering to read file.
     */
    ReadFileInit();




    /* ========================= Adjust the Graphics mode ====================== */
    AdjustGraphicsMode(MachineInfo);



#ifndef _DEBUG
    DisplayLoadingLogo();
#endif



#ifdef _DEBUG
    // RootDirList();
#endif




    /* ========================= Print Memory Map ====================== */
    GetEfiMemMap(ImageHandle, SystemTable, MachineInfo);

    /* ========================= Find Acpi Configuration ====================== */
    FindAcpiTable(MachineInfo);



    /* ==================== Load krnl, ccldr and icons ==================== */

    /*
     * Before entering bsp_init function to remap kernel code, 
     * we need to dynamiclly calcuate the kernel space reservation 
     * size based on the installed physical RAM size, as well as 
     * how much memory ccldr should use to store temporary page tables. 
     */

    /*
     * Kernel Space Size = Size of Installed Ram / 4;
     */
    KernelSpaceSize = ((MachineInfo->MemoryInformation.HighestPhysicalAddress + EFI_PAGE_SIZE) >> 2);

    /*
     * The smallest size of Kernel Space is 512 MiB.
     */
    KernelSpaceSize = (KernelSpaceSize + MM512MB - 1) & ~(MM512MB - 1);

    if (KernelSpaceSize == MM512MB)
        CcldrSpaceSize = (MM1MB << 2);
    else
        CcldrSpaceSize = (MM1MB << 2) * (KernelSpaceSize / MM512MB);

#ifdef _DEBUG
    Print(L"Allocating memmory for Kernel Space.\n\r");
    Print(L"Kernel Space size is %f GiB.\n\r", (float)(KernelSpaceSize / MM1GB));
    Print(L"Ccldr Space size is %f MiB.\n\r", (float)(CcldrSpaceSize / MM1MB));
#endif




    /*
     * Allocate and Zero memory for Kernel Space.
     */
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, (KernelSpaceSize >> EFI_PAGE_SHIFT), &KrnlImageBase);
    if (EFI_ERROR(Status))
    {
        Print(L"[ERROR]: Kernel Space Size: %llx, Allocate memory for krnl failed...\n\r", KernelSpaceSize);
        goto ExitUefi;
    }
    gBS->SetMem((VOID*)KrnlImageBase, KernelSpaceSize, 0);




    /*
     * Allocate and Zero memory for Ccldr Space.
     */
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, CcldrSpaceSize >> EFI_PAGE_SHIFT, &CcldrBase); 
    if (EFI_ERROR(Status))
    {
        Print(L"[ERROR]: Allocate memory for ccldr failed...\n\r");
        goto ExitUefi;
    }
    gBS->SetMem((VOID*)CcldrBase, CcldrSpaceSize, 0);


    /*
     * Copy machine_info into ccldr space.
     */
    memcpy((void*)CcldrBase, (void*)MachineInfo, MACHINE_INFO_STRUCT_SIZE);
    MachineInfo = (LOADER_MACHINE_INFORMATION*)CcldrBase;
    CcldrBase += (MACHINE_INFO_STRUCT_SIZE);


    /**
     * Free the original machine_info memory.
     */
    Status = gBS->FreePages(InfoAddress, MACHINE_INFO_STRUCT_SIZE >> EFI_PAGE_SHIFT);
    if (EFI_ERROR(Status))
    {
        Print(L"[ERROR]: Free Machine Information Structure failed with error code: %d\n\r", Status);
        goto ExitUefi;
    }

    /**
     * Allocate pages to store start-up routine code for application processors initialization.
     */
    Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, STARTUP_ROUTINE_SIZE >> EFI_PAGE_SHIFT, &StartUpRoutineAddress); 
    while (EFI_ERROR(Status))
    {
        StartUpRoutineAddress += EFI_PAGE_SIZE;
        if (StartUpRoutineAddress >= MM1MB)
        {
            Print(L"[ERROR]: Allocate memory for start-up routine failed...\n\r");
            goto ExitUefi;
        }
        Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, STARTUP_ROUTINE_SIZE >> EFI_PAGE_SHIFT, &StartUpRoutineAddress); 
    }

#ifdef _DEBUG
    Print(L"Kernel Space at %llx, size: %llx\n\r", KrnlImageBase, KernelSpaceSize);
    Print(L"Ccldr Space at %llx, size: %llx\n\r", CcldrBase, CcldrSpaceSize);
#endif

    /*
     * Read images into ram.
     */
    ReadFileToBufferAt(KRNL_PATH, KrnlImageBase, &KernelImageBufferSize);
    ReadFileToBufferAt(CCLDR_PATH, CcldrBase, &CcldrBufferSize);
    ReadFileToBufferAt(AP_PATH, StartUpRoutineAddress, &StartUpRoutineBufferSize);


    /*
     * Set corresponding members.
     */
    MachineInfo->MemorySpaceInformation[0].BaseAddress = KrnlImageBase;
    MachineInfo->MemorySpaceInformation[0].Size = KernelSpaceSize;
    MachineInfo->MemorySpaceInformation[1].BaseAddress = (UINTN)MachineInfo;
    MachineInfo->MemorySpaceInformation[1].Size = CcldrSpaceSize;
    MachineInfo->MemorySpaceInformation[2].BaseAddress = KrnlImageBase;
    MachineInfo->MemorySpaceInformation[2].Size = KernelImageBufferSize;
    MachineInfo->MemorySpaceInformation[3].BaseAddress = StartUpRoutineAddress;
    MachineInfo->MemorySpaceInformation[3].Size = STARTUP_ROUTINE_SIZE;

#ifdef _DEBUG
    Print(L"AP routine at %llx, size: %llx\n\r", MachineInfo->MemorySpaceInformation[3].BaseAddress, MachineInfo->MemorySpaceInformation[3].Size);
#endif



    /*
     * These is one thing to note here that we need to construct a temporary 4-level page-table for ap startup routine.
     * The key point corresponds the value of cr3 of bsp large than 4-GB. So, we might need to switch long-mode before
     * load bsp's cr3 into ap. Luckily, the temporary page-table won't become very complicated since the ap startup
     * routine is only located in 0x0000 ~ MM1MB.
     *
     * Here, we will build a temporary page-table and place the temporary page-table, which follows ap startup routine.
     * The mapped memory space by the page-table can hold the whole ap startup routine space.
     */
#ifndef REMOVE_TMP_PT

    UINTN* Pml4 = (UINTN*)(StartUpRoutineAddress + 0x1000);
    UINTN* PDPT = (UINTN*)(StartUpRoutineAddress + 0x2000);
    UINTN* PD = (UINTN*)(StartUpRoutineAddress + 0x3000);
    UINTN* PT = (UINTN*)(StartUpRoutineAddress + 0x4000);

    Pml4[0] = (UINTN)PDPT | 0x3;
    PDPT[0] = (UINTN)PD | 0x3;
    PD[0] = (UINTN)PT | 0x3;
    for (int i = 0; i < (STARTUP_ROUTINE_SIZE >> EFI_PAGE_SHIFT); i++)
        PT[(StartUpRoutineAddress >> EFI_PAGE_SHIFT) + i] = (StartUpRoutineAddress + (i << EFI_PAGE_SHIFT)) | 0x3;

#endif

    /*
     * Read turetype fonts into kernel image space and update the value of the related member.
     */
    MachineInfo->SumOfSizeOfFilesInPages = (PAGE_ALIGNED(KernelImageBufferSize) >> EFI_PAGE_SHIFT);

    FileBuffer = KrnlImageBase + (MachineInfo->SumOfSizeOfFilesInPages << EFI_PAGE_SHIFT);
    ReadFileToBufferAt(AgeFonts001_TTF_PATH, FileBuffer, &FileSize);
    MachineInfo->Font[1].TTF_Addr = FileBuffer;
    MachineInfo->Font[1].TTF_Size = FileSize;

    MachineInfo->SumOfSizeOfFilesInPages += (PAGE_ALIGNED(FileSize) >> EFI_PAGE_SHIFT);
    FileBuffer = KrnlImageBase + (MachineInfo->SumOfSizeOfFilesInPages << EFI_PAGE_SHIFT);
    ReadFileToBufferAt(SourceHanSansSCVF_TTF_PATH, FileBuffer, &FileSize);
    MachineInfo->Font[0].TTF_Addr = FileBuffer;
    MachineInfo->Font[0].TTF_Size = FileSize;



    /*
     * Read wallpaper file into kernel image space.
     * Parse the file and record the wallpaper size.
     */
    MachineInfo->SumOfSizeOfFilesInPages += (PAGE_ALIGNED(FileSize) >> EFI_PAGE_SHIFT);
    FileBuffer = KrnlImageBase + (MachineInfo->SumOfSizeOfFilesInPages << EFI_PAGE_SHIFT);
    ReadFileToBufferAt(BG_PATH, FileBuffer, &FileSize);
    // GetPNGSize(BG_PNG_PATH, &PNGWidth, &PNGHeight);
    MachineInfo->Background.Width = (UINT16)1920;
    MachineInfo->Background.Height = (UINT16)1080;
    MachineInfo->Background.Addr = (UINTN)FileBuffer;
    MachineInfo->Background.Size = (UINT32)FileSize;

    MachineInfo->SumOfSizeOfFilesInPages += (PAGE_ALIGNED(FileSize) >> EFI_PAGE_SHIFT);



    /*
     * Since we already finish all file I/O work, so we ned to call the fnit function.
     */
    ReadFileFnit();


#ifdef _DEBUG

    Print(L"Start-up Routine at %llx\n\r", StartUpRoutineAddress);
    Print(L"Kernel Space at %llx, size: %llx\n\r", MachineInfo->MemorySpaceInformation[0].BaseAddress, MachineInfo->MemorySpaceInformation[0].Size);
    Print(L"Ccldr Space at %llx, size: %llx\n\r", MachineInfo->MemorySpaceInformation[1].BaseAddress, MachineInfo->MemorySpaceInformation[1].Size);
    Print(L"Ram Size: %llx, Highest Physical Address: %llx\n\r", MachineInfo->MemoryInformation.RamSize, MachineInfo->MemoryInformation.HighestPhysicalAddress);
    Print(L"FADT Version: %d, at %llx\n\r", (UINTN)MachineInfo->AcpiInformation.Fadt->Header.Revision, (UINTN)MachineInfo->AcpiInformation.Fadt);
    Print(L"DSDT at %llx\n\r", (UINTN)MachineInfo->AcpiInformation.Dsdt);
    Print(L"HorizontalResolution: %d, VerticalResolution: %d\n\r", MachineInfo->GraphicsOutputInformation.HorizontalResolution, MachineInfo->GraphicsOutputInformation.VerticalResolution);
    Print(L"FrameBufferBase at %llx, size: %llx\n\r", MachineInfo->GraphicsOutputInformation.FrameBufferBase, MachineInfo->GraphicsOutputInformation.FrameBufferSize);

#endif

    /*
     * Parpering to enter ccldr.
     */
    void (*ccldr_start)(LOADER_MACHINE_INFORMATION *MachineInfo) = (void *)(CcldrBase);

    /* ========================== Exit Services ================================ */
    gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DescriptorSize, &DesVersion);
    gBS->ExitBootServices(ImageHandle, MapKey);


    /*
     * translate the control of cpu to ccldr and never be returned.
     */
    ccldr_start(MachineInfo);

    return EFI_SUCCESS;

    // Exit with error
ExitUefi:
    while (1)
    {
        /* code */
    }
    return Status;
}
