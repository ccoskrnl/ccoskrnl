#include "../../../include/types.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/libk/string.h"
#include "../../../include/libk/hashtable.h"



#define MM_MEMLEAK_TRACING_DEBUG_INFORMATION                0x0d00

#ifdef MM_MEMLEAK_TRACING_DEBUG_INFORMATION
#include "../../../include/go/go.h"
#endif 


#include "mm_arch.h"
#include "mm_pfn.h"
#include "mm_pool.h"

extern pool* _mm_pools[12];
extern pool_free_list_head _mm_non_paged_pool_free_list_array[NON_PAGED_POOL_LIST_HEADS_MAXIMUM];

hashtable_t* _mm_memleak_tracing_tag_table;

void _mm_memleak_tracing_init()
{
    _mm_memleak_tracing_tag_table = hashtable_create(MM_MEMLEAK_TRACING_TAG_MANAGER_HASH_SIZE);
}

status_t _mm_new_tag_manager(uint32_t tag)
{
    status_t status = ST_SUCCESS;

    // allocate memory for the new tag bucket
    mm_memleak_tracing_tag_manager_t* new_tag_manager = (mm_memleak_tracing_tag_manager_t*)_mm_kmalloc(sizeof(mm_memleak_tracing_tag_manager_t));
    if (new_tag_manager == NULL)
    {
        krnl_panic(L"Can not allocate memory for new tag manager...");
        return ST_FAILED;
    }

    new_tag_manager->tag = tag;
    new_tag_manager->alloc_record_list.flink = NULL;
    new_tag_manager->alloc_record_list.blink = NULL;


#ifdef MM_MEMLEAK_TRACING_RECORD_USING_RBTREE
    status = new_a_rbtree(&new_tag_manager->alloc_record_rbtree);
    if (ST_ERROR(status))
    {
        krnl_panic(L"Can not initialize rbtree for new tag manager...");
        return status;
    }
#endif

    hashtable_insert(_mm_memleak_tracing_tag_table, (byte*)&tag, sizeof(uint32_t), new_tag_manager);

    return status;
}

status_t _mm_del_tag_manager(uint32_t tag)
{
    status_t status;

    mm_memleak_tracing_tag_manager_t* removed_tag_manager = 
        (mm_memleak_tracing_tag_manager_t*)hashtable_remove(
            _mm_memleak_tracing_tag_table, (byte*)&tag, sizeof(uint32_t));


#ifdef MM_MEMLEAK_TRACING_RECORD_USING_RBTREE
    if (removed_tag_manager->alloc_record_rbtree->num_of_nodes != 0)
        status = MM_ALLOCATION_TRACING_MEMLEAK;
    else
        status = MM_ALLOCATION_TRACING_NO_MEMLEAK;
#else
    if (removed_tag_manager->alloc_record_list.flink != NULL 
        || removed_tag_manager->alloc_record_list.blink != NULL)
        status = MM_ALLOCATION_TRACING_MEMLEAK;
    else
        status = MM_ALLOCATION_TRACING_NO_MEMLEAK;

#endif

    if (status == MM_ALLOCATION_TRACING_MEMLEAK)
    {

#ifdef MM_MEMLEAK_TRACING_DEBUG_INFORMATION
        puts(dbg_window, "Check memory leaks. Automatic freeing...\n");
#endif

        // cleanup the allocation records.

        // situation 1, only one allocation record exists
        if (removed_tag_manager->alloc_record_list.flink == removed_tag_manager->alloc_record_list.blink)
        {
            _mm_kfree(removed_tag_manager->alloc_record_list.flink);
        }
        else 
        {
            // situation 2, multiple allocation records exist
            list_node_t* removed_node = _list_pop(&removed_tag_manager->alloc_record_list);
            while (removed_node != NULL)
            {
                mm_allocation_record_t* alloc_record = (mm_allocation_record_t*)struct_base(mm_allocation_record_t, node, removed_node);

#ifdef MM_MEMLEAK_TRACING_DEBUG_INFORMATION
                char tag_str[5] = { 0 };
                *(uint32_t*)tag_str = swap_endian_32(alloc_record->tag);
                tag_str[4] = '\0';
                putsxs(dbg_window, "Allocate Address: ", alloc_record->ptr, ", ");
                putsds(dbg_window, "Size: ", alloc_record->size, ", ");
                putss(dbg_window, "TAG: ", tag_str);
                puts(dbg_window, "\n");
#endif
                _mm_kfree(alloc_record);
                removed_node = _list_pop(&removed_tag_manager->alloc_record_list);
            }
        
        }
    }

    _mm_kfree(removed_tag_manager);
    return status;
}

