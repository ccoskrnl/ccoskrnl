#include "include/FrameWork.h"
#include "include/ReadFile.h"
#include "include/Wrapper.h"

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *gSimpleFileSystem;
EFI_FILE_PROTOCOL *RootProtocol;
extern EFI_SYSTEM_TABLE *gSystemTable;


VOID ReadFileInit()
{
    EFI_STATUS Status;
    WpLocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID **)&gSimpleFileSystem, L"SimpleFileSystemProtocol");
    Status = gSimpleFileSystem->OpenVolume(gSimpleFileSystem, &RootProtocol);
    if (EFI_ERROR(Status))
    {
        Print(L"Can't open root volume.\n\r");
        UEFI_PANIC;
    }
}

VOID ReadFileFnit()
{
    WpFileClose(RootProtocol, L"Root Volume");
}

EFI_STATUS
ReadFileToBuffer(
    IN CHAR16 *FileName,
    IN OUT VOID **Buffer,
    IN OUT UINTN *BufferSize)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *File;
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = SIZE_OF_EFI_FILE_INFO + 200;

    WpFileOpen(RootProtocol, &File, FileName, EFI_FILE_MODE_READ, 0);

    // Allocate memory for file info
    FileInfo = AllocateZeroPool(FileInfoSize);
    if (FileInfo == NULL)
    {
        // Debug
        UEFI_PANIC;

        return EFI_OUT_OF_RESOURCES;
    }

    // Get file info
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status))
    {
        FreePool(FileInfo);
        // Debug
        UEFI_PANIC;
        return Status;
    }

    // Allocate buffer for file contents
    *BufferSize = FileInfo->FileSize;
    *Buffer = AllocateZeroPool(*BufferSize);
    if (*Buffer == NULL)
    {
        FreePool(FileInfo);
        // Debug
        UEFI_PANIC;
        return EFI_OUT_OF_RESOURCES;
    }

    // Read file contents into buffer
    WpFileRead(File, BufferSize, *Buffer, FileName);
    FreePool(FileInfo);
    WpFileClose(File, FileName);
    return EFI_SUCCESS;
}



EFI_STATUS
ReadFileToBufferAt(
    IN CHAR16 *FileName,
    IN EFI_PHYSICAL_ADDRESS BufferAddress,
    IN OUT UINTN *BufferSize)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *FileProtocol;
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = SIZE_OF_EFI_FILE_INFO + 200;

    WpFileOpen(RootProtocol, &FileProtocol, FileName, EFI_FILE_MODE_READ, 0);

    // Allocate memory for file info
    FileInfo = AllocateZeroPool(FileInfoSize);
    if (FileInfo == NULL)
    {
        // Debug
        UEFI_PANIC;
        return EFI_OUT_OF_RESOURCES;
    }

    // Get file info
    Status = FileProtocol->GetInfo(FileProtocol, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status))
    {
        UEFI_PANIC;
    }

    *BufferSize = FileInfo->FileSize;

    WpFileRead(FileProtocol, BufferSize, (VOID*)BufferAddress, FileName);

    FreePool(FileInfo);

    WpFileClose(FileProtocol, FileName);

    return EFI_SUCCESS;
}