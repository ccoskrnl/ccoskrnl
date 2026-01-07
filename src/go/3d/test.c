#include "3d.h"

#include "../../include/types.h"
#include "../../include/libk/list.h"
#include "../../include/libk/stdlib.h"

#include "../../include/arch/lib.h"

#include "../graphics.h"

#include "../window/window.h"
#include "../window/window_common.h"
#include "../screen.h"

static window_common_t* win = NULL;

static const go_blt_pixel_t BACKGROUND_COLOR = { 16, 16, 16, 0 };
static const go_blt_pixel_t FOREGROUND_COLOR = { 90, 255, 90, 0 };


point_f_t project(point_3d_f_t p)
{
    point_f_t result = { p.x / p.z, p.y / p.z };


    return result;
}


point_i_t screen(point_f_t p)
{
    point_i_t result = { 
        (int)((p.x + 1) / 2 * win->window.width),
        (int)((1 - (p.y + 1) / 2) * win->window.height)
    };

    return result;

}

void point(point_i_t p) {
    int s = 10;
    win->DrawRectangle(win, p.x - s/2, p.y - s/2, s, s, FOREGROUND_COLOR);
}
void clear() {
    win->ClearWindow(win);
}

const uint8_t FPS = 60;
int dz = 0;

void frame()
{
    const float dt = (float)1 / (float)FPS;
    dz += 1 * dt;
    point_3d_f_t p = { 0.5, 0, 1 + dz };
    clear();

    point(screen(project(p)));
    // sleep_ms(1000 / 60);
    
}

void test_for_3d()
{

    status_t status;

    window_style_t style = { 0 };
    style.color = BACKGROUND_COLOR;
    style.flags = WINDOW_STYLE_NO_BORDER | WINDOW_STYLE_BG_COLOR;

    status = new_a_window(
            '  3d',
            WindowCommon,
            NULL,
            _go_def_screen,
            NULL,
            style,
            0, 0, 1280, 720, (void**)&win);

    if (ST_ERROR(status))
        krnl_panic(NULL);

    win->Register(win);

    win->ShowWindow(win);

    // point_f_t p = { 0, -0.5 };
    // point(screen(p));

    // for (int i = 0; i < 600; i++)
    // {
    //     frame();
    // }

}
