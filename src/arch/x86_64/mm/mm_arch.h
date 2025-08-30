/*

@file: mm.h

*/



#ifndef __X86_MM_ARCH_H__
#define __X86_MM_ARCH_H__

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


/*
 *  The structure is used to record virtual address and physical address
 *  respectively.
 */
typedef struct _mm_addr
{
    uint64_t virt_addr;
    uint64_t phys_addr;
} mm_addr_t;


// The macro indicates the size of page frame in bytes. At default, it's 4 KBytes.
// Kernel uses 4-level page table mapping scheme to manage all RAM resources.
#define PAGE_SIZE                                           0x1000

// The macro is used to declare how many bits are needed to represend a page frame.
// By shifting the macro, it can replace the multipication or division of page size.
#define PAGE_SHIFT                                          0xC

#define PT_ENTRY_SHIFT                                      0x9

#define PML4_SHIFT                                          (PAGE_SHIFT + (PT_ENTRY_SHIFT << 1) + PT_ENTRY_SHIFT)
#define PDPT_SHIFT                                          (PAGE_SHIFT + (PT_ENTRY_SHIFT << 1))
#define PD_SHIFT                                            (PAGE_SHIFT + PT_ENTRY_SHIFT)
#define PT_SHIFT                                            (PAGE_SHIFT)

#define PML4_SIZE                                           ((uint64_t)(1UL << PML4_SHIFT))

#define PML4_ALIGNED(x) \
    (((uint64_t)x + PAGE_SIZE - 1) & ~(PML4_SHIFT - 1))

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
#define PML4E_MIN_OFFSET_OF_KRNL_SPACE              0x100UL
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
extern uint64_t _mm_pt_pml4_start;
/*  Page Directory Page Table Address */
extern uint64_t _mm_pt_pdpt_pool_start;
/*  Page Directory Pool Address */
extern uint64_t _mm_pt_pdt_pool_start;
/*  Page Table Pool Address */
extern uint64_t _mm_pt_pt_pool_start;

#define __get_pte_by_virt_addr(address)         \
    (mmpte*)((((uint64_t)address & 0xFFFFFFFFFFFFUL) >> PT_ENTRY_SHIFT ) + _mm_pt_pt_pool_start)


void _mm_flush_tlb(uint64_t vaddr);

uint64_t mm_set_mmio(uint64_t base_addr, uint64_t num_of_pages);
void mm_init();

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#endif
