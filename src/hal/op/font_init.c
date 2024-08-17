#include "../../include/hal/op/font.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/libk/stdlib.h"

static void set(char** str, int* n)
{
    char* s = *str;
    while (*s++ != '=');
    *n = stoi(s, 0);
    *str = s;
}

static void add_a_char(char** str, struct _char* __char)
{
    set(str, &__char->x);
    set(str, &__char->y);
    set(str, &__char->width);
    set(str, &__char->height);
    set(str, &__char->xoffset);
    set(str, &__char->yoffset);
    set(str, &__char->xadvance);
    set(str, &__char->page);
    set(str, &__char->chnl);
}

void _op_ascii_font_init(
	_in_ char* fnt_addr, 
	_in_ char* buf_addr,
    _in_ uint16_t width,
    _in_ uint16_t height,
	_in_ _out_ struct _ascii_font* font
) {

    char* str = fnt_addr;
    char* endptr;
    int i = 0;
    int char_id = 0;
    int xadvance = 0;

    // face
    while (*str++ != '\"');
    i = 0;
    while (*str != '\"')
        font->info.face[i++] = *str++;
    
    set(&str, &font->info.size);
    set(&str, &font->info.bold);
    set(&str, &font->info.italic);

    // charset
    while (*str++ != '\"');
    i = 0;
    while (*str != '\"')
        font->info.face[i++] = *str++;

    set(&str, &font->info.unicode);
    set(&str, &font->info.stretchH);
    set(&str, &font->info.smooth);
    set(&str, &font->info.aa);

    while (*str++ != '=');
    font->info.padding.up = stoi(str, 0);
    while (*str++ != ',');
    font->info.padding.right = stoi(str, 0);
    while (*str++ != ',');
    font->info.padding.down = stoi(str, 0);
    while (*str++ != ',');
    font->info.padding.left = stoi(str, 0);
    
    while (*str++ != '=');
    font->info.spacing.horizontal = stoi(str, 0);
    while (*str++ != ',');
    font->info.spacing.vertical = stoi(str, 0);

    set(&str, &font->info.outline);


    // common
    set(&str, &font->common.lineHeight);
    set(&str, &font->common.base);
    set(&str, &font->common.scaleW);
    set(&str, &font->common.scaleH);
    set(&str, &font->common.pages);
    set(&str, &font->common.packed);
    set(&str, &font->common.alphaChnl);
    set(&str, &font->common.redChnl);
    set(&str, &font->common.greenChnl);
    set(&str, &font->common.blueChnl);

    // page
    if (font->common.pages != 1)
    {
        // only support English architecture fonts
        krnl_panic();
    }
    
    set(&str, &font->page_file.id);
    while (*str++ != '\"');
    i = 0;
    while (*str != '\"')
        font->page_file.file[i++] = *str++;
    
    // chars
    set(&str, &font->count);

    for (i = 0; i < font->count; i++)
    {
        set(&str, &char_id);
        font->chars[char_id].id = char_id;
        font->chars[char_id].present = 1;
        
        add_a_char(&str, &font->chars[char_id]);

        // set maximum char xadvance as default font advance
        if (font->chars[char_id].xadvance > xadvance)
            xadvance = font->chars[char_id].xadvance;
    }

    font->chars['\b'].id = '\b';
    font->chars['\b'].present = 1;
    font->chars['\t'].id = '\t';
    font->chars['\t'].present = 1;
    font->chars[' '].id = ' ';
    font->chars[' '].present = 1;
    font->chars['\r'].id = '\r';
    font->chars['\r'].present = 1;
    font->chars['\n'].id = '\n';
    font->chars['\n'].present = 1;

    font->common.xadvance = xadvance;
    font->blt_buf.buf_addr = (go_blt_pixel_t*)buf_addr;
    font->blt_buf.width = width;
    font->blt_buf.height = height;
}