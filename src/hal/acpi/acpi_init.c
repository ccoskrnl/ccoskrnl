#include "../../include/types.h"
#include "./sdt.h"
#include "../../include/libk/string.h"
#include "../../include/go/go.h"
#include "../../include/machine_info.h"
#include "../../include/arch/io.h"

extern uint64_t fixup_acpi_table_addr(uint64_t phys_addr);

static struct _RSDP* rsdp_ptr;
static struct _XSDT* xsdt_ptr;
static struct _FADT* fadt_ptr;
static struct _DSDT* dsdt_ptr;

static dword SMI_CMD;
static byte ACPI_ENABLE;
static byte ACPI_DISABLE;
static dword PM1a_CNT;
static dword PM1b_CNT;
static word SLP_TYPa;
static word SLP_TYPb;
static word SLP_EN;
static word SCI_EN;
static byte PM1_CNT_LEN;

static const char S5_SIGNATURE[] = {'_', 'S', '5', '_' };


//
// bytecode of the \_S5 object
// -----------------------------------------
//        | (optional) |    |    |    |   
// NameOP | \          | _  | S  | 5  | _
// 08     | 5A         | 5F | 53 | 35 | 5F
//
// -----------------------------------------------------------------------------------------------------------
//           |           |              | ( SLP_TYPa   ) | ( SLP_TYPb   ) | ( Reserved   ) | (Reserved    )
// PackageOP | PkgLength | NumElements  | byteprefix Num | byteprefix Num | byteprefix Num | byteprefix Num
// 12        | 0A        | 04           | 0A         05  | 0A          05 | 0A         05  | 0A         05
//
//----this-structure-was-also-seen----------------------
// PackageOP | PkgLength | NumElements |
// 12        | 06        | 04          | 00 00 00 00
//
// (Pkglength bit 6-7 encode additional PkgLength bytes [shouldn't be the case here])
//
// PkgLength was encoded 
void acpi_init()
{

    rsdp_ptr = (struct _RSDP*)fixup_acpi_table_addr((uint64_t)_current_machine_info->acpi_info.rsdp);
    xsdt_ptr = (struct _XSDT*)fixup_acpi_table_addr((uint64_t)_current_machine_info->acpi_info.xsdt);
    fadt_ptr = (struct _FADT*)fixup_acpi_table_addr((uint64_t)_current_machine_info->acpi_info.fadt);
    dsdt_ptr = (struct _DSDT*)fixup_acpi_table_addr((uint64_t)_current_machine_info->acpi_info.dsdt);

    // search the \_S5 package in the dsdt
    uint8_t* s5addr = (uint8_t*)((uint64_t)dsdt_ptr + 36);    // sizeof(ACPISDTHeader) == 36
    int dsdt_lenght = *(int*)((uint64_t)dsdt_ptr + 4);
    while (0 < dsdt_lenght--)
    {
        if (*(uint32_t*)s5addr == 0x5F35535f)
        {
            break;
        }
        s5addr++;
        
    }
    if (dsdt_lenght > 0)
    {

        if ((*(s5addr - 1) == 0x08) || (*(s5addr - 2) == 0x08 && *(s5addr - 1) == '\\') && *(s5addr+4) == 0x12)
        {
            s5addr += 5;

            // PkgLength is encoded into a sequence of 1 to 4 bytes in the data 
            // stream, with the highest two bits in byte 0 indicating how many 
            // subsequent bytes are included in PkgLength encoding, and the 
            // remaining two bits used only in one byte encoding.
            s5addr += ((*s5addr & 0xC0) >> 6) + 2;      // calculate PkgLength size
                                                        // ignore PkgLength field and NumElement field

            if (*s5addr == 0x0A)
            {
                s5addr++;       // skip byteprefix
            }

            SLP_TYPa = *(s5addr) << 10;
            s5addr++;

            if (*s5addr == 0x0A)
            {
                s5addr++;
            }
            SLP_TYPb = *(s5addr) << 10;

            SMI_CMD = (dword)fadt_ptr->SMI_CommandPort;
            ACPI_ENABLE = fadt_ptr->AcpiEnable;
            ACPI_DISABLE = fadt_ptr->AcpiDisable;

            PM1a_CNT = fadt_ptr->PM1aControlBlock;
            PM1b_CNT = fadt_ptr->PM1bControlBlock;

            PM1_CNT_LEN = fadt_ptr->PM1ControlLength;

            SLP_EN = 1 << 13;
            SCI_EN = 1;

        }

        
    }
    else
    {
        puts(0, "[ERROR]::acpi_init()  _S5_ Object not found...");
    }
        
}

static int acpi_enable()
{
    // check if acpi is enabled
    if ((inw(PM1a_CNT ) & SCI_EN) == 0)
    {
        // check if acpi can be enabled
        if (SMI_CMD != 0 && ACPI_ENABLE != 0)
        {
            // send acpi enable command
            outb(SMI_CMD, ACPI_ENABLE);

            while(1)
            {
                if ((inw(PM1a_CNT) & SCI_EN) == 1)
                {
                    break;
                }
            }

            if (PM1b_CNT != 0)
            {
                while(1)
                {
                    if ((inw(PM1b_CNT) & SCI_EN) == 1)
                    {
                        break;
                    }
                }
            }
            
            
        }
        else
        {
            puts(0, "No known way to enable ACPI.\n");
            return -1;
        }

        
    }
    return 0;
    
}


void power_off()
{

    if (SCI_EN == 0)
    {
        return;
    }

    putsxs(0, "SMI_CMD\t", SMI_CMD, "\n");
    putsxs(0, "ACPI_ENABLE\t", ACPI_ENABLE, "\n");
    putsxs(0, "ACPI_DISABLE\t", ACPI_DISABLE, "\n");
    putsxs(0, "PM1a_CNT\t", PM1a_CNT, "\n");
    putsxs(0, "PM1b_CNT\t", PM1b_CNT, "\n");
    putsxs(0, "SLP_TYPa\t", SLP_TYPa, "\n");
    putsxs(0, "SLP_TYPb\t", SLP_TYPb, "\n");
    putsxs(0, "SLP_EN\t", SLP_EN, "\n");
    putsxs(0, "SCI_EN\t", SCI_EN, "\n");
    putsxs(0, "PM1_CNT_LEN\t", PM1_CNT_LEN, "\n");

    acpi_enable();

    uint16_t current_pm1a_cnt = inw(PM1a_CNT);
    uint16_t current_slp_typa = (current_pm1a_cnt >> 10) & 0x7;

    if (current_slp_typa == SLP_TYPa)
    {
        puts(0, "The PM1a_CNT value is correct.\n");
    }
    else
    {
        puts(0, "The PM1a_CNT value is incorrect.");
        putsxs(0, "Expected SLP_TYPa: ", SLP_TYPa, NULL);
        putsxs(0, ", but got ", current_slp_typa, ".\n");
    }
    

    outw(PM1a_CNT, SLP_TYPa | SLP_EN);
    if (PM1b_CNT != 0)
    {
        outw(PM1b_CNT, SLP_TYPb | SLP_EN);
    }

    puts(0, "ACPI power off failed.\n");
    
}