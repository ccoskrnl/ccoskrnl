#include "include/Wrapper.h"

VOID WpFileOpen(
    IN EFI_FILE_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN CHAR16 *FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes)
{
    EFI_STATUS Status;
    Status = This->Open(This, NewHandle, FileName, OpenMode, Attributes);
    if (EFI_ERROR(Status))
    {
        Print(L"Open %s failed with error code: %d\n\r", FileName, Status);
        UEFI_PANIC;
    }
}

VOID WpFileRead(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer,
    IN CHAR16 *FileName)
{

    EFI_STATUS Status;
    Status = This->Read(This, BufferSize, Buffer);
    if (EFI_ERROR(Status))
    {
        Print(L"Read %s failed with error code: %d\n\r", FileName, Status);
        UEFI_PANIC;
    }
}

VOID WpCloseProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    IN EFI_HANDLE AgentHandle,
    IN EFI_HANDLE ControllerHandle,
    IN CHAR16 *ProtocolName)
{
    EFI_STATUS Status;
    Status = gBS->CloseProtocol(Handle, Protocol, AgentHandle, ControllerHandle);
    if (EFI_ERROR(Status))
    {
        Print(L"Unable to close %s with error code: %d \n\r", ProtocolName, Status);
        switch (Status)
        {
        case EFI_INVALID_PARAMETER:
            Print(L"Handle or Protocol is NULL.\n\r");
            break;
        case EFI_NOT_FOUND:
            Print(L"Handle does not support the protocol specified by Protocol.\n\r");
            Print(L"Or, The protocol interface specified by Handle and Protocol is not currently open by AgentHandle and ControllerHandle.\n\r");
            break;
        default:
            break;
        }
        UEFI_PANIC;
    }
}

VOID WpLocateProtocol(
    IN EFI_GUID *Protocol,
    IN VOID *Registration OPTIONAL,
    OUT VOID **Interface,
    IN CHAR16 *ProtocolName)
{
    EFI_STATUS Status;
    Status = gBS->LocateProtocol(Protocol, Registration, Interface);
    if (EFI_ERROR(Status))
    {
        Print(L"Unable to locate %s\n\r", ProtocolName);
        switch (Status)
        {

        case EFI_INVALID_PARAMETER:
            Print(L"Interface is NULL. Protocol is NULL.\n\r");
            break;
        case EFI_NOT_FOUND:
            Print(L"No protocol instances were found that match Protocol and Registration.\n\r");
            break;
        default:
            break;
        }
        UEFI_PANIC;
    }
}

VOID WpFileClose(
    IN EFI_FILE_PROTOCOL *This,
    IN CHAR16 *FileName)
{
    EFI_STATUS Status;
    Status = This->Close(This);
    if (EFI_ERROR(Status))
    {
        Print(L"Can't close %s\n\r", FileName);
        UEFI_PANIC;
    }
}