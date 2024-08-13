#ifndef __INCLUDE_FRAME_WORK_H__
#define __INCLUDE_FRAME_WORK_H__

#include <Uefi.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiTable.h>
#include <Guid/Acpi.h>

#include <Protocol/Rng.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/HiiImageDecoder.h>

#include <Guid/FileInfo.h>

#define UEFI_PANIC                          while(1)

// #define _DEBUG                              0x6964616B73 // Debug Magic Number

#endif
