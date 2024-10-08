#ifndef __X86_CPU_H__
#define __X86_CPU_H__

#include "../../../include/types.h"
#include "../../../include/libk/list.h"
#include "mtrr.h"




/*================ Global Descriptor Table Definition ================*/

#define SEG_DESC_TYPE_READ_WRITE            0x2
#define SEG_DESC_TYPE_EXECUTE_READ          0xA
#define SEG_DESC_MAXIMUM                    0x10
#define IDT_DESC_MAXIMUM                    256

#define SEG_DESC_TYPE_64_TSS                0x9
#define SEG_DESC_TYPE_64_TSS_BUSY           0xB

#define SEG_SEL_KRNL_CODE                   (1 << 3)
#define SEG_SEL_KRNL_DATA                   (2 << 3)

typedef struct _seg_desc{
    uint16_t limit0;                    // Segment limit 15:00
    uint16_t base0;                     // Base Address 15:00
    uint8_t base1;                      // Base Address 23:16
    uint16_t type : 4;                  // Segment Type
    uint16_t s : 1;                     // System Segment(S flag is clear) or code or data(S flag is set)
    uint16_t dpl : 2;                   // Descriptor Privilege level
    uint16_t p : 1;                     // Indicates whether the segment is present in memory(set) or not present(clear)
    uint16_t limit1 : 4;                // Segment limit 19::16
    uint16_t avl : 1;                   // available for use by system software
    uint16_t l : 1;                     // long mode

    // The flag is called the D flag and it indicates the default length for effective addresses 
    // and operands referenced by instructions in the segment.
    // The flag is called the B flag and it specifies the upper bound of the segment.
    uint16_t db : 1;

    //  Determines the scaling of the segment limit field. when flag is set, the segment limit is interpreted in 4-KByte units.
    uint16_t g : 1;
    uint8_t base2;                      // Base Address 31:24
}__attribute__ ((packed)) seg_desc_t; 

typedef struct _pseudo_desc
{
    uint16_t limit;
    uint64_t base;
} __attribute__ ((packed)) pseudo_desc_t;

void __set_gdtr(struct _pseudo_desc *gdtr);

void __read_gdtr(struct _pseudo_desc *gdtr);

void __set_ldtr(uint16_t selector);

uint16_t __read_ldtr();

// Function: reload_seg_regs - Reload segment registers
// Parameters:
//   code_seg_sel - Code segment selector
//   data_seg_sel - Data segment selector
void _cpu_reload_seg_regs(uint16_t code_seg_sel, uint16_t data_seg_sel);







/*================ Task Segment Definition ================*/

#define TSS_SIZE 104

