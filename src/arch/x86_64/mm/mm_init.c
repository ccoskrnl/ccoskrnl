/*

@file mm_init.c

Memory Manager Initialization.

*/



#include "../../../include/types.h"
#include "../../../include/machine_info.h"
#include "../../../include/libk/string.h"
#include "../../../include/libk/bitmap.h"
#include "../../../include/libk/stdlib.h"

#include "../cpu/cpu.h"
#include "../cpu/cpu_features.h"

#include "mm_arch.h"
#include "mm_pfn.h"
#include "mm_pool.h"
#include "mm_pt_pool.h"


/*
 * Virtual Address Space (512-GiB page aligned)                                                                                                                    
 *                                                                                                                                                                 
 * ┌─────────────────────────────┐                                                                                                                                 
 * │                             │                                                                                                                                 
 * │                             │                                                                                                                                 
 * │                             │                                                                                                                                 
 * │         framebuffer         │                                                                                                                                 
 * │                             │                       Kernel Physical Address Space (4-KiB page aligned)                                                        
 * │                             │                                                                                                                                 
 * │                             │                             ┌───────────────────────┐◄──────── _mm_reserved_mem_phys_end = _mm_krnl_space_phys_end              
 * ├─────────────────────────────┤                             │                       │                                                                           
 * │                             │                             │     reserved_mem      │                                                                           
 * │                             │                             │                       │                                                                           
 * │                             │     ┌───────────────────────►───────────────────────┤◄──────── _mm_sys_pte_pool_phys_end = _mm_reserved_mem_phys_start          
 * │         hardware            │     │                       │                       │                                                                           
 * │                             │     │                       │                       │                                                                           
 * │                             │     │                       │     sys_pte_pool      │                                                                           
 * │                             │     │                       │                       │                                                                           
 * ├─────────────────────────────┼─────┘                       │                       │                                                                           
 * │                             │            ┌────────────────►───────────────────────┤◄──────── _mm_non_paged_pool_phys_end = _mm_sys_pte_pool_phys_start        
 * │                             │            │                │                       │                                                                           
 * │                             │            │                │                       │                                                                           
 * │        sys_pte_pool         │            │                │                       │                                                                           
 * │                             │            │                │                       │                                                                           
 * │                             │            │                │                       │                                                                           
 * │                             │            │                │     non_paged_pool    │                                                                           
 * ├─────────────────────────────┼────────────┘                │                       │                                                                           
 * │                             │                             |                       |                                                                           
 * │                             │                             │                       │                                                                           
 * │                             │                             │                       │                                                                           
 * │       non_paged_pool        │                             │                       │                                                                           
 * │                             │       ┌─────────────────────►───────────────────────┤◄──────── _mm_pfn_db_phys_end = _mm_non_paged_pool_phys_start              
 * │                             │       │                     │                       │                                                                           
 * │                             │       │                     │     pfn_database      │                                                                           
 * ├─────────────────────────────┼───────┘                     │                       │                                                                           
 * │                             │            ┌────────────────►───────────────────────|◄──────── _mm_stack_top_of_initialization_thread = _mm_pfn_db_phys_start   
 * │                             │            │                │                       │                                                                           
 * │                             │            │                │image, rec_files, stack│                                                                           
 * │        pfn_database         │            │                │                       │                                                                           
 * │                             │            │      ┌─────────►───────────────────────┘◄──────── _mm_krnl_space_phys_base_addr = 0x000000005bb5c000 (random value)
 * │                             │            │      │                                                                                                             
 * │                             │            │      │                                                                                                             
 * ├─────────────────────────────┼────────────┘      │                                                                                                             
 * │                             │                   │                                                                                                             
 * │                             │                   │                                                                                                             
 * │                             │                   │                                                                                                             
 * │   image, rec_files, stack   │                   │                                                                                                             
 * │                             │                   │                                                                                                             
 * │                             │                   │                                                                                                             
 * │                             │                   │                                                                                                             
 * └─────────────────────────────┴───────────────────┘                                                                                                             
*/


/* ================================================================================ */
/* ================== Global Variable that Memory Manager maintained ============== */
/* ================================================================================ */

/*  The base physical address of kernel space */
uint64_t _mm_krnl_space_phys_base_addr;
uint64_t _mm_krnl_space_phys_end;

/*  The base virtual address of kernel space */
uint64_t _mm_krnl_space_start;

/*  The default available size of kernel space in bytes */
/*  Actually, kernel during initialization doesn't need so much space */
uint64_t _mm_krnl_space_size;

/*  The base address of krnl image. It equals variable _mm_krnl_space_start on 
    default case. */
uint64_t _mm_krnl_image_start;

/*  The size of krnl image in bytes. */
uint64_t _mm_krnl_image_size;

/*  The number of available free pages  */
uint64_t _mm_available_free_pages;

/*  The heigest addressable physical address */
uint64_t _mm_highest_addressable_physical_addr;


/*  

    Current machine information passed by OS Loader. It may includes memory map structure, 
    ACPI tables, GOP frame buffer base addr and so on. The functions responsible for kernel
    initialization, especially memory initialization function is necessary to initlize kernel 
    based on this informations.

    The base address of variable _current_machine_info. It equals
    variable _mm_non_paged_pool_start. The earlier data will be copied to the
    start address of non paged pool when memory be initializing and it's
    16 KBytes aligned. 

*/
struct _LOADER_MACHINE_INFORMATION *_current_machine_info;

/*  The end address of _current_machine_info. */
uint64_t _machine_info_end;



// The start address of PML4
uint64_t _mm_pt_pml4_start;
// The start address of Page-Directory-Pointer-Table zone 
uint64_t _mm_pt_pdpt_pool_start;
// The start address of Page Directory Table zone
uint64_t _mm_pt_pd_pool_start;
// The start address of Page Table zone
uint64_t _mm_pt_pt_pool_start;



// The start address of PFN database
mmpfn *_mm_pfn_db_start;
// The physical start address of PFN database
uint64_t _mm_pfn_db_phys_start;
// The size of PFN database in bytes
uint64_t _mm_pfn_db_size;
// The end address of PFN database
uint64_t _mm_pfn_db_end;
// The physical end address of PFN database
uint64_t _mm_pfn_db_phys_end;

// The list head of zero physical pages linked list
mm_pfn_list _mm_zero_phys_page_list_head =
    {0, zero_page_list, -1, -1};

// The list head of free physical pages linked list
mm_pfn_list _mm_free_phys_page_list_head =
    {0, free_page_list, -1, -1};

// The list head of bad physical pages linked list
mm_pfn_list _mm_bad_phys_page_list_head =
    {0, bad_page_list, -1, -1};

// The list head of ROM physical pages linked list
mm_pfn_list _mm_rom_phys_page_list_head =
    {0, rom_page_list, -1, -1};

// linked list array about physical page
mm_pfn_list *_mm_phys_page_lists[NUMBER_OF_PHYSICAL_PAGE_LISTS] = {

    &_mm_zero_phys_page_list_head,

    &_mm_free_phys_page_list_head,

    &_mm_bad_phys_page_list_head,

    &_mm_rom_phys_page_list_head,

    // Reserved
    NULL,
    NULL,
    NULL,
    NULL,
};



