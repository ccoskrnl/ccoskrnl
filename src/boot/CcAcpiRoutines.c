#include "include/FrameWork.h"
#include "include/Wrapper.h"
#include "include/AcpiSDT.h"
#include "include/MachineInfo.h"
#include <alloca.h>

// STATIC EFI_ACPI_SDT_PROTOCOL *gAcpiSdt;
extern EFI_SYSTEM_TABLE *gSystemTable;


STATIC UINT32 CompareGuid(CONST UINT64 *dst, CONST UINT64 *src)
{
  if (dst[0] == src[0] && dst[1] == src[1])
  {
    return 1;
  }
  return 0;
}

STATIC EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *ExamineEfiConfigurationTable()
{
  EFI_GUID AcpiTableGuid = EFI_ACPI_20_TABLE_GUID;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp;
  for (UINTN i = 0; i < gSystemTable->NumberOfTableEntries; i++)
  {
    EFI_CONFIGURATION_TABLE *ConfigurationTable = &gSystemTable->ConfigurationTable[i];
    // Check if the current entry is the extended system description pointer
    if (CompareGuid((UINT64 *)ConfigurationTable, (UINT64 *)&AcpiTableGuid))
    {
      // Found the extended system description pointer
      Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)ConfigurationTable->VendorTable;
      break;
    }
  }
  return Rsdp;
}

VOID FindAcpiTable(IN LOADER_MACHINE_INFORMATION *MachineInfo)
{
  MachineInfo->AcpiInformation.Rsdp = 0;
  MachineInfo->AcpiInformation.Xsdt = 0;
  MachineInfo->AcpiInformation.EntryCount = 0;
  MachineInfo->AcpiInformation.EntryPtr = 0;
  MachineInfo->AcpiInformation.Fadt = 0;
  MachineInfo->AcpiInformation.Dsdt = 0;
  MachineInfo->AcpiInformation.Madt = 0;
  MachineInfo->AcpiInformation.Hpet = 0;

  MachineInfo->AcpiInformation.Rsdp = ExamineEfiConfigurationTable();
  MachineInfo->AcpiInformation.Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)MachineInfo->AcpiInformation.Rsdp->XsdtAddress;
  MachineInfo->AcpiInformation.EntryCount = (MachineInfo->AcpiInformation.Xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) >> 3;
  MachineInfo->AcpiInformation.EntryPtr = (UINT64*)(MachineInfo->AcpiInformation.Xsdt + 1);

  // Find FADT
  for (UINTN i = 0; i < MachineInfo->AcpiInformation.EntryCount; i++)
  {
    EFI_ACPI_DESCRIPTION_HEADER *Header = (EFI_ACPI_DESCRIPTION_HEADER *)(*(MachineInfo->AcpiInformation.EntryPtr + i));
    if (Header->Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE)
    {
      MachineInfo->AcpiInformation.Fadt = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)Header;
      MachineInfo->AcpiInformation.Dsdt = MachineInfo->AcpiInformation.Fadt->Dsdt;
#ifdef _DEBUG
      Print(L"FADT table found!\n\r");
#endif
    }
    if (Header->Signature == EFI_ACPI_5_1_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE)
    {
      MachineInfo->AcpiInformation.Madt = Header;
#ifdef _DEBUG
      Print(L"MADT table found!\n\r");
#endif
    }
    if (Header->Signature == EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE)
    {
      MachineInfo->AcpiInformation.Hpet = Header;
#ifdef _DEBUG
      Print(L"HPET table found!\n\r");
#endif
    }
    if (Header->Signature == 0x4746434D)
    {
      MachineInfo->AcpiInformation.Mcfg = Header;
#ifdef _DEBUG
      Print(L"MCFG table found!\n\r");
#endif
    }

  }


#ifdef _DEBUG
  UINT8* Dsdt;
  Dsdt = (UINT8*) MachineInfo->AcpiInformation.Dsdt;
  Print(L"%c%c%c%c table found!\n\r", Dsdt[0],Dsdt[1],Dsdt[2], Dsdt[3]);
  Print(L"All tables have been found!\n\r");
#endif

}