static status_t insert_alloc_record(uint32_t tag, uintptr_t allocated_address, size_t allocated_size)
{
    status_t status = ST_SUCCESS;

    // find the tag manager
    mm_memleak_tracing_tag_manager_t* tag_manager = (mm_memleak_tracing_tag_manager_t*)hashtable_search(_mm_memleak_tracing_tag_table, (byte*)&tag, sizeof(uint32_t));
    if (!tag_manager)
    {
        return ST_FAILED;
    }

    // allocate memory for the new allocation record
    mm_allocation_record_t* alloc_record = (mm_allocation_record_t*)_mm_kmalloc(sizeof(mm_allocation_record_t));
    alloc_record->ptr = allocated_address;
    alloc_record->size = allocated_size;
    alloc_record->tag = tag;

#ifdef MM_MEMLEAK_TRACING_RECORD_USING_RBTREE
    // insert the record into the red-black tree
    alloc_record->rbnode.key = allocated_address;
    tag_manager->alloc_record_rbtree->Insert(tag_manager->alloc_record_rbtree, &alloc_record->rbnode);
#endif

    // insert the record into the linked list
    _list_push(&tag_manager->alloc_record_list, &alloc_record->node);

    return status;
}

static status_t remove_alloc_record(uint32_t tag, uintptr_t allocated_address, size_t allocated_size)
{
    status_t status = ST_SUCCESS;
    mm_allocation_record_t* alloc_record = NULL;

    // find the tag manager
    mm_memleak_tracing_tag_manager_t* tag_manager = (mm_memleak_tracing_tag_manager_t*)hashtable_search(_mm_memleak_tracing_tag_table, (byte*)&tag, sizeof(uint32_t));
    if (!tag_manager)
    {
        return ST_FAILED;
    }

#ifdef MM_MEMLEAK_TRACING_RECORD_USING_RBTREE

    // find the allocation record
    mm_allocation_record_t* alloc_record = 
        (mm_allocation_record_t*)struct_base(
            mm_allocation_record_t, 
            rbnode, 
            tag_manager->alloc_record_rbtree->Search(tag_manager->alloc_record_rbtree, allocated_address));

    // remove the record from the red-black tree
    tag_manager->alloc_record_rbtree->Delete(tag_manager->alloc_record_rbtree, &alloc_record->rbnode);

#else

    // iterate through the tag_manager.alloc_record_list to find the allocation record
    list_node_t* current = tag_manager->alloc_record_list.flink;
    while (current)
    {
        alloc_record = (mm_allocation_record_t*)struct_base(mm_allocation_record_t, node, current);
        if (alloc_record->ptr == allocated_address)
        {
            // found the record
            break;
        }
        current = current->flink;
    }

#endif

    // check if the record is valid
    assert(alloc_record->size == allocated_size);

    // remove the record from the linked list
    _list_remove(&tag_manager->alloc_record_list, &alloc_record->node);

    // free the memory allocated for the record
    _mm_kfree(alloc_record);

    return status;
}


