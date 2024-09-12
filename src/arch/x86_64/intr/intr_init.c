#include "../../../include/types.h"
#include "../../../include/machine_info.h"
#include "../../../include/hal/acpi.h"
#include "../../../include/libk/list.h"
#include "../../../include/libk/string.h"
#include "../../../include/go/go.h"
#include "../../../include/libk/stdlib.h"

#include "../cpu/cpu_features.h"
#include "../cpu/cpu.h"
#include "../mm/mm_arch.h"
#include "../io/io.h"
#include "apic.h"
#include "hpet.h"
#include "pit.h"
#include "isr/intr_isr.h"
#include "../lib/lib.h"

// #define INTR_DEBUG





extern uint64_t fixup_acpi_table_addr(uint64_t phys_addr);
extern void keyboard_isr_wrapper();

// Bootstrap Processor descriptor
extern cpu_core_desc_t bsp;

static struct _MADT *madt_ptr;

// HPET
volatile uint64_t hpet_addr;
hpet_table_t *hpet_table;


/**
 * Every structure declare the interrupt features of the machine.
 * This structure is a list to collect all structures that have different types.
 **/
intr_ctr_struct_head_t intr_ctr_structs[MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES];

extern lapic_ver_t lapic_ver;



static void parse_madt(_in_ struct _MADT *madt_ptr, _in_ _out_ intr_ctr_struct_head_t* intr_ctr_structs)
{
    uint8_t *madt_end = (uint8_t *)madt_ptr + madt_ptr->h.Length;
    uint8_t *madt_structures = (uint8_t *)madt_ptr + sizeof(*madt_ptr);

    InterruptControllerStructureType type;
    uint8_t size;

    // initialize all list heads
    for (uint8_t i = 0; i < MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES; i++)
    {
        list_node_t *head = &(intr_ctr_structs + i)->head;
        (intr_ctr_structs + i)->total = 0;
        _list_init(head);
    }

    // walk madt, add to corresponding list based on it's type value.
    while (madt_structures < madt_end)
    {
        type = *madt_structures;
        size = *(madt_structures + 1);

        switch (type)
        {

        case ProcessorLocalAPIC:
        {
            processor_local_apic_t *lapic =
                (processor_local_apic_t *)malloc(sizeof(processor_local_apic_t));
            memcpyb(lapic, madt_structures, size);

            (intr_ctr_structs + type)->total++;
            _list_push(
                &(intr_ctr_structs + type)->head,
                (list_node_t *)((uint8_t *)lapic + sizeof(processor_local_apic_t) - sizeof(list_node_t)));
        }
        break;

        case IOAPIC:
        {
            io_apic_t *ioapic =
                (io_apic_t *)malloc(sizeof(io_apic_t));
            memcpyb(ioapic, madt_structures, size);

            (intr_ctr_structs + type)->total++;
            _list_push(
                &(intr_ctr_structs + type)->head,
                (list_node_t *)((uint8_t *)ioapic + sizeof(io_apic_t) - sizeof(list_node_t)));
        }
        break;

        case InterruptSourceOverride:
        {
            iso_t *iso =
                (iso_t *)malloc(sizeof(iso_t));
            memcpyb(iso, madt_structures, size);

            (intr_ctr_structs + type)->total++;
            _list_push(
                &(intr_ctr_structs + type)->head,
                (list_node_t *)((uint8_t *)iso + sizeof(iso_t) - sizeof(list_node_t)));
        }
        break;

        case NonMaskableInterruptSource:
        {
            nmi_t *nmi =
                (nmi_t *)malloc(sizeof(nmi_t));
            memcpyb(nmi, madt_structures, size);

            (intr_ctr_structs + type)->total++;
            _list_push(
                &(intr_ctr_structs + type)->head,
                (list_node_t *)((uint8_t *)nmi + sizeof(nmi_t) - sizeof(list_node_t)));
        }
        break;

        case LocalAPICNMI:
        {
            local_apic_addr_override_t *lapic_addr_or =
                (local_apic_addr_override_t *)malloc(sizeof(local_apic_addr_override_t));
            memcpyb(lapic_addr_or, madt_structures, size);

            (intr_ctr_structs + type)->total++;
            _list_push(
                &(intr_ctr_structs + type)->head,
                (list_node_t *)((uint8_t *)lapic_addr_or +
                              sizeof(local_apic_addr_override_t) - sizeof(list_node_t)));
        }
        break;

        default:
            break;
        }

        madt_structures += size;
    }


#ifdef INTR_DEBUG
    // walk through list
    list_node_t *head;
    list_node_t *intr_ctr_struct_node;
    if ((intr_ctr_structs + ProcessorLocalAPIC)->total != 0)
    {
        puts(0, "Processor Local APIC: \n");
        head = &(intr_ctr_structs + ProcessorLocalAPIC)->head;
        processor_local_apic_t *lapic = struct_base(processor_local_apic_t, node, head->flink);
        intr_ctr_struct_node = head->flink;

        for (; intr_ctr_struct_node != 0; intr_ctr_struct_node = intr_ctr_struct_node->flink)
        {
            putsds(0, "Local APIC ID: ", lapic->apic_id, "\n");
            lapic = struct_base(processor_local_apic_t, node, lapic->node.flink);
        }
    }


    if ((intr_ctr_structs + IOAPIC)->total != 0)
    {
        puts(0, "I/O APIC: \n");
        head = &(intr_ctr_structs + IOAPIC)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + IOAPIC)->total; i++, intr_ctr_struct_node = intr_ctr_struct_node->flink)
        {
            io_apic_t *ioapic = struct_base(io_apic_t, node, intr_ctr_struct_node);
            putsds(0, "IO APIC ID: ", ioapic->io_apic_id, "\n");
            putsxs(0, "IO APIC Address: ", ioapic->ioapic_addr, "\n");
            putsxs(0, "Global System Interrupt Base: ", ioapic->global_sys_interrupt_base, "\n");
        }
    }

    if ((intr_ctr_structs + InterruptSourceOverride)->total != 0)
    {
        puts(0, "Interrupt Source Override: \n");
        head = &(intr_ctr_structs+InterruptSourceOverride)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + InterruptSourceOverride)->total; i++, intr_ctr_struct_node = intr_ctr_struct_node->flink)
        {
            iso_t *iso = struct_base(iso_t, node, intr_ctr_struct_node);
            putsds(0, "Bus: ", iso->bus, "\n");
            putsds(0, "Bus-relative interrupt source(IRQ): ", iso->source, "\n");
            putsds(0, "Global System Interrupt: ", iso->global_sys_interrupt, "\n");
            putsxs(0, "Flags: ", iso->flags, "\n");
        }
    }

    if ((intr_ctr_structs + NonMaskableInterruptSource)->total != 0)
    {
        puts(0, "Non-maskable Interrupt Source: \n");
        head = &(intr_ctr_structs+NonMaskableInterruptSource)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + NonMaskableInterruptSource)->total; i++, intr_ctr_struct_node = intr_ctr_struct_node->flink)
        {
            nmi_t *nmi = struct_base(nmi_t, node, intr_ctr_struct_node);
            putsds(0, "Processor ID: ", nmi->acpi_processor_id, "\n");
            putsxs(0, "Flags: ", nmi->flags, "\n");
            putsds(0, "Local LINTn pin number: ", nmi->lint, "\n");
        }
    }

    if ((intr_ctr_structs + LocalAPICAddressOverride)->total != 0)
    {
        puts(0, "Local APIC Address Override: \n");
        head = &(intr_ctr_structs + LocalAPICAddressOverride)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + LocalAPICAddressOverride)->total; i++, intr_ctr_struct_node = intr_ctr_struct_node->flink)
        {
            local_apic_addr_override_t *local_apic_addr_override = struct_base(local_apic_addr_override_t, node, intr_ctr_struct_node);
            putsxs(0, "Local APIC Address: ", local_apic_addr_override->local_apic_addr, "\n");
        }
    }