// The start address of non paged pool
uint64_t _mm_non_paged_pool_start;

// The physical start address of non paged pool
uint64_t _mm_non_paged_pool_phys_start;

// The size of non paged pool in bytes.
uint64_t _mm_non_paged_pool_size;

// The end address of non paged pool.
uint64_t _mm_non_paged_pool_end;

// The physical end address of non paged pool.
uint64_t _mm_non_paged_pool_phys_end;


/*  

    Free list array of Non paged pool.

    Memory manager uses lists to manage non paged pool pages. By default, there have
4 (NON_PAGED_POOL_LIST_HEADS_MAXIMUM is defined by macros) lists. The 
_mm_non_paged_pool_free_list_array[0] contains all consecutive pages with a quantily 
greater than or equal to 4. And the _mm_non_paged_pool_free_list_array[1] includes 
all single pages. In the same way, the third item and fourth item also are linked list
and are similar to the second item. 

*/ 
pool_free_list_head 
_mm_non_paged_pool_free_list_array[NON_PAGED_POOL_LIST_HEADS_MAXIMUM] =
{
        {0, NULL, NULL},
        {0, NULL, NULL},
        {0, NULL, NULL},
        {0, NULL, NULL},
};

// The number of memory pool
uint16_t _mm_number_of_pool;

// The default memory pool for kernel
pool _mm_krnl_pool;

// The pointer that points to all memory pools.
pool* _mm_pools[12];




// The start address of system page table entry pool.
uint64_t _mm_sys_pte_pool_start;

// The physical start address of system page table entry pool.
uint64_t _mm_sys_pte_pool_phys_start;

// The size of system page table entry in bytes.
uint64_t _mm_sys_pte_pool_size;

// The end address of system page table entry pool.
uint64_t _mm_sys_pte_pool_end;

// The physical end address of system page table entry pool.
uint64_t _mm_sys_pte_pool_phys_end;

// System PTE bitmap
bitmap_t _pt_bitmap;



// The start address of reserved memory.
uint64_t _mm_reserved_mem_start;

// The physical start address of reserved memory.
uint64_t _mm_reserved_mem_phys_start;

// The size of reserved memory.
uint64_t _mm_reserved_mem_size;

// The end address of system page table entry pool.
uint64_t _mm_reserved_mem_end;

// The physical end address of system page table entry pool.
uint64_t _mm_reserved_mem_phys_end;



// The stack top of kernel initialization thread
uint64_t _mm_stack_top_of_initialization_thread;

// The size of stack of kernel initialization thread in bytes.
uint64_t _mm_size_of_stack_of_initialization_thread_in_bytes;

// The stack buttom of kernel initialization thread
uint64_t _mm_stack_buttom_of_initialization_thread;



/*  

Routine Description:

    The function flushs the specified address tlb using hardware instruction.

Parameters:

    vaddr - Specified virtual address.

Returned Value:

    None.

*/
void _mm_flush_tlb(uint64_t vaddr)
{
    /*  By specifying "memory" as a clobber, you tell the compiler that
        the inline assembly code modifies memory in some way, so it can
        take appropriate precautions to ensure that memory is not cached
        or optimized in a way that would interfere with the inline assembly code.  */
    __asm__ volatile("invlpg (%0)" ::"r"(vaddr)
                     : "memory");
}



/*  

    The offset of the first free page from the beginning of 
the system page table entry pool. 

*/
static uint64_t offset_of_first_free_sys_pte = 0;

/*

Routine Description:

    The routine adds 1 to the static variable offset_of_first_free_sys_pte then
returns the original value of variable offset_of_first_free_sys_pte.

Parameters:

    None.

Returned Value:

    Eealier value of variable offset_of_first_free_sys_pte before calling the routine.

*/
static uint64_t _mm_get_sys_pte_next_page()
{
    return ((offset_of_first_free_sys_pte++) << PAGE_SHIFT);
}




/*  
Routine Description:
    This function calculates the number of page directory pointer table entries (PDPTEs),
    page directory entries (PDEs), and page table entries (PTEs) required to map a
    memory region of specified size starting at a given base address.

    The calculation accounts for:
    1. The size of the memory region
    2. The base address alignment relative to different page table levels
    3. Potential crossing of boundaries at each page table level

Parameters:
    base - Starting virtual address of the memory region
    size - Size of the memory region in bytes
    p_num_of_pdptes - Output pointer for the number of PDPTEs required
    p_num_of_pdes - Output pointer for the number of PDEs required
    p_num_of_ptes - Output pointer for the number of PTEs required

Return Value:
    None
*/
static void calculate_num_of_ptes(uint64_t base, uint64_t size, uint64_t *p_num_of_pdptes,
                               uint64_t *p_num_of_pdes, uint64_t *p_num_of_ptes)
{
    uint64_t remainder;
    uint64_t offset;

    // Calculate number of PDPTEs required
    // Each PDPTE covers 1GB of address space (2^(PT_ENTRY_SHIFT*2 + PAGE_SHIFT))
    remainder = size & ((1UL << ((PT_ENTRY_SHIFT << 1) + PAGE_SHIFT)) - 1);
    if (remainder != 0)
    {
        // If size isn't aligned to 1GB boundary, need an extra PDPTE
        *p_num_of_pdptes = (size >> ((PT_ENTRY_SHIFT << 1) + PAGE_SHIFT)) + 1;
    }
    else
        // Size is aligned to 1GB boundary
        *p_num_of_pdptes = size >> ((PT_ENTRY_SHIFT << 1) + PAGE_SHIFT);

    // Calculate number of PDEs required
    // Each PDE covers 2MB of address space (2^(PT_ENTRY_SHIFT + PAGE_SHIFT))
    remainder = size & ((1UL << ((PT_ENTRY_SHIFT) + PAGE_SHIFT)) - 1);
    if (remainder != 0)
    {
        // If size isn't aligned to 2MB boundary, need an extra PDE
        *p_num_of_pdes = (size >> ((PT_ENTRY_SHIFT) + PAGE_SHIFT)) + 1;
    }
    else
        // Size is aligned to 2MB boundary
        *p_num_of_pdes = size >> ((PT_ENTRY_SHIFT) + PAGE_SHIFT);
    
    // Calculate offset of base address within the PDPT structure
    // This determines which PDPTE the region starts in
    offset = base >> ((PT_ENTRY_SHIFT) + PAGE_SHIFT);
    offset &= ((1UL << ((PT_ENTRY_SHIFT))) - 1);

    // Check if the region crosses a PDPTE boundary
    // If the combined offset and number of PDEs exceeds what can be contained
    // in the initially calculated PDPTEs, we need an additional PDPTE
    if ((offset + (*p_num_of_pdes)) > ((*p_num_of_pdptes) << PT_ENTRY_SHIFT))
    {
        *p_num_of_pdptes += 1;
    }

    // Calculate number of PTEs required
    // Each PTE covers 4KB of address space (2^PAGE_SHIFT)
    remainder = size & ((1UL << ((PAGE_SHIFT))) - 1);
    if (remainder != 0)
    {
        // If size isn't aligned to 4KB boundary, need an extra PTE
        *p_num_of_ptes = (size >> PAGE_SHIFT) + 1;
    }
    else
        // Size is aligned to 4KB boundary
        *p_num_of_ptes = size >> PAGE_SHIFT;

    // Calculate offset of base address within the PD structure
    // This determines which PDE the region starts in
    offset = base >> PAGE_SHIFT;
    offset &= ((1UL << ((PT_ENTRY_SHIFT))) - 1);
        
    // Check if the region crosses a PDE boundary
    // If the combined offset and number of PTEs exceeds what can be contained
    // in the initially calculated PDEs, we need an additional PDE
    if ((offset + (*p_num_of_ptes)) > ((*p_num_of_pdes) << PT_ENTRY_SHIFT))
    {
        *p_num_of_pdes += 1 ;
    }
}


