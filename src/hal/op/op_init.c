#include "../../include/types.h"
// #include "../../include/hal/op/font/font_fnt.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font/font_ttf.h"
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

struct _font_ttf* _font_family_agefonts001;

struct _font_ttf* _font_family_SourceHanSansSCVF;

// all installed font family
struct _installed_font_ttfs _op_font_ttfs;

boolean _op_has_been_initialize;

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
    _op_def_screen->clear_framebuffer(_op_def_screen);

    background.buf = (go_blt_pixel_t*)_current_machine_info->bg.addr;
    background.height = _current_machine_info->bg.height;
    background.width = _current_machine_info->bg.width;
    background.size = _current_machine_info->bg.size;
    _op_bg = &background;
  

    status = new_a_font(&_font_family_SourceHanSansSCVF);
    if (ST_ERROR(status)) {
        krnl_panic();
    }
    _font_family_SourceHanSansSCVF->init(_font_family_SourceHanSansSCVF, (uint8_t*)_current_machine_info->font[0].ttf_addr);

    status = new_a_font(&_font_family_agefonts001);
    if (ST_ERROR(status)) {
        krnl_panic();
    }
    _font_family_SourceHanSansSCVF->init(_font_family_agefonts001, (uint8_t*)_current_machine_info->font[1].ttf_addr);

    _op_font_ttfs.fonts[0] = _font_family_SourceHanSansSCVF;
    _op_font_ttfs.fonts[1] = _font_family_agefonts001;
    _op_font_ttfs.num = 2;



    // uncomment the following two lines to set age language as default font
    // _op_font_ttfs.fonts[0] = _font_family_agefonts001;
    // _op_font_ttfs.fonts[1] = _font_family_SourceHanSansSCVF;


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

    _op_has_been_initialize = true;
}