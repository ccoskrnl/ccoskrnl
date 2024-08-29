#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/window.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/libk/string.h"
#include "../../include/libk/stdlib.h"
#include "../../include/machine_info.h"


/* Default Wallpaper */
static struct _go_image_output background;
struct _go_image_output *_op_bg;


/**
 * Screen
*/
// the first screen detected by ccloader.
static op_screen_desc screen0;
// all screens installed on this machine.
struct _installed_screens _op_installed_screens;
// the first screen
op_screen_desc* _op_def_screen;


/**
 * Font.
 */
struct _font_ttf* _font_family_agefonts001;
struct _font_ttf* _font_family_SourceHanSansSCVF;

// all installed font family
struct _installed_font_ttfs _op_font_ttfs;


/**
 * Windows
 */

window_text_t* default_window;

/**
 * A boolean type to indicate whether output module has
 * been initialized or not. Other modules should check
 * the value, if it's true. This means output module can
 * already be used.
 */
boolean _op_has_been_initialize;

void op_init()
{

    status_t status;

    // initialize first screen.
    _op_install_a_screen(
            &screen0,
            (go_blt_pixel_t*)_current_machine_info->graphics_info.FrameBufferBase,
            _current_machine_info->graphics_info.FrameBufferSize,
            _current_machine_info->graphics_info.HorizontalResolution,
            _current_machine_info->graphics_info.VerticalResolution,
            _current_machine_info->graphics_info.PixelsPerScanLine);

    _op_installed_screens.num++;
    _op_installed_screens.screen[0] = &screen0;
    _op_def_screen = _op_installed_screens.screen[0];

    // clear screen
    _op_def_screen->ClearFrameBuffers(_op_def_screen);

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

    // point_i_t base = { 5, 0};
    // go_blt_pixel_t col = {143,188,143};

    // screen0.DrawString(&screen0, L"Hello, World", base,_font_family_SourceHanSansSCVF, 18, col, 0);


    // set wallpaper
    if (_op_def_screen->horizontal == background.width && _op_def_screen->vertical == background.height)
        memcpy(_op_def_screen->frame_buf_base, _op_bg->buf, _op_bg->size);

    window_style_t def_wnd = { 
        WINDOW_STYLE_COLOR, 
        // {143,188,143}, 
        {0xc0,0xc0,0xc0}, 
        { 0 }
    };
    
    status = new_a_window(
        0x646566,
        WindowText,
        NULL,
        _op_def_screen,
        L"默认窗口",
        def_wnd,
        200,
        100,
        800,
        600,
        (void**)&default_window
    );

    if (ST_ERROR(status)) {
        krnl_panic();
    }

    default_window->Register(
        default_window,
        _font_family_agefonts001,
        POINT_SIZE,
        3
    );

    default_window->ShowWindow(default_window);




    _op_has_been_initialize = true;
}
