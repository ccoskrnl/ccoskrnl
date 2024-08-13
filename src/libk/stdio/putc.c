#include "../../include/libk/stdio.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/limits.h"
#include "../../include/libk/string.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/font.h"
#include "../../include/hal/op/screen.h"


void putc(char c)
{
    struct _ascii_font *font;
    status_t status;

    int64_t i;
    for (i = 0; i < _op_font.num; i++)
    {
        struct _char* _ch = &_op_font.font[i]->chars[c];
        if (_ch->present)
        {
            font = _op_font.font[i];
            break;
        }
        
    }
    

    if (!(i < _op_font.num))
    {
        return;
    }

    status = _op_draw_char(_op_def_screen, font, c);
    if (ST_ERROR(status))
    {
        krnl_panic();
    }
    
}