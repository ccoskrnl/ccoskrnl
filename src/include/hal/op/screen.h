#ifndef __OP_SCREEN_H__
#define __OP_SCREEN_H__

#include "../../types.h"
#include "graphics.h"
#include "font.h"

#define MAX_INSTALLED_SCREEN								63
#define BACKBUFFER_INDEX									1
#define MAX_FRAMEBUFFER										2


typedef status_t (*_op_blt)(
    _in_ void                   *_this,
    _in_ _out_ go_blt_pixel_t     *blt_buffer,
    _in_ GO_BLT_OPERATIONS      blt_operation,
    _in_ uint64_t               src_x,
    _in_ uint64_t               src_y,
    _in_ uint64_t               dest_x,
    _in_ uint64_t               dest_y,
    _in_ uint64_t               width,
    _in_ uint64_t               height,
    _in_ _optional_ int         buffer_index );

typedef status_t (*_op_swap_framebuffer)(
    _in_ void                   *_this,
	_in_ uint8_t                dest_buf_index,
	_in_ uint8_t                src_buf_index
);

typedef status_t (*_op_draw_char)(
    _in_ void*                                      _this,
    _in_ struct _ascii_font                         *font,
    _in_ char                                       c
);

typedef status_t (*_op_draw_hollow_rectangle)(
    _in_ void*                          _this,
    _in_ uint32_t                       x,
    _in_ uint32_t                       y,
    _in_ uint32_t                       width,
    _in_ uint32_t                       height,
    _in_ uint32_t                       stroke_size,
    _in_ go_blt_pixel_t                   color
);


typedef status_t (*_op_draw_second_order_bezier_curve)(
    _in_ void                       *_this,
    _in_ point_f_t                   p0,
    _in_ point_f_t                   p1,
    _in_ point_f_t                   p2,
    _in_ go_blt_pixel_t              color
);

typedef status_t (*_op_draw_bresenhams_line)(
    _in_ void                       *_this,
    _in_ point_i_t                   p0,
    _in_ point_i_t                   p1,
    _in_ go_blt_pixel_t              color
);

typedef status_t (*_op_clear_framebuffer)(
    _in_ void                   *_this
);

typedef struct _op_screen_desc
{

	// go_screen_desc_base
	struct
	{
		go_blt_pixel_t* frame_buf_base;
		go_blt_pixel_t* secondary_buf;
        go_blt_pixel_t* frame_bufs[MAX_FRAMEBUFFER];
		uint64_t frame_buf_size;
		int64_t vertical;
		int64_t horizontal;
		int64_t pixels_per_scanline;
	};

	struct _coordinates_2d_i cursor;
    struct 
    {
        _op_blt blt;
        _op_swap_framebuffer swap_two_buffers;
        _op_draw_char draw_char;
        _op_draw_hollow_rectangle draw_hollow_rectangle;
        _op_draw_second_order_bezier_curve draw_second_order_bezier_curve;
        _op_draw_bresenhams_line draw_bresenhams_line;
        _op_clear_framebuffer clear_framebuffer;
    };
} op_screen_desc;

struct _installed_screens {
	int64_t num;
	struct _op_screen_desc* screen[MAX_INSTALLED_SCREEN];
};


extern struct _installed_screens _op_installed_screens;
extern op_screen_desc* _op_def_screen;


#endif
