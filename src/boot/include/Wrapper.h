#ifndef __INCLUDE_WRAPPER_H__
#define __INCLUDE_WRAPPER_H__

#include "FrameWork.h"

VOID WpFileOpen(
    IN EFI_FILE_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN CHAR16 *FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes);

VOID WpLocateProtocol(
    IN EFI_GUID *Protocol,
    IN VOID *Registration OPTIONAL,
    OUT VOID **Interface,
    IN CHAR16 *ProtocolName);

VOID WpCloseProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    IN EFI_HANDLE AgentHandle,
    IN EFI_HANDLE ControllerHandle,
    IN CHAR16 *ProtocolName);

VOID WpFileClose(
    IN EFI_FILE_PROTOCOL *This,
    IN CHAR16 *FileName);

VOID WpFileRead(
  IN EFI_FILE_PROTOCOL        *This,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer,
  IN CHAR16                   *FileName
  );

#endif