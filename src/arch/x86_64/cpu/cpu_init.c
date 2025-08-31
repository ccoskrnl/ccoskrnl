#include "../../../include/types.h"
#include "../../../include/machine_info.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/go/go.h"

#include "cpu.h"
#include "cpu_features.h"
#include "../mm/mm_arch.h"
#include "../mm/mm_pool.h"
#include "mtrr.h"

// references from ../intr/intr_handlers_entry.asm
// which stored all Protected-Mode Exceptions and Interrupts Handlers.
extern handler_t _intr_handler_entry_table[0x20];

// All cores only use the same GDT and IDT.
seg_desc_t gdt[SEG_DESC_MAXIMUM] = { 0 };
idt_desc_t idt[IDT_DESC_MAXIMUM] = { 0 };
tss_t tss0 = { 0 };

// The Bootstrap Processor Core descriptor.
cpu_core_desc_t bsp = { 0 };

list_node_t cpu_cores_list = { 0 };
cpu_core_desc_t *cpu_cores[MAX_CPU_CORES] = { 0 };

/**
 * @brief  
 * @note   
 * 
 *      MP initialization protocol defines two classes of processors: the bootstrap processor (BSP) and the application 
 * processors (APs). Following a power-up or RESET of an MP system, system hardware dynamically selects one of 
 * the processors on the system bus as the BSP. The remaining processors are designated as APs.
 *      As part of the BSP selection mechanism, the BSP flag is set in the IA32_APIC_BASE MSR (see Figure 11-5) of the 
 * BSP, indicating that it is the BSP. This flag is cleared for all other processors.
 * 
 * @param[in]   is_first    Indicates if this is the first time the function is called.
 */ 
void preparing_for_bsp(boolean is_first)
{
    if (is_first)
    {

        // Detect cpu features and set some global variables about cpu info.
        cpu_feature_detect();

        // initialize CPU0's GDT
        // Because of we already defined GDT during OS loading, so we just need to copy defined gdt
        // into new gdt space.
        memcpyb(
                gdt, 
                (void*)(_current_machine_info->memory_space_info[1].base_address + MACHINE_INFO_SIZE + CCLDR_ROUTINE_SIZE), 
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

        if (cpu_feature_support(X86_FEATURE_XSAVE))
        {
            // Set cr4_t
            uint64_t cr4 = 0;
            __get_cr4(cr4);
            // Enable XSAVE and AVX in cr4_t
            cr4 |= (1 << 18);
            __write_cr4(cr4);

            // Set AVX state and SSE state
            if (cpu_feature_support(X86_FEATURE_AVX))
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
        bsp.lapic_id = _cpuid_get_apic_id();
        cpu_cores[bsp.lapic_id] = &bsp;

        bsp.tag = CPU_CORE_TAG_BSP;
        cpu_cores_list.blink = &bsp.node;
        cpu_cores_list.flink = &bsp.node;

        mtrr_init();

    }



}

cpu_core_desc_t* ap_setting(uint64_t lapic_id)
{
    cpu_core_desc_t *cpu = cpu_cores[lapic_id];
    cpu->tag = CPU_CORE_AP_TAG(lapic_id);

    cpu->gdt = gdt;
    cpu->gdtr.limit = (sizeof(seg_desc_t) * SEG_DESC_MAXIMUM) - 1;
    cpu->gdtr.base = (uint64_t)(cpu->gdt);

    cpu->idt = idt;
    cpu->idtr.limit = (sizeof(idt_desc_t) * IDT_DESC_MAXIMUM) - 1;
    cpu->idtr.base = (uint64_t)(cpu->idt);

    __set_gdtr(&cpu->gdtr);
    __set_idtr(&cpu->idtr);
    _cpu_reload_seg_regs(SEG_SEL_KRNL_CODE, SEG_SEL_KRNL_DATA);

    if (cpu_feature_support(X86_FEATURE_XSAVE))
    {
        // Set cr4_t
        uint64_t cr4 = 0;
        __get_cr4(cr4);
        // Enable XSAVE and AVX in cr4_t
        cr4 |= (1 << 18);
        __write_cr4(cr4);

        // Set AVX state and SSE state
        if (cpu_feature_support(X86_FEATURE_AVX))
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


    // TSS descriptor is at index 3 in the GDT
    __load_tr(3 << 3);

    return cpu;
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
