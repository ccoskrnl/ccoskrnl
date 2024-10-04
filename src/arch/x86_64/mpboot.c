#include "../../include/types.h"
#include "../../include/libk/stdlib.h"
#include "../../include/machine_info.h"

#include "../../include/go/window.h"
#include "../../include/go/go.h"

#include "./cpu/cpu_features.h"
#include "./cpu/mtrr.h"
#include "./intr/apic.h"
#include "./lib/lib.h"
#include "./io/io.h"
#include "cpu/cpu.h"

// Bootstrap Processor descriptor
extern cpu_core_desc_t bsp;

extern void op_init_for_ap(int lapic_id);

/*

Routine Description:

    The routine fixes the all instructions that rely on relocation. We can't assume
that the UEFI-boot always placed our startup routine at stationary address. In 
default case, we use 0x1000 as the entrypoint address of our startup routine. 
Different machine might have different memory layouts. Since the address
0x1000 may was reserved for the other machine, so we need to fix manually these
address references.

Parameters:

    startip - The entrypoint of application processors.

    ap_entry - Then entrypoint of application processors after they have been
initializated.

Return Value:

    None.

*/
void fixup_reloc_of_entry(ptr_t startip, ptr_t ap_entry)
{
    uint32_t ebx = startip;
    uint32_t* mov_ebx_ip = (uint32_t*)(0x11 + startip);
    *mov_ebx_ip = ebx;

    // fixup gdtr
    uint32_t gdt_addr = (0x100 + startip);
    uint32_t* gdtr_base = (uint32_t*)(0xf2 + startip);
    *gdtr_base = gdt_addr;

    // fixup lgdt
    uint32_t gdtr_addr = (0xf0 + startip);
    // mov eax, gdtr_addr
    uint32_t* lgdt = (uint32_t*)(0x3e + startip);
    *lgdt = gdtr_addr;

    // fixup idtr
    uint32_t idtr_addr = (0x120 + startip);
    uint32_t* lidt = (uint32_t*)(0x49 + startip);
    *lidt = idtr_addr;

    uint32_t pm_entry = 0x64 + startip; 
    uint32_t* jmp2pm = (uint32_t*)(0x5e + startip);
    *jmp2pm = pm_entry;

    uint32_t lm_entry = 0xb8 + startip;
    uint32_t* jmp2lm = (uint32_t*)(0xb2 + startip);
    *jmp2lm = lm_entry;

    uint64_t* jmp2entry = (uint64_t*)(0xdf + startip);
    *jmp2entry = ap_entry;
}

void save_cpu_info_at_shared_data_area(ptr_t startip)
{
    uint64_t *mtrrs = (uint64_t*)(startip + 0x800);
    pseudo_desc_t *lm_gdt = (pseudo_desc_t*)(startip + 0x900);
    uint64_t *cpu_info = (uint64_t*)(startip + 0xA00);

    uint64_t cr3;
    uint64_t* fix_reg_ptr;
    uint64_t* phys_reg_ptr;

    fix_reg_ptr = (uint64_t*)mtrr_state.fixed_range;
    phys_reg_ptr = (uint64_t*)mtrr_state.phys_desc;

    memcpyb(lm_gdt, (void*)&bsp.gdtr, sizeof(pseudo_desc_t));
    lm_gdt->base = (startip + 0x910);
    memcpyb((void*)(startip + 0x910), (void*)bsp.gdt, sizeof(seg_desc_t) * SEG_DESC_MAXIMUM);

    __get_cr3(cr3);

    // cpu_info[0] = cr3;
    *cpu_info++ = cr3;
    // cpu_info[1] = number of running processors;
    *cpu_info++ = 1;

    if (cpu_feature_support(X86_FEATURE_MTRR))
    {
        if (mtrr_state.fix) 
        {
            *mtrrs++ = fix_reg_ptr[0];
            for (int i = 0; i < nr_fix16k; i++)
                *mtrrs++ = fix_reg_ptr[1 + i];

            for (int i = 0; i < nr_fix4k; i++)
                *mtrrs++ = fix_reg_ptr[3 + i];
        }
        for (int i = 0; i < mtrr_state.vcnt; i++) 
        {
            *mtrrs++ = phys_reg_ptr[i];
        }
    }
}

/*
 * Wake up AP by INIT, INIT, STARTUP sequence.
 */
