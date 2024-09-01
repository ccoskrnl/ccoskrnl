#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/window.h"
#include "../../include/hal/op/window_text.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/libk/string.h"
#include "../../include/libk/stdlib.h"
#include "../../include/machine_info.h"


/* Default Wallpaper */
static struct _go_buffer background;
struct _go_buffer *_go_default_wallpaper;


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

window_text_t* bsp_window;

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
    _go_default_wallpaper = &background;
  

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

    // go_blt_pixel_t col = {143,188,143};
    go_blt_pixel_t font_color = {255,255,255};

    // set wallpaper
    if (_op_def_screen->horizontal == background.width && _op_def_screen->vertical == background.height)
        memcpy(_op_def_screen->frame_buf_base, _go_default_wallpaper->buf, _go_default_wallpaper->size);

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
        L"Bootstrap Processor",
        def_wnd,
        20,
        20,
        800,
        800,
        (void**)&bsp_window
    );

    if (ST_ERROR(status)) {
        krnl_panic();
    }

    bsp_window->Register(
        bsp_window,
        _font_family_SourceHanSansSCVF,
        POINT_SIZE,
        0
    );
    
    bsp_window->ShowWindow(bsp_window);
    bsp_window->PutString(bsp_window, L"你好，我是Bootstrap Processor。\n", font_color);

    font_color.Red = 238;
    font_color.Green = 44;
    font_color.Blue = 44;

    window_text_t *sp1_window;
    status = new_a_window(
        0x646566,
        WindowText,
        NULL,
        _op_def_screen,
        L"SP1",
        def_wnd,
        1000,
        20,
        800,
        400,
        (void**)&sp1_window
    );

    if (ST_ERROR(status)) {
        krnl_panic();
    }

    sp1_window->Register(
        sp1_window,
        _font_family_agefonts001,
        80,
        0
    );
    
    sp1_window->ShowWindow(sp1_window);
    sp1_window->PutChar(sp1_window, 'V', font_color, true);
    

    _op_has_been_initialize = true;
}