/*

Routine Description:

    This routine maps a kernel memory zone to the specified virtual address space by setting up
    the necessary page directory pointer table entries (PDPTEs), page directory entries (PDEs),
    and page table entries (PTEs). It uses a system-allocated pool of page table entries for 
    creating the required page tables.

Parameters:

    pdpte_addr - Virtual address pointer to the start of the PDPT where entries should be created
    number_of_pdptes - Number of PDPTEs required for the mapping
    number_of_pdes - Number of PDEs required for the mapping
    number_of_ptes - Number of PTEs required for the mapping
    zone_addr - Physical base address of the memory zone to be mapped
    size - Size of the memory zone in bytes (not directly used in this function but provided for context)

Returned Value:

    Returns a pointer to the next available PDPTE after setting up all required entries for this zone.
    This allows the caller to continue mapping additional zones sequentially.

*/
static mmpte_hardware_t *map_zone(mmpte_hardware_t *pdpte_addr, uint64_t number_of_pdptes,
                                  uint64_t number_of_pdes, uint64_t number_of_ptes, uint64_t zone_addr, uint64_t size)
{
    /* Physical address of Page-Directory Table. */
    uint64_t pd_phys_addr;

    /* Physical address of Page Table. */
    uint64_t pt_phys_addr;

    /* Physical address of system page table entry pool. */
    uint64_t sys_pte_pool_phys_addr;

    /* Virtual address of system page table entry pool. */
    uint64_t sys_pte_pool_virt_addr;

    /* Offset of next page from the beginning of the sys pte pool. */
    uint64_t offset_of_sys_pte_next_page;

    /* Hardware pte pointers that point a page-directory and page-table respectively. */
    mmpte_hardware_t *pd, *pt;

    /* Counter of page table entry.*/
    uint32_t i;

    /* Initialize local variables with system page table pool addresses */
    sys_pte_pool_phys_addr = _mm_sys_pte_pool_phys_start;
    // Calculate virtual address of system PTE pool using the physical-to-virtual mapping offset
    sys_pte_pool_virt_addr = (_mm_sys_pte_pool_phys_start - _mm_krnl_space_phys_base_addr) + _mm_krnl_space_start;

    // Set up PDPTEs (Page Directory Pointer Table Entries)
    // Each PDPTE points to a Page Directory (PD)
    offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page(); // Get next available page from system PTE pool
    pd_phys_addr = sys_pte_pool_phys_addr + offset_of_sys_pte_next_page; // Physical address of the Page Directory
    pd = (mmpte_hardware_t *)(sys_pte_pool_virt_addr + offset_of_sys_pte_next_page); // Virtual address of the Page Directory

    i = 0;
    do
    {
        // Configure the PDPTE to point to the Page Directory
        pdpte_addr->present = 1;    // Mark entry as present
        pdpte_addr->rw = 1;         // Mark entry as read/write
        pdpte_addr->address = (pd_phys_addr >> PAGE_SHIFT); // Set physical address of PD (shifted by page size)

        i++;
        pdpte_addr++; // Move to next PDPTE slot

        // If we need more PDPTEs, allocate another Page Directory from system pool
        if (i < number_of_pdptes)
        {
            offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page();
            pd_phys_addr = sys_pte_pool_phys_addr + offset_of_sys_pte_next_page;
        }
    } while (i < number_of_pdptes);

    // Set up PDEs (Page Directory Entries)
    // Each PDE points to a Page Table (PT)
    offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page(); // Get next available page from system PTE pool
    pt_phys_addr = sys_pte_pool_phys_addr + offset_of_sys_pte_next_page; // Physical address of the Page Table
    pt = (mmpte_hardware_t *)(sys_pte_pool_virt_addr + offset_of_sys_pte_next_page); // Virtual address of the Page Table

    i = 0;
    do
    {
        // Configure the PDE to point to the Page Table
        pd[i].present = 1;    // Mark entry as present
        pd[i].rw = 1;         // Mark entry as read/write
        pd[i].address = (pt_phys_addr >> PAGE_SHIFT); // Set physical address of PT (shifted by page size)
        
        i++;
        
        // If we need more PDEs, allocate another Page Table from system pool
        if (i < number_of_pdes)
        {
            offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page();
            pt_phys_addr = sys_pte_pool_phys_addr + offset_of_sys_pte_next_page;
        }
    } while (i < number_of_pdes);

    // Set up PTEs (Page Table Entries)
    // Each PTE directly maps to a physical page in the zone
    i = 0;
    do
    {
        // Configure the PTE to point to the physical page in the zone
        pt[i].present = 1;    // Mark entry as present
        pt[i].rw = 1;         // Mark entry as read/write
        pt[i].address = (zone_addr >> PAGE_SHIFT); // Set physical address of the zone page (shifted by page size)
        
        zone_addr += 0x1000;  // Move to next 4KB page in the zone
        i++;
    } while (i < number_of_ptes);

    // Return pointer to the next available PDPTE for potential future mappings
    return pdpte_addr;
}

