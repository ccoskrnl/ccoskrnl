#include "../../include/libk/stdio.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/limits.h"
#include "../../include/libk/string.h"
#include "../../include/hal/op/graphics.h"
// #include "../../include/hal/op/font/font_fnt.h"
#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/hal/op/screen.h"

static go_blt_pixel_t color = {255, 255, 255, 0};

void putwc(wch_t wch)
{
    status_t status = ST_SUCCESS;
    font_ttf_t* family = _op_font_ttfs.fonts[0];
    status = _op_def_screen->draw_ch(_op_def_screen, wch, family, color);
    if (ST_ERROR(status)) {
        krnl_panic();
    }

}

void putc(char c)
{
    putwc(c);
    // struct _font_fnt *font;
    // status_t status;

    // int64_t i;
    // for (i = 0; i < _op_font_fnts.num; i++)
    // {
    //     struct _font_fnt_char* _ch = &_op_font_fnts.fonts[i]->chars[c];
    //     if (_ch->present)
    //     {
    //         font = _op_font_fnts.fonts[i];
    //         break;
    //     }
        
    // }
    

    // if (!(i < _op_font_fnts.num))
    // {
    //     return;
    // }

    // status = _op_def_screen->draw_char(_op_def_screen, font, c);
    // if (ST_ERROR(status))
    // {
    //     krnl_panic();
    // }
    
}