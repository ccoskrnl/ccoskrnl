#ifndef __OP_SCREEN_H__
#define __OP_SCREEN_H__

#include "../include/types.h"
#include "../include/libk/rbtree.h"
#include "../include/libk/lock.h"
#include "graphics.h"
#include "./font/font_ttf.h"

#define MAX_INSTALLED_SCREEN					                    16
#define FRAMEBUFFER_INDEX                                           0
#define BACKBUFFER_INDEX					                        1
#define MAX_FRAMEBUFFER						                        2


typedef status_t (*_go_blt_t)(
    _in_ void                               *_this, 
    _in_ _out_ go_blt_pixel_t               *blt_buffer,
    _in_ GO_BLT_OPERATIONS                  blt_operation, 
    _in_ uint64_t                           src_x,
    _in_ uint64_t                           src_y, 
    _in_ uint16_t                           src_buf_width, 
    _in_ uint16_t                           src_buf_height, 
    _in_ uint64_t                           dest_x,
    _in_ uint64_t                           dest_y, 
    _in_ uint16_t                           drawing_width,
    _in_ uint16_t                           drawing_height, 
    _in_ _optional_ int                     buffer_index
);

typedef status_t (*_go_swap_framebuffer_t)(
	_in_ void                   *_this,
	_in_ uint8_t                dest_buf_index,
	_in_ uint8_t                src_buf_index
);


typedef status_t (*_go_draw_rectangle_t)(
    _in_ void                                   *_this,
    _in_ int                                    x,
    _in_ int                                    y,
    _in_ int                                    width,
    _in_ int                                    height,
    _in_ go_blt_pixel_t                         color,
	_in_ int                            		buf_index
);

typedef status_t (*_go_draw_hollow_rectangle_t)(
	_in_ void                           	*_this,
	_in_ uint32_t                       	x,
	_in_ uint32_t                       	y,
	_in_ uint32_t                       	width,
	_in_ uint32_t                       	height,
	_in_ uint32_t                       	stroke_size,
	_in_ go_blt_pixel_t                 	color,
	_in_ int                            	buf_index
);


typedef status_t (*_go_draw_second_order_bezier_curve_t)(
	_in_ void                       			*_this,
	_in_ point_f_t                   			p0,
	_in_ point_f_t                   			p1,
	_in_ point_f_t                   			p2,
	_in_ go_blt_pixel_t              			color
);

typedef status_t (*_go_draw_bresenhams_line_t)(
	_in_ void                       			*_this,
	_in_ point_i_t                   			p0,
	_in_ point_i_t                   			p1,
	_in_ go_blt_pixel_t              			color
);

typedef status_t (*_go_clear_screen_t)(
	_in_ void 									*_this
);

typedef status_t (*_go_clear_framebuffers_t)(
	_in_ void                   				*_this
);


typedef struct _go_screen_desc
{

    go_blt_pixel_t* frame_buf_base;
    go_blt_pixel_t* secondary_buf;
    go_buf_t frame_bufs[MAX_FRAMEBUFFER];
    uint64_t frame_buf_size;
    int64_t vertical;
    int64_t horizontal;
    int64_t pixels_per_scanline;

    spinlock_t spinlock;
    rbtree_t *windows;

    _go_blt_t Blt;
    _go_swap_framebuffer_t SwapTwoBuffers;
    _go_draw_rectangle_t DrawRectangle;
    _go_draw_hollow_rectangle_t DrawHollowRectangle;
    _go_draw_second_order_bezier_curve_t DrawSecondOrderBezierCurve;
    _go_draw_bresenhams_line_t DrawBresenhamsLine;
    _go_clear_framebuffers_t ClearFrameBuffers;
    _go_clear_screen_t ClearScreen;

} go_screen_desc;

struct _installed_screens {
    int64_t num;
    struct _go_screen_desc* screen[MAX_INSTALLED_SCREEN];
};

#define __lerp_point_x_f(p1, p2, t)   \
    (p1.x * t + p2.x * (1 - t))

#define __lerp_point_y_f(p1, p2, t)   \
    (p1.y * t + p2.y * (1 - t))

#define __draw_at_point(desc, buf_id, p, color)    \
    desc->frame_bufs[buf_id].buf[(uint16_t)p.y * desc->horizontal + (uint16_t)p.x] = color


void _go_install_a_screen(
    struct _go_screen_desc* screen,
    go_blt_pixel_t* frame_buf_base,
    size_t frame_buf_size,
    int horizontal_resolution,
    int vertical_resolution,
    int pixels_per_scan_line
);


extern struct _go_buffer *_go_default_wallpaper;
extern struct _installed_screens _go_installed_screens;
extern go_screen_desc* _go_def_screen;
extern boolean _go_has_been_initialize;

#endif
