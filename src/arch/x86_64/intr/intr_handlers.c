#include "../../../include/types.h"
#include "../../../include/hal/op/screen.h"
#include "../../../include/libk/stdio.h"

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
    struct _op_screen_desc* desc = _op_def_screen;
    desc->ClearScreen(desc);

    put_check(false, L"General Protection Fault!\n");
    putsxs("Interrupt Vector Number: ", error_code & 0xFF, "\n");
    putsxs("Exception Code: ", (error_code >> 32), "\n");
    putsxs("RIP: ", RIP, "\n");
    putsxs("CS: ", CS, "\n");
    putsxs("EFLAGS: ", EFLAGS, "\n");
    putsxs("RSP: ", RSP, "\n");
    putsxs("SS: ", SS, "\n");
}