/*  

Routine Description:

    The routine initializes kernel virtual address space based on kernel space layout
variables be setted by mm_init() routine. The routine is used to divide kernel
space layout and to set on further kernel space layout variables(includes _pt_bitmap).

Parameters:

    None.

Returned Value:

    None.

*/
static void mem_map_init()
{

    // Physical address of PML4
    uint64_t pml4t_phys_addr;

    // Virtual address of PML4
    uint64_t pml4t_virt_addr;

    // Page-directory self-mapping scheme
    uint64_t offset_of_pml4e;

    // Number of pml4 entries.
    uint16_t num_of_pml4tes;


    // Physical address of PDPT
    uint64_t krnl_pdpt_phys_addr;

    // Virtual address of PDPT
    uint64_t pdpt_virt_addr;


    /* Physical address of system page table entry pool. */
    uint64_t sys_pte_pool_phys_addr;

    /* Virtual address of system page table entry pool. */
    uint64_t sys_pte_pool_virt_addr;

    /* Offset of next page from the beginning of the sys pte pool. */
    uint64_t offset_of_sys_pte_next_page;

    uint64_t zone_addr;
    uint64_t zone_size;
    uint64_t num_of_pdptes;
    uint64_t num_of_pdes;
    uint64_t num_of_ptes;

    mmpte_hardware_t *pml4t, *krnl_pdpt;
    uint64_t random_num;

    // cr3_t register
    uint64_t cr3;

    // get a random number
    random_num = rdtsc();

    // Initilize local variables.
    sys_pte_pool_phys_addr = _mm_sys_pte_pool_phys_start;

    // The desired virtual address of the system page table entry pool.
    sys_pte_pool_virt_addr = 
        (_mm_sys_pte_pool_phys_start - _mm_krnl_space_phys_base_addr) + _mm_krnl_space_start;


    // The first page of system pte pool holds kernel PML4 table.
    offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page();
    pml4t_phys_addr = sys_pte_pool_phys_addr + offset_of_sys_pte_next_page;
    pml4t_virt_addr = sys_pte_pool_virt_addr + offset_of_sys_pte_next_page;

    // The second page of system pte pool holds kernel PDPT table.
    offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page();
    krnl_pdpt_phys_addr = sys_pte_pool_phys_addr + offset_of_sys_pte_next_page;
    pdpt_virt_addr = sys_pte_pool_virt_addr + offset_of_sys_pte_next_page;

    // the numerical range of offset_of_pml4e is 0x100 - PML4E_OFFSET_OF_KRNL_SPACE
    offset_of_pml4e = ((random_num
        + _current_machine_info->memory_space_info[0].base_address
        + _current_machine_info->memory_space_info[1].base_address) % (PML4E_OFFSET_OF_KRNL_SPACE - 0x100) + 0x100);

    

    // Calculate the number of PML4 entries that mapping kernel space needed.
    if (_mm_krnl_space_size & (PML4_SIZE - 1))
        // When the condition is met, it means the kernel space size is not aligned to PML4_SIZE.
        // That's means we need to add one more PML4 entry. The practice also applies 
        // to when kernel space size lesses PML4_SIZE, in this situation, we only need an extra pml4 entry.
        num_of_pml4tes = 1 + (_mm_krnl_space_size >> PML4_SHIFT);
    else
        num_of_pml4tes = _mm_krnl_space_size >> PML4_SHIFT;


    // The size of kernel space can not greater than 512 GBytes.
    // Because the maximum offset of pml4e is PML4E_OFFSET_OF_KRNL_SPACE.
    assert(num_of_pml4tes <= (PML4E_OFFSET_OF_HARDWARE - PML4E_OFFSET_OF_KRNL_SPACE));


    // Set PML4
    pml4t = (mmpte_hardware_t *)pml4t_virt_addr;

    /* For Page-Directory Self-Mapping Scheme.*/
    pml4t[offset_of_pml4e].present = 1;
    pml4t[offset_of_pml4e].rw = 1;
    pml4t[offset_of_pml4e].address = ((uint64_t)pml4t_phys_addr >> PAGE_SHIFT);

    /* Set up the pml4 entry for kernel space. */
    pml4t[PML4E_OFFSET_OF_KRNL_SPACE].present = 1;
    pml4t[PML4E_OFFSET_OF_KRNL_SPACE].rw = 1;
    pml4t[PML4E_OFFSET_OF_KRNL_SPACE].address = (((uint64_t)krnl_pdpt_phys_addr) >> PAGE_SHIFT);

    krnl_pdpt = (mmpte_hardware_t *)pdpt_virt_addr;



    // Page directory self-mapping scheme.
    _mm_pt_pml4_start = 0xFFFF000000000000 | 
        (offset_of_pml4e << 12) | 
        (offset_of_pml4e << 21) | 
        (offset_of_pml4e << 30) | 
        (offset_of_pml4e << 39);

    _mm_pt_pdpt_pool_start = 0xFFFF000000000000 | 
        (offset_of_pml4e << 21) | 
        (offset_of_pml4e << 30) | 
        (offset_of_pml4e << 39);

    _mm_pt_pd_pool_start = 0xFFFF000000000000 | 
        (offset_of_pml4e << 30) | 
        (offset_of_pml4e << 39);

    _mm_pt_pt_pool_start = 0xFFFF000000000000 | 
        (offset_of_pml4e << 39);





    // Map krnl image
    zone_addr = _mm_krnl_space_phys_base_addr;
    zone_size = _mm_pfn_db_phys_start - _mm_krnl_space_phys_base_addr;

    calculate_num_of_ptes(zone_addr, zone_size, &num_of_pdptes, 
        &num_of_pdes, &num_of_ptes);

    krnl_pdpt = map_zone(krnl_pdpt, num_of_pdptes, num_of_pdes,
        num_of_ptes, zone_addr, zone_size);
    


    // Upate MM variables
    _mm_pfn_db_start = (mmpfn*)(_mm_krnl_space_start + ((num_of_pdptes << PDPT_SHIFT)));
    _mm_pfn_db_end = _mm_pfn_db_size + (uint64_t)_mm_pfn_db_start;

    // Map PFN database
    zone_addr = _mm_pfn_db_phys_start;
    zone_size = _mm_non_paged_pool_phys_start - _mm_pfn_db_phys_start;

    calculate_num_of_ptes(zone_addr, zone_size, &num_of_pdptes, 
        &num_of_pdes, &num_of_ptes);

    krnl_pdpt = map_zone(krnl_pdpt, num_of_pdptes, num_of_pdes,
        num_of_ptes, zone_addr, zone_size);



    // Upate MM variables
    _mm_non_paged_pool_start = (uint64_t)(_mm_pfn_db_start + ((num_of_pdptes) << PDPT_SHIFT));
    _mm_non_paged_pool_end = _mm_non_paged_pool_size + _mm_non_paged_pool_start;

    // Map non paged pool
    zone_addr = _mm_non_paged_pool_phys_start;
    zone_size = _mm_sys_pte_pool_phys_start - _mm_non_paged_pool_phys_start;

    calculate_num_of_ptes(zone_addr, zone_size, &num_of_pdptes, 
        &num_of_pdes, &num_of_ptes);

    krnl_pdpt = map_zone(krnl_pdpt, num_of_pdptes, num_of_pdes,
        num_of_ptes, zone_addr, zone_size);
    


    // Upate MM variables
    _mm_sys_pte_pool_start = (uint64_t)(_mm_non_paged_pool_start + (num_of_pdptes << PDPT_SHIFT));
    _mm_sys_pte_pool_end = _mm_sys_pte_pool_size + _mm_sys_pte_pool_start;

    // Map system page table entry pool 
    zone_addr = _mm_sys_pte_pool_phys_start;
    zone_size = _mm_sys_pte_pool_phys_end - _mm_sys_pte_pool_phys_start;

    calculate_num_of_ptes(zone_addr, zone_size, &num_of_pdptes, 
        &num_of_pdes, &num_of_ptes);

    krnl_pdpt = map_zone(krnl_pdpt, num_of_pdptes, num_of_pdes,
        num_of_ptes, zone_addr, zone_size);

    

    // Map FrameBuffer
    offset_of_sys_pte_next_page = _mm_get_sys_pte_next_page();
    krnl_pdpt = (mmpte_hardware_t*)(sys_pte_pool_virt_addr + offset_of_sys_pte_next_page);

    // Set PML4
    pml4t[PML4E_OFFSET_OF_FRAMEBUFFER].present = 1;
    pml4t[PML4E_OFFSET_OF_FRAMEBUFFER].rw = 1;
    pml4t[PML4E_OFFSET_OF_FRAMEBUFFER].address = 
        ((sys_pte_pool_phys_addr + offset_of_sys_pte_next_page) >> PAGE_SHIFT);
        
    zone_addr = _current_machine_info->graphics_info.FrameBufferBase;
    zone_size = _current_machine_info->graphics_info.FrameBufferSize;

    calculate_num_of_ptes(zone_addr, zone_size, &num_of_pdptes, 
        &num_of_pdes, &num_of_ptes);

    krnl_pdpt = map_zone(krnl_pdpt, num_of_pdptes, num_of_pdes,
        num_of_ptes, zone_addr, zone_size);



    // Update machine_info
    _current_machine_info->graphics_info.FrameBufferBase = 0xFFFF000000000000UL |
        (PML4E_OFFSET_OF_FRAMEBUFFER << PML4_SHIFT);

    _current_machine_info = (struct _LOADER_MACHINE_INFORMATION *)_mm_non_paged_pool_start;
    _machine_info_end = _mm_non_paged_pool_start + 0x10000;


    // update cr3 
    __get_cr3(cr3);
    cr3 &= 0xfff0000000000fff;
    cr3 |= (uint64_t)pml4t_phys_addr;
    __write_cr3(cr3);

}




