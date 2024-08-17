#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font.h"
#include "../../include/libk/string.h"
#include "../../include/libk/stdlib.h"
#include "../../include/machine_info.h"

void _op_install_a_screen(struct _op_screen_desc* screen);

// the first screen detected by ccloader.
static op_screen_desc screen0;

// wallpaper
static struct _go_image_output background;
struct _go_image_output *_op_bg;

// all screens installed on this machine.
struct _installed_screens _op_installed_screens;
// the first screen
op_screen_desc* _op_def_screen;

// seaborn font family
static struct _ascii_font secondary_font;
// default font family
static struct _ascii_font primary_font;
// all installed font family
struct _installed_fonts _op_font;

void op_init()
{

    status_t status;

    // initialize first screen.
    screen0.frame_buf_base = (go_blt_pixel_t*)_current_machine_info->graphics_info.FrameBufferBase;
    screen0.frame_buf_size = _current_machine_info->graphics_info.FrameBufferSize;
    screen0.vertical = _current_machine_info->graphics_info.VerticalResolution;
    screen0.horizontal = _current_machine_info->graphics_info.HorizontalResolution;
    screen0.pixels_per_scanline = _current_machine_info->graphics_info.PixelsPerScanLine;

    _op_install_a_screen(&screen0);

    _op_installed_screens.num++;
    _op_installed_screens.screen[0] = &screen0;
    _op_def_screen = _op_installed_screens.screen[0];

    // clear screen
    for (int i = 0; i < MAX_FRAMEBUFFER; i++) {
        memzero(_op_def_screen->frame_bufs[i], _op_def_screen->frame_buf_size);
    }

    background.buf = (go_blt_pixel_t*)_current_machine_info->bg.addr;
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


}