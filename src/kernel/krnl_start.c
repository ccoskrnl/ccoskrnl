#include "../include/types.h"
#include "../include/machine_info.h"

#include "../include/go/go.h"

#include "../include/arch/mm.h"
#include "../include/arch/lib.h"

#include "../include/libk/stdlib.h"


extern void preparing_for_bsp(boolean is_first);
extern void mm_init();

extern void op_init();
extern void acpi_init();
extern void intr_init();
extern void active_aps(void);

extern wch_t* tengwangge;
extern wch_t* chushibiao;

/*  Stack top of temporary stack of kernel initialization thread  */
extern uint64_t _mm_stack_top_of_initialization_thread;
/*  The size of stack of kernel initialization thread in bytes  */
extern uint64_t _mm_size_of_stack_of_initialization_thread_in_bytes;
/*  Stack buttom of temporary stack of kernel initialization thread  */
extern uint64_t _mm_stack_buttom_of_initialization_thread;

void krnl_init();
void krnl_exit();

/**
 * Preparing base kernel code execution environment for continuing to initialize.
 * 
 * @param[in]       ptr                         machine_info passed by ccldr
 * 
 * @retval          never be returned.
*/
void krnl_start(LOADER_MACHINE_INFORMATION* ptr, uint64_t is_ap)
{
    if (is_ap != 0) {
        while (true) {
            ;
        }
    }
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

    // prepare to execute initialization code.
    preparing_for_bsp(true);
    
    // Memory Management Initialization
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
    put_check(0, true, L"Ccoskrnl loaded.\n");
    put_check(0, true, L"Bootstrap Processor initializated.\n");
    put_check(0, true, L"Kernel memory layout has divided.\n");
    put_check(0, true, L"Dynamic memory allocator is ready to be used.\n");
    put_check(0, true, L"Default font family has been parsed completely.\n");

    // Initialize OSPM
    acpi_init();

    // Interrupt module initialization. 
    put_check(0, -1, L"Interrupt module initializing...\n");
    intr_init();
    put_check(0, true, L"Interrupt module has initialized.\n");

    active_aps();

    put_check(0, true, L"System diagnostic completed. All systems nominal.\n");

    // Set interrupt flag, os can handle interrupts or exceptions.
    __asm__ ("sti"); 

    put_check(0, false, L"Power Off.\n");

    krnl_exit();
    
}

void krnl_exit()
{
    sleep(3600);
    krnl_panic(NULL);
}
