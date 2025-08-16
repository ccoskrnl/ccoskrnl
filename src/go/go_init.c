#include "../include/types.h"
#include "./graphics.h"
#include "./window/window.h"
#include "./window/window_text.h"
#include "./screen.h"
#include "./font/font_ttf.h"
#include "../include/libk/string.h"
#include "../include/libk/stdlib.h"
#include "../include/machine_info.h"


/* Default Wallpaper */
static struct _go_buffer background;
struct _go_buffer *_go_default_wallpaper;


/**
 * Screen
*/
// the first screen detected by ccloader.
static go_screen_desc screen0;
// all screens installed on this machine.
struct _installed_screens _go_installed_screens;
// the first screen
go_screen_desc* _go_def_screen;


/**
 * Font.
 */
struct _font_ttf* _font_family_agefonts001;
struct _font_ttf* _font_family_SourceHanSansSCVF;

// all installed font family
struct _installed_font_ttfs _go_font_ttfs;


/**
 * Windows
 */
extern uint32_t _cpuid_get_apic_id();
int output_bsp;

window_text_t *_go_cpu_output_window[32];
wch_t *_go_weclome_texts[32] = 
{
    L"Bootstrap Processor",
    L"Application Processor 1",
    L"Application Processor 2",
    L"Application Processor 3",
    L"Application Processor 4",
    L"Application Processor 5",
    L"Application Processor 6",
    L"Application Processor 7",
    L"Application Processor 8",
    L"Application Processor 9",
    L"Application Processor 10",
    L"Application Processor 11",
    L"Application Processor 12",
    L"Application Processor 13",
    L"Application Processor 14",
    L"Application Processor 15",
};


static window_style_t def_wnd = { 
    WINDOW_STYLE_COLOR,
    {0xc0,0xc0,0xc0}, 
    { 0 }
};

/**
 * A boolean type to indicate whether output module has
 * been initialized or not. Other modules should check
 * the value, if it's true. This means output module can
 * already be used.
 */
boolean _go_has_been_initialize;

void op_init()
{

    status_t status;

    // initialize first screen.
    _go_install_a_screen(
            &screen0,
            (go_blt_pixel_t*)_current_machine_info->graphics_info.FrameBufferBase,
            _current_machine_info->graphics_info.FrameBufferSize,
            _current_machine_info->graphics_info.HorizontalResolution,
            _current_machine_info->graphics_info.VerticalResolution,
            _current_machine_info->graphics_info.PixelsPerScanLine);

    _go_installed_screens.num++;
    _go_installed_screens.screen[0] = &screen0;
    _go_def_screen = _go_installed_screens.screen[0];

    // clear screen
    _go_def_screen->ClearFrameBuffers(_go_def_screen);

    background.buf = (go_blt_pixel_t*)_current_machine_info->bg.addr;
    background.height = _current_machine_info->bg.height;
    background.width = _current_machine_info->bg.width;
    background.size = _current_machine_info->bg.size;
    _go_default_wallpaper = &background;
  

    status = new_a_font(&_font_family_SourceHanSansSCVF);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }
    _font_family_SourceHanSansSCVF->init(_font_family_SourceHanSansSCVF, (uint8_t*)_current_machine_info->font[0].ttf_addr);

    status = new_a_font(&_font_family_agefonts001);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }
    _font_family_SourceHanSansSCVF->init(_font_family_agefonts001, (uint8_t*)_current_machine_info->font[1].ttf_addr);

    _go_font_ttfs.fonts[0] = _font_family_SourceHanSansSCVF;
    _go_font_ttfs.fonts[1] = _font_family_agefonts001;
    _go_font_ttfs.num = 2;


    // uncomment the following two lines to set age language as default font
    // _op_font_ttfs.fonts[0] = _font_family_agefonts001;
    // _op_font_ttfs.fonts[1] = _font_family_SourceHanSansSCVF;

    // set wallpaper
    if (_go_def_screen->horizontal == background.width && _go_def_screen->vertical == background.height)
        memcpy(_go_def_screen->frame_buf_base, _go_default_wallpaper->buf, _go_default_wallpaper->size);

    output_bsp = _cpuid_get_apic_id();

    // In initialization phrase, we create a text window for bootstrap processor.
    window_text_t *win = NULL;
    
    status = new_a_window(
        output_bsp,
        WindowText,
        NULL,
        _go_def_screen,
        _go_weclome_texts[output_bsp],
        def_wnd,
        20,
        20,
        800,
        800,
        (void**)&win
    );

    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }

    win->Register(
        win,
        _font_family_SourceHanSansSCVF,
        34,
        0
    );
    
    win->ShowWindow(win);
    _go_cpu_output_window[output_bsp] = win; 

    putwsc(output_bsp, L"Ciallo～(∠・ω< )⌒★", SPRINT_GREEN2);

    win->Register(
        win,
        _font_family_SourceHanSansSCVF,
        14,
        0
    );

    _go_has_been_initialize = true;
}

void op_init_for_ap(int lapic_id)
{
    window_text_t *win = NULL;
    status_t status;

    status = new_a_window(
        lapic_id,
        WindowText,
        NULL,
        _go_def_screen,
        _go_weclome_texts[lapic_id],
        def_wnd,
        900,
        lapic_id * 40,
        800,
        400,
        (void**)&win
    );

    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }

    win->Register(
        win,
        _font_family_SourceHanSansSCVF,
        13,
        0
    );
    
    win->ShowWindow(win);
    _go_cpu_output_window[lapic_id] = win; 

}
