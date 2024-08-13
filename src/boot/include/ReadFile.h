#ifndef _INCLUDE_READ_FILE_H__
#define _INCLUDE_READ_FILE_H__

#include "FrameWork.h"

VOID ReadFileInit();
VOID ReadFileFnit();

EFI_STATUS
ReadFileToBuffer(
    IN CHAR16 *FileName,
    IN OUT VOID **Buffer,
    IN OUT UINTN *BufferSize);

EFI_STATUS
ReadFileToBufferAt(
    IN CHAR16 *FileName,
    IN EFI_PHYSICAL_ADDRESS BufferAddress,
    IN OUT UINTN *BufferSize);

#endif