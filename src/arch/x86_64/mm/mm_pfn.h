#ifndef __X86_64_MM_PFN_H__
#define __X86_64_MM_PFN_H__

#include "mm_arch.h"

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



#endif