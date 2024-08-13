#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/font.h"
#include "../../include/hal/op/screen.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/string.h"


/**
 * Swap framebuffer.
 * 
 * @param[in]       _this                       Descriptor instance pointer.
 * @param[in]       dest_buf_index              The index of buffer will be updated in screen descriptor.
 * @param[in]       src_buf_index               The index of source buffer in screen descriptor.
 * 
 * @retval          ST_SUCCESS                  The destination framebuffer was updated.
 * @retval          ST_INVALID_PARAMETER        One of parameters is not valid.
*/
static status_t swap_framebuffer(
    _in_ void                   *_this,
	_in_ uint8_t                dest_buf_index,
	_in_ uint8_t                src_buf_index
)
{
    status_t status = ST_SUCCESS;
    if (_this == NULL
        || !(dest_buf_index < MAX_FRAMEBUFFER)
        || !(src_buf_index < MAX_FRAMEBUFFER))
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    
    go_screen_desc* desc = (go_screen_desc*)_this;
    memcpy(desc->frame_buf_base, desc->secondary_buf, desc->frame_buf_size);

    return status;
}

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
static status_t blt(
    _in_ void                   *_this,
    _in_ _out_ go_blt_pixel     *blt_buffer,
    _in_ GO_BLT_OPERATIONS      blt_operation,
    _in_ uint64_t               src_x,
    _in_ uint64_t               src_y,
    _in_ uint64_t               dest_x,
    _in_ uint64_t               dest_y,
    _in_ uint64_t               width,
    _in_ uint64_t               height,
    _in_ _optional_ int         buffer_index )
{
    go_screen_desc* desc = (go_screen_desc*)_this;
    go_blt_pixel* framebuffer;
    status_t status = ST_SUCCESS;
    uint64_t row, column;

    if (buffer_index == BACKBUFFER_INDEX)
    {
        framebuffer = desc->secondary_buf;
    }
    else
    {
        framebuffer = desc->frame_buf_base;
    }
    

    switch (blt_operation)
    {
    case GoBltVideoFill:
        for (row = 0; row < height; row++)
        {
            for (column = 0; column < width; column++)
            {
                framebuffer[(dest_y + row) * desc->pixels_per_scanline + (dest_x + column)] = *blt_buffer;
            }
            
        }
        break;

    case GoBltVideoToBltBuffer:
        for (row = 0; row < height; row++)
        {
            for (column = 0; column < width; column++)
            {
                blt_buffer[row * width + column] = 
                    framebuffer[(src_y + row) * desc->pixels_per_scanline + (src_x + column)];
            }
        }
        break;

    case GoBltBufferToVideo:
        for (row = 0; row < height; row++)
        {
            for (column = 0; column < width; column++)
            {
                framebuffer[(dest_y + row) * desc->pixels_per_scanline + (dest_x + column)] = blt_buffer[row * width + column]; 
            }
        }
        break;

    case GoBltVideoToVideo:
        break;
    default:
        status = ST_INVALID_PARAMETER;
        break;
    }
    return status;
}

/**
 * Draw a hollow rectangle on specified screen.
 * 
 * @param[in]   _this               A pointer to the go_screen_desc instance that is the
 *                                  screen descriptor to the graphics screen.
 * @param[in]   x                   The x position of the upper left hand of a rectangle
 *                                  on specified graphics screen.
 * @param[in]   y                   The y position of the upper left hand of a rectangle
 *                                  on specified graphics screen.
 * @param[in]   width               The width of the rectangle.
 * @param[in]   height              The height of the rectangle.
 * @param[in]   stroke_size         The pixel number of stroke of the rectangle.
 * @param[in]   color               The hollow rectangle color.
 * 
 * @retval      ST_SUCCESS                  The rectangle was drawn.
 * @retval      ST_OUT_OF_RESOURCES         Not engouh resources were available to draw a rectangle.
*/
status_t _op_draw_hollow_rectangle(
    _in_ struct _go_screen_desc*        desc,
    _in_ uint32_t                       x,
    _in_ uint32_t                       y,
    _in_ uint32_t                       width,
    _in_ uint32_t                       height,
    _in_ uint32_t                       stroke_size,
    _in_ go_blt_pixel                   color
)
{
    status_t status;
    go_blt_pixel* bitbuffer;
    uint32_t i;


    // Allocate buffer to blt operation
    bitbuffer = malloc(width * height);
    if (bitbuffer == NULL)
    {
        status = ST_OUT_OF_RESOURCES;
        return status;
    }

    // fill the buffer with the specified color
    for (i = 0; i < stroke_size * width; i++)
    {
        bitbuffer[i] = color;
    }

    // draw top side
    status = desc->blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x, y, width, stroke_size, 1);
    if (ST_ERROR(status))
    {
        free(bitbuffer);
        return status;
    }

    // draw bottom side
    status = desc->blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x, y + height - stroke_size, width, stroke_size, 1);
    if (ST_ERROR(status))
    {
        free(bitbuffer);
        return status;
    }

    // reallocate buffer for vertical sides
    free(bitbuffer);
    bitbuffer = malloc(stroke_size * height * sizeof(go_blt_pixel));     
    if (bitbuffer == NULL)
    {
        return ST_OUT_OF_RESOURCES;
    }

    // fill the buffer with specified color
    for (i = 0; i < stroke_size * height; i++)
    {
        bitbuffer[i] = color;
    }

    status = desc->blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x, y, stroke_size, height, 1);
    if (ST_ERROR(status))
    {
        free(bitbuffer);
        return status;
    }

    status = desc->blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x + width - stroke_size, y, stroke_size, height, 1);
    if (ST_ERROR(status))
    {
        free(bitbuffer);
        return status;
    }

    free(bitbuffer);     
    return desc->swap_buf(desc, 0, BACKBUFFER_INDEX);
}

