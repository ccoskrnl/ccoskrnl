#include "../../../include/go/go.h"
#include "../../../include/hal/acpi.h"
#include "../../../include/libk/list.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/libk/string.h"
#include "../../../include/machine_info.h"
#include "../../../include/types.h"

#include "../cpu/cpu.h"
#include "../cpu/cpu_features.h"
#include "../io/io.h"
#include "../lib/lib.h"
#include "../mm/mm_arch.h"
#include "apic.h"
#include "hpet.h"
#include "isr/intr_isr.h"
#include "pit.h"

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
intr_ctr_struct_head_t
intr_ctr_structs[MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES];

extern lapic_ver_t lapic_ver;

/*
 * Routine Description:
 *    Parses the ACPI MADT (Multiple APIC Description Table) to extract
 * information about the system's interrupt controller configuration, including
 * Local APICs, I/O APICs, interrupt source overrides, and other related
 * structures.
 *
 *    The MADT table provides essential information for configuring the Advanced
 *    Programmable Interrupt Controller (APIC) architecture in x86 systems.
 *
 *    MADT Table Structure:
 *    - Standard ACPI table header (signature "APIC")
 *    - Local APIC address (32-bit physical address)
 *    - Flags (32-bit)
 *    - Variable number of interrupt controller structures
 *
 *    Each interrupt controller structure has:
 *    - Type (1 byte): Identifies the structure type
 *    - Length (1 byte): Size of the structure
 *    - Type-specific data
 *
 * Parameters:
 *    madt_ptr - Pointer to the MADT table structure
 *    intr_ctr_structs - Array of linked list heads for storing different types
 * of interrupt controller structures found in the MADT
 *
 * Returned Value:
 *    None
 */
