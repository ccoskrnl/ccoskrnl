#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/screen.h"
#include "../../include/libk/math.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/string.h"
#include "../../include/libk/rbtree.h"
#include "../../include/types.h"

/**
 * @brief Clear all framebuffers.
 *
 * @param[in]       _this                       Descriptor instance pointer.
 *
 * @retval          ST_SUCCESS                  The destination framebuffer was updated.
 * @retval          ST_INVALID_PARAMETER        One of parameters is not valid.
 */
static status_t clear_screen(
        _in_ void                               *this
        ) 
{
    status_t status = ST_SUCCESS;
    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    op_screen_desc *desc = (op_screen_desc *)this;

    for (int i = 1; i < MAX_FRAMEBUFFER; i++)
        memzero(desc->frame_bufs[i].buf, desc->frame_buf_size);

    if (_go_default_wallpaper->buf != NULL)
        memcpy(desc->frame_buf_base, _go_default_wallpaper->buf, _go_default_wallpaper->size);

    return status;
}

/**
 * @brief Clear all framebuffers.
 *
 * @param[in]       _this                       Descriptor instance pointer.
 *
 * @retval          ST_SUCCESS                  The destination framebuffer was updated.
 * @retval          ST_INVALID_PARAMETER        One of parameters is not valid.
 */
static status_t clear_framebuffer(
        _in_ void                           *this
        ) 
{
    status_t status = ST_SUCCESS;
    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    op_screen_desc *desc = (op_screen_desc *)this;
    for (int i = 0; i < MAX_FRAMEBUFFER; i++)
        memzero(desc->frame_bufs[i].buf, desc->frame_buf_size);

    return status;
}

/**
 * @brief Draw a second order bezier curve.
 *
 * @param[in]       _this                       Descriptor instance pointer.
 * @param[in]       p0                          (The type of member must be float) The point is on bezier curve.
 * @param[in]       p1                          (The type of member must be float) The point is not on bezier curve.
 * @param[in]       p2                          (The type of member must be float) The point is on bezier curve.
 * @param[in]       color                       The color of bezier curve..
 *
 * @retval          ST_SUCCESS                  The destination framebuffer was updated.
 * @retval          ST_INVALID_PARAMETER        One of parameters is not valid.
 */
static status_t draw_second_order_bezier_curve(
        _in_ void                               *this,
        _in_ point_f_t                          p0,
        _in_ point_f_t                          p1,
        _in_ point_f_t                          p2,
        _in_ go_blt_pixel_t                     color
        ) 
{

    const float delta = 0.0005f;
    op_screen_desc *desc;
    status_t status = ST_SUCCESS;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    desc = (op_screen_desc *)this;

    for (float i = 0.0f; i <= 1.0f; i += delta) {
        point_f_t t1, t2, p;
        point_i_t p_i;
        t1.x = __lerp_point_x_f(p0, p1, i);
        t1.y = __lerp_point_y_f(p0, p1, i);
        t2.x = __lerp_point_x_f(p1, p2, i);
        t2.y = __lerp_point_y_f(p1, p2, i);
        p.x = __lerp_point_x_f(t1, t2, i);
        p.y = __lerp_point_y_f(t1, t2, i);

        p_i.x = round(p.x);
        p_i.y = round(p.y);

        __draw_at_point(desc, 0, p_i, color);
    }

    return status;
}

/**
 * @brief Draw a Bresenham's Line.
 *
 * @param[in]       _this                       Descriptor instance pointer.
 * @param[in]       p0                          (The type of memeber must be int) The start point of Bresenham’s Line.
 * @param[in]       p1                          (The type of memeber must be int) The end point of Bresenham’s Line.
 * @param[in]       color                       The color of the Bresenham's Line.

 * @retval          ST_SUCCESS                  The destination framebuffer was updated.
 * @retval          ST_INVALID_PARAMETER        One of parameters is not valid.
 */
