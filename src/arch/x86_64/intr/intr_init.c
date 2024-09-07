#include "../../../include/types.h"
#include "../../../include/machine_info.h"
#include "../../../include/hal/acpi.h"
#include "../../../include/libk/list.h"
#include "../../../include/libk/string.h"
#include "../../../include/go/go.h"
#include "../../../include/libk/stdlib.h"

#include "../cpu/cpu_features.h"
#include "../cpu/cpu.h"
#include "../io/io.h"
#include "apic.h"
#include "hpet.h"
#include "pit.h"

// #define INTR_DEBUG


extern uint64_t fixup_acpi_table_addr(uint64_t phys_addr);

extern cpu_core_desc_t bsp;

static struct _MADT *madt_ptr;


/**
 * APIC-Timer-always-running feature
 *
 * The processor's APIC timer runs at a constant rate regardless of P-state
 * transitions and it continues to run at the same rate in deep C-state.
 * If the value is false, the APIC timer may temporarily stop while the
 * processor is in deep C-states or during transitions caused by Enhanced
 * Intel SpeedStep® Technology.
 * */
boolean support_apic_time_always_running;

// Processor Base Frequency (in MHz).
uint16_t processor_base_frequency_in_MHz;

// Maximum Frequency (in MHz).
uint16_t processor_maximum_frequency_in_MHz;

// The processor’s bus clock frequency refers to the speed at which the processor
// communicates with the memory and other components via the front-side bus (FSB)
// or other interconnects.
uint16_t processor_bus_clock_frequency_in_MHz;

// The core crystal clock frequency is a fixed frequency provided by a crystal
// oscillator on the motherboard. It serves as a stable reference clock for various
// components in the processor.
uint64_t core_crystal_clock_frequency;

// The core clock frequency is the operating frequency of the CPU cores.
// It determines how many cycles per second the CPU can execute instructions.
uint64_t core_clock_frequency;

// Max Performance Frequency Clock Count
uint64_t max_perf_freq_clock_count;
// Actual Performance Frequency Clock Count
uint64_t actual_perf_freq_clock_count;

uint64_t tsc_frequency;
boolean tsc_invariant;

volatile uint64_t hpet_addr;
hpet_table_t *hpet_table;
intr_ctr_struct_head_t intr_ctr_structs[MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES];



void keyboard_isr();

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

// void send_ipi(uint8_t apic_id, uint8_t vector)
// {
//     // ICR_HIGH: Set destination APIC ID.
//     write_apic_register(0x310, apic_id << 24);
//     // ICR_LOW: Send INIT IPI
//     write_apic_register(0x300, 0x00004500 | vector);
//
//     for (volatile int i = 0; i < 100000; i++); // Short delay
//     write_apic_register(0x300, 0x00004600 | vector); // ICR_LOW: Send STARTUP IPI
// }
//
//
// void ap_entry()
// {
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
//     asm("nop");
// }
// // Function to activate other CPU cores
// void activate_cores() {
//     // Assuming APIC IDs are 1, 2, 3, etc.
//     for (uint8_t apic_id = 1; apic_id < 4; apic_id++) {
//         send_ipi(apic_id, (uint8_t)((uint64_t)ap_entry >> 12));
//     }
// }

