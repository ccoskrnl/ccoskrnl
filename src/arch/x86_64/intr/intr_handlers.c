#include "../../../include/types.h"
#include "../../../include/go/go.h"
#include "../../../include/go/window.h"

#define MagicNumber                                 0x43433035

static wch_t *pm_msg[] = {
    L"#DE Divide Error\n\r\0",
    L"#DB RESERVED\n\r\0",
    L"--  NMI Interrupt\n\r\0",
    L"#BP Breakpoint\n\r\0",
    L"#OF Overflow\n\r\0",
    L"#BR BOUND Range Exceeded\n\r\0",
    L"#UD Invalid Opcode (Undefined Opcode)\n\r\0",
    L"#NM Device Not Available (No Math Coprocessor)\n\r\0",
    L"#DF Double Fault\n\r\0",
    L"    Coprocessor Segment Overrun (reserved)\n\r\0",
    L"#TS Invalid TSS\n\r\0",
    L"#NP Segment Not Present\n\r\0",
    L"#SS Stack-Segment Fault\n\r\0",
    L"#GP General Protection\n\r\0",
    L"#PF Page Fault\n\r\0",
    L"--  (Intel reserved. Do not use.)\n\r\0",
    L"#MF x87 FPU Floating-Point Error (Math Fault)\n\r\0",
    L"#AC Alignment Check\n\r\0",
    L"#MC Machine Check\n\r\0",
    L"#XF SIMD Floating-Point Exception\n\r\0",
    L"#VE Virtualization Exception\n\r\0",
    L"#CP Control Protection Exception\n\r\0",
};


/**
 * @brief Protected-Mode Exceptions and Interrupts Handler
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
 **/
void _intr_pm_handler (
    uint64_t error_code,
    uint64_t RIP,
    uint64_t CS,
    uint64_t EFLAGS,
    uint64_t RSP,
    uint64_t SS
    )
{
    _go_cpu_output_window[0]->ClearWindow((window_t*)_go_cpu_output_window[0]);
    _go_cpu_output_window[0]->ShowWindow(_go_cpu_output_window[0]);

    put_check(0, false, pm_msg[(error_code >> 32) & 0xFF]);
    putsxs(0, "Interrupt Vector Number: ", (error_code >> 32) & 0xFF, "\n");

    if ((error_code & 0xFFFFFFFF) != MagicNumber)
        putsxs(0, "Exception Code: ", error_code & 0xFFFFFFFF, "\n");

    putsxs(0, "RIP: ", RIP, "\n");
    putsxs(0, "CS: ", CS, "\n");
    putsxs(0, "EFLAGS: ", EFLAGS, "\n");
    putsxs(0, "RSP: ", RSP, "\n");
    putsxs(0, "SS: ", SS, "\n");
}