// Define the TSS structure
typedef struct _tss {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

#define SEG_SEL_TSS                             (3 << 3)

void __load_tr(uint16_t selector);






/*================ Control Registers Definition ================*/

// Structure for cr0_t Control Register in x86
typedef struct _cr0 {
    uint64_t pe : 1;   // Protected Mode Enable
    uint64_t mp : 1;   // Monitor Coprocessor
    uint64_t em : 1;   // Emulation
    uint64_t ts : 1;   // Task Switched
    uint64_t et : 1;   // Extension Type
    uint64_t ne : 1;   // Numeric Error
    uint64_t reserved0 : 10; // Reserved bits
    uint64_t wp : 1;   // Write Protect
    uint64_t reserved1 : 1;  // Reserved bit
    uint64_t am : 1;   // Alignment Mask
    uint64_t reserved2 : 10; // Reserved bits
    uint64_t nw : 1;   // Not Write-through
    uint64_t cd : 1;   // Cache Disable
    uint64_t pg : 1;   // Paging
    uint64_t reserved3 : 32; // Reserved bits
} __attribute__ ((packed)) cr0_t;

// Structure for cr3_t - Control Register 3 in x86
typedef struct _cr3 {
    uint64_t ignored1 : 3;       // Ignored bits
    uint64_t pwt : 1;            // Page-level Write-Through
    uint64_t pcd : 1;            // Page-level Cache Disable
    uint64_t ignored2 : 7;       // Ignored bits
    uint64_t address : 40;       // Physical address of the page directory
    uint64_t reserved : 12;      // Reserved bits
} __attribute__ ((packed)) cr3_t;

// Structure for cr4_t Control Register in x86
typedef struct _cr4 {
    uint64_t vme : 1;      // Virtual-8086 Mode Extensions
    uint64_t pvi : 1;      // Protected-Mode Virtual Interrupts
    uint64_t tsd : 1;      // Time Stamp Disable
    uint64_t de : 1;       // Debugging Extensions
    uint64_t pse : 1;      // Page Size Extensions
    uint64_t pae : 1;      // Physical Address Extension
    uint64_t mce : 1;      // Machine Check Exception
    uint64_t pge : 1;      // Page Global Enable
    uint64_t pce : 1;      // Performance Monitoring Counter Enable
    uint64_t osfxsr : 1;   // OS Support for FXSAVE/FXRSTOR
    uint64_t osxmmexcpt : 1; // OS Support for Unmasked SIMD F.P. Exceptions
    uint64_t reserved0 : 2; // Reserved bits
    uint64_t vmxe : 1;     // Virtual Machine Extensions Enable
    uint64_t smxe : 1;     // Safer Mode Extensions Enable
    uint64_t reserved1 : 1; // Reserved bit
    uint64_t fsgsbase : 1; // Enable RDFSBASE/RDGSBASE/WRFSBASE/WRGSBASE
    uint64_t pcide : 1;    // PCID Enable
    uint64_t osxsave : 1;  // XSAVE and Processor Extended States Enable
    uint64_t kl : 1;        // Key-Locker-Enable Bit
    uint64_t smep : 1;     // Supervisor Mode Execution Protection Enable
    uint64_t smap : 1;     // Supervisor Mode Access Protection Enable
    uint64_t pke : 1;      // Protection Key Enable
    uint64_t reserved3 : 41; // Reserved bits
} __attribute__ ((packed)) cr4_t;

#define __get_cr0(cr0) \
    __asm__(    \
        "mov %%cr0, %0\n\t" \
        : "=r"(cr0) :: "memory" \
    )  \

#define __get_cr3(cr3) \
    __asm__(    \
        "mov %%cr3, %0\n\t" \
        : "=r"(cr3) :: "memory" \
    )  \

#define __get_cr4(cr4) \
    __asm__(    \
        "mov %%cr4, %0\n\t" \
        : "=r"(cr4) :: "memory" \
    )  \

#define __write_cr0(cr0) \
    __asm__(    \
        "mov %0, %%cr0\n\t" \
        : : "r"(cr0) : \
    )

#define __write_cr3(cr3) \
    __asm__(    \
        "mov %0, %%cr3\n\t" \
        : : "r"(cr3) : \
    )

#define __write_cr4(cr4) \
    __asm__(    \
        "mov %0, %%cr4\n\t" \
        : : "r"(cr4) : \
    )


uint64_t read_xcr0();
void write_xcr0(uint64_t value);








/*================ Interrupt Descriptor Table Definition ================*/

// Protected-Mode Exceptions and Interrupts were reserved by Intel.
#define IDT_RESERVED_ENTRY                              0x20
// 64-bit Call Gate
#define IDT_DESC_TYPE_CALL_GATE                         0xC
// 64-bit Interrupt Gate
#define IDT_DESC_TYPE_INTERRUPT_GATE                    0xE
// 64-bit Trap Gate
#define IDT_DESC_TYPE_TRAP_GATE                         0xF

// 64-Bit IDT Gate Descriptors
typedef struct _idt_desc {

    uint16_t offset0;               // Offset to procedure entry point
    uint16_t selector;              // Segment Selector for destination code segment 
    uint16_t ist : 3;               // Interrupt Stack Table
    uint16_t zero0 : 5;
    uint16_t type : 4;              // IDT descriptor type
    uint16_t zero1 : 1;             
    uint16_t dpl : 2;               // Descriptor Privilege Level
    uint16_t present : 1;           // Segment Present flag
    uint16_t offset1;
    uint32_t offset2;
    uint32_t reserved;

} __attribute__ ((packed)) idt_desc_t;

void __set_idtr(struct _pseudo_desc *idtr);
void __read_idtr(struct _pseudo_desc *idtr);






/*================ CPU Core Descriptor Definition ================*/

#define MAX_CPU_CORES                                                       128

#define CPU_CORE_TAG_BSP                                                    0x425350
#define CPU_CORE_AP_TAG(lapic_id)                                           (0x415000 | (uint8_t)lapic_id)

typedef struct _cpu_core_desc
{

    uint128_t core;
    seg_desc_t* gdt;
    pseudo_desc_t gdtr;
    idt_desc_t* idt;
    pseudo_desc_t idtr;
    uint32_t lapic_id;
    uint32_t processor_id;

    uint64_t tag;
    list_node_t node;

} cpu_core_desc_t;

extern list_node_t cpu_cores_list;
extern cpu_core_desc_t *cpu_cores[MAX_CPU_CORES];

cpu_core_desc_t* ap_setting(uint64_t lapic_id);
void _cpu_install_isr(cpu_core_desc_t *cpu, uint8_t vector, void* routine, uint8_t type, uint8_t ist_index);


#endif 
