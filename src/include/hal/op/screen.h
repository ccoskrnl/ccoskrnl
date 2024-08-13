#ifndef __OP_SCREEN_H__
#define __OP_SCREEN_H__

#include "../../types.h"
#include "graphics.h"
#include "font.h"

#define MAX_INSTALLED_SCREEN								63
#define BACKBUFFER_INDEX									1
#define MAX_FRAMEBUFFER										2


struct _coordinates_2d
{
	int64_t x;
	int64_t y;
};

/**
 * Blt a rectangle of pixels on the graphics screen. Blt stands for Block Transfer.
 * 
 * @param[in]       _this                       Descriptor instance pointer.
 * @param[in out]   blt_buffer                  The data to transfer to the graphics screen.
 *                                              Size is at least width * height * sizeof(go_blt_pixel).
 * @param[in]       blt_operation               The operation to perfrom when copying blt_buffer on to the graphics screen.
 * @param[in]       src_x                       The x coordinate of source for the blt_operation.
 * @param[in]       src_y                       The y coordinate of source for the blt_operation.
 * @param[in]       dest_x                      The x coordinate of destination for the blt_operation.
 * @param[in]       dest_y                      The y coordinate of destination for the blt_operation.
 * @param[in]       width                       The width of a rectangle in the blt rectangle in pixels.
 * @param[in]       height                      The height of a rectangle in the blt rectangle in pixels.
 * @param[in]       buffer_index                Which buffer will be drawn.
 * 
 * 
 * @retval          ST_SUCCESS                  blt_buffer was drawn to the graphics screen.
 * @retval          ST_INVALID_PARAMETER        blt_operation is not valid.
*/
typedef status_t (*_go_blt)(
    _in_ void                   *_this,
    _in_ _out_ go_blt_pixel     *blt_buffer,
    _in_ GO_BLT_OPERATIONS      blt_operation,
    _in_ uint64_t               src_x,
    _in_ uint64_t               src_y,
    _in_ uint64_t               dest_x,
    _in_ uint64_t               dest_y,
    _in_ uint64_t               width,
    _in_ uint64_t               height,
    _in_ _optional_ int         buffer_index );

typedef status_t (*_go_swap_framebuffer)(
    _in_ void                   *_this,
	_in_ uint8_t                dest_buf_index,
	_in_ uint8_t                src_buf_index
);


typedef struct _go_screen_desc
{

	// go_screen_desc_base
	struct
	{
		go_blt_pixel* frame_buf_base;
		go_blt_pixel* secondary_buf;
		uint64_t frame_buf_size;
		int64_t vertical;
		int64_t horizontal;
		int64_t pixels_per_scanline;
	};

	struct _coordinates_2d cursor;

	_go_blt blt;
	_go_swap_framebuffer swap_buf;
} go_screen_desc;

struct _installed_screen {
	int64_t num;
	struct _go_screen_desc* screen[MAX_INSTALLED_SCREEN];
};

status_t _op_draw_char(
    _in_ struct _go_screen_desc*                    _this,
    _in_ struct _ascii_font                         *font,
    _in_ char                                       c
);

status_t _op_draw_hollow_rectangle(
    _in_ struct _go_screen_desc*        desc,
    _in_ uint32_t                       x,
    _in_ uint32_t                       y,
    _in_ uint32_t                       width,
    _in_ uint32_t                       height,
    _in_ uint32_t                       stroke_size,
    _in_ go_blt_pixel                   color
);


extern struct _installed_screen _op_screen;
extern go_screen_desc* _op_def_screen;


#endif