status_t _mm_alloc_pages(uint64_t size, void **addr)
{
    status_t status;
    // the number of requested pages
    uint64_t number_of_pages;
    // previous page
    pool_free_page_entry *prev_page;
    // next page
    pool_free_page_entry *rear_page;

    // the start addr routine returned
    pool_free_page_entry *start_page;

    pool_free_list_head *free_list;
    mmpte *pte;
    pool_free_page_entry **allocate_address;

    // check if the parameters are legal.
    assert(size != 0);

    allocate_address = (pool_free_page_entry **)addr;

    // We perfrom page-alignment on the size of requested.
    // Hence, we obtain the number of pages of requested size.
    number_of_pages = page_aligned(size) >> PAGE_SHIFT;

    // lower memory pool manager uses finite lists to manage all free
    // pages the memory pool has. the amount of lists be defined by 
    // NON_PAGED_POOL_LIST_HEADS_MAXIMUM macro. the first item of list array 
    // collected all free pages which their size exceeded the defined maximum limit.

    // check if these has a suitable lists that contained contiguous free pages that
    // the number of the pages equals the requested amount.
    if (number_of_pages < NON_PAGED_POOL_LIST_HEADS_MAXIMUM)
    {
        // we found the list and have ensured it is non-empty.
        free_list = &_mm_non_paged_pool_free_list_array[number_of_pages];
        if (free_list->total > 0)
        {

            if (free_list->total > 1)
            {
                // remove the last item of the list.
                // start_page points suitable size page at this time, 
                // we can return the page after we have set the necessary attributes.
                start_page = free_list->rear;
                free_list->rear = start_page->node.blink;
                free_list->rear->node.flink = 0;
                free_list->total--;
            }
            else if (free_list->total == 1)
            {
                start_page = free_list->rear;
                free_list->rear = 0;
                free_list->front = 0;
                free_list->total--;
            }

            // clear flags
            start_page->node.flink = 0;
            start_page->node.blink = 0;
            start_page->owner = 0;
            start_page->number_of_pages = 0;

            // update corresponding item in PFN database.
            pte = __get_pte_by_virt_addr(start_page);
            _mm_pfn_db_start[pte->pte.hardware.address].state.StartOfAllocation = 1;
            _mm_pfn_db_start[(pte->pte.hardware.address) + (number_of_pages - 1)].state.EndOfAllocation = 1;
            *allocate_address = start_page;
            status = ST_SUCCESS;
            goto __exit_alloc;
        }
    }

    // in this case, we not found suitable list, hence, we need to
    // find a contiguous pages and separate it as appropriate.
    free_list = &_mm_non_paged_pool_free_list_array[0];
    if (free_list->total == 0)
    {
        status = NO_MORE_FREE_MEMORY;
        goto __exit_alloc;
    }


    // iterate backward through the list

    // get the last item of the list
    rear_page = free_list->rear;

__construct_pages:

    // inspect if the number of the contiguous pages greater than requested amount.
    if (rear_page->number_of_pages > number_of_pages)
    {
        // divide the contiguous pages into two contiguous pages
        // the front will be added into corresponding list, and the
        // back will be returned to caller. 
        rear_page->number_of_pages -= number_of_pages;

        // start_page records the start of contiguous pages which will be returned
        start_page = (pool_free_page_entry *)(((rear_page->number_of_pages) << PAGE_SHIFT) + (uint64_t)rear_page);

        for (size_t i = 0; i < number_of_pages; i++)
        {
            pool_free_page_entry *page = start_page;
            page->node.blink = 0;
            page->node.flink = 0;
            page->number_of_pages = 0;
            page->owner = 0;
            page += 0x1000;
        }

        pte = __get_pte_by_virt_addr(start_page);
        _mm_pfn_db_start[pte->pte.hardware.address].state.StartOfAllocation = 1;
        _mm_pfn_db_start[(pte->pte.hardware.address) + (number_of_pages - 1)].state.EndOfAllocation = 1;
        *allocate_address = start_page;
        status = ST_SUCCESS;

        // we need to add the front into corresponding list if its number_of_pages
        // less than NON_PAGED_POOL_LIST_HEADS_MAXIMUM. 
        if (rear_page->number_of_pages < NON_PAGED_POOL_LIST_HEADS_MAXIMUM)
        {
            // remove the remained contiguous pages from the list that it belonged.
            if (free_list->total == 1)
            {
                free_list->front = NULL;
                free_list->rear = NULL;
            }
            else
            {
                free_list->rear = free_list->rear->node.blink;
            }
            free_list->total--;

            // add it back into new list
            free_list = &_mm_non_paged_pool_free_list_array[rear_page->number_of_pages];
            if (free_list->total == 0)
            {
                free_list->front = rear_page;
                free_list->rear = rear_page;
            }
            else
            {
                rear_page->node.blink = free_list->rear;
                free_list->rear->node.flink = rear_page;
                free_list->rear = rear_page;
            }

            free_list++;
        }
    }

    // inspect if the number of the contiguous pages equals requested amount.
    else if (rear_page->number_of_pages == number_of_pages)
    {
        if (free_list->total == 1)
        {
            free_list->rear = NULL;
            free_list->front = NULL;
            free_list->total = 0;
        }
        else
        {
            start_page = rear_page;
            rear_page->node.blink->node.flink = NULL;
            free_list->rear = rear_page->node.blink;
            free_list->total--;
        }

        pte = __get_pte_by_virt_addr(start_page);
        _mm_pfn_db_start[pte->pte.hardware.address].state.StartOfAllocation = 1;
        _mm_pfn_db_start[(pte->pte.hardware.address) + (number_of_pages - 1)].state.EndOfAllocation = 1;
        *allocate_address = start_page;
        status = ST_SUCCESS;

    }

    else
    {
        if (free_list->total == 1)
        {
            status = NO_MORE_FREE_MEMORY;
        }

        // iterate backward through the list
        else
        {
            while (free_list->total > 1 && (rear_page->number_of_pages < number_of_pages)) 
            {
                prev_page = rear_page->node.blink;

                // merge the previous contiguous pages and this contiguous pages into 
                // single contiguous pages. 
                prev_page->number_of_pages += rear_page->number_of_pages;
                rear_page->node.blink = 0;
                rear_page->node.flink = 0;

                for (size_t i = 0; i < rear_page->number_of_pages; i++)
                {
                    pool_free_page_entry *page = rear_page;
                    page->owner = prev_page;
                    page += PAGE_SIZE;
                }
                rear_page->number_of_pages = 0;

                rear_page = prev_page;
                free_list->rear = rear_page;
                free_list->total--;

            }

            goto __construct_pages;
        }
    }

__exit_alloc:

    return status;
}