#endif

}


void init_hpet()
{
    // Check if HPET is exist. Whether it exists or not, we also don't use. {:?}
    if (_current_machine_info->acpi_info.hpet != 0)
        hpet_table = (hpet_table_t *)fixup_acpi_table_addr(
            (uint64_t)_current_machine_info->acpi_info.hpet);

}

void enable_apic()
{
    uint64_t value;
    uint64_t lapic_reg;

    value = rdmsr(IA32_APIC_BASE_MSR);
    value |= 1 << 8;
    wrmsr(IA32_APIC_BASE_MSR, value);

    // APIC Software Enable/Disable in 8th bit.
    lapic_reg = 0xFF | (1 << 8);
    write_lapic_register(LOCAL_APIC_SPURIOUS_INTERRUPT_VEC_REG, lapic_reg);

}

// The start address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_start;
// The physical start address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_phys_start;

extern uint64_t _mm_get_sys_pte_next_page();

void mapping_startup_routine(uint64_t startup_addr)
{
    uint64_t new_page;
    uint64_t vpage = startup_addr >> PAGE_SHIFT;
    mmpte_hardware_t *startup_pml4_entry;
    mmpte_hardware_t *startup_pdpt;
    mmpte_hardware_t *startup_pd;
    mmpte_hardware_t *startup_pt;

    new_page = _mm_get_sys_pte_next_page();
    startup_pdpt = (mmpte_hardware_t*)(new_page + _mm_sys_pte_pool_start);

    startup_pml4_entry = (mmpte_hardware_t*)(_mm_pt_pml4_start);
    startup_pml4_entry->present = 1;
    startup_pml4_entry->rw = 1;
    startup_pml4_entry->address = ((uint64_t)(new_page + _mm_sys_pte_pool_phys_start)) >> PAGE_SHIFT;

    new_page = _mm_get_sys_pte_next_page();
    startup_pd = (mmpte_hardware_t*)(new_page + _mm_sys_pte_pool_start);

    startup_pdpt[0].present = 1;
    startup_pdpt[0].rw = 1;
    startup_pdpt[0].address = ((uint64_t)(new_page + _mm_sys_pte_pool_phys_start)) >> PAGE_SHIFT;

    new_page = _mm_get_sys_pte_next_page();
    startup_pt = (mmpte_hardware_t*)(new_page + _mm_sys_pte_pool_start);

    startup_pd[0].present = 1;
    startup_pd[0].rw = 1;
    startup_pd[0].address = ((uint64_t)(new_page + _mm_sys_pte_pool_phys_start)) >> PAGE_SHIFT;

    startup_pt[vpage].present = 1;
    startup_pt[vpage].rw = 1;
    startup_pt[vpage].address = vpage;

    _mm_flush_tlb(startup_addr);

    struct _startup_routine_data* startup_data = (struct _startup_routine_data*)
        (startup_addr + STARTUP_STACK_TOP + STARTUP_PM_GDT_SIZE + STARTUP_LM_GDT_SIZE);

    uint64_t cr3, cr0, cr4, xcr0;
    __get_cr3(cr3); 
    __get_cr0(cr0);
    __get_cr0(cr3);
    xcr0 = read_xcr0();

    startup_data->cr3 = cr3;
    startup_data->cr0 = cr0;
    startup_data->cr4 = cr4;
    startup_data->xcr0 = xcr0;

    startup_data->number_of_running_cores = 1;
    
}