void intr_init()
{
    uint64_t value;
    uint32_t lapicreg;
    uint32_t eax, ebx, ecx, edx;
    uint32_t ticks_in_10ms;
    uint8_t divide_value = 0x3;

    // Fix up MADT address
    madt_ptr = (struct _MADT *)fixup_acpi_table_addr(
        (uint64_t)_current_machine_info->acpi_info.madt);

    // Parse madt and push structures to corresponding lists based on theirs type.
    parse_madt(madt_ptr, intr_ctr_structs);

    // Prepare the PIT to sleep for 10ms (10000µs)

    // Check if HPET is exist. Whether it exists or not, we also don't use.
    if (_current_machine_info->acpi_info.hpet != 0)
        hpet_table = (hpet_table_t *)fixup_acpi_table_addr(
            (uint64_t)_current_machine_info->acpi_info.hpet);

    // Get local apic address after it is mapped to virtual space.

    // Map local apic address and IOAPIC address. We don't handle mutilprocessor system.
    map_lapic_and_ioapic(
        madt_ptr->local_apic_addr, 
        &intr_ctr_structs[IOAPIC]);


    outb(0x21, 0xFF); // Mask all interrupts on PIC1
    outb(0xA1, 0xFF); // Mask all interrupts on PIC2


    // Now, we can operate APIC registers.
    // First things first, we need to enable local apic.
    value = rdmsr(IA32_APIC_BASE_MSR);
    value |= 1 << 8;
    wrmsr(IA32_APIC_BASE_MSR, value);

    /**
     * Sporious Interrupt
     *
     * A special situation may occur when a processor raises its task priority to
     * be greater than or equal to the level of the interrupt for which the
     * processor INTR signal is currently being asserted. If at the time the INTA
     * cycle is issued, the interrupt that was to be dispensed has become masked
     * (programmed by software), the local APIC will deliver a spurious-interrupt
     * vector. Dispensing the spurious-interrupt vector does not affect the ISR,
     * so the handler for this vector should return without an EOI.
     *
     * */
    lapicreg = 0xFF | (1 << 8);
    write_lapic_register(LOCAL_APIC_SPURIOUS_INTERRUPT_VEC_REG, lapicreg);

    /**
     * Configure I/O APIC.(Reference the I/O APIC specification.)
     *
     * The IOAPIC has a message unit for sending and receiving APIC messages over
     * the APIC bus. I/O devices inject interrupts into the system by asserting one
     * of the interrupt lines to the IOAPIC. The IOAPIC selects the corresponding
     * entry in the Redirection Table and uses the information in that entry to format
     * an interrupt request message.
     *
     */

    /**
     * Configure redirection table registers.
     *
     * There are 24 I/O Redirection table entry registers.
     *
     */

    /**
     * APIC Timer Configuration.
     *
     * @brief APIC timer has three mode: one-shot, periodic and TSC-Deadline(We not use).
     *
     * @In one-short mode, the value of initial-count register is copied into the
     * current-count register and count-down begins. After the timer reaches zero,
     * a timer interrupt is generated and the timer remians at its 0 value until
     * reprogrammed.
     *
     * In periodic mode, its mechanism is basically similar to one-shot mode, it
     * only differs in that the current-count register is automatically reloaded
     * from the initial-count register when the count reaches 0 and the count-down
     * is repeated.
     *
     * @note A write of 0 to the initial-count register effectively stops the local APIC
     * timer, in both one-shot and periodic mode.
     *
     * TSC-deadline mode allows software to use the local APIC timer to signal an
     * interrupt at an absolute time. Unlike one-shot and periodic mode using
     * initial-count and current-count registers, these registers will be ignored
     * in TSC-deadline mode. Instead, timer behavior is controlled using the
     * IA_32_TSC_DEADLINE MSR.
     *
     */

    // Thrermal and power management
    cpuid(CPUID_LEFT_THERMAL_AND_POWER_MANAGEMENT,
          &eax, &ebx, &ecx, &edx);
    support_apic_time_always_running = (boolean)((eax >> 2) & 1);

    // TscInvariant
    cpuid(CPUID_EX_LEFT_APMF, &eax, &ebx, &ecx, &edx);
    // The TSC rate is ensured to be invariant across all P-States, C-States, and stop grant
    // transitions (such as STPCLK Throttling); therefore the TSC is suitable for use as a
    // source of time.
    // No such guarantee is made and software should avoid attempting to use the TSC as a source of time.
    tsc_invariant = (boolean)((edx >> 8) & 1);

    // Configuration to APIC Timer
    if (!memcmp(cpu_vendor_id, CPUID_VENDOR_INTEL, sizeof(CPUID_VENDOR_INTEL)))
    {
        /**
         * For Intel 64
         *
         * The APIC timer frequency is derived from the processor’s bus clock or
         * the core crystal clock frequency. If the TSC (Time Stamp Counter) to
         * core crystal clock ratio is enumerated in CPUID leaf 0x15, the core
         * crystal clock frequency is used.
         *
         * The formula for the APIC timer frequency is:
         *          APIC Timer frequency = Core crystal clock frequency / Divide Value.
         *
         *
         * */

        cpuid(CPUID_LEFT_TSC, &eax, &ebx, &ecx, &edx);
        if (eax == 0 || ebx == 0)
        {
            // CPUID leaf 0x15 is not supported or not providing TSC/core crystal clock information
        }
        else
        {

            // The TSC frequency can be calculated as follows:
            // TSC Frequency = Core Crystal Clock Frequency * EBX/EAX
            // If ECX contains a non-zero value, it is the nominal core crystal clock frequency
            core_crystal_clock_frequency = ecx;
            tsc_frequency = core_crystal_clock_frequency ? (uint64_t)ecx * ebx / eax : 0;
        }

        cpuid(CPUID_LEFT_PROCESSOR_FREQUENCY_INFO, &eax, &ebx, &ecx, &edx);
        processor_base_frequency_in_MHz = eax;
        processor_maximum_frequency_in_MHz = ebx;
        processor_bus_clock_frequency_in_MHz = ecx;

        if (core_crystal_clock_frequency)
        {
            ticks_in_10ms = (core_crystal_clock_frequency >> 4) * 0.01;     // 10ms
            write_lapic_register(LOCAL_APIC_DIVIDE_CONF_REG, divide_value);
            write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, ticks_in_10ms);
            write_lapic_register(LOCAL_APIC_LVT_TIMER_REG, 0x20 | (1 << 17));  // Periodic mode
        }
        else
            goto _manually_calculate_initial_count;
        
        
    }
    else if (!memcmp(cpu_vendor_id, CPUID_VENDOR_AMD, sizeof(CPUID_VENDOR_AMD)))
    {
        /**
         * For AMD
         *
         * The APIC timer frequency is typically derived from the processor’s core clock frequency.
         * AMD processors do not use the core crystal clock frequency in the same way as Intel processors.
         *
         * The formula for the APIC timer frequency is:
         *          APIC Timer frequency = Core clock frequency / Divide Value.
         *
         */

        cpuid(CPUID_EX_LEFT_APMF, &eax, &ebx, &ecx, &edx);
        // Read-only effective frequency interface
        if ((edx >> 10) & 1)
        {
            // if edx: bit 10 = 1, indicates presence of MSR C000_00E7[Read-Only Max Performance
            // Frequency Clock Count (MPerfReadOnly)] and MSRC000_00E8 [Read-Only Actual
            // Performance Frequency Clock Count (APerfReadOnly)].
            max_perf_freq_clock_count = rdmsr(0xC00000E7);
            actual_perf_freq_clock_count = rdmsr(0xC00000E8);
        }


_manually_calculate_initial_count:
        // Manually calcul  ate the value of initial count register that we need to configure it, which makes 
        // it can generate once clcok interrupt every 10ms.
        pit_prepare_sleep(10000);
        write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, 0xFFFFFFFF);
        pit_perform_sleep(); 
        write_lapic_register(LOCAL_APIC_LVT_TIMER_REG, 1 << 16);
        ticks_in_10ms = 0xFFFFFFFF - read_lapic_register(LOCAL_APIC_CURRENT_COUNT_REG);
        write_lapic_register(LOCAL_APIC_DIVIDE_CONF_REG, divide_value);
        write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, ticks_in_10ms);
        write_lapic_register(LOCAL_APIC_LVT_TIMER_REG, 0x20 | (1 << 17));

    }
    else
    {
        krnl_panic(NULL);
    }

    uint32_t low = read_ioapic_register(0x12);
    uint32_t high = read_ioapic_register(0x13);

    low = (low & 0xFFFFFF00) | 0x21;
    high = (high & 0x00FFFFFF) | 0;

    write_ioapic_register(0x12, low);
    write_ioapic_register(0x13, high);

    _cpu_install_isr(&bsp, 0x21, keyboard_isr, IDT_DESC_TYPE_INTERRUPT_GATE, 0);

    // asm ("sti\n");

    // for (uint32_t i = 0; i < 0xfffffff; i++);
    // for (uint32_t i = 0; i < 0xfffffff; i++);
    // for (uint32_t i = 0; i < 0xfffffff; i++);



    /**
     * Interrupt Command Register
     *
     * @brief The interrupt command register (ICR) is a 64-bit local APIC register
     * that allows software running on the processor to specify and send
     * interprocessor interrupts (IPIs) to other processors in the system.
     *
     * @memberof Interrupt Command Register
     * @property{bit 11}    Destination Mode        Selects either physical (0) or logical (1) destination mode
     *
     */

    /**
     * Destination Mode
     * @brief Selects either physical or logical destination mode.
     */

    /**
     * @brief Introduction to Physical destination mode.
     * 
     * In physical destination mode, the destination processor is specified by its local APIC ID
     * 
     * @note The number of local APICs that can be addressed on the system bus may be restricted by hardware.
     */
}
