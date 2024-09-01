/*

@file: mm.h

Definitions for memory manager.

*/



#ifndef __X86_64_MM_MM_H__
#define __X86_64_MM_MM_H__

#include "../../../include/types.h"
#include "../../../include/libk/list.h"
#include "../../../include/machine_info.h"

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



// The base virtual address of kernel space. It's also the base address of krnl image.
#define KERNEL_SPACE_BASE_ADDR                              0xFFFFF00000000000UL

// The default size of kernel initialization thread.
#define KERNEL_STACK_DEFAULT_SIZE                           0x100000        


// The macro is used to set rsp register using inline assembly supported by GNU compiler.
#define set_krnl_stack(rsp) \
    __asm__( \
        "movq %0, %%rsp\n\t" \
        : : "r"(rsp) :  \
    )




// The macro indicates the size of page frame in bytes. At default, it's 4 KBytes.
// Kernel uses 4-level page table mapping scheme to manage all RAM resources.
#define PAGE_SIZE                                           0x1000

// The macro is used to declare how many bits are needed to represend a page frame.
// By shifting the macro, it can replace the multipication or division of page size.
#define PAGE_SHIFT                                          0xC


// The macro is used to align an address(64 bits). It always rounds up. 
// The macro adds (PAGE_SIZE - 1) to x,then clears lower PAGE_SHIFI bits.
#define page_aligned(x) \
    (((uint64_t)x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))



// The macro is used to check if an address is page aligned.
#define is_page_aligned(x)  \
    ((((uint64_t)x & (PAGE_SIZE - 1)) == 0))


/*

Kernel takes Page Directory Self-mapping scheme from Windows NT. 

The key of the scheme is that a item of PML4 points PML4 self. And the item is randomized
, meaning the base address of PML4 also is randomized, which further means the base address
is different every time the memory management is initialized. What's even more important is
that PML4 entry, PDPT entry, PD entry and PT entry are all compatible. So that every entry in 
PML4 also can be as PDPT entry. In the same way, PDPT entry, PD entry and PT entry are the 
same as PML4 entry.

*/
#define DEFAULT_OFFSET_OF_PML4T_BASE                0x199UL             // 110011001(binary represention)
#define DEFAULT_PML4T_BASE_ADDR                     0xFFFFCCE673399000UL
#define PML4E_OFFSET_OF_KRNL_SPACE                  0x1E0UL
#define PML4E_OFFSET_OF_HARDWARE                    0x1F0UL
#define PML4E_OFFSET_OF_FRAMEBUFFER                 0x1F8UL

// #define HARDWARE_ADDRESS                            

// Structure for mmpte_hardware_t - Page Table Entry
typedef struct _mmpte_hardware_t{
    uint64_t present : 1;        // Present bit, must be 1 for mapping to be valid
    uint64_t rw : 1;             // Read/write bit, if 0 writes may not be allowed
    uint64_t user : 1;           // User/supervisor bit, if 0 user-mode accesses are not allowed
    uint64_t pwt : 1;            // Page-level write-through
    uint64_t pcd : 1;            // Page-level cache disable
    uint64_t accessed : 1;       // Accessed; must be set to 0 when the entry is created
    uint64_t ignored1 : 1;       // Ignored
    uint64_t reserved1 : 1;      // Must be 0 for blocks that are not 1GB or larger
    uint64_t ignored2 : 4;       // Ignored
    uint64_t address : 40;       // Physical address of the next level paging structure
    uint64_t reserved2 : 11;     // Reserved (must be 0)
    uint64_t xd : 1;             // Execute disable bit
} mmpte_hardware_t;


// Reserved. To be implemented in the future.
typedef struct _mmpte_software_t {
    uint64_t reserved;
} mmpte_software_t;

/* 

Page Table Entry definiton.
PTE can be reinterpreted by hardware or software.

*/
typedef struct _mm_pte {
    union {
        uint64_t LongPtr;
        mmpte_hardware_t hardware;
        mmpte_software_t software;
    } pte;
} mmpte;


/*  PML4T Address */
extern uint64_t _mm_pt_pml4t_start;
/*  Page Directory Page Table Address */
extern uint64_t _mm_pt_pdpt_pool_start;
/*  Page Directory Pool Address */
extern uint64_t _mm_pt_pdt_pool_start;
/*  Page Table Pool Address */
extern uint64_t _mm_pt_pt_pool_start;

#define __get_pte_by_virt_addr(address)         \
    (mmpte*)((((uint64_t)address & 0xFFFFFFFFFFFFUL) >> 9 ) + _mm_pt_pt_pool_start)

/* ================================================================================ */
/* =========================== PFN Database Definition ============================ */
/* ================================================================================ */

/*
    PFN Database Structure Definition

Introduction:
    PFN database describes the state of each page in physical memory.

*/
typedef uint64_t pfn_number;

#define StartOfAllocation   read_in_process
#define EndOfAllocation     write_in_process

typedef enum _mm_page_state {
    zero_page_list,
    free_page_list,
    
    bad_page_list,
    rom_page_list,
    active_and_valid,
} mm_page_state;    

