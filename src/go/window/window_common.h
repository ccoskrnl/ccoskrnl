#ifndef __OP_WINDOW_COMMON_H__
#define __OP_WINDOW_COMMON_H__

#include "../../include/types.h"
#include "../../include/libk/rbtree.h"
#include "../font/font_ttf.h"
#include "../graphics.h"
#include "./window.h"

typedef void (*_window_clear_t)(
        _in_ void                   *_this
);

typedef status_t (*_window_show_t)(
        _in_ void                   *_this
);

typedef status_t (*_window_register_t)(
        _in_ void                  *_this
);

typedef status_t (*_window_draw_rectangle_t)(
        _in_ void                                   *_this,
        _in_ int                                    x,
        _in_ int                                    y,
        _in_ int                                    width,
        _in_ int                                    height,
        _in_ go_blt_pixel_t                         color
);

typedef struct _window_common_t {

    /**
     * Window
     **/
    window_t                   window;

    _window_register_t         Register;
    _window_show_t             ShowWindow;
    _window_clear_t            ClearWindow;
    _window_draw_rectangle_t    DrawRectangle;

} window_common_t;

#endif
