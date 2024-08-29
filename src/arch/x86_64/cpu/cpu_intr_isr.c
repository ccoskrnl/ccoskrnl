#include "../../../include/types.h"
#include "../../../include/libk/string.h"
#include "../../../include/libk/stdlib.h"
#include "../../../include/hal/op/graphics.h"
#include "../../../include/hal/op/screen.h"

#include "cpu.h"
#include "../mm/mm.h"

static void apic_timer_isr()
{
    // balabala
}

static void gp_isr()
{
    go_blt_pixel_t pixel = {76, 153, 0, 0};
    for (int i = 0; i < (_op_def_screen->frame_buf_size >> 2); i++) {
        _op_def_screen->secondary_buf[i] = pixel;
    }
    _op_def_screen->SwapTwoBuffers(_op_def_screen, 0, 1);
    // _op_def_screen->clear_framebuffer(_op_def_screen);
    krnl_panic();
}

// In IA-32e mode, the RSP is aligned to a 16-byte boundary before pushing the stack frame. The stack 
// frame itself is aligned on a 16-byte boundary when the interrupt handler is called. The processor can arbitrarily 
// realign the new RSP on interrupts because the previous (possibly unaligned) RSP is unconditionally saved on the 
// newly aligned stack. The previous RSP will be automatically restored by a subsequent IRET.
void* _cpu_get_irs(uint8_t vector)
{
    void* isr = NULL;
    switch (vector) {
        case 13:
            isr = gp_isr;
            break;
        case 0x20:
            isr = apic_timer_isr;
            break;
        default:
            return NULL;
    }

    return isr;
}