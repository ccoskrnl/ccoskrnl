#include "../../../include/types.h"
#include "../../../include/go/go.h"
#include "mm_pool.h"

void mm_alloc_test()
{

    status_t status = ST_SUCCESS;

    void* alloc1 = NULL;
    void* alloc2 = NULL;
    void* alloc3 = NULL;
    void* alloc4 = NULL;

    status = _mm_new_tag_manager('0d00');
    if (ST_ERROR(status))
    {
        puts(dbg_window, "mm_alloc_test: _mm_new_tag_manager failed for TAG: 0d00.\n");
        return;
    }

    alloc1 = _mm_kmemleak_alloc(8, '0d00');
    if (alloc1 == NULL)
    {
        puts(dbg_window, "mm_alloc_test: memory allocation failed for alloc1.\n");
        return;
    }

    alloc2 = _mm_kmemleak_alloc(16, '0d00');
    if (alloc2 == NULL)
    {
        puts(dbg_window, "mm_alloc_test: memory allocation failed for alloc2.\n");
        return;
    }

    status = _mm_new_tag_manager('0721');
    if (ST_ERROR(status))
    {
        puts(dbg_window, "mm_alloc_test: _mm_new_tag_manager failed for TAG: 0721.\n");
        return;
    }

    _mm_kfree(alloc1);
    _mm_kfree(alloc2);

    status = _mm_del_tag_manager('0d00');
    if (status == MM_ALLOCATION_TRACING_MEMLEAK) {
        puts(dbg_window, "Check memory leaks for TAG: 0d00\n");
    }

    alloc3 = _mm_kmemleak_alloc(32, '0721');
    if (alloc3 == NULL)
    {
        puts(dbg_window, "mm_alloc_test: memory allocation failed for alloc3.\n");
        return;
    }

    alloc4 = _mm_kmemleak_alloc(64, '0721');
    if (alloc4 == NULL)
    {
        puts(dbg_window, "mm_alloc_test: memory allocation failed for alloc4.\n");
        return;
    }

    status = _mm_del_tag_manager('0721');
    if (status == MM_ALLOCATION_TRACING_MEMLEAK) {
        puts(dbg_window, "Check memory leaks for TAG: 0721\n");
    }

}