static status_t draw_bresenhams_line(
        _in_ void                     *this, 
        _in_ point_i_t                p0,
        _in_ point_i_t                p1,
        _in_ go_blt_pixel_t           color
        ) 
{

    op_screen_desc *desc;

    status_t status = ST_SUCCESS;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    desc = (op_screen_desc *)this;

    if (p0.x >= desc->horizontal || p1.x >= desc->horizontal ||
            p0.y >= desc->vertical || p1.y >= desc->vertical) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    int64_t dx = abs(p1.x - p0.x);
    int64_t dy = abs(p1.y - p0.y);
    int sx = (p0.x < p1.x) ? 1 : -1;
    int sy = (p0.y < p1.y) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        __draw_at_point(desc, 0, p0, color);

        if (p0.x == p1.x && p0.y == p1.y)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            p0.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            p0.y += sy;
        }
    }

    return status;
}



/**
 * @brief Swap framebuffer.
 *
 * @param[in]       _this                       Descriptor instance pointer.
 * @param[in]       dest_buf_index              The index of buffer to indicate which buffer will be updated in screen descriptor.
 * @param[in]       src_buf_index               The index of source buffer in screen descriptor.
 *
 * @retval          ST_SUCCESS                  The destination framebuffer was updated.
 * @retval          ST_INVALID_PARAMETER        One of parameters is not valid.
 */
static status_t swap_framebuffer(
        _in_ void                   *this, 
        _in_ uint8_t                dest_buf_index,
        _in_ uint8_t                src_buf_index
        )
{
    op_screen_desc *desc;
    status_t status = ST_SUCCESS;
    if (this == NULL || !(dest_buf_index < MAX_FRAMEBUFFER) ||
            !(src_buf_index < MAX_FRAMEBUFFER)) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    desc = (op_screen_desc *)this;

    memcpy(desc->frame_bufs[dest_buf_index].buf, desc->frame_bufs[src_buf_index].buf,
            desc->frame_buf_size);

    return status;
}

/**
 * Blt a rectangle of pixels on the graphics screen. Blt stands for Block
 * Transfer.
 *
 * @param[in]       _this                       Descriptor instance pointer.
 * @param[in out]   blt_buffer                  The data to transfer to the graphics screen. Size is at least width * height * sizeof(go_blt_pixel).
 * @param[in]       blt_operation               The operation to perfrom when copying blt_buffer on to the graphics screen.
 * @param[in]       src_x                       The x coordinate of source for the blt_operation.
 * @param[in]       src_y                       The y coordinate of source for the blt_operation.
 * @param[in]       dest_x                      The x coordinate of destination for the blt_operation.
 * @param[in]       dest_y                      The y coordinate of destination for the blt_operation.
 * @param[in]       width                       The width of a rectangle in the blt rectangle in pixels.
 * @param[in]       height                      The height of a rectangle in the blt rectangle in pixels.
 * @param[in]       buffer_index                Which buffer will be drawn.
 *
 * @retval          ST_SUCCESS                  blt_buffer was drawn to the graphics screen.
 * @retval          ST_INVALID_PARAMETER        blt_operation is not valid.
 */
