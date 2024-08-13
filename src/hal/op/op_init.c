#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font.h"
#include "../../include/libk/string.h"
#include "../../include/libk/stdlib.h"
#include "../../include/machine_info.h"

void screen_install_funcs(struct _go_screen_desc* screen);

static go_screen_desc screen0;
static struct _go_image_output background;
struct _go_image_output *_op_bg;

struct _installed_screen _op_screen;
go_screen_desc* _op_def_screen;

// seaborn font family
static struct _ascii_font secondary_font;
// default font family
static struct _ascii_font primary_font;
// all installed font family
struct _installed_fonts _op_font;

void op_init()
{

    status_t status;

    screen0.frame_buf_base = (go_blt_pixel*)_current_machine_info->graphics_info.FrameBufferBase;
    screen0.frame_buf_size = _current_machine_info->graphics_info.FrameBufferSize;
    screen0.vertical = _current_machine_info->graphics_info.VerticalResolution;
    screen0.horizontal = _current_machine_info->graphics_info.HorizontalResolution;
    screen0.pixels_per_scanline = _current_machine_info->graphics_info.PixelsPerScanLine;

    screen0.secondary_buf = (go_blt_pixel*)malloc(screen0.frame_buf_size);
    screen_install_funcs(&screen0);

    _op_screen.num++;
    _op_screen.screen[0] = &screen0;

    background.buf = (go_blt_pixel*)_current_machine_info->bg.addr;
    background.height = _current_machine_info->bg.height;
    background.width = _current_machine_info->bg.width;
    background.size = _current_machine_info->bg.size;
    _op_bg = &background;

    _op_ascii_font_init(
        (char*)_current_machine_info->font[0].fnt_addr,
        (char*)_current_machine_info->font[0].img.addr,
        (uint16_t)_current_machine_info->font[0].img.width,
        (uint16_t)_current_machine_info->font[0].img.height,
        &secondary_font
    );
    _op_font.num++;
    _op_font.font[1] = &secondary_font;

    _op_ascii_font_init(
        (char*)_current_machine_info->font[1].fnt_addr,
        (char*)_current_machine_info->font[1].img.addr,
        (uint16_t)_current_machine_info->font[1].img.width,
        (uint16_t)_current_machine_info->font[1].img.height,
        &primary_font
    );

    _op_font.num++;
    _op_font.font[0] = &primary_font;

    // uncomment the following two lines to set seaborn language as default font
    // _op_font.font[0] = &secondary_font;
    // _op_font.font[1] = &primary_font;


    // Kernel only uses one screen to output images or characters.
    struct _go_screen_desc* desc = _op_screen.screen[0];

    // // set wallpaper
    // if (desc->horizontal == background.width && desc->vertical == background.height)
    // {
    //     desc->blt(desc, background.buf, GoBltBufferToVideo, 0, 0, 0, 0, background.width, background.height, BACKBUFFER_INDEX);
    //     status = desc->swap_buf(desc, 0, BACKBUFFER_INDEX);
    //     if(ST_ERROR(status))
    //     {
    //         krnl_panic();
    //     }
    // }

    memzero(desc->secondary_buf, desc->frame_buf_size);
    desc->swap_buf(desc, 0, BACKBUFFER_INDEX);

    _op_def_screen = desc;

}