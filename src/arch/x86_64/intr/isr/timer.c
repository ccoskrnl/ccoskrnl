#include "../../../../include/types.h"
#include "../../../../include/machine_info.h"
#include "../../../../include/hal/acpi.h"
#include "../../../../include/libk/list.h"
#include "../../../../include/libk/string.h"
#include "../../../../include/go/go.h"
#include "../../../../include/libk/stdlib.h"

#include "../../cpu/cpu_features.h"
#include "../../cpu/cpu.h"
#include "../../io/io.h"
#include "../apic.h"
#include "../hpet.h"
#include "../pit.h"
#include "intr_isr.h"


// Bootstrap Processor descriptor
extern cpu_core_desc_t bsp;


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

// Time stamp counter frequency.
uint64_t tsc_frequency;

// The ratio of time stamp counter frequency.
double tsc_freq_ratio;

// Max Performance Frequency Clock Count
uint64_t max_perf_freq_clock_count;
// Actual Performance Frequency Clock Count
uint64_t actual_perf_freq_clock_count;



// Tsc Invariant
// The TSC rate is ensured to be invariant across all P-States, C-States, and stop grant
// transitions (such as STPCLK Throttling); therefore the TSC is suitable for use as a
// source of time.
// No such guarantee is made and software should avoid attempting to use the TSC as a source of time.
boolean tsc_invariant;

static uint64_t apic_timer_freq = 0;


// Helper function to calculate APIC timer ticks in a given time interval.
static inline uint64_t calculate_apic_ticks(uint64_t apic_freq, double interval_seconds)
{
    return apic_freq * interval_seconds;
}

static uint64_t manually_measure_ticks_in_10ms()
{
    uint32_t ticks_in_10ms;
    uint8_t divide_reg = 0x3;

    // The initial count value is copied into the current-count register and
    // count-down begins. Its value descrease by 1 per clock cycle.
    // So we can manually measure core clock frequency through this mechanism.

    // Manually measure the value of initial count register that we need to configure it, which makes 
    // it can generate once clock interrupt every 10ms.

    // Tell APIC Timer to use divider 16
    write_lapic_register(LOCAL_APIC_DIVIDE_CONF_REG, divide_reg);

    // Prepare the PIT to sleep for 10ms.
    pit_prepare_sleep(10000);

    // Set APIC init counter to -1.
    write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, 0xFFFFFFFF);

    // Perfrom PIT-supported sleep.
    pit_perform_sleep(); 

    // Write 0 to the initial-count register to effectively stop the local
    // APIC timer.
    write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, 0);
    // Mask timer interrupt.
    write_lapic_register(LOCAL_APIC_LVT_TIMER_REG, LOCAL_APIC_LVT_INT_MASKED);

    // Now we know how often the APIC timer has ticked in 10ms.
    ticks_in_10ms = 0xFFFFFFFF - read_lapic_register(LOCAL_APIC_CURRENT_COUNT_REG);

    return ticks_in_10ms;
}

void timer_isr()
{
    asm ("cli");

    // Handle the timer interrupt.
    send_eoi();

    asm ("sti");
}


/**
 * The routines installs timer interrupt on IDT[0x20].
 * By default, We use apic timer to emit timer interrupt.
 **/
void enable_timer_intr()
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t ticks_in_10ms;
    uint8_t divide_reg = 0x3;

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

    cpuid(CPUID_EX_LEFT_APMF, &eax, &ebx, &ecx, &edx);
    tsc_invariant = (boolean)((edx >> 8) & 1);


    // The APIC timer frequency need to be divided by the value specified in the
    // divide configuration register.

    // Configuration to APIC Timer
    if (!memcmp(cpu_feat_id.vendor_id, CPUID_VENDOR_INTEL, sizeof(CPUID_VENDOR_INTEL)))
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

        // If ebx is 0, the TSC/core crystal clock ratio is not enumerated.
        // If ecx is 0, the nominal core crystal clock frequency is not enumerated.
        if (ecx == 0 || ebx == 0)
        {
            ticks_in_10ms = manually_measure_ticks_in_10ms();
        }
        else
        {
            // The TSC frequency can be calculated as follows:
            // TSC Frequency = Core Crystal Clock Frequency * EBX/EAX
            // If ECX contains a non-zero value, it is the nominal core crystal clock frequency
            core_crystal_clock_frequency = ecx;
            tsc_freq_ratio = (double)ebx / (double)eax;
            tsc_frequency = core_crystal_clock_frequency * tsc_freq_ratio;
            ticks_in_10ms = calculate_apic_ticks(tsc_frequency / 16, 0.01);

            // These informations should not to be used by OS.
            cpuid(CPUID_LEFT_PROCESSOR_FREQUENCY_INFO, &eax, &ebx, &ecx, &edx);
            processor_base_frequency_in_MHz = eax;
            processor_maximum_frequency_in_MHz = ebx;
            processor_bus_clock_frequency_in_MHz = ecx;

        }


    }
    else if (!memcmp(cpu_feat_id.vendor_id, CPUID_VENDOR_AMD, sizeof(CPUID_VENDOR_AMD)))
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
            ticks_in_10ms = calculate_apic_ticks(actual_perf_freq_clock_count, 0.01);
        }
        else
        {
            ticks_in_10ms = manually_measure_ticks_in_10ms();
        }


    }
    else
    {
        ticks_in_10ms = manually_measure_ticks_in_10ms();
    }

    // Start timer as periodic on IRQ 0, divider 16, with the number of
    // ticks we counted.
    write_lapic_register(LOCAL_APIC_DIVIDE_CONF_REG, divide_reg);

    // In periodic mode, the current-count register is automatically
    // reloaded from the initial-count register when the count reaches 0 and
    // a timer interrupt is generated, and the count-down is repeated.
    write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, ticks_in_10ms);
    write_lapic_register(LOCAL_APIC_LVT_TIMER_REG, INTR_ISR_TIMER_INT | LOCAL_APIC_LVT_TIMER_PERIODIC);

    _cpu_install_isr(&bsp, 0x21, timer_isr, IDT_DESC_TYPE_INTERRUPT_GATE, 0);


    // Mask timer interrupt.
    write_lapic_register(LOCAL_APIC_INIT_COUNT_REG, 0);
    read_lapic_register(eax);
    eax |= LOCAL_APIC_LVT_INT_MASKED;
    write_lapic_register(LOCAL_APIC_LVT_TIMER_REG, eax);

}
