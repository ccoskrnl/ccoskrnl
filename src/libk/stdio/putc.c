#include "../../include/libk/stdio.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/limits.h"
#include "../../include/libk/string.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/hal/op/screen.h"

static go_blt_pixel_t default_color = {255, 255, 255, 0};


void putwccf(wch_t wch, go_blt_pixel_t color, font_ttf_t* family)
{
    status_t status = ST_SUCCESS;
    status = _op_def_screen->draw_ch(_op_def_screen, 0, wch, family, color);
    if (ST_ERROR(status)) {
        krnl_panic();
    }
    // if (buffer_idx >= OUTPUT_BUFFER_MAX)
    // {
    //     buffer_idx = 0;
    //     stdio_out_buf_start = 0;
    // }
    // output_buffer[buffer_idx++] = wch;
    // if () {
    
    // }

}

void putwcc(wch_t wch, go_blt_pixel_t color)
{
    font_ttf_t* family = _op_font_ttfs.fonts[0];
    putwccf(wch, color, family);
}


void putwc(wch_t wch)
{
    putwcc(wch, default_color);
}


void putc(char c)
{
    putwc(c);
}