static status_t blt(
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
) 
{

    op_screen_desc *this;
    go_blt_pixel_t *framebuffer;
    status_t status = ST_SUCCESS;
    uint64_t row, column;


    if (_this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    this = (op_screen_desc *)_this;

    if (   dest_x >= this->horizontal 
        || dest_y >= this->vertical
        || dest_x < 0
        || dest_y < 0
        || src_x < 0
        || src_y < 0
        || src_x >= drawing_width 
        || src_y >= drawing_height 
       ) 
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    framebuffer = this->frame_bufs[buffer_index].buf;

    switch (blt_operation) {
        case GoBltVideoFill:
            for (row = 0; row < drawing_height; row++) {
                for (column = 0; column < drawing_width; column++) {
                    framebuffer[(dest_y + row) * this->pixels_per_scanline +
                        (dest_x + column)] = *blt_buffer;
                }
            }
            break;

        case GoBltVideoToBltBuffer:
            for (row = 0; row < drawing_height; row++) {
                for (column = 0; column < drawing_width; column++) {
                    blt_buffer[row * drawing_width + column] =
                        framebuffer[(src_y + row) * this->pixels_per_scanline +
                        (src_x + column)];
                }
            }
            break;

        case GoBltBufferToVideo:
            for (row = 0; row < drawing_height; row++) 
            {
                memcpy(
                    this->frame_bufs[buffer_index].buf + (dest_y + row) * this->pixels_per_scanline + dest_x,
                    blt_buffer + (src_y + row) * src_buf_width + src_x,
                    drawing_width * sizeof(go_blt_pixel_t)
                );
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
 * @brief Draw a hollow rectangle on specified screen.
 *
 * @param[in]   _this               A pointer to the go_screen_desc instance that is the screen descriptor to the graphics screen.
 * @param[in]   x                   The x position of the upper left hand of a rectangle on specified graphics screen.
 * @param[in]   y                   The y position of the upper left hand of a rectangle on specified graphics screen.
 * @param[in]   width               The width of the rectangle.
 * @param[in]   height              The height of the rectangle.
 * @param[in]   stroke_size         The pixel number of stroke of the rectangle.
 * @param[in]   color               The hollow rectangle color.
 *
 * @retval      ST_SUCCESS                  The rectangle was drawn.
 * @retval      ST_OUT_OF_RESOURCES         Not engouh resources were available to draw a rectangle.
 * @retval      ST_INVALID_PARAMETER        One of parameters is not valid.
 */
static status_t draw_hollow_rectangle(
        _in_ void                               *_this, 
        _in_ uint32_t                           x,
        _in_ uint32_t                           y, 
        _in_ uint32_t                           width,
        _in_ uint32_t                           height,
        _in_ uint32_t                           stroke_size,
        _in_ go_blt_pixel_t                     color,
        _in_ int                                buf_index
        ) 
{
    status_t status;
    go_blt_pixel_t *bitbuffer;
    uint32_t i;
    op_screen_desc *this;

    if (_this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    this = _this;

    if (   x >= this->horizontal 
        || y >= this->vertical
        || width > this->horizontal
        || height > this->vertical 
        || width == 0
        || height == 0
       ) 
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    go_blt_pixel_t* buf = this->frame_bufs[buf_index].buf + y * this->horizontal + x;

    // The maximum stroke size is the minimum value of the height or width of the rectangle
    if  (stroke_size > width || stroke_size > height)
        stroke_size = width < height ? width : height;
    
    boolean do_we_need_to_draw_right_side = x + width - stroke_size <= this->horizontal;
    boolean do_we_need_to_draw_buttom_side = y + height - stroke_size <= this->vertical;

    // draw top side
    int drawing_width = x + width <= this->horizontal ? width : this->horizontal - x;
    int drawing_height = y + stroke_size <= this->vertical ? stroke_size : this->vertical - y;

    // draw top side
    drawing_height = stroke_size;
    for (int i = 0; i < drawing_height; i++) 
        memsetd((uint32_t*)(buf + i * this->horizontal), *(uint32_t*)&color, drawing_width);

    // draw left side     
    if (y + stroke_size > this->vertical)
    {
        return status;
    }
    else
    {
        buf = this->frame_bufs[buf_index].buf + (y + stroke_size) * this->horizontal + x;
        drawing_height = y + height <= this->vertical ? height - stroke_size : this->vertical - y - stroke_size;
        if (y + height <= this->vertical)
        {
            drawing_height = height - stroke_size;
        }
        else
        {
            drawing_height = this->vertical - y - stroke_size;
            do_we_need_to_draw_buttom_side = false;
        }

        if (x + stroke_size <= this->horizontal) 
        {
            drawing_width = stroke_size;
        }
        else
        {
            drawing_width = this->horizontal - x;
            do_we_need_to_draw_right_side = false;
        }

        for (int i = 0; i < drawing_height; i++) 
            memsetd((uint32_t*)(buf + i * this->horizontal), *(uint32_t*)&color, drawing_width);
    }

    // draw right side
    if (do_we_need_to_draw_right_side)
    {
        buf = this->frame_bufs[buf_index].buf + (y + stroke_size) * this->horizontal + (x + width - stroke_size);
        drawing_width = this->horizontal - x - width >= 0 ? stroke_size : (this->horizontal - x - width) + stroke_size;
        for (int i = 0; i < drawing_height; i++) 
            memsetd((uint32_t*)(buf + i * this->horizontal), *(uint32_t*)&color, drawing_width);
    }
    
    if (do_we_need_to_draw_buttom_side)
    {
        go_blt_pixel_t* buf = this->frame_bufs[buf_index].buf + (y + height - stroke_size) * this->horizontal + x;
        int drawing_width = x + width <= this->horizontal ? width : this->horizontal - x;
        int drawing_height = y + height <= this->vertical ? stroke_size : this->vertical - y - height + stroke_size;

        for (int i = 0; i < drawing_height; i++) 
            memsetd((uint32_t*)(buf + i * this->horizontal), *(uint32_t*)&color, drawing_width);
    }


    return status;
}


static status_t draw_rectangle(
    _in_ void                                   *_this,
    _in_ int                                    x,
    _in_ int                                    y,
    _in_ int                                    width,
    _in_ int                                    height,
    _in_ go_blt_pixel_t                         color,
	_in_ int                            		buf_index
)
{
    op_screen_desc *this = _this;
    status_t status;
    go_blt_pixel_t col = color;
    uint32_t c = *(uint32_t*)&col;
    go_blt_pixel_t* buf = this->frame_bufs[buf_index].buf + y * this->horizontal + x;

    if (   x >= this->horizontal 
        || y >= this->vertical
        || width > this->horizontal
        || height > this->vertical 
       ) 
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    int drawing_width = x + width <= this->horizontal ? width : this->horizontal - x;
    int drawing_height = y + height <= this->vertical ? height : this->vertical - y;

    for (int i = 0; i < drawing_height; i++) 
        memsetd((uint32_t*)(buf + i * this->horizontal), c, drawing_width);

    return status;
}

void _op_install_a_screen(
    _in_ _out_ struct _op_screen_desc       *screen,
    _in_ go_blt_pixel_t                     *frame_buf_base, 
    _in_ size_t                             frame_buf_size,
    _in_ int                                horizontal_resolution, 
    _in_ int                                vertical_resolution,
    _in_ int                                pixels_per_scan_line
) 
{

    memzero(screen, sizeof(*screen));

    screen->frame_buf_base = frame_buf_base;
    screen->frame_buf_size = frame_buf_size;
    screen->horizontal = horizontal_resolution;
    screen->vertical = vertical_resolution;
    screen->pixels_per_scanline = pixels_per_scan_line;

    screen->frame_bufs[0].buf = screen->frame_buf_base;
    screen->frame_bufs[0].size = screen->frame_buf_size;
    screen->frame_bufs[0].height = screen->vertical;
    screen->frame_bufs[0].width = screen->horizontal;

    for (int i = 1; i < MAX_FRAMEBUFFER; i++) {
        screen->frame_bufs[i].buf = (go_blt_pixel_t *)malloc(screen->frame_buf_size);
        screen->frame_bufs[i].size = screen->frame_buf_size;
        screen->frame_bufs[i].height = screen->vertical;
        screen->frame_bufs[i].width = screen->horizontal;
    }

    screen->secondary_buf = screen->frame_bufs[BACKBUFFER_INDEX].buf;

    status_t status = new_a_rbtree(&screen->windows);
    if (ST_ERROR(status)) {
        krnl_panic();
    }

    screen->Blt = blt;
    screen->SwapTwoBuffers = swap_framebuffer;
    screen->DrawRectangle = draw_rectangle;
    screen->DrawHollowRectangle = draw_hollow_rectangle;
    screen->DrawSecondOrderBezierCurve = draw_second_order_bezier_curve;
    screen->DrawBresenhamsLine = draw_bresenhams_line;
    screen->ClearFrameBuffers = clear_framebuffer;
    screen->ClearScreen = clear_screen;
}