/*

Routine Description:

    The routine initializes PFN database. Its work is just walks the EFI_MEMORY_DESCRIPTOR
structure array checks memory type, then sets the mmpfn::state member and adds to 
corresponding linked list.

Parameters:

    _pt_bitmap.offset = offset_of_first_free_sys_pte;

    None.

Returned Value:

    None.

*/
static void pfn_init()
{
    uint64_t i_mem_info;
    uint64_t mem_info_count;

    // the fist page address of physical address kernel space 
    uint64_t krnl_space_start_page;
    // the last page address of physical address kernel space 
    uint64_t krnl_space_end_page;

    pfn_number i_pfn_element;

    krnl_space_start_page = _mm_krnl_space_phys_base_addr >> PAGE_SHIFT;

    // the last page address is equal to the end address of system pte pool 
    krnl_space_end_page = _mm_sys_pte_pool_phys_end >> PAGE_SHIFT;


    // efi memory descriptor counter
    mem_info_count = _current_machine_info->memory_info.efi_mem_desc_count;


    // walk the efi memory descriptor array.
    for (i_mem_info = 0; i_mem_info < mem_info_count; i_mem_info++)
    {
        EFI_MEMORY_DESCRIPTOR *desc = &_current_machine_info->memory_info.efi_mem_desc[i_mem_info];
        uint64_t phys_page_start = desc->PhysicalStart >> PAGE_SHIFT;
        switch (desc->Type)
        {
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:

        /*  contiguous free physical pages list */
        {
            for (i_pfn_element = phys_page_start;
                 i_pfn_element < (desc->NumberOfPages + phys_page_start);
                 i_pfn_element++)
            {

                _mm_pfn_db_start[i_pfn_element].state.page_state = free_page_list;

                if (_mm_free_phys_page_list_head.flink == -1)
                // This breach corresponds to the first time a member is added to the linked list.
                {
                    _mm_free_phys_page_list_head.flink = _mm_free_phys_page_list_head.blink = i_pfn_element;
                }

                else
                // add i_pfn_element to list.
                {

                    _mm_pfn_db_start[_mm_free_phys_page_list_head.blink].u1.flink = i_pfn_element;
                    _mm_pfn_db_start[i_pfn_element].u2.blink = _mm_free_phys_page_list_head.blink;

                    // the list rear points current item(i_pfn_element).
                    _mm_free_phys_page_list_head.blink = i_pfn_element;

                }

                // update the total number of list members.
                _mm_free_phys_page_list_head.total++;
            }
        }
        break;

        case EfiConventionalMemory:
        case EfiPersistentMemory:


        /*  contiguous free physical page list & contiguous active physical page list */
        {
            for (i_pfn_element = phys_page_start;
                 i_pfn_element < (desc->NumberOfPages + phys_page_start);
                 i_pfn_element++)
            {
                // Since we allocated a contiguous free physical memory space during os loading. Hence, we need to set
                // pfn items of the allocated memory space to active state, then fill corresponding attributes
                if (i_pfn_element >= krnl_space_start_page && i_pfn_element < krnl_space_end_page)
                {
                    _mm_pfn_db_start[i_pfn_element].state.page_state = active_and_valid;
                    _mm_pfn_db_start[i_pfn_element].pte_addr =
                        __get_pte_by_virt_addr((_mm_krnl_space_start + ((i_pfn_element - krnl_space_start_page) << 12)));
                    _mm_pfn_db_start[i_pfn_element].u2.share_count++;
                }
                else
                {
                    _mm_pfn_db_start[i_pfn_element].state.page_state = free_page_list;

                    if (_mm_free_phys_page_list_head.flink == -1)
                    {
                        _mm_free_phys_page_list_head.flink = _mm_free_phys_page_list_head.blink = i_pfn_element;
                    }
                    else
                    {
                        _mm_pfn_db_start[_mm_free_phys_page_list_head.blink].u1.flink = i_pfn_element;
                        _mm_pfn_db_start[i_pfn_element].u2.blink = _mm_free_phys_page_list_head.blink;
                        _mm_free_phys_page_list_head.blink = i_pfn_element;
                    }
                    _mm_free_phys_page_list_head.total++;
                }
            }
        }
        break;
        case EfiReservedMemoryType:
        case EfiUnusableMemory:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
        case EfiUnacceptedMemoryType:
        case EfiMaxMemoryType:
            break;

        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS:
        
        /*  ROM physical consecutive pages list */
        {
            for (i_pfn_element = phys_page_start;
                 i_pfn_element < (desc->NumberOfPages + phys_page_start);
                 i_pfn_element++)
            {

                _mm_pfn_db_start[i_pfn_element].state.page_state = rom_page_list;

                if (_mm_rom_phys_page_list_head.flink == -1)
                {
                    _mm_rom_phys_page_list_head.flink = _mm_rom_phys_page_list_head.blink = i_pfn_element;
                }
                else
                {
                    _mm_pfn_db_start[_mm_rom_phys_page_list_head.blink].u1.flink = i_pfn_element;
                    _mm_pfn_db_start[i_pfn_element].u2.blink = _mm_rom_phys_page_list_head.blink;
                    _mm_rom_phys_page_list_head.blink = i_pfn_element;
                }
                _mm_rom_phys_page_list_head.total++;
            }
        }
        break;

        default:
            break;
        }
    }

    // Set PFN items of machine information data space to assist dynamic memory page manager to free the
    // memory space it occupies.
    mmpfn *_loader_data_pfn = &_mm_pfn_db_start[(_mm_non_paged_pool_phys_start >> PAGE_SHIFT)];
    _loader_data_pfn->state.read_in_process = 1;
    (_loader_data_pfn + (MACHINE_INFO_SIZE >> PAGE_SHIFT))->state.write_in_process = 1;
}