void _mm_free_pages(void *addr)
{
    status_t status;
    pfn_number index;
    mmpte *pte;
    pool_free_page_entry *start_addr;
    pool_free_page_entry *page;
    pool_free_list_head *free_list;

    // clear the flag in corresponding pfn item.
    start_addr = page = addr;
    pte = __get_pte_by_virt_addr(addr);
    index = pte->pte.hardware.address;
    _mm_pfn_db_start[index].state.StartOfAllocation = 0;
    memzero(page,PAGE_SIZE);

    start_addr->number_of_pages++;
    while (_mm_pfn_db_start[index].state.EndOfAllocation == 0)
    {
        page = (pool_free_page_entry *)((uint64_t)page + PAGE_SIZE);
        memzero(page, PAGE_SIZE);
        page->owner = start_addr;

        start_addr->number_of_pages++;
        index++;
    }
    _mm_pfn_db_start[index].state.EndOfAllocation = 0;

    // inspect which list do we need to add it to
    if (start_addr->number_of_pages < NON_PAGED_POOL_LIST_HEADS_MAXIMUM)
    {

        free_list = &_mm_non_paged_pool_free_list_array[start_addr->number_of_pages];
    }
    else
    {
        free_list = &_mm_non_paged_pool_free_list_array[0];
    }

    if (free_list->total == 0)
    {
        free_list->rear = start_addr;
        free_list->front = start_addr;
    }
    else
    {
        start_addr->node.blink = free_list->rear;
        free_list->rear->node.flink = start_addr;
        free_list->rear = start_addr;
    }

    free_list->total++;
}


