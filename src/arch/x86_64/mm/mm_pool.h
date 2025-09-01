#ifndef __X86_MM_POOL_H__
#define __X86_MM_POOL_H__

#include "mm_arch.h"
#include "mm_pfn.h"
#include "../../../include/libk/rbtree.h"
#include "../../../include/libk/hashtable.h"

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


#define POOL_TAG_NO_TAG                         0x0


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
void *_mm_kmemleak_alloc(uint64_t size, uint32_t tag);
void _mm_kfree(void* addr);


// #define MM_MEMLEAK_TRACING_RECORD_USING_RBTREE  1

typedef struct _allocation_record {

    uintptr_t ptr;
    size_t size;
    uint32_t tag;

    list_node_t node;

#ifdef MM_MEMLEAK_TRACING_RECORD_USING_RBTREE
    rbtree_node_t rbnode;
#endif

} mm_allocation_record_t;;

typedef struct _mm_memleak_tracing_tag_manager {

    uint32_t tag;

    // For more efficient allocation tracking
    rbtree_t* alloc_record_rbtree;


    // Manage allocation records
    list_node_t alloc_record_list;

    // Managed by _mm_memleak_tracing_tag_table
    list_node_t tag_list_node;

} mm_memleak_tracing_tag_manager_t;

#define MM_MEMLEAK_TRACING_TAG_MANAGER_HASH_SIZE  64

#define MM_ALLOCATION_TRACING_NO_MEMLEAK        ST_SUCCESS
#define MM_ALLOCATION_TRACING_MEMLEAK           99

/**
 * @brief Global hashtable that maps tags to their corresponding tag managers.
 * 
 */
extern hashtable_t* _mm_memleak_tracing_tag_table;

/**
 * @brief Initialize memory leak tracing.
 *
 */
void _mm_memleak_tracing_init();

/**
 * @brief Create a new tag manager.
 *
 * @param tag The tag to associate with the new manager.
 * @return status_t The status of the operation.
 */
status_t _mm_new_tag_manager(uint32_t tag);

/**
 * @brief Delete a tag manager.
 *
 * @param tag The tag associated with the manager to delete.
 * @return status_t The status of the operation.
 */
status_t _mm_del_tag_manager(uint32_t tag);

#endif
