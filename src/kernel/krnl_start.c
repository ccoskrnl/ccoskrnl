#include "../include/types.h"
#include "../include/machine_info.h"

#include "../include/go/go.h"

#include "../include/arch/mm.h"
#include "../include/arch/lib.h"

#include "../include/libk/stdlib.h"
#include "../include/libk/bitmap.h"

#include "../include/arch/cpu_features.h"

extern void transition_to_p0();

extern void op_init();
extern void acpi_init();
extern void pci_init();
extern void intr_init();
extern void active_aps(void);

extern wch_t* tengwangge;
extern wch_t* chushibiao;


/*
 * ┌────────────────────────┐                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * │                        │                    
 * |────────────────────────┤◄─────- stack top
 * │                        │                    
 * │    krnl temp stack     │ 0x100000                   
 * │                        │                    
 * |────────────────────────┤◄────── stack bottom
 * │                        │                    
 * │    background file     │                    
 * │                        │                    
 * |────────────────────────┤                    
 * │                        │                    
 * │      font files        │                    
 * │                        │                    
 * |────────────────────────┤                    
 * │                        │                    
 * │  ccoskrnl image file   │                    
 * │                        │                    
 * └────────────────────────┘◄────── kernel space base addr
 */

/*  Stack top of temporary stack of kernel initialization thread  */
extern uint64_t _mm_stack_top_of_initialization_thread;
/*  The size of stack of kernel initialization thread in bytes  */
extern uint64_t _mm_size_of_stack_of_initialization_thread_in_bytes;
/*  Stack buttom of temporary stack of kernel initialization thread  */
extern uint64_t _mm_stack_buttom_of_initialization_thread;

static void krnl_init();
static void krnl_exit();

/**
 * Preparing base kernel code execution environment for continuing to initialize.
 * 
 * @param[in]       ptr                         machine_info passed by ccldr
 * 
 * @retval          never be returned.
*/
void krnl_start(LOADER_MACHINE_INFORMATION* ptr)
{

    _current_machine_info = ptr;

    // Set variables about Stack of kernel initialization thread
    _mm_stack_buttom_of_initialization_thread = 
        (KERNEL_SPACE_BASE_ADDR + (_current_machine_info->sum_of_size_of_files_in_pages << PAGE_SHIFT));

    // _mm_stack_buttom_of_initialization_thread = (KERNEL_SPACE_BASE_ADDR + 
    //         (_current_machine_info->bg.addr - _current_machine_info->memory_space_info[0].base_address)
    //          + page_aligned(_current_machine_info->bg.size));

    _mm_size_of_stack_of_initialization_thread_in_bytes = KERNEL_STACK_DEFAULT_SIZE;

    _mm_stack_top_of_initialization_thread = 
        _mm_stack_buttom_of_initialization_thread + _mm_size_of_stack_of_initialization_thread_in_bytes;

    // Reset RSP 
    // From now on, the earlier data stored in previous stack becomes unavailable. The variable 
    // _current_machine_info needs to be updated through mm_init(). 
    set_krnl_stack(_mm_stack_top_of_initialization_thread);

    /**
     * Prepare the system for the Bootstrap Processor (BSP) initialization.
     * This function sets up the necessary environment for the BSP to start executing.
     * Copy GDT from ccldr area to kernel area. Set up the GDT and IDT for BSP. Then
     * we reload the segment selector register.
     * 
     * We also need to check current cpu features and set some global variables about cpu info.
     * (e.g. XCR0, AVX, XSAVE ...)
     * 
     */
    preparing_for_bsp(true);
    
    /**
     * @brief Memory Management initialization.
     * 
     * Set up global variables about memory layout. (e.g. kernel space, pfn database
     * non paged pool and system pte pool.)
     * 
     * After we divided the memory layout, we need to remap kernel space and construct
     * new kernel page directories.
     */
    mm_init();

    // Memory layout already was constructed, 
    // we are going to initialize other components.
    krnl_init();
}


/*

Routine Description:

    The routine be called by krnl_start to initialize other components.    

Parameters:

    None.

Return Value:

    None.

*/

void krnl_init()
{

    preparing_for_bsp(false);


    // Output Initialization
    op_init();
    put_check(output_bsp, true, L"Ccoskrnl loaded.\n");
    put_check(output_bsp, true, L"Bootstrap Processor has already been initializated.\n");
    put_check(output_bsp, -1, L"Memory Manager perparing...\n");
    put_check(output_bsp, true, L"Dynamic memory allocator is ready to be used.\n");
    put_check(output_bsp, true, L"Default font family has been parsed completely.\n");

    // Interrupt module initialization. 
    put_check(output_bsp, -1, L"Interrupt module initializing...\n");
    intr_init();
    put_check(output_bsp, true, L"Interrupt module has already been initialized.\n");
    put_check(output_bsp, true, L"Interrupt enabled.\n");

    // Set interrupt flag, os can handle interrupts or exceptions.
    __asm__ ("sti"); 

    // Initialize OSPM
    put_check(output_bsp, -1, L"OSPM initializing...\n");
    acpi_init();
    // transition_to_p0();

    put_check(output_bsp, true, L"OSPM has already been initialized.\n");

    putc(output_bsp, '\n');

    // Initialize PCIe
    // put_check(output_bsp, -1, L"PCIe initializing...\n");
    // pci_init();
    // put_check(output_bsp, true, L"PCIe has already been initialized.\n");

    // active_aps();

    // put_check(output_bsp, false, L"Power Off.\n");


    krnl_exit();
    
}

void krnl_exit()
{

    __asm__("hlt\n\t");
    // sleep(3600);
    // krnl_panic(NULL);
}