/*  Upper Memory Pool Services  */
void *_mm_malloc(uint64_t size, uint16_t pool_index, uint32_t tag)
{
    list_node_t *ret_block;
    pool_header *ret_block_head;

    list_node_t *separate_block;
    pool_header *separate_block_head;

    pool_header *next_block_head;

    boolean does_it_need_to_be_updated;

    pool *pool_desc;
    list_node_t *free_list;
    uint64_t suitable_block_size;
    uint64_t list_index;

    does_it_need_to_be_updated = false;

    // Adjust the size of requested block. There are three situations here.

    // case 1: the requested smaller than smallest block
    if (size < POOL_SMALLEST_BLOCK)
    {
        // For this case, we just adjust the size of requested block to
        // smallest size.
        size = POOL_SMALLEST_BLOCK;
    }

    // case 2: adjust the size to Multiple of PAGE_SIZE
    else if (size & POOL_BLOCK_SHIFT_MASK)
    {
        size = (size & ~POOL_BLOCK_SHIFT_MASK) + (1 << POOL_BLOCK_SHIFT);
    }

    // case3 : the size larger than maximum allocatable block.
    if (size > POOL_BUDDY_MAX)
    {
        // In the case, we calls lower dynamic memory management routine.
        // We perfrom page-alignment on the size of requested.
        // It is noteworthy that there exist serious internal-fragmention wasting.
        // For reason of page-alignment, (aligned_size - requested_size) bytes 
        // will be treated as internal-fragmention.

        // Now, let's me assume that caller passes size = 0x1001 each time to
        // call the function so that will cause the function will perform 
        // page-alignment on the size. Actually, the size after page-aligend is
        // 0x2000. In other words, 0xFFF bytes will be treated as 
        // internal-fragmention.

        size = page_aligned(size);

        if (ST_ERROR(_mm_alloc_pages(size, (void **)&ret_block)))
        {
            return NULL;
        }
        else
        {
            return (void *)ret_block;
        }
    }

    // found corresponding pool by pool_index.
    pool_desc = _mm_pools[pool_index];

    // To find suitable list.
    list_index = size >> POOL_BLOCK_SHIFT;

    // Check if corresponding list is not empty. Otherwise,
    // increase the requested size and continue to check until
    // found list is not empty.
    do
    {
        free_list = &pool_desc->list_heads[list_index];
        if (free_list->flink != 0)
        {
            break;
        }

    } while (list_index++ < POOL_LIST_HEADS);

    // When list_index larger than or equal to POOL_LIST_HEADS, it's means
    // we not found suitable list. In this situation, we need to call lower
    // dynamic memory management function to allocate a new free page to 
    // respond the request.

    // The new free page will be separate. The front block will be returned
    // and the back block will be added to suitable free list.
    if (list_index >= POOL_LIST_HEADS)
    {
        uint64_t new_page;

        // if out of memory resource, request failed. 
        if (ST_ERROR(_mm_alloc_pages((1 << PAGE_SHIFT), (void **)&new_page)))
        {
            return NULL;
        }

        // separate the new allocated page.
        else
        {
            // separate_block points back block here.
            separate_block_head = (pool_header *)(new_page + size + POOL_HEAD_OVERHEAD);
            separate_block = (list_node_t *)((uint64_t)separate_block_head + POOL_HEAD_OVERHEAD);

            // it will be added to free list.
            separate_block_head->prev_size = (size) >> POOL_BLOCK_SHIFT;
            separate_block_head->block_size =
                (POOL_PAGE_SIZE - size - (POOL_HEAD_OVERHEAD << 1)) >> POOL_BLOCK_SHIFT;

            separate_block_head->pool_type = POOL_TYPE_FREE;
            separate_block_head->pool_index = POOL_INDEX_FREE_INDEX;

            // **TAG**
            separate_block_head->pool_tag = POOL_FREE_TAG;


            // set the block header of block will be returned.
            ret_block_head = (pool_header *)new_page;
            ret_block = (list_node_t *)((uint64_t)ret_block_head + POOL_HEAD_OVERHEAD);
            ret_block_head->prev_size = 0;
            ret_block_head->block_size = separate_block_head->prev_size;
            ret_block_head->pool_type = POOL_TYPE_ACTIVE;
            ret_block_head->pool_index = pool_index;

            // **TAG**
            ret_block_head->pool_tag = tag;



            // add the separate_block to free list
            free_list = &pool_desc->list_heads[separate_block_head->block_size];
            _list_push(free_list, separate_block);

            // return constructed block a moment ago.
            goto __exit_ret_block;
            // return (void *)ret_block;
        }
    }

    // Assuming we found a suitable list.
    suitable_block_size = list_index << POOL_BLOCK_SHIFT;

    // Using the last item to construct a block which need to return to caller
    separate_block = free_list->blink;
    separate_block_head = (pool_header *)((uint64_t)separate_block - POOL_HEAD_OVERHEAD);

    // Suppose that the separate_block we get just now is not stored at the final position of page
    // it belonged, meaning we need to update the pool head of block that closely following separate_block 
    if (((uint64_t)separate_block + (separate_block_head->block_size << POOL_BLOCK_SHIFT)) < page_aligned(separate_block))
    {
        does_it_need_to_be_updated = true;
        next_block_head = (pool_header *)((uint64_t)separate_block + (separate_block_head->block_size << POOL_BLOCK_SHIFT));
    }

    // check if the size of remained block after suitable_block_size minus
    // requested size lesses than POOL_FREE_BLOCK_OVERHEAD, then return the
    // suitable_block and not to separate. Otherwise, separate it and add remianed
    // block to corresponding list.
    if ((suitable_block_size - size) < (POOL_FREE_BLOCK_OVERHEAD))
    {
        ret_block_head = separate_block_head;
        ret_block_head->pool_type = POOL_TYPE_ACTIVE;
        ret_block_head->pool_index = pool_index;

        // **TAG**
        ret_block_head->pool_tag = tag;


        ret_block = separate_block;

        // remove the block from original list.
        if (ret_block->blink == 0) {
            krnl_panic(NULL);
        }
        _list_remove(free_list, ret_block);

        goto __exit_ret_block;
        // return (void *)ret_block;
    }



    // Now we already known the suitable_block need to separate, but there
    // has two cases need to consider.

    // if previous block is not equal to zero, it means current sutiable_block
    // is not the first block in page which it belonged.
    if (separate_block_head->prev_size != 0)
    {

        // remove the block from original list.
        if (separate_block->blink == 0) {
            krnl_panic(NULL);
        }
        _list_remove(free_list, separate_block);

        separate_block_head->block_size -= ((size + POOL_HEAD_OVERHEAD) >> POOL_BLOCK_SHIFT);

        // add to new list
        free_list = &pool_desc->list_heads[separate_block_head->block_size];
        _list_push(free_list, separate_block);

        ret_block_head =
            (pool_header *)((uint64_t)separate_block + (separate_block_head->block_size << POOL_BLOCK_SHIFT));
        ret_block = (list_node_t *)((uint64_t)ret_block_head + POOL_HEAD_OVERHEAD);

        ret_block_head->prev_size = separate_block_head->block_size;
        ret_block_head->block_size = (size >> POOL_BLOCK_SHIFT);
        ret_block_head->pool_type = POOL_TYPE_ACTIVE;


        ret_block_head->pool_tag = tag;


        ret_block_head->pool_index = pool_index;

        if (does_it_need_to_be_updated)
        {
            // update the next_block's head
            next_block_head->prev_size = ret_block_head->block_size;
        }
    }

    // another case, separate_block is stored at the first position of page it belonged
    else
    {
        ret_block_head = separate_block_head;
        ret_block = separate_block;

        separate_block_head = (pool_header *)((uint64_t)ret_block + size);
        separate_block = (list_node_t *)((uint64_t)separate_block_head + POOL_HEAD_OVERHEAD);

        separate_block_head->block_size =
            ret_block_head->block_size - ((size + POOL_HEAD_OVERHEAD) >> POOL_BLOCK_SHIFT);

        // remove from original list
        ret_block_head->block_size = (size >> POOL_BLOCK_SHIFT);
        ret_block_head->pool_type = POOL_TYPE_ACTIVE;
        ret_block_head->pool_tag = tag;
        ret_block_head->pool_index = pool_index;

        if (ret_block->blink == 0) {
            krnl_panic(NULL);
        }
        _list_remove(free_list, ret_block);


        separate_block_head->prev_size = ret_block_head->block_size;

        separate_block_head->pool_type = POOL_TYPE_FREE;
        separate_block_head->pool_index = POOL_INDEX_FREE_INDEX;
        separate_block_head->pool_tag = POOL_FREE_TAG;

        if (does_it_need_to_be_updated)
        {
            next_block_head->prev_size = separate_block_head->block_size;
        }

        free_list = &pool_desc->list_heads[separate_block_head->block_size];
        _list_push(free_list, separate_block);
    }

__exit_ret_block:

    ret_block_head = (pool_header*)((uint64_t)ret_block - POOL_HEAD_OVERHEAD);
    if (ret_block_head->pool_tag != POOL_TAG_NO_TAG)
    {
        status_t status = insert_alloc_record(
            ret_block_head->pool_tag, 
            (uintptr_t)ret_block, 
            size
        );

        if (ST_ERROR(status))
        {
            _mm_kfree((void*)ret_block);
            ret_block = NULL;
        }
    }

    return (void *)ret_block;
}