static void parse_madt(_in_ struct _MADT *madt_ptr,
        _in_ _out_ intr_ctr_struct_head_t *intr_ctr_structs) {
    // Calculate the end address of the MADT table
    uint8_t *madt_end = (uint8_t *)madt_ptr + madt_ptr->h.Length;
    // Pointer to the start of interrupt controller structures (after MADT header)
    uint8_t *madt_structures = (uint8_t *)madt_ptr + sizeof(*madt_ptr);

    InterruptControllerStructureType type;
    uint8_t size;

    // Initialize all list heads for different interrupt controller structure
    // types
    for (uint8_t i = 0; i < MAX_MADT_INTURRUPT_CONTROLLER_STRUCTURE_TYPES; i++) {
        list_node_t *head = &(intr_ctr_structs + i)->head;
        (intr_ctr_structs + i)->total =
            0;            // Initialize count for this structure type
        _list_init(head); // Initialize the linked list
    }

    // Walk through all structures in the MADT table
    while (madt_structures < madt_end) {
        type = *madt_structures;       // Get structure type (first byte)
        size = *(madt_structures + 1); // Get structure size (second byte)

        // Process structure based on its type
        switch (type) {
            case ProcessorLocalAPIC: {
                                         // Allocate memory for Processor Local APIC structure
                                         processor_local_apic_t *lapic =
                                             (processor_local_apic_t *)malloc(sizeof(processor_local_apic_t));
                                         // Copy structure data from MADT to our allocated memory
                                         memcpyb(lapic, madt_structures, size);

                                         // Update count and add to the linked list for this structure type
                                         (intr_ctr_structs + type)->total++;
                                         _list_push(&(intr_ctr_structs + type)->head,
                                                 (list_node_t *)((uint8_t *)lapic +
                                                     sizeof(processor_local_apic_t) -
                                                     sizeof(list_node_t)));

                                         // Check if this is not the Bootstrap Processor (BSP)
                                         if (lapic->apic_id != bsp.lapic_id) {
                                             // Allocate and initialize a CPU core descriptor
                                             cpu_core_desc_t *core =
                                                 (cpu_core_desc_t *)calloc(sizeof(cpu_core_desc_t));
                                             core->lapic_id = lapic->apic_id;
                                             core->lapic_id = lapic->acpi_processor_id;

                                             // Add to global list of CPU cores
                                             _list_push(&cpu_cores_list, &core->node);

                                             // Store in array indexed by APIC ID
                                             cpu_cores[lapic->apic_id] = core;
                                         } else {
                                             // Update BSP information
                                             bsp.processor_id = lapic->acpi_processor_id;
                                         }
                                     } break;

            case IOAPIC: {
                             // Allocate memory for I/O APIC structure
                             io_apic_t *ioapic = (io_apic_t *)malloc(sizeof(io_apic_t));
                             // Copy structure data
                             memcpyb(ioapic, madt_structures, size);

                             // Update count and add to linked list
                             (intr_ctr_structs + type)->total++;
                             _list_push(&(intr_ctr_structs + type)->head,
                                     (list_node_t *)((uint8_t *)ioapic + sizeof(io_apic_t) -
                                         sizeof(list_node_t)));
                         } break;

            case InterruptSourceOverride: {
                                              // Allocate memory for Interrupt Source Override structure
                                              iso_t *iso = (iso_t *)malloc(sizeof(iso_t));
                                              // Copy structure data
                                              memcpyb(iso, madt_structures, size);

                                              // Update count and add to linked list
                                              (intr_ctr_structs + type)->total++;
                                              _list_push(&(intr_ctr_structs + type)->head,
                                                      (list_node_t *)((uint8_t *)iso + sizeof(iso_t) -
                                                          sizeof(list_node_t)));
                                          } break;

            case NonMaskableInterruptSource: {
                                                 // Allocate memory for Non-Maskable Interrupt structure
                                                 nmi_t *nmi = (nmi_t *)malloc(sizeof(nmi_t));
                                                 // Copy structure data
                                                 memcpyb(nmi, madt_structures, size);

                                                 // Update count and add to linked list
                                                 (intr_ctr_structs + type)->total++;
                                                 _list_push(&(intr_ctr_structs + type)->head,
                                                         (list_node_t *)((uint8_t *)nmi + sizeof(nmi_t) -
                                                             sizeof(list_node_t)));
                                             } break;

            case LocalAPICNMI: {
                                   // Allocate memory for Local APIC Address Override structure
                                   local_apic_addr_override_t *lapic_addr_or =
                                       (local_apic_addr_override_t *)malloc(
                                               sizeof(local_apic_addr_override_t));
                                   // Copy structure data
                                   memcpyb(lapic_addr_or, madt_structures, size);

                                   // Update count and add to linked list
                                   (intr_ctr_structs + type)->total++;
                                   _list_push(&(intr_ctr_structs + type)->head,
                                           (list_node_t *)((uint8_t *)lapic_addr_or +
                                               sizeof(local_apic_addr_override_t) -
                                               sizeof(list_node_t)));
                               } break;

            default:
                               // Ignore unknown structure types
                               break;
        }

        // Move to the next structure in the MADT
        madt_structures += size;
    }

    // Debug output section (compiled only if INTR_DEBUG is defined)
#ifdef INTR_DEBUG
    // Walk through lists and print information about each structure type

    list_node_t *head;
    list_node_t *intr_ctr_struct_node;

    // Print Processor Local APIC information
    if ((intr_ctr_structs + ProcessorLocalAPIC)->total != 0) {
        puts(output_bsp, "Processor Local APIC: \n");
        head = &(intr_ctr_structs + ProcessorLocalAPIC)->head;
        processor_local_apic_t *lapic =
            struct_base(processor_local_apic_t, node, head->flink);
        intr_ctr_struct_node = head->flink;

        for (; intr_ctr_struct_node != 0;
                intr_ctr_struct_node = intr_ctr_struct_node->flink) {
            putsds(output_bsp, "Local APIC ID: ", lapic->apic_id, "\n");
            lapic = struct_base(processor_local_apic_t, node, lapic->node.flink);
        }
    }

    // Print I/O APIC information
    if ((intr_ctr_structs + IOAPIC)->total != 0) {
        puts(output_bsp, "I/O APIC: \n");
        head = &(intr_ctr_structs + IOAPIC)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + IOAPIC)->total;
                i++, intr_ctr_struct_node = intr_ctr_struct_node->flink) {
            io_apic_t *ioapic = struct_base(io_apic_t, node, intr_ctr_struct_node);
            putsds(output_bsp, "IO APIC ID: ", ioapic->io_apic_id, "\n");
            putsxs(output_bsp, "IO APIC Address: ", ioapic->ioapic_addr, "\n");
            putsxs(output_bsp, "Global System Interrupt Base: ",
                    ioapic->global_sys_interrupt_base, "\n");
        }
    }

    // Print Interrupt Source Override information
    if ((intr_ctr_structs + InterruptSourceOverride)->total != 0) {
        puts(output_bsp, "Interrupt Source Override: \n");
        head = &(intr_ctr_structs + InterruptSourceOverride)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + InterruptSourceOverride)->total;
                i++, intr_ctr_struct_node = intr_ctr_struct_node->flink) {
            iso_t *iso = struct_base(iso_t, node, intr_ctr_struct_node);
            putsds(output_bsp, "Bus: ", iso->bus, "\n");
            putsds(output_bsp, "Bus-relative interrupt source(IRQ): ", iso->source,
                    "\n");
            putsds(output_bsp, "Global System Interrupt: ", iso->global_sys_interrupt,
                    "\n");
            putsxs(output_bsp, "Flags: ", iso->flags, "\n");
        }
    }

    // Print Non-Maskable Interrupt Source information
    if ((intr_ctr_structs + NonMaskableInterruptSource)->total != 0) {
        puts(output_bsp, "Non-maskable Interrupt Source: \n");
        head = &(intr_ctr_structs + NonMaskableInterruptSource)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0;
                i < (intr_ctr_structs + NonMaskableInterruptSource)->total;
                i++, intr_ctr_struct_node = intr_ctr_struct_node->flink) {
            nmi_t *nmi = struct_base(nmi_t, node, intr_ctr_struct_node);
            putsds(output_bsp, "Processor ID: ", nmi->acpi_processor_id, "\n");
            putsxs(output_bsp, "Flags: ", nmi->flags, "\n");
            putsds(output_bsp, "Local LINTn pin number: ", nmi->lint, "\n");
        }
    }

    // Print Local APIC Address Override information
    if ((intr_ctr_structs + LocalAPICAddressOverride)->total != 0) {
        puts(output_bsp, "Local APIC Address Override: \n");
        head = &(intr_ctr_structs + LocalAPICAddressOverride)->head;
        intr_ctr_struct_node = head->flink;
        for (size_t i = 0; i < (intr_ctr_structs + LocalAPICAddressOverride)->total;
                i++, intr_ctr_struct_node = intr_ctr_struct_node->flink) {
            local_apic_addr_override_t *local_apic_addr_override =
                struct_base(local_apic_addr_override_t, node, intr_ctr_struct_node);
            putsxs(output_bsp,
                    "Local APIC Address: ", local_apic_addr_override->local_apic_addr,
                    "\n");
        }
    }