/**
 * Application Processor Startup universal algorithm from MPspec B.4.
 **/

void send_sipi(uint8_t apic_id, uint8_t vector)
{
    lapic_icr_t icr = { 0 };
    uint32_t low, high;
    uint8_t retry = 3;

    icr.dest_field = apic_id;
    icr.dest_mode = LOCAL_APIC_ICR_DEST_MODE_PHYS;
    // Send INIT IPI
    icr.delivery_mode = LOCAL_APIC_ICR_DELIVERY_MODE_INIT;
    // For the INIT level de-assert delivery mode this flag must be set to 0;
    icr.level = 0;

    // The trigger mode is edge-triggered, and the Vector field must =00h
    icr.trigger_mode = LOCAL_APIC_ICR_TRIGGER_MODE_EDGE;


    // write to lapic_reg
    low = *(uint32_t*)(&icr);
    high = *(uint32_t*)((uintptr_t)&icr + sizeof(uint32_t));

    write_lapic_register(LOCAL_APIC_ICR_HIGH, high);
    write_lapic_register(LOCAL_APIC_ICR_LOW, low);

    // delay
    // 10 millisecond
    pit_prepare_sleep(10000);
    pit_perform_sleep();

    while (retry--) {

        if (_lapic_check_delivery_status())
        {
            break;
        }

        write_lapic_register(LOCAL_APIC_ICR_LOW, low);

        // delay
        // 10 millisecond
        pit_prepare_sleep(10000);
        pit_perform_sleep();
    }
    retry = 3;

    // set startup routine and sipi
    icr.vector = vector;
    icr.level = 1;
    icr.delivery_mode = LOCAL_APIC_ICR_DELIVERY_MODE_STARTUP;
    low = *(uint32_t*)(&icr);

    if (!_lapic_is_82489DX(lapic_ver.version)) 
    {
        // Send Startup IPI
        write_lapic_register(LOCAL_APIC_ICR_LOW, low);
        // 1000 us
        udelay(1000);

        write_lapic_register(LOCAL_APIC_ICR_LOW, low);
    
    }


}

