#include "../../../include/types.h"
#include "../../../include/libk/bitmap.h"
#include "../../../include/libk/stdlib.h"

#include "./mm_arch.h"
#include "./mm_pt_pool.h"


mm_addr_t _pt_alloc()
{
    mm_addr_t new_pt;
    uint64_t offset;

    offset = _bitmap_alloc(&_pt_bitmap, 1);

    new_pt.phys_addr = _mm_sys_pte_pool_phys_start + (offset << PAGE_SHIFT);
    new_pt.virt_addr = _mm_sys_pte_pool_start + (offset << PAGE_SHIFT);
    memzero((void*)new_pt.virt_addr, PAGE_SIZE);

    return new_pt;
}

void _pt_free(mm_addr_t pt_addr)
{
    uint64_t offset;

    if (pt_addr.phys_addr != 0) 
    {
        offset = (pt_addr.phys_addr - _mm_sys_pte_pool_phys_start) >> PAGE_SHIFT;
    }
    else
    {
        offset = (pt_addr.virt_addr - _mm_sys_pte_pool_start) >> PAGE_SHIFT;
    }

    // unset the bit in _pt_bitmap.
    _bitmap_set(&_pt_bitmap, offset, 0);
}

uint64_t _pt_get_virt_addr_of_pt(uint64_t phys_addr_of_pt)
{
    return _mm_sys_pte_pool_start + (phys_addr_of_pt - _mm_sys_pte_pool_phys_start);
}

uint64_t _pt_get_phys_addr_of_pt(uint64_t virt_addr_of_pt)
{
    return _mm_sys_pte_pool_phys_start + (virt_addr_of_pt - _mm_sys_pte_pool_start);
}