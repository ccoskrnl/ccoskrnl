#include "../../../include/types.h"
#include "../../../include/machine_info.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/go/go.h"

#include "cpu.h"
#include "cpu_features.h"
#include "../mm/mm_arch.h"
#include "../mm/mm_pool.h"

// references from ../intr/intr_handlers_entry.asm
// which stored all Protected-Mode Exceptions and Interrupts Handlers.
extern handler_t _intr_handler_entry_table[0x20];

// All cores only use the same GDT and IDT.
seg_desc_t gdt[SEG_DESC_MAXIMUM] = { 0 };
idt_desc_t idt[IDT_DESC_MAXIMUM] = { 0 };
tss_t tss0 = { 0 };

// The Bootstrap Processor Core descriptor.
cpu_core_desc_t bsp = { 0 };

// CPU Vendor.
char cpu_vendor_id[16] = { 0 };

// Current Machine cpu brand.
char cpu_brand_string[49] = { 0 };  // 48 characters + null terminator

// rdrand instruction to generate a random number.
boolean support_rdrand;

// AVX2. Supports Intel® Advanced Vector Extensions 2 (Intel® AVX2) if 1.
boolean support_avx2;
// AVX instruction support
boolean support_avx;
// XSAVE (and related) instructions are enabled.
boolean support_osxsave;
// XSAVE (and related) instructions are supported by hardware. .
boolean support_xsave;
// AES instruction support.
boolean support_aes;
// SSE4.2 instruction support.
boolean support_sse42;
// SSE4.1 instruction support. 
boolean support_sse41;
// CMPXCHG16B instruction
boolean support_cmpxchg16b;
// FMA instruction support.
boolean support_fma;
// supplemental SSE3 instruction support.
boolean support_ssse3;
// SSE3 instruction support.
boolean support_sse3;




/**
 * @brief  
 * @note   
 * 
 *      MP initialization protocol defines two classes of processors: the bootstrap processor (BSP) and the application 
 * processors (APs). Following a power-up or RESET of an MP system, system hardware dynamically selects one of 
 * the processors on the system bus as the BSP. The remaining processors are designated as APs.
 *      As part of the BSP selection mechanism, the BSP flag is set in the IA32_APIC_BASE MSR (see Figure 11-5) of the 
 * BSP, indicating that it is the BSP. This flag is cleared for all other processors.
 */ 
void preparing_for_bsp(boolean is_first)
{
    if (is_first)
    {
        // initialize CPU0's GDT
        // Because of we already defined GDT during OS loading, so we just need to copy defined gdt
        // into new gdt space.
        memcpyb(
            gdt, 
            (void*)(_current_machine_info->memory_space_info[1].base_address + MACHINE_INFO_SIZE + 0x2000), 
            sizeof(seg_desc_t) * 3);

        bsp.gdt = gdt;
        bsp.gdtr.limit = (sizeof(seg_desc_t) * SEG_DESC_MAXIMUM) - 1;
        bsp.gdtr.base = (uint64_t)(bsp.gdt);

        bsp.idt = idt;
        bsp.idtr.limit = (sizeof(idt_desc_t) * IDT_DESC_MAXIMUM) - 1;
        bsp.idtr.base = (uint64_t)(bsp.idt);

        __set_gdtr(&bsp.gdtr);
        __set_idtr(&bsp.idtr);
        _cpu_reload_seg_regs(SEG_SEL_KRNL_CODE, SEG_SEL_KRNL_DATA);

        if (support_xsave)
        {
            // Set cr4_t
            uint64_t cr4 = 0;
            __get_cr4(cr4);
            // Enable XSAVE and AVX in cr4_t
            cr4 |= (1 << 18);
            __write_cr4(cr4);

            // Set AVX state and SSE state
            if (support_avx2)
            {
                uint64_t xcr0 = read_xcr0();

                // Note that bit 0 of XCR0 (corresponding to x87 state) must be set to 1; 
                // the instruction will cause a #GP(0) if an attempt is made to clear this 
                // bit. In addition, the instruction causes a #GP(0) if an attempt is made 
                // to set XCR0[2] (AVX state) while clearing XCR0[1] (SSE state); it is 
                // necessary to set both bits to use AVX instructions; 
                xcr0 |= ((1 << 2) | (1 << 1));
                write_xcr0(xcr0);
            }
            
        }
    }

    // If it's not the first time calling the funciton, we assume that kernel basic components has already
    // been initialized. It enables we can further initialize bsp.
    else
    {
        // we only defined kernel code seg and kernel data seg during os loading. but now we need to
        // define TSS seg for further initializing.

        // Set TSS descriptor.
        bsp.gdt[3].type = SEG_DESC_TYPE_64_TSS;
        bsp.gdt[3].limit0 = (sizeof(tss_t) - 1) & 0xFFFF;
        bsp.gdt[3].limit1 = ((sizeof(tss_t) - 1) >> 16) & 0xFFFF;
        bsp.gdt[3].base0 = (uint64_t)(&tss0) & 0xFFFF;
        bsp.gdt[3].base1 = ((uint64_t)(&tss0) >> 16) & 0xFF;
        bsp.gdt[3].base2 = ((uint64_t)(&tss0) >> 24) & 0xFF;
        bsp.gdt[3].dpl = 0;
        bsp.gdt[3].p = 1;
        *(uint32_t*)(&bsp.gdt[4]) = ((uint64_t)(&tss0) >> 32) & 0xFFFFFFFF;

        void* rsp0;
        void* ist0;
        _mm_alloc_pages(0x10, &rsp0);
        _mm_alloc_pages(0x10, &ist0);
        tss0.ist[0] = (uint64_t)ist0 + (0x10 << PAGE_SHIFT);
        tss0.rsp[0] = (uint64_t)rsp0 + (0x10 << PAGE_SHIFT);
        tss0.iomap_base = sizeof(tss_t);


        // TSS descriptor is at index 3 in the GDT
        __load_tr(3 << 3);

        // Install all PM exceptions and interrupts handlers.
        for (int i = 0; i < IDT_RESERVED_ENTRY; i++) 
        {
            _cpu_install_isr(&bsp, i, _intr_handler_entry_table[i], IDT_DESC_TYPE_INTERRUPT_GATE, 0); 
        }

    }
    

    
}