#define NUMBER_OF_PHYSICAL_PAGE_LISTS           8  

typedef struct _mm_pfn_list
{
    uint64_t total;
    mm_page_state list_name;
    uint64_t flink;
    uint64_t blink;

}mm_pfn_list;


typedef struct _mm_pfn_entry {

    uint64_t modified : 1;

    // StartOfAllocation
    uint64_t read_in_process : 1;
    // EndOfAllocation
    uint64_t write_in_process : 1;

    // indicate the pfn entry refers a prototype pte
    uint64_t prototype_pte : 1;
    // record which pfn lists does it belong to
    uint64_t page_state : 3;

    uint64_t ref_count : 57;

} mm_pfn_entry;


typedef struct _mm_pfn {
    union {
        pfn_number flink;       // the next item in zero or free page lists
    } u1;

    // virtual address of pte that points this page
    mmpte* pte_addr;

    union {

        uint64_t share_count;
        pfn_number blink;

    } u2;

    //  The state of this pfn item
    mm_pfn_entry state;

    // Original PTE; 
    // When state == rom_page_list, the value of original_pte field is a
    // pointer that points virtual address to which the current physical
    // page is mapped.
    mmpte original_pte;

    union {
        struct {
            // the pfn number of page table which this page belongs to
            uint64_t pte_frame : 57;
            uint64_t in_page_error : 1;
            uint64_t priority : 3;
        };
    } u3;

} mmpfn, *pmmpfn;

#define PFN_ITEM_SIZE                           48

extern mmpfn* _mm_pfn_db_start;


/* ================================================================================ */
/* ========================= Non Paged Pool Definition ============================ */
/* ================================================================================ */


/* The lower memory pool manager use free page list scheme to manage non paged memory pool*/
#define NON_PAGED_POOL_LIST_HEADS_MAXIMUM       4

typedef struct _pool_free_page_entry {

    struct 
    {
        struct _pool_free_page_entry* flink;
        struct _pool_free_page_entry* blink;
    } node;
    pfn_number number_of_pages;
    struct _pool_free_page_entry* owner;

} pool_free_page_entry;

typedef struct _pool_free_list_head {

    uint64_t total;
    struct _pool_free_page_entry* front;
    struct _pool_free_page_entry* rear;

} pool_free_list_head;

// Non Paged Pool Error Status
#define NO_MORE_FREE_MEMORY                     0x1


status_t _mm_alloc_pages(uint64_t size, void** addr);

void _mm_free_pages(void* addr);


/*  The upper memory pool manager use lookaside list scheme to provide finer granularity memory 
    allocation service */
typedef struct _pool_header {

    union {
        struct 
        {
            uint16_t prev_size : 9;
            uint16_t pool_index : 7;
            uint16_t block_size : 9;
            uint16_t pool_type : 7;
        };
        uint32_t UInt1;
    };
    union 
    {
        uint32_t pool_tag;
        struct 
        {
            uint16_t allocator_back_trace_index;
            uint16_t pool_tag_hash;
        };
        
    };

} pool_header;

//  free block type
#define POOL_TYPE_FREE                          0x0
//  active block type
#define POOL_TYPE_ACTIVE                        0x1

#define POOL_PAGE_SIZE                          PAGE_SIZE

//  the size of smallest block is 16 bytes. block must can hold list_node_t structure.
#define POOL_SMALLEST_BLOCK                     0x10
#define POOL_BLOCK_SHIFT                        3
#define POOL_BLOCK_SHIFT_MASK                   (0x7)

// the overhead of block head
#define POOL_HEAD_OVERHEAD                      ((uint64_t)sizeof(pool_header))

// the overhead of free block 
#define POOL_FREE_BLOCK_OVERHEAD                (POOL_HEAD_OVERHEAD + (uint64_t)sizeof(list_node_t))

// define a free block tag
#define POOL_FREE_TAG                           0

// The maximum block upper memory pool manager once can allocate
// if the POOL_PAGE_SIZE is 4096, BUDDY_MAX is equal to (4096 - 
// 8 - 8 - 16 = 4064 )
#define POOL_BUDDY_MAX  \
    (POOL_PAGE_SIZE - (POOL_HEAD_OVERHEAD << 1) - POOL_SMALLEST_BLOCK)

// The number of lists. the first two items not be used by default.
#define POOL_LIST_HEADS \
    ((POOL_PAGE_SIZE >> POOL_BLOCK_SHIFT) - 1)

// Memory Pool Definition
typedef struct _pool {

    uint16_t pool_type : 7;
    uint16_t pool_index : 9;
    
    // memory alignment, reserved.
    uint16_t reserved1;
    uint32_t reserved2;
    uint64_t reserved3;

    // The first two items is reserved. we not use them.
    list_node_t list_heads[POOL_LIST_HEADS];

} pool;

#define POOL_INDEX_FREE_INDEX                   0

#define POOL_INDEX_KERNEL_DEFAULT               1

// Dynamically memory-management functions.
void* _mm_kmalloc(uint64_t size);
void _mm_kfree(void* addr);

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#endif