/*

Routine Description:

    The routine initializes the memory pool for memory alloc&free functions.

Parameters:

    None.

Returned Value:

    None.

*/
static void memory_pool_init()
{
    uint64_t non_paged_pool;
    pool_free_page_entry *free_root_entry;

    // default kernel memory pool
    pool *pool;

    _mm_pools[POOL_INDEX_KERNEL_DEFAULT] = &_mm_krnl_pool;

    non_paged_pool = _mm_non_paged_pool_start + 0x10000;
    free_root_entry = (pool_free_page_entry *)(non_paged_pool);
    free_root_entry->number_of_pages = (_mm_non_paged_pool_size - 0x10000) >> PAGE_SHIFT;

    for (non_paged_pool += PAGE_SIZE; non_paged_pool < _mm_non_paged_pool_end; non_paged_pool += PAGE_SIZE)
    {
        pool_free_page_entry *free_entry = (pool_free_page_entry *)non_paged_pool;
        free_entry->owner = free_root_entry;
    }
    _mm_non_paged_pool_free_list_array[0].total = 1;
    _mm_non_paged_pool_free_list_array[0].front = free_root_entry;
    _mm_non_paged_pool_free_list_array[0].rear = free_root_entry;

    // initialize the default kernel memory pool
    pool = _mm_pools[POOL_INDEX_KERNEL_DEFAULT];
    pool->pool_index = POOL_INDEX_KERNEL_DEFAULT;
    pool->pool_type = 0;
    memzero(&pool->list_heads, sizeof(list_node_t) * POOL_LIST_HEADS);
}

/*

Routine Description:

    The routine initializes the system page table entry pool. We use a bitmap 
to manage the free system page table entries. We already used some ptes
(use offset_of_first_free_sys_pte to record) before we initializing system pte pool.
The reason is what we have not initialized the kernel memory allocator, we can't
allocate memory for bitmap initialization. In this routine, we store the bitmap
into non paged pool and set a portion of the pages at the beginning of sys pte pool
as used(mark these bits to 1).
    

Parameters:

    None.

Returned Value:

    None.

*/
static void sys_pte_pool_init()
{
    _bitmap_init(&_pt_bitmap, _mm_sys_pte_pool_size);
    _pt_bitmap.offset = offset_of_first_free_sys_pte;

    int bytes = offset_of_first_free_sys_pte >> ITEM_SHIFT;
    memset(_pt_bitmap.bits, 0xff, bytes);

    int bits = offset_of_first_free_sys_pte % ITEM_BITS;
    for (int i = 0; i < bits; i++) 
    {
        _pt_bitmap.bits[bytes] |= (ITEM)(1UL << ((ITEM_BITS - 1) - i));
    }

}



/*  

Routine Description:

    The routine maps physical pages that stores hardware information into virtual 
address space, then sets the PFN item corresponding to these physical pages and fixes
up the member of _current_machine_info that relates to these physical pages.

Parameters:

    None.

Returned Value:

    None.

*/

extern mmpte_hardware_t* hardware_base_pdpt;
extern mmpte_hardware_t* hardware_base_pd;
extern mmpte_hardware_t* hardware_base_pt;
extern uint64_t _mm_offset_of_hardware_pt;

static void map_rom_zone()
{
    uint64_t number_of_pdptes;
    uint64_t number_of_pdes;
    uint64_t number_of_ptes;


    mm_addr_t new_pt_addr;

    mmpte_hardware_t* pdpt, *pd, *pt;

    uint64_t pfn_index;

    // Counter
    uint64_t i;

    calculate_num_of_ptes(0,
        (_mm_phys_page_lists[3]->total << PAGE_SHIFT),
        &number_of_pdptes, &number_of_pdes, &number_of_ptes);
    
    // Allocate a page table for hardware pdpt.
    new_pt_addr = _pt_alloc();
    hardware_base_pdpt = pdpt = (mmpte_hardware_t*)new_pt_addr.virt_addr;

    // Set Hardware entry in PML4.
    ((mmpte_hardware_t*)_mm_pt_pml4_start)[PML4E_OFFSET_OF_HARDWARE].present = 1;
    ((mmpte_hardware_t*)_mm_pt_pml4_start)[PML4E_OFFSET_OF_HARDWARE].rw = 1;
    ((mmpte_hardware_t*)_mm_pt_pml4_start)[PML4E_OFFSET_OF_HARDWARE].address = 
        ((new_pt_addr.phys_addr) >> PAGE_SHIFT);
        
    // Allocate new page table for PD.
    new_pt_addr = _pt_alloc();
    hardware_base_pd = pd = (mmpte_hardware_t*)(new_pt_addr.virt_addr);

    i = 0;
    do
    {
        pdpt[i].present = 1;
        pdpt[i].rw = 1;
        pdpt[i].address = (new_pt_addr.phys_addr >> PAGE_SHIFT);

        new_pt_addr = _pt_alloc();

        i++;

    } while (i < number_of_pdptes);

    new_pt_addr = _pt_alloc();
    hardware_base_pt = pt = (mmpte_hardware_t*)(new_pt_addr.virt_addr);

    i = 0;
    do
    {
        pd[i].present = 1;
        pd[i].rw = 1;
        pd[i].address = (new_pt_addr.phys_addr >> PAGE_SHIFT);

        new_pt_addr = _pt_alloc();

        i++;

    } while (i < number_of_pdes);
    
    pfn_index = _mm_phys_page_lists[3]->flink;
    for (i = 0; i < _mm_phys_page_lists[3]->total; i++)
    {
        _mm_pfn_db_start[pfn_index].original_pte.pte.LongPtr = 0xFFFF000000000000UL |
            (i << 12) |
            (PML4E_OFFSET_OF_HARDWARE << 39);
        pt[i].present = 1;

        // Read-only
        pt[i].rw = 0;

        pt[i].address = pfn_index;
        _mm_flush_tlb(_mm_pfn_db_start[pfn_index].original_pte.pte.LongPtr);
        pfn_index = _mm_pfn_db_start[pfn_index].u1.flink;
    }

    _mm_offset_of_hardware_pt = i;
}

/*  

Routine Description:

    The routine maps application processors startup routine into corresponding virtual address
space. The bootstrap processor will save datas that application processors needed in the space.
The address of the memory space is located by UEFI allocated(The default address is located at 
0x1000).

Parameters:

    None.

Returned Value:

    None.

*/