void detecting_cpu()
{
    uint64_t value;
    uint32_t eax, ebx, ecx, edx;
    char* processor_name = cpu_brand_string;

    // Get CPU Vendor
    cpuid(CPUID_LEFT_LARGEST_STD_FUNC_NUM, &eax, &ebx, &ecx, &edx);
    {
        char uc;
        int i = 3;
        do
        {
            uc = (char)(ebx >> (i * 8));
            cpu_vendor_id[i] = uc;
            
        } while (i-- >= 0);
        i = 7;
        do
        {
            uc = (char)(edx >> (i * 8));
            cpu_vendor_id[i] = uc;
            
        } while (i-- > 4);
        i = 11;
        do
        {
            uc = (char)(ecx >> (i * 8));
            cpu_vendor_id[i] = uc;
            
        } while (i-- > 8);

    }
    
    // Get CPU Brand String
    for (size_t i = 0; i < 3; i++)
    {
        cpuid(CPUID_LEFT_PROCESSOR_BRAND + i, &eax, &ebx, &ecx, &edx);
        *(uint32_t*)processor_name = eax;
        processor_name += 4;
        *(uint32_t*)processor_name = ebx;
        processor_name += 4;
        *(uint32_t*)processor_name = ecx;
        processor_name += 4;
        *(uint32_t*)processor_name = edx;
        processor_name += 4;
    }
    

    // Miscellaneous Feature Identifiers
    cpuid(CPUID_LEFT_FEATURE_IDENTIFIERS,
        &eax, &ebx, &ecx, &edx);
    support_avx = (boolean)((ecx >> 28) & 1); 
    support_osxsave = (boolean)((ecx >> 27) & 1);
    support_xsave = (boolean)((ecx >> 26) & 1);
    support_aes = (boolean)((ecx >> 25) & 1);
    support_sse42 = (boolean)((ecx >> 20) & 1);
    support_sse41 = (boolean)((ecx >> 19) & 1);
    support_cmpxchg16b = (boolean)((ecx >> 13) & 1);
    support_fma = (boolean)((ecx >> 12) & 1);
    support_ssse3 = (boolean)((ecx >> 9) & 1);
    support_sse3 = (boolean)((ecx >> 0) & 1);
    support_rdrand = (boolean)((ecx >> 30) & 1);

    // Structured Extended Feature Flags Enumeration
    cpuid_ex(CPUID_LEFT_STRUCTURED_EX_FEATURE_FLAGS, 0,
        &eax, &ebx, &ecx, &edx);
    support_avx2 = (boolean)((ebx >> 5) & 1);

}


void _cpu_install_isr(cpu_core_desc_t *cpu, uint8_t vector, void* routine, uint8_t type, uint8_t ist_index)
{
    cpu->idt[vector].present = 1; 
    cpu->idt[vector].dpl = 0;
    cpu->idt[vector].ist = ist_index;
    cpu->idt[vector].offset0 = ((uint64_t)routine & 0xFFFF);
    cpu->idt[vector].selector = SEG_SEL_KRNL_CODE;
    cpu->idt[vector].offset1 = ((uint64_t)routine >> 16) & 0xFFFF;
    cpu->idt[vector].offset2 = ((uint64_t)routine >> 32) & 0xFFFFFFFF;
    cpu->idt[vector].type = type;
}
