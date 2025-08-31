#include "../../../include/types.h"

#include "./mm_arch.h"
#include "./mm_pt_pool.h"

uint64_t _mm_offset_of_hardware_pt;
mmpte_hardware_t* hardware_base_pdpt;
mmpte_hardware_t* hardware_base_pd;
mmpte_hardware_t* hardware_base_pt;


/*  

    Routine Description:

    The routine maps memory mapped address space into virtual address space so 
    that the processors can correctly access the space even if the paging mechanism
    is enabled.

Parameters:

base_addr - The physical base address of memory mapped address space.

num_of_pages - The number of pages of memory mapped address space.

Returned Value:

The virtual base address of the memory mapped address space.

*/
uint64_t mm_set_mmio(uint64_t base_addr, uint64_t num_of_pages)
{
    uint64_t return_virt_addr, virt_addr;
    mm_addr_t new_pt_addr;
    uint64_t offset_of_pdpt = _mm_offset_of_hardware_pt >> (PT_ENTRY_SHIFT << 1);
    uint64_t offset_of_pd = _mm_offset_of_hardware_pt >> (PT_ENTRY_SHIFT);
    uint64_t offset_of_pt = _mm_offset_of_hardware_pt & ((1UL << PT_ENTRY_SHIFT) - 1);

    return_virt_addr = virt_addr = 0xFFFF000000000000UL |
        (_mm_offset_of_hardware_pt << 12) |
        (PML4E_OFFSET_OF_HARDWARE << 39);

    _mm_offset_of_hardware_pt += num_of_pages;
    mmpte_hardware_t *pd, *pt;

    pd = (mmpte_hardware_t*)_pt_get_virt_addr_of_pt(hardware_base_pdpt[offset_of_pdpt].address << PAGE_SHIFT);
    pt = (mmpte_hardware_t*)_pt_get_virt_addr_of_pt(pd[offset_of_pd].address << PAGE_SHIFT);


    // Do we need to add a new page table?
    if ((((_mm_offset_of_hardware_pt + num_of_pages - 1) >> PT_ENTRY_SHIFT) >
                (_mm_offset_of_hardware_pt >> PT_ENTRY_SHIFT)) || (offset_of_pt == 0))
    {
        for (; (offset_of_pt != 0) || (offset_of_pt < ((1UL << PT_ENTRY_SHIFT) - 1)); offset_of_pt++) 
        {
            pt[offset_of_pt].present = 1;
            pt[offset_of_pt].rw = 1;
            pt[offset_of_pt].address = (base_addr >> PAGE_SHIFT);

            _mm_flush_tlb(virt_addr);
            virt_addr += PAGE_SIZE;

            base_addr += PAGE_SIZE;
            num_of_pages--;
        }

        // Allocate a new page table.
        new_pt_addr = _pt_alloc();
        pd[offset_of_pd + 1].present = 1;
        pd[offset_of_pd + 1].rw = 1;
        pd[offset_of_pd + 1].address = new_pt_addr.phys_addr;

        pt = (mmpte_hardware_t*)(new_pt_addr.virt_addr);
        offset_of_pt = 0;

        for(int i = 0; i < num_of_pages || (offset_of_pt < ((1UL << PT_ENTRY_SHIFT) - 1)); offset_of_pt++, i++)
        {
            pt[offset_of_pt].present = 1;
            pt[offset_of_pd].rw = 1;
            pt[offset_of_pd].address = (base_addr >> PAGE_SHIFT);

            _mm_flush_tlb(virt_addr);
            virt_addr += PAGE_SIZE;

            base_addr += PAGE_SIZE;
        }

    }
    else
    {
        for (int i = 0; i < num_of_pages; i++) 
        {
            pt[offset_of_pt + i].present = 1;
            pt[offset_of_pt + i].rw = 1;
            pt[offset_of_pt + i].address = (base_addr >> PAGE_SHIFT) + i;

            _mm_flush_tlb(virt_addr);
            virt_addr += PAGE_SIZE;
        }
    }

    return return_virt_addr;
}
