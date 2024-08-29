#ifndef __OP_SCREEN_H__
#define __OP_SCREEN_H__

#include "../../types.h"
#include "../../libk/rbtree.h"
#include "graphics.h"
#include "./font/font_ttf.h"

#define MAX_INSTALLED_SCREEN								        63
#define FRAMEBUFFER_INDEX                                           0
#define BACKBUFFER_INDEX									        1
#define MAX_FRAMEBUFFER										        2
#define OUTPUT_BUF_SIZE                                             2048

typedef status_t (*_op_blt_t)(
	_in_ void                   *_this,
	_in_ _out_ go_blt_pixel_t     *blt_buffer,
	_in_ GO_BLT_OPERATIONS      blt_operation,
	_in_ uint64_t               src_x,
	_in_ uint64_t               src_y,
	_in_ uint64_t               dest_x,
	_in_ uint64_t               dest_y,
	_in_ uint64_t               width,
	_in_ uint64_t               height,
	_in_ _optional_ int         buffer_index 
);

typedef status_t (*_op_swap_framebuffer_t)(
	_in_ void                   *_this,
	_in_ uint8_t                dest_buf_index,
	_in_ uint8_t                src_buf_index
);


typedef status_t (*_op_draw_rectangle_t)(
    _in_ void                                   *_this,
    _in_ int                                    x,
    _in_ int                                    y,
    _in_ int                                    width,
    _in_ int                                    height,
    _in_ go_blt_pixel_t                         color,
	_in_ int                            		buf_index
);

typedef status_t (*_op_draw_hollow_rectangle_t)(
	_in_ void                           	*_this,
	_in_ uint32_t                       	x,
	_in_ uint32_t                       	y,
	_in_ uint32_t                       	width,
	_in_ uint32_t                       	height,
	_in_ uint32_t                       	stroke_size,
	_in_ go_blt_pixel_t                 	color,
	_in_ int                            	buf_index
);


typedef status_t (*_op_draw_second_order_bezier_curve_t)(
	_in_ void                       			*_this,
	_in_ point_f_t                   			p0,
	_in_ point_f_t                   			p1,
	_in_ point_f_t                   			p2,
	_in_ go_blt_pixel_t              			color
);

typedef status_t (*_op_draw_bresenhams_line_t)(
	_in_ void                       			*_this,
	_in_ point_i_t                   			p0,
	_in_ point_i_t                   			p1,
	_in_ go_blt_pixel_t              			color
);

typedef status_t (*_op_clear_screen_t)(
	_in_ void 									*_this
);

typedef status_t (*_op_clear_framebuffers_t)(
	_in_ void                   				*_this
);

typedef status_t (*_op_draw_string_t)(
    _in_ void                                   *_this,
    _in_ wch_t                                  *string,
    _in_ point_i_t                              origin,
    _in_ font_ttf_t                             *font_family,
    _in_ double                                 point_size,
    _in_ go_blt_pixel_t                         color,
	_in_ int                            		buf_index
);


typedef struct _op_screen_desc
{

    go_blt_pixel_t* frame_buf_base;
    go_blt_pixel_t* secondary_buf;
    go_blt_pixel_t* frame_bufs[MAX_FRAMEBUFFER];
    uint64_t frame_buf_size;
    int64_t vertical;
    int64_t horizontal;
    int64_t pixels_per_scanline;

    rbtree_t *windows;

    _op_blt_t Blt;
    _op_swap_framebuffer_t SwapTwoBuffers;
    _op_draw_string_t DrawString;
    _op_draw_rectangle_t DrawRectangle;
    _op_draw_hollow_rectangle_t DrawHollowRectangle;
    _op_draw_second_order_bezier_curve_t DrawSecondOrderBezierCurve;
    _op_draw_bresenhams_line_t DrawBresenhamsLine;
    _op_clear_framebuffers_t ClearFrameBuffers;
    _op_clear_screen_t clear_screen;

    size_t output_buf_index;
    size_t which_output_buf;

    uint64_t output_buf[2][2048];

} op_screen_desc;

struct _installed_screens {
    int64_t num;
    struct _op_screen_desc* screen[MAX_INSTALLED_SCREEN];
};

#define __lerp_point_x_f(p1, p2, t)   \
    (p1.x * t + p2.x * (1 - t))

#define __lerp_point_y_f(p1, p2, t)   \
    (p1.y * t + p2.y * (1 - t))

#define __draw_at_point(desc, buf_id, p, color)    \
    desc->frame_bufs[buf_id][(uint16_t)p.y * desc->horizontal + (uint16_t)p.x] = color


#define TAB_SIZE                                        4



/* Define output area in screen */
// output area top left-hand corner 
#define OUTPUT_AREA_TLC_X                               40
#define OUTPUT_AREA_TLC_Y                               24

// output area bottom right-hand corner
#define OUTPUT_AREA_BRC_X                               1210
#define OUTPUT_AREA_BRC_Y                               1050


void _op_install_a_screen(
    struct _op_screen_desc* screen,
    go_blt_pixel_t* frame_buf_base,
    size_t frame_buf_size,
    int horizontal_resolution,
    int vertical_resolution,
    int pixels_per_scan_line
);


extern struct _go_image_output *_op_bg;
extern struct _installed_screens _op_installed_screens;
extern op_screen_desc* _op_def_screen;
extern boolean _op_has_been_initialize;

#endif
