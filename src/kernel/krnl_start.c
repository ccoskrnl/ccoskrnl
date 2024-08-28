#include "../include/types.h"
#include "../include/machine_info.h"
#include "../include/hal/op/graphics.h"
#include "../include/hal/op/screen.h"
#include "../include/arch/mm.h"
#include "../include/libk/stdio.h"
#include "../include/libk/stdlib.h"

#include "../arch/x86_64/intr/hpet.h"
#include "../arch/x86_64/intr/pit.h"

extern void preparing_for_bsp(boolean is_first);
extern void detecting_cpu();
extern void op_init();
extern void mm_init();
extern void intr_init();
extern void acpi_init();

/*  Stack top of temporary stack of kernel initialization thread  */
extern uint64_t _mm_stack_top_of_initialization_thread;
/*  The size of stack of kernel initialization thread in bytes  */
extern uint64_t _mm_size_of_stack_of_initialization_thread_in_bytes;
/*  Stack buttom of temporary stack of kernel initialization thread  */
extern uint64_t _mm_stack_buttom_of_initialization_thread;

extern boolean tsc_invariant;
extern char cpu_brand_string[49];  // 48 characters + null terminator

extern hpet_table_t *hpet_table;

void krnl_init();
void print_machine_info();
void krnl_exit();


extern wch_t* tengwangge;
extern wch_t* chushibiao;

extern void _intr_intr_handler_test();

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

    // Detect cpu features and set some global variables about cpu info.
    detecting_cpu();

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
    put_check(true, L"Ccoskrnl loaded.\n");
    put_check(true, L"Bootstrap Processor initializated.\n");
    put_check(true, L"Kernel memory layout divided.\n");
    put_check(true, L"Dynamic memory alloctor is ready to be used.\n");
    put_check(true, L"Default font family has been parsed completely.\n");

    // Initialize OSPM
    acpi_init();

    // Interrupt module initialization. 
    // when the routines finish, it will set interrupt flag.
    // intr_init();
    
    put_check(true, L"System diagnostic completed. All systems nominal.\n");
    put_check(false, L"ACPI initialized.\n");
    put_check(false, L"Power Off.\n");

    __asm__ ("sti"); 


    // _intr_intr_handler_test();
    while (1) {
        puts("y\n");
    }
    putws(chushibiao);
    putws(tengwangge);
    
    // print_machine_info();

    krnl_exit();
    
}


void print_machine_info()
{
    
    
    // puts("System diagnostic completed. All systems nominal.\n");

    // puts("Processor Name: ");
    // puts(cpu_brand_string);
    // putc('\n');

    // puts("TSC invariant: ");
    // puts(tsc_invariant ? "true\n" : "false\n");

    // putsx("Kernel Space Size: ", _current_machine_info->memory_space_info[0].size);
    // putc('\n');
    // putsx("RAM Size: ", _current_machine_info->memory_info.ram_size);
    // putc('\n');
    // putsd("Screen Width: ", _current_machine_info->graphics_info.HorizontalResolution);
    // putc('\n');
    // putsd("Screen Height: ", _current_machine_info->graphics_info.VerticalResolution);
    // putc('\n');

    // puts("IA-PC HPET: ");
    // puts(_current_machine_info->acpi_info.hpet != 0 ? "true\n" : "false\n");
    // if (_current_machine_info->acpi_info.hpet != 0)
    // {
    //     putsxs("IA-PC HPET Address: ", hpet_table->BaseAddressLower32Bit.Address, "\n");
    // }
    // while (1) {
    //     ;
    // }
    
    
}

void krnl_exit()
{
    krnl_panic();
}