void _mm_free(void *addr, uint16_t pool_index)
{
    pool_header *released_block_head;
    pool_header *prev_block_head;
    pool_header *next_block_head;
    pool_header *predict_block_head;
    boolean does_back_block_need_to_update;

    list_node_t *released_block;
    list_node_t *prev_block;
    list_node_t *next_block;

    pool *pool_desc;
    list_node_t *free_list;

    uint16_t free_size;


    // The variable indicates whether it is necessary to update the header
    // of the active block that follows the last free block that follows
    // released block.

    // The default value is false. Because if released_block is the last block
    // in current page, we don't enter situation 1 loop, which means this variable
    // is not to be updated.
    does_back_block_need_to_update = false;


    // check if addr is not page_aligned. Otherwise, we diectly
    // call lower dynamic memory management function.
    if (is_page_aligned(addr))
    {
        return _mm_free_pages(addr);
    }

    // locate at corresponding pool
    pool_desc = _mm_pools[pool_index];

    // be released block
    released_block = (list_node_t *)(addr);
    released_block_head = (pool_header *)((uint64_t)released_block - POOL_HEAD_OVERHEAD);

    // check pool_type
    if (released_block_head->pool_type != POOL_TYPE_ACTIVE)
    {
        // Buggy
        krnl_panic(NULL);
    }

    if (released_block_head->pool_tag != POOL_TAG_NO_TAG)
    {
        status_t status = remove_alloc_record(
            released_block_head->pool_tag, 
            (uintptr_t)addr, 
            released_block_head->block_size << POOL_BLOCK_SHIFT
        );

        if (ST_ERROR(status))
        {
            krnl_panic(L"Cannot remove allocation record with tag.");
        }
    }

    // record releasalbe size.
    free_size = (released_block_head->block_size << POOL_BLOCK_SHIFT) + POOL_HEAD_OVERHEAD;

    // situation 1: the be released block is not the last block in the page.
    if (((uint64_t)released_block + (released_block_head->block_size << POOL_BLOCK_SHIFT)) < page_aligned(released_block))
    {

        // record next block
        next_block_head = (pool_header *)((uint64_t)released_block +
                (released_block_head->block_size << POOL_BLOCK_SHIFT));
        next_block = (list_node_t *)((uint64_t)next_block_head + POOL_HEAD_OVERHEAD);

        // check if prev_size in next block header is not equal to the size of released block
        // Normally, they are equal.
        if (released_block_head->block_size != next_block_head->prev_size)
        {
            // Buggy
            krnl_panic(NULL);
        }

        // It is necessary to update the header of the active block that follows the 
        // last free block that follows released block.
        // Because of all free blocks that follows released_block will be merged with
        // released_blcok. For this reason, the size of released_block will be changed.
        // The prev_size of the active block alose must to be changed.

        // On another situation, the block that closly following released_block is active.
        // We also need to update the header.

        // In a word, if existing a active block in range of behind the released_block, it's
        // header must to be update.
        does_back_block_need_to_update = true;

        // We walk the list, record which blocks we need to merge.
        while (next_block_head->pool_type == POOL_TYPE_FREE)
        {
            free_size += (next_block_head->block_size << POOL_BLOCK_SHIFT) + POOL_HEAD_OVERHEAD;
            free_list = &pool_desc->list_heads[next_block_head->block_size];
            if (next_block->blink == 0) {
                krnl_panic(NULL);
            }
            _list_remove(free_list, next_block);

            // check if address of next_block_head is outside the scope of current page.
            if (((uint64_t)next_block + (next_block_head->block_size << POOL_BLOCK_SHIFT)) >= page_aligned(released_block))
            {
                // becauce of the last block in current page is free. There is nothing to
                // update in the range behind the released_block. We don't to update.
                does_back_block_need_to_update = false;

                break;
            }

            // the next block
            next_block_head = (pool_header *)((uint64_t)next_block +
                    (next_block_head->block_size << POOL_BLOCK_SHIFT));
            next_block = (list_node_t *)((uint64_t)next_block_head + POOL_HEAD_OVERHEAD);
        }
    }

    // situation 2: the be released block is not the first block in the page.
    if (released_block_head->prev_size != 0)
    {

        prev_block = (list_node_t *)((uint64_t)released_block_head -
                (released_block_head->prev_size << POOL_BLOCK_SHIFT));
        prev_block_head = (pool_header *)((uint64_t)prev_block - POOL_HEAD_OVERHEAD);

        // if the block in front of released_block is free.
        if (prev_block_head->pool_type == POOL_TYPE_FREE)
        {

            // walk forward.
            while (true)
            {
                free_size += (prev_block_head->block_size << POOL_BLOCK_SHIFT) + POOL_HEAD_OVERHEAD;
                free_list = &pool_desc->list_heads[prev_block_head->block_size];

                // remove the block from origianl list.
                if (prev_block->blink == 0) {
                    krnl_panic(NULL);
                }
                _list_remove(free_list, prev_block);

                // If the prev_block is the first block in current page.
                if (prev_block_head->prev_size == 0)
                {
                    break;
                }

                // The predict block is in front of the block in front of current block.
                predict_block_head = (pool_header *)((uint64_t)prev_block_head -
                        (prev_block_head->prev_size << POOL_BLOCK_SHIFT) - POOL_HEAD_OVERHEAD);

                if (predict_block_head->pool_type != POOL_TYPE_FREE)
                {
                    break;
                }

                // the block in front of the released_block.
                prev_block = (list_node_t *)((uint64_t)prev_block_head -
                        (prev_block_head->block_size << POOL_BLOCK_SHIFT));
                prev_block_head = (pool_header *)((uint64_t)prev_block - POOL_HEAD_OVERHEAD);
            }

            // check if we walked forward all block, in this case, the sum of size of all can be merged block
            // may be equal to PAGE_SIZE, we need to free the page.
            if (free_size == POOL_PAGE_SIZE)
            {
                return _mm_free_pages(prev_block_head);
            }

            // Otherwise, the reason that caused while loop break is that encountered non-free block
            // when walking forward. But we need to re-locate the free block that closest to the page starting
            // position because we used the predict_block.
            else
            {

                // the prev_block was removed from original list in while loop and the free_size was
                // recorded the size of the prev_block.
                // adjust the free_size to decrease POOL_HEAD_OVERHEAD.
                free_size -= POOL_HEAD_OVERHEAD;
                prev_block_head->block_size = free_size >> POOL_BLOCK_SHIFT;

                // // add free block to suitable list 
                // if (does_back_block_need_to_update)
                // {
                //     next_block_head->prev_size = released_block_head->block_size;
                // }
                // // memzero((void *)released_block, free_size);

                // free_list = &pool_desc->list_heads[(free_size >> POOL_BLOCK_SHIFT)];
                // _list_push(free_list, prev_block);
            }
        }
        else
        {
            goto __free_released_block;
        }

        // memzero((void *)((uint64_t)prev_block), free_size);

        if (does_back_block_need_to_update)
        {
            // update the prev_size of next_block_head
            next_block_head->prev_size = prev_block_head->block_size;
        }

        // add merged block to suitable list 
        free_list = &pool_desc->list_heads[(free_size >> POOL_BLOCK_SHIFT)];
        _list_push(free_list, prev_block);
    }

    // situation 3: released_block is the frist block in page.
    else
    {

__free_released_block:

        if (free_size == PAGE_SIZE)
        {
            _mm_free_pages(released_block_head);
        }
        else
        {
            free_size -= POOL_HEAD_OVERHEAD;

            released_block_head->block_size = (free_size >> POOL_BLOCK_SHIFT);
            released_block_head->pool_tag = POOL_FREE_TAG;
            released_block_head->pool_type = POOL_TYPE_FREE;
            released_block_head->pool_index = POOL_INDEX_FREE_INDEX;

            if (does_back_block_need_to_update)
            {
                next_block_head->prev_size = released_block_head->block_size;
            }

            // memzero((void *)released_block, free_size);

            free_list = &pool_desc->list_heads[released_block_head->block_size];
            _list_push(free_list, released_block);
        }
    }
}

// default kernel memory pool
void *_mm_kmalloc(uint64_t size)
{
    return _mm_malloc(size, POOL_INDEX_KERNEL_DEFAULT, 0);
}

void *_mm_kmemleak_alloc(uint64_t size, uint32_t tag)
{
    return _mm_malloc(size, POOL_INDEX_KERNEL_DEFAULT, tag);
}

void _mm_kfree(void *addr)
{
    _mm_free(addr, POOL_INDEX_KERNEL_DEFAULT);
}