void unmap_startup_routine(uint64_t startup_addr, uint64_t number_of_pages)
{
    mmpte_hardware_t *startup_pml4_entry;
    startup_pml4_entry = (mmpte_hardware_t*)(_mm_pt_pml4_start);

#ifdef REMOVE_TMP_PT

    mmpte_hardware_t *startup_pdpt = (mmpte_hardware_t*)(_pt_get_virt_addr_of_pt(startup_pml4_entry->address << PAGE_SHIFT));
    startup_pml4_entry->present = 0;
    startup_pml4_entry->rw = 0;
    startup_pml4_entry->address = 0;


    mmpte_hardware_t *startup_pd = (mmpte_hardware_t*)(_pt_get_virt_addr_of_pt(startup_pdpt->address << PAGE_SHIFT));
    startup_pdpt->present = 0;
    startup_pdpt->rw = 0;
    startup_pdpt->address = 0;

    mmpte_hardware_t *startup_pt = (mmpte_hardware_t*)(_pt_get_virt_addr_of_pt(startup_pd->address << PAGE_SHIFT));
    

    mm_addr_t pt_addr = { 0 };

    pt_addr.virt_addr = (uint64_t)startup_pt;
    _pt_free(pt_addr);
    pt_addr.virt_addr = (uint64_t)startup_pd;
    _pt_free(pt_addr);
    pt_addr.virt_addr = (uint64_t)startup_pdpt;
    _pt_free(pt_addr);


#else
    startup_pml4_entry->present = 0;
    startup_pml4_entry->rw = 0;
    startup_pml4_entry->address = 0;
#endif
}

void mapping_startup_routine(uint64_t startup_addr, uint64_t number_of_pages)
{

    /*
     * Since we have constructed a temporary pml4 entry during ccloader
     * initialization. In here, hence, we only need to do one thing is set
     * the first entry of our kernel pml4 table.
     */

    mmpte_hardware_t *startup_pml4_entry;
    startup_pml4_entry = (mmpte_hardware_t*)(_mm_pt_pml4_start);

#ifdef REMOVE_TMP_PT
    mm_addr_t new_pt_addr;
    uint64_t vpage = startup_addr >> PAGE_SHIFT;
    mmpte_hardware_t *startup_pdpt;
    mmpte_hardware_t *startup_pd;
    mmpte_hardware_t *startup_pt;


    new_pt_addr = _pt_alloc();
    startup_pdpt = (mmpte_hardware_t*)(new_pt_addr.virt_addr);

    startup_pml4_entry->present = 1;
    startup_pml4_entry->rw = 1;
    startup_pml4_entry->address = (new_pt_addr.phys_addr) >> PAGE_SHIFT;

    new_pt_addr = _pt_alloc();
    startup_pd = (mmpte_hardware_t*)(new_pt_addr.virt_addr);

    startup_pdpt[0].present = 1;
    startup_pdpt[0].rw = 1;
    startup_pdpt[0].address = (new_pt_addr.phys_addr) >> PAGE_SHIFT;

    new_pt_addr = _pt_alloc();
    startup_pt = (mmpte_hardware_t*)(new_pt_addr.virt_addr);

    startup_pd[0].present = 1;
    startup_pd[0].rw = 1;
    startup_pd[0].address = (new_pt_addr.phys_addr) >> PAGE_SHIFT;

    for (int i = 0; i < number_of_pages; i++) 
    {
        startup_pt[vpage + i].present = 1;
        startup_pt[vpage + i].rw = 1;
        startup_pt[vpage + i].address = vpage + i;

        _mm_flush_tlb(startup_addr + (i << PAGE_SHIFT));
    }
#else
    startup_pml4_entry->present = 1;
    startup_pml4_entry->rw = 1;
    startup_pml4_entry->address = (startup_addr + 0x2000) >> PAGE_SHIFT;
#endif

}


uint64_t fixup_acpi_table_addr(uint64_t phys_addr)
{
    return (_mm_pfn_db_start[(phys_addr >> PAGE_SHIFT)].original_pte.pte.LongPtr + (phys_addr & (PAGE_SIZE - 1)));
}




static void dynamic_memory_management_test()
{
    int64_t *alloc0;
    int64_t *alloc1;
    int64_t *alloc2;
    int64_t *alloc3;
    int64_t *alloc4;
    int64_t *alloc5;
    int64_t *alloc6;
    int64_t *alloc7;
    int64_t *alloc8;
    int64_t *alloc9;
    status_t status;

    status = _mm_alloc_pages((5 << PAGE_SHIFT), (void **)&alloc0);
    if (ST_ERROR(status))
    {
        krnl_panic(NULL);
    }

    _mm_free_pages(alloc0);

    status = _mm_alloc_pages((15 << PAGE_SHIFT), (void **)&alloc0);
    if (ST_ERROR(status))
    {
        krnl_panic(NULL);
    }

    status = _mm_alloc_pages((8 << PAGE_SHIFT), (void **)&alloc1);
    if (ST_ERROR(status))
    {
        krnl_panic(NULL);
    }
    status = _mm_alloc_pages((1 << PAGE_SHIFT), (void **)&alloc2);
    if (ST_ERROR(status))
    {
        krnl_panic(NULL);
    }

    _mm_free_pages(alloc1);
    _mm_free_pages(alloc0);

    status = _mm_alloc_pages((39 << PAGE_SHIFT), (void **)&alloc3);
    if (ST_ERROR(status))
    {
        krnl_panic(NULL);
    }
    status = _mm_alloc_pages((28 << PAGE_SHIFT), (void **)&alloc4);
    if (ST_ERROR(status))
    {
        krnl_panic(NULL);
    }
    _mm_free_pages(alloc2);
    _mm_free_pages(alloc4);
    _mm_free_pages(alloc3);


    for (int i = 0; i < 1000; i++) {
    

    alloc0 = _mm_kmalloc(16);
    alloc1 = _mm_kmalloc(16);
    alloc1[0] = 0x123456789abcdef0;
    alloc1[1] = 0x123456789abcdef0;
    _mm_kfree(alloc1);
    alloc2 = _mm_kmalloc(16);
    alloc2[1] = 0x123456789abcdef0;
    alloc2[0] = 0x123456789abcdef0;
    _mm_kfree(alloc0);
    _mm_kfree(alloc2);

    status = _mm_alloc_pages(1 << PAGE_SHIFT, (void**)&alloc0);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }
    status = _mm_alloc_pages(1 << PAGE_SHIFT, (void**)&alloc1);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }
    status = _mm_alloc_pages(1 << PAGE_SHIFT, (void**)&alloc2);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }


    alloc0 = _mm_kmalloc_tag(8, '0D00');
    _mm_kfree(alloc0);

    // case 1: 
    alloc0 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc1 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc2 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc3 = _mm_kmalloc(sizeof(int64_t) * 8);

    _mm_kfree(alloc2);
    _mm_kfree(alloc1);
    _mm_kfree(alloc0);
    _mm_kfree(alloc3);

    // case 2: 
    alloc0 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc1 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc2 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc3 = _mm_kmalloc(sizeof(int64_t) * 8);
    _mm_kfree(alloc0);
    _mm_kfree(alloc3);
    _mm_kfree(alloc1);
    _mm_kfree(alloc2);

    alloc1 = _mm_kmalloc(sizeof(int64_t) * 8);
    alloc2 = _mm_kmalloc(sizeof(int64_t) * 18);
    alloc3 = _mm_kmalloc(sizeof(int64_t) * 2);
    alloc4 = _mm_kmalloc(sizeof(int64_t) * 3);
    alloc5 = _mm_kmalloc(sizeof(int64_t) * 3);
    alloc6 = _mm_kmalloc(sizeof(int64_t) * 25);
    _mm_kfree(alloc4);
    _mm_kfree(alloc3);
    alloc7 = _mm_kmalloc(sizeof(int64_t) * 100);
    _mm_kfree(alloc1);
    alloc8 = _mm_kmalloc(sizeof(int64_t) * 1024);
    _mm_kfree(alloc2);
    alloc9 = _mm_kmalloc(sizeof(int64_t) * 97);
    _mm_kfree(alloc5);
    _mm_kfree(alloc6);
    _mm_kfree(alloc7);
    _mm_kfree(alloc8);
    _mm_kfree(alloc9);

    }
    alloc0 = _mm_kmalloc(POOL_BUDDY_MAX);
    _mm_kfree(alloc0);

    void* alloc[POOL_BUDDY_MAX];
    for (int i = 0; i < 1000; i++) {
        alloc[i] = malloc(i);
    }
    for (int i = 0; i < 1000; i++) {
        _mm_kfree(alloc[i]);
    }

    // void* alloc[1000];
    // for (int i = 0; i < 1000; i++) {
    //     alloc[i] = _mm_kmalloc(13);
    // }
    // for (int i = 0; i < 1000; i++) {
    //     _mm_kfree(alloc[i]);
    // }
    

}