status_t _op_draw_char(
    _in_ struct _go_screen_desc*                    _this,
    _in_ struct _ascii_font                         *font,
    _in_ char                                       c
)
{
    status_t status = ST_SUCCESS;
    uint32_t x_of_upper_left_hand;
    uint32_t y_of_upper_left_hand;
    uint32_t row, column;
    uint32_t font_buf_width, font_buf_height;
    // uint64_t last_line;
    go_blt_pixel *pixel;
    struct _char* ch;
    
    ch = &font->chars[c];
    x_of_upper_left_hand = _this->cursor.x + ch->xoffset;
    y_of_upper_left_hand = _this->cursor.y + ch->yoffset;

    font_buf_width = font->blt_buf.width;
    font_buf_height = font->blt_buf.height;
    // last_line = (_this->vertical - font->common.lineHeight) * _this->pixels_per_scanline;

    switch (c)
    {
    case '\b':
        _this->cursor.x -= font->common.xadvance;
        break;
    case '\t':
        _this->cursor.x += (font->common.xadvance << 2);
        break;
    case ' ':
        _this->cursor.x += font->common.xadvance;
        break;
    case '\r':
        _this->cursor.x = 0;
        break;
    case '\n':
        _this->cursor.x = 0;
        _this->cursor.y += font->common.lineHeight;
        break;
    default:

        for (row = 0; row < ch->height; row++)
        {
            for (column = 0; column < ch->width; column++)
            {
                pixel = &font->blt_buf.buf_addr[(ch->y + row) * font_buf_width + ch->x + column];
                // if (pixel->Reserved != 0)
                // {
                    status = _this->blt(
                        _this,
                        pixel,
                        GoBltVideoFill,
                        0,
                        0,
                        column + x_of_upper_left_hand,
                        row + y_of_upper_left_hand,
                        1,
                        1,
                        BACKBUFFER_INDEX
                    );
                // }
                
            }
            
        }

        for (uint32_t i = 0; i < font->common.lineHeight; i++)
        {
            memcpy(
                &_this->frame_buf_base[(y_of_upper_left_hand + i) * _this->pixels_per_scanline + _this->cursor.x],
                &_this->secondary_buf[(y_of_upper_left_hand + i)* _this->pixels_per_scanline + _this->cursor.x],
                ch->xadvance * sizeof(go_blt_pixel)
            );
        }
        _this->cursor.x += ch->xadvance;
        break;

    }

    if (_this->cursor.x < 0)
        _this->cursor.x = 0;

    if (_this->cursor.x >= _this->horizontal)
    {
        _this->cursor.x = 0;
        _this->cursor.y += font->common.lineHeight;
    }

    if ((_this->vertical - _this->cursor.y) <= font->common.lineHeight)
    {
        memcpy(
            _this->secondary_buf,
            &_this->frame_buf_base[(font->common.lineHeight * _this->pixels_per_scanline)],
            _this->pixels_per_scanline * (_this->cursor.y - font->common.lineHeight) * sizeof(go_blt_pixel)
        );
        memzero(
            &_this->secondary_buf[_this->pixels_per_scanline * (_this->cursor.y - font->common.lineHeight)],
            _this->pixels_per_scanline * (_this->vertical - (_this->cursor.y - font->common.lineHeight)) * sizeof(go_blt_pixel)
        );

        _this->cursor.x = 0;
        _this->cursor.y -= font->common.lineHeight;
        _this->swap_buf(_this, 0, BACKBUFFER_INDEX);
    }
    
    return status;
}

void screen_install_funcs(struct _go_screen_desc* screen)
{
    screen->blt = blt;
    screen->swap_buf = swap_framebuffer;
}