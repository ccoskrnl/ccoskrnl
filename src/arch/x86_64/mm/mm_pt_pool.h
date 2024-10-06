#ifndef _X86_64_MM_PT_POOL_H__
#define _X86_64_MM_PT_POOL_H__

#include "../../../include/types.h"
#include "../../../include/libk/bitmap.h"

#include "./mm_arch.h"


// System PTE bitmap
extern bitmap_t _pt_bitmap;

// The start address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_start;

// The physical start address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_phys_start;

// The size of system page table entry in bytes.
extern uint64_t _mm_sys_pte_pool_size;

// The end address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_end;

// The physical end address of system page table entry pool.
extern uint64_t _mm_sys_pte_pool_phys_end;


mm_addr_t _pt_alloc();

void _pt_free(mm_addr_t pt_addr);

uint64_t _pt_get_virt_addr_of_pt(uint64_t phys_addr_of_pt);
uint64_t _pt_get_phys_addr_of_pt(uint64_t virt_addr_of_pt);

#endif