/*

Routine Description:

    The routine fixes up all address that points a file information in _current_machine_info.
These addresses will correctly points file information in virtual address space.

Parameters:

    None.

Returned Value:

    None.


*/
static inline void fixup_machine_info()
{
    uint64_t fixup = KERNEL_SPACE_BASE_ADDR - _mm_krnl_space_phys_base_addr;
    
    _current_machine_info->font[0].ttf_addr += fixup;
    _current_machine_info->font[1].ttf_addr += fixup;

    _current_machine_info->bg.addr += fixup;

}

/*  

Routine Descrption:

    The function divides kernel space layout and updates global variables at first. 
And the second, it will build new kernel space page table which mapping PFN database, 
non paged pool, system pte pool and hardware informations. At the end, It executes PFN 
initialization, Non Paged Pool initiazation and System Pte Pool Initializaton separately.

Parameters:

    None.

Returned Value:

    None.

*/
void mm_init()
{
    // The address to which _current_machine_info will be copied 
    uint64_t loader_data_start;

    // Set variables about kernel space
    _mm_krnl_space_phys_base_addr = 
        _current_machine_info->memory_space_info[0].base_address;

    _mm_krnl_space_phys_end = 
        _mm_krnl_space_phys_base_addr + _current_machine_info->memory_space_info[0].size;

    _mm_krnl_space_start = KERNEL_SPACE_BASE_ADDR;

    _mm_krnl_space_size = _current_machine_info->memory_space_info[0].size;





    // Set variables about krnl image
    _mm_krnl_space_phys_base_addr = 
        _current_machine_info->memory_space_info[0].base_address;

    _mm_krnl_image_start = _mm_krnl_space_start;

    _mm_krnl_image_size = _current_machine_info->memory_space_info[2].size;

    fixup_machine_info();

    // Calculate the number of available free pages
    _mm_available_free_pages = _current_machine_info->memory_info.ram_size >> PAGE_SHIFT;
    
    // Calculate the heigest addressable physical address on current machine
    _mm_highest_addressable_physical_addr = 
        _current_machine_info->memory_info.highest_physical_addr + PAGE_SIZE - 1;



    // Calculate and set variables about PFN Database
    _mm_pfn_db_phys_start = (_mm_stack_top_of_initialization_thread - _mm_krnl_space_start) + 
        _mm_krnl_space_phys_base_addr;

    _mm_pfn_db_size = 
        (((_current_machine_info->memory_info.highest_physical_addr >> PAGE_SHIFT) + 1) * PFN_ITEM_SIZE);

    _mm_pfn_db_phys_end = _mm_pfn_db_phys_start + _mm_pfn_db_size;



    // Calculate and set variables about Non paged pool 
    _mm_non_paged_pool_phys_start = page_aligned(_mm_pfn_db_phys_end);
    
    /*
     * The size of the memory pool is half of the _mm_krnl_space_size.
     */
    _mm_non_paged_pool_size = _mm_krnl_space_size >> 1;

    _mm_non_paged_pool_phys_end = 
        _mm_non_paged_pool_phys_start + _mm_non_paged_pool_size;



    // Copy the data about current machine information stored in low address
    // space to kernel. It will be copied to the beginning of non paged pool 
    // on default case.
    loader_data_start = _mm_non_paged_pool_phys_start - 
        _mm_krnl_space_phys_base_addr + _mm_krnl_space_start;

    memcpy((void *)(loader_data_start), _current_machine_info, MACHINE_INFO_SIZE);

    // Reset _current_machine_info
    _current_machine_info = (struct _LOADER_MACHINE_INFORMATION *)(loader_data_start);

    _machine_info_end = (uint64_t)_current_machine_info + MACHINE_INFO_SIZE;



    // Calculate and set variables about system page table entry pool 
    _mm_sys_pte_pool_phys_start = page_aligned(_mm_non_paged_pool_phys_end);

    /*
     * The size of sys pte is one fourth of the _mm_krnl_space_size.
     */
    _mm_sys_pte_pool_size = _mm_krnl_space_size >> 2;

    _mm_sys_pte_pool_phys_end = page_aligned(_mm_sys_pte_pool_phys_start + 
        _mm_sys_pte_pool_size);


    _mm_reserved_mem_phys_start = page_aligned(_mm_sys_pte_pool_phys_end);
    _mm_reserved_mem_size = _mm_krnl_space_size - (_mm_sys_pte_pool_phys_end - _mm_krnl_space_phys_base_addr);
    _mm_reserved_mem_phys_end = page_aligned(_mm_krnl_space_phys_base_addr + _mm_krnl_space_size);


    // Build new kernel page table
    mem_map_init();

    // PFN initialization
    pfn_init();

    // Memory Pool initialization
    memory_pool_init();
    // Dynamic Memory Management Functions test
    // dynamic_memory_management_test();

    // System page table initialization 
    sys_pte_pool_init();

    // Map ROM Zone
    map_rom_zone();

    mapping_startup_routine(
        _current_machine_info->memory_space_info[3].base_address, 
        _current_machine_info->memory_space_info[3].size >> PAGE_SHIFT
    );

}