#endif
}

void init_hpet() {
    // Check if HPET is exist. Whether it exists or not, we also don't use. {:?}
    if (_current_machine_info->acpi_info.hpet != 0)
        hpet_table = (hpet_table_t *)fixup_acpi_table_addr(
                (uint64_t)_current_machine_info->acpi_info.hpet);
}

void enable_apic() {
    uint64_t value;
    uint64_t lapic_reg;

    value = rdmsr(IA32_APIC_BASE_MSR);
    value |= 1 << 8;
    wrmsr(IA32_APIC_BASE_MSR, value);

    // APIC Software Enable/Disable in 8th bit.
    lapic_reg = 0xFF | (1 << 8);
    write_lapic_register(LOCAL_APIC_SPURIOUS_INTERRUPT_VEC_REG, lapic_reg);
}

void intr_init() {

    // Fix up the MADT address
    madt_ptr = (struct _MADT *)fixup_acpi_table_addr(
            (uint64_t)_current_machine_info->acpi_info.madt);

    // Parse madt and push structures to corresponding lists based on theirs type.
    parse_madt(madt_ptr, intr_ctr_structs);

    // Map local apic address and IOAPIC address. We don't handle mutilprocessor
    // system.
    map_lapic_and_ioapic(madt_ptr->local_apic_addr, &intr_ctr_structs[IOAPIC]);

    // Since we use the processor local APIC and the I/O APIC,
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

    ioapic_rte_t kbd_rte = {0};
    kbd_rte.intr_vector = 0x21;
    kbd_rte.dest_field = 0;

    uint32_t low = *(uint32_t *)&kbd_rte;
    uint32_t high = *(uint32_t *)((uint64_t)&kbd_rte + 4);

    write_ioapic_register(ioapics[0], IOAPIC_RET_ENTRY_OFFSET(IRQ_KEYBOARD), low);
    write_ioapic_register(ioapics[0], IOAPIC_RET_ENTRY_OFFSET(IRQ_KEYBOARD) + 1,
            high);

    _cpu_install_isr(&bsp, 0x21, keyboard_isr_wrapper,
            IDT_DESC_TYPE_INTERRUPT_GATE, 0);
}
