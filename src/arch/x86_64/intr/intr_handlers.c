#include "../../../include/types.h"
#include "../../../include/go/go.h"
#include "../../../include/go/window.h"

/*
 * @brief General Protection Fault Handler
 *
 * @param[in]               error_code          Exception code
 * @param[in]               RIP                 RIP register
 * @param[in]               CS                  Code segment
 * @param[in]               EFLAGS              EFLAGS register
 * @param[in]               RSP                 RSP register
 * @param[in]               SS                  Stack segment
 *
 * @retval                  None
 *
 * */
void _intr_gpf_handler (
    uint64_t error_code,
    uint64_t RIP,
    uint64_t CS,
    uint64_t EFLAGS,
    uint64_t RSP,
    uint64_t SS
    )
{
    _go_cpu_output_window[0]->ClearWindow((window_t*)_go_cpu_output_window[0]);

    put_check(0, false, L"General Protection Fault!\n");
    putsxs(0, "Interrupt Vector Number: ", error_code & 0xFF, "\n");
    putsxs(0, "Exception Code: ", (error_code >> 32), "\n");
    putsxs(0, "RIP: ", RIP, "\n");
    putsxs(0, "CS: ", CS, "\n");
    putsxs(0, "EFLAGS: ", EFLAGS, "\n");
    putsxs(0, "RSP: ", RSP, "\n");
    putsxs(0, "SS: ", SS, "\n");
}
