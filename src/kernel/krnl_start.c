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

    // Initialize OSPM
    acpi_init();

    // Interrupt module initialization. 
    // when the routines finish, it will set interrupt flag.
    // intr_init();

    // __asm__ ("sti"); 
    putws(L"\t你好，世界！A \bBC\n");
    putws(L"Apple\n");
    
    // print_machine_info();

    // int* p = NULL;
    // *p = 5;   

    krnl_exit();
    
}

void print_skadi()
{

    putss("\n"," $$$$$$\\        $$\\                                   $$\\       $$\\       $$\\ \n");
    puts("$$  __$$\\       $$ |                                  $$ |      \\__|      $$ |\n");
    puts("$$ /  \\__|      $$ |  $$\\        $$$$$$\\         $$$$$$$ |      $$\\       $$ |\n");
    puts("\\$$$$$$\\        $$ | $$  |       \\____$$\\       $$  __$$ |      $$ |      $$ |\n");
    puts(" \\____$$\\       $$$$$$  /        $$$$$$$ |      $$ /  $$ |      $$ |      \\__|\n");
    puts("$$\\   $$ |      $$  _$$<        $$  __$$ |      $$ |  $$ |      $$ |          \n");
    puts("\\$$$$$$  |      $$ | \\$$\\       \\$$$$$$$ |      \\$$$$$$$ |      $$ |      $$\\ \n");
    putss(" \\______/       \\__|  \\__|       \\_______|       \\_______|      \\__|      \\__|\n", "\n\n");
                                                                              
                                                                              
                                                                              

    putss("\n","                                     ';>[|1:                                    \n");
    puts("                                  '>|z0Zwpd(                                    \n");
    puts("                                ,[zOmOOmqppr                  .'^,:l>++i`       \n");
    puts("                             .:1JmZOOOmmwqqY>^           ^l-1tuXJQ0mqpbX:       \n");
    puts("      ....                 ^]vLZZZZZmmmmmmmmmZQYn1>^^l}fuJ0mmwmmmmmmpO|\"        \n");
    puts("       .'!-i:`           '_YwZOO0YUmqwmmmqwmwqqqppmCJOwmmmmmmwwmmmwpC<          \n");
    puts("         `}uXXu\\-;'     ;nmmZQYr1?uqwwwqOXcmwmmmmmZwqmZOZmmmmmmmmmqc,           \n");
    puts("          `[nrv0QCzr(]?}Xm00Ov]-+|Zppq0n{+vwmmmwmOZmmmmmmmmmmmmwwqU+~]{1(\\||/); \n");
    puts("           .~r\\}|uLwpwmmZO00Or_)n0wqmj}++}OqqqQc|}XqwmmmmmmmwpqO00xtt\\|)11{}{z> \n");
    puts("             :)j\\}]|uJQZZZOZmLXZpwmqX__[(Xqwqv[+<_UpwmmmmwqwQzt{[|?????]|]\?\?(v\" \n");
    puts("               :{xt{--1nXCZwmmmZZmmwUfc0wqwwO{+?{cmqwmmmwZUUj]????|]|]???|?{v_  \n");
    puts("                 ^~(tt\\[+_[|uLQ0OZmZZmwwmmmwmcuLmwwwwZZQn}<_nz{?]|]|]?????{v}.  \n");
    puts("                   ._{~<+~~>})f}1Skadi6ttjxuczUzCOCQ0000Yr|-<<~<+fc}]}?]????    \n");
    puts("                  ^?_ll!!!i[(:}]~+~~>i!<~+\\Ilc/-?][}_~<<~~++~~\\X(??]??[|\\}\"     \n");
    puts("    ^-:        ..i{<IIIlI~(t+i<t+!i>!!l<_]v?~}Xn?~~>+~ii><~<>i!/z[]{\\xj?\"       \n");
    puts("   \"njf>.      .>|i>lIl!-(..______Illll]{______.&t);l!!!!iii!l:^zu\\([i`         \n");
    puts("  ,ftl~n}\"     ;\\>_1ll+\\<  1(){    <lli}  vt/|}  Cr;lll!~!!i!!; |u'             \n");
    puts(" !v(,;:I{]^    -];(}I!ir}  ZCJY<   +-~>[  JYJJ?  ;(lIIll)_!!llI.]z`             \n");
    puts(" `]r<;;~)]\"    _!`f_ll;{L  ZXU0<    `;I;  CzYZ1  `t>l!ll1\\l!lll'_t'             \n");
    puts("  .1v>)ti'      '^j>Il;)0   ncf            XY2   :n>lllI]c><ill\";['             \n");
    puts("   ,vv(^         ^f>I!_v?                        _x!lllI?Y++>ll,!)              \n");
    puts("   .;<'          `t-;!n{                        'j{Illl;}Y~~!l!:}+              \n");
    puts("                  {tII~|}!           \\_/        <uiIIIIlux+<i!Iit\"              \n");
    puts("                  ,f+I;l+]_                    <n?;lII;)X?_+>!I<)               \n");
    puts("                   \"+!I>>;~j[[1\\jvJQLCJJt!;I!}nv]!llI;1z]~__il;[]               \n");
    puts("                    <<Ii_l]j]?~]cmwqpppddmZmqqt!Il!li|u]<+++!lI/!               \n");
    puts("                    >{Il><_v]{{?/0mmmmmmmwwqO(;::;Ii})+++++~lI,tl               \n");
    puts("                    ']1!I>]uf+{JmwwmmmmwqwqQ[:;+)jcQU1<~+++<l:'/~               \n");
    puts("                     ._)]<-[)<zppmmmmmwqqqp|,+cmpdpqpwj+~~+>;\".+/'              \n");
    puts("                       `;;l_tXqmmmmmmmmwwqq+-0qmmmmmmwqu+~+>!I``f<              \n");
    puts("                       \"<|YmqwmmmmmmZZmmmwqnCwmmmmmmmmwwn__<il\" [f'             \n");
    putss("                    .I)cvwqwmmmmZZmZZZZwmmZwmmmmmmmmmmmwwt+~il:.,n<             \n", "\n");
}

void print_machine_info()
{
    // while (1)
    // {
    //     print_skadi();
    //     for (int i = 0; i < 300; i++)
    //     {
    //         pit_prepare_sleep(10000);
    //         pit_perform_sleep();
    //     }
    // }

    // putws(L"你好！");
    
    
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