static void send_init_sequence(int lapic_id)
{
    lapic_icr_t icr = { 0 };
    uint32_t low, high;
    int retry = 3;

    if (lapic_ver.maxlvt > 3)
    {
        write_lapic_register(LOCAL_APIC_ERROR_STATUS_REG, 0);
    }
    read_lapic_register(LOCAL_APIC_ERROR_STATUS_REG);


    icr.dest_field = lapic_id;
    icr.dest_mode = LOCAL_APIC_ICR_DEST_MODE_PHYS;
    // Send INIT IPI
    icr.delivery_mode = LOCAL_APIC_ICR_DELIVERY_MODE_INIT;
    // For the INIT level de-assert delivery mode this flag must be set to 0;
    icr.level = LOCAL_APIC_ICR_LEVEL_ASSERT;
    // The trigger mode is edge-triggered, and the Vector field must =00h
    icr.trigger_mode = LOCAL_APIC_ICR_TRIGGER_MODE_EDGE;

    // write to lapic_reg
    low = *(uint32_t*)(&icr);
    high = *(uint32_t*)((uintptr_t)&icr + sizeof(uint32_t));

    write_lapic_register(LOCAL_APIC_ICR_HIGH, high);
    write_lapic_register(LOCAL_APIC_ICR_LOW, low);

    while (retry--) 
    {

        if (_lapic_check_delivery_status())
            break;

        write_lapic_register(LOCAL_APIC_ICR_HIGH, high);
        write_lapic_register(LOCAL_APIC_ICR_LOW, low);

        // delay
        udelay(100);
    }
    retry = 3;

    udelay(100);
    icr.level = LOCAL_APIC_ICR_LEVEL_DEASSERT;
    low = *(uint32_t*)(&icr);

    write_lapic_register(LOCAL_APIC_ICR_HIGH, high);
    write_lapic_register(LOCAL_APIC_ICR_LOW, low);
    
    while (retry--) 
    {

        if (_lapic_check_delivery_status())
            break;

        write_lapic_register(LOCAL_APIC_ICR_HIGH, high);
        write_lapic_register(LOCAL_APIC_ICR_LOW, low);

        // delay
        udelay(100);
    }
    retry = 3;

}

/*
 * Application Processor Startup universal algorithm from MPspec B.4.
 */
void boot_processor(int lapic_id, ptr_t startip)
{
    lapic_icr_t icr = { 0 };
    uint32_t low, high;
    int retry = 3;
    int send_status, accpet_status;

    outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
    outb(CMOS_PORT+1, 0x0A);

    send_init_sequence(lapic_id);

    // Set startup routine and sipi
    icr.dest_field = lapic_id;
    icr.dest_mode = LOCAL_APIC_ICR_DEST_MODE_PHYS;
    icr.vector = (startip >> 12);
    icr.level = LOCAL_APIC_ICR_LEVEL_ASSERT;
    icr.delivery_mode = LOCAL_APIC_ICR_DELIVERY_MODE_STARTUP;
    icr.trigger_mode = LOCAL_APIC_ICR_TRIGGER_MODE_EDGE;

    low = *(uint32_t*)(&icr);
    high = *(uint32_t*)((uintptr_t)&icr + sizeof(uint32_t));

    if (!_lapic_is_82489DX(lapic_ver.version)) 
    {
        for (int i = 0; i < 2; i++) 
        {
            if (lapic_ver.maxlvt > 3)
            {
                write_lapic_register(LOCAL_APIC_ERROR_STATUS_REG, 0);
            }
            read_lapic_register(LOCAL_APIC_ERROR_STATUS_REG);

            // Send Startup IPI
            write_lapic_register(LOCAL_APIC_ICR_HIGH, high);
            write_lapic_register(LOCAL_APIC_ICR_LOW, low);

            /*
             * @note There is a very strange question here.
             * If the delay time is very long, such as 100000, machine(I use QEMU) will
             * receive a exception or fault. Then the machine will restart. This question
             * have been bothering me for two weeks.  
             */
            udelay(1);

            while (retry--) 
            {

                send_status = _lapic_check_delivery_status();
                if (send_status)
                    break;

                // delay
                udelay(10);
            }
            retry = 3;

            /*
             * Give the other processor some time to accept the IPI.
             */
            udelay(10);

            if (lapic_ver.maxlvt > 3)
            {
                write_lapic_register(LOCAL_APIC_ERROR_STATUS_REG, 0);
            }
            accpet_status = (read_lapic_register(LOCAL_APIC_ERROR_STATUS_REG) & 0xEF);

            if (accpet_status || send_status) 
                break;
        }
    }

}

void ap_entry()
{
    uint64_t lapic_id = _lapic_get_apic_id();
    // cpu_core_desc_t *core_desc = ap_setting(lapic_id);

    // Set interrupt flag, os can handle interrupts or exceptions.
    // __asm__ ("sti"); 

    // op_init_for_ap(lapic_id);

    while (true) {
        asm("nop");
    }
    
}

void active_aps(void)
{

    save_cpu_info_at_shared_data_area(_current_machine_info->memory_space_info[3].base_address);
    fixup_reloc_of_entry(_current_machine_info->memory_space_info[3].base_address, (ptr_t)ap_entry);

    list_node_t *core_node = cpu_cores_list.flink;
    while (core_node != NULL) {

        cpu_core_desc_t* core = struct_base(cpu_core_desc_t, node, core_node);
        if (core->lapic_id != bsp.lapic_id) 
            boot_processor(core->lapic_id, _current_machine_info->memory_space_info[3].base_address);
    
        core_node = core_node->flink;
    }
    
}