/**
 * Activate application processors. 
**/
void activate_aps()
{

    // walk through list
    list_node_t *list_head;
    list_node_t *intr_ctr_struct_node;
    processor_local_apic_t *lapic;

    // Get address of ap startup routine.
    uintptr_t ap_entry = (uintptr_t)_current_machine_info->memory_space_info[3].base_address;

    list_head = &(intr_ctr_structs + ProcessorLocalAPIC)->head;
    intr_ctr_struct_node = list_head->flink;

    for (; intr_ctr_struct_node != 0; intr_ctr_struct_node = intr_ctr_struct_node->flink)
    {
        lapic = struct_base(processor_local_apic_t, node, intr_ctr_struct_node);
        // The bsp's apic id is always 0. we can ignore it.
        if (lapic->apic_id == bsp.lapic_id)
            continue;

        send_sipi(lapic->apic_id, (uint8_t)(ap_entry >> PAGE_SHIFT));
    }

}


void intr_init()
{

    // Fix up the MADT address
    madt_ptr = (struct _MADT *)fixup_acpi_table_addr(
        (uint64_t)_current_machine_info->acpi_info.madt);


    // Parse madt and push structures to corresponding lists based on theirs type.
    parse_madt(madt_ptr, intr_ctr_structs);


    // Map local apic address and IOAPIC address. We don't handle mutilprocessor system.
    map_lapic_and_ioapic(
        madt_ptr->local_apic_addr, 
        &intr_ctr_structs[IOAPIC]);


    // Because we use the processor local APIC and the I/O APIC,
    // so we must first disable the PIC.
    // This is done by masking every single interrupt.
    outb(0x21, 0xFF); // Mask all interrupts on PIC1
    outb(0xA1, 0xFF); // Mask all interrupts on PIC2


    // Now, we can operate APIC registers.
    // First things first, we need to enable local apic.
    enable_apic();


    /**
     * The routines installs timer interrupt on IDT[0x20].
     * By default, We use apic timer to emit timer interrupt.
     **/
    enable_timer_intr();


    /**
     * Enable Keyboard Interrupt.
     **/

    ioapic_rte_t kbd_rte = { 0 }; 
    kbd_rte.intr_vector = 0x21;
    kbd_rte.dest_field = 0;

    uint32_t low = *(uint32_t*)&kbd_rte;
    uint32_t high = *(uint32_t*)((uint64_t)&kbd_rte + 4);

    write_ioapic_register(ioapics[0], IOAPIC_RET_ENTRY_OFFSET(IRQ_KEYBOARD), low);
    write_ioapic_register(ioapics[0], IOAPIC_RET_ENTRY_OFFSET(IRQ_KEYBOARD) + 1, high);

    _cpu_install_isr(&bsp, 0x21, keyboard_isr_wrapper, IDT_DESC_TYPE_INTERRUPT_GATE, 0);


    /**
     * Activate application processors. 
     **/
    mapping_startup_routine(_current_machine_info->memory_space_info[3].base_address);
    // activate_aps(); 

}
