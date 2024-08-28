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
        memzero(desc->frame_bufs[i], desc->frame_buf_size);

    memcpy(desc->frame_buf_base, _op_bg->buf, _op_bg->size);

    desc->cursor.x = (int)LSB_SIZE + OUTPUT_AREA_TLC_X;
    desc->cursor.y = OUTPUT_AREA_TLC_Y;

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
        memzero(desc->frame_bufs[i], desc->frame_buf_size);

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

    memcpy(desc->frame_bufs[dest_buf_index], desc->frame_bufs[src_buf_index],
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
        _in_ void                               *this, 
        _in_ _out_ go_blt_pixel_t               *blt_buffer,
        _in_ GO_BLT_OPERATIONS                  blt_operation, 
        _in_ uint64_t                           src_x,
        _in_ uint64_t                           src_y, 
        _in_ uint64_t                           dest_x,
        _in_ uint64_t                           dest_y, 
        _in_ uint64_t                           width,
        _in_ uint64_t                           height, 
        _in_ _optional_ int                     buffer_index
        ) 
{

    op_screen_desc *desc;
    go_blt_pixel_t *framebuffer;
    status_t status = ST_SUCCESS;
    uint64_t row, column;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    desc = (op_screen_desc *)this;

    if (buffer_index == BACKBUFFER_INDEX) {
        framebuffer = desc->secondary_buf;
    } else {
        framebuffer = desc->frame_buf_base;
    }

    switch (blt_operation) {
        case GoBltVideoFill:
            for (row = 0; row < height; row++) {
                for (column = 0; column < width; column++) {
                    framebuffer[(dest_y + row) * desc->pixels_per_scanline +
                        (dest_x + column)] = *blt_buffer;
                }
            }
            break;

        case GoBltVideoToBltBuffer:
            for (row = 0; row < height; row++) {
                for (column = 0; column < width; column++) {
                    blt_buffer[row * width + column] =
                        framebuffer[(src_y + row) * desc->pixels_per_scanline +
                        (src_x + column)];
                }
            }
            break;

        case GoBltBufferToVideo:
            for (row = 0; row < height; row++) {
                for (column = 0; column < width; column++) {
                    framebuffer[(dest_y + row) * desc->pixels_per_scanline +
                        (dest_x + column)] = blt_buffer[row * width + column];
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
        _in_ void                               *this, 
        _in_ uint32_t                           x,
        _in_ uint32_t                           y, 
        _in_ uint32_t                           width,
        _in_ uint32_t                           height,
        _in_ uint32_t                           stroke_size,
        _in_ go_blt_pixel_t                     color
        ) 
{
    status_t status;
    go_blt_pixel_t *bitbuffer;
    uint32_t i;
    op_screen_desc *desc;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    desc = this;

    // Allocate buffer to blt operation
    bitbuffer = malloc(width * height);
    if (bitbuffer == NULL) {
        status = ST_OUT_OF_RESOURCES;
        return status;
    }

    // fill the buffer with the specified color
    for (i = 0; i < stroke_size * width; i++) {
        bitbuffer[i] = color;
    }

    // draw top side
    status = desc->Blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x, y, width,
            stroke_size, 1);
    if (ST_ERROR(status)) {
        free(bitbuffer);
        return status;
    }

    // draw bottom side
    status = desc->Blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x,
            y + height - stroke_size, width, stroke_size, 1);
    if (ST_ERROR(status)) {
        free(bitbuffer);
        return status;
    }

    // reallocate buffer for vertical sides
    free(bitbuffer);
    bitbuffer = malloc(stroke_size * height * sizeof(go_blt_pixel_t));
    if (bitbuffer == NULL) {
        return ST_OUT_OF_RESOURCES;
    }

    // fill the buffer with specified color
    for (i = 0; i < stroke_size * height; i++) {
        bitbuffer[i] = color;
    }

    status = desc->Blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0, x, y,
            stroke_size, height, 1);
    if (ST_ERROR(status)) {
        free(bitbuffer);
        return status;
    }

    status = desc->Blt(desc, bitbuffer, GoBltBufferToVideo, 0, 0,
            x + width - stroke_size, y, stroke_size, height, 1);
    if (ST_ERROR(status)) {
        free(bitbuffer);
        return status;
    }

    free(bitbuffer);
    return desc->SwapTwoBuffers(desc, 0, BACKBUFFER_INDEX);
}

static status_t scroll_screen(
        _in_ void                               *this, 
        _in_ font_ttf_t                         *family
        ) 
{
    status_t status = ST_SUCCESS;
    op_screen_desc *desc;
    // int old_buffer_index;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    desc = this;

    uint64_t *lf = desc->output_buf[desc->which_output_buf];

    for (; *(wch_t *)lf != 0; ++lf) {
        if (*(wch_t *)lf == '\n') {
            lf++;
            break;
        }
    }

    desc->which_output_buf ^= 1;
    desc->cursor.x = LSB_SIZE + OUTPUT_AREA_TLC_X;
    desc->cursor.y = OUTPUT_AREA_TLC_Y;
    desc->output_buf_index = 0;

    memzero(desc->output_buf[desc->which_output_buf], OUTPUT_BUF_SIZE);
    memcpy(desc->frame_bufs[BACKBUFFER_INDEX], _op_bg->buf, _op_bg->size);

    while (*lf != 0) {
        go_blt_pixel_t *color = (go_blt_pixel_t *)((uint64_t)(lf) + sizeof(wch_t));
        wch_t wc = *lf & 0xffffffff;

        status = desc->DrawChar(desc, BACKBUFFER_INDEX, wc, family, *color);
        if (ST_ERROR(status)) {
            krnl_panic();
        }

        lf++;
    }

    desc->SwapTwoBuffers(desc, 0, BACKBUFFER_INDEX);

    return status;
}

static status_t draw_ch(
        _in_ void                               *this, 
        _in_ int                                buf_id, 
        _in_ wch_t                              wch,
        _in_ font_ttf_t                         *family, 
        _in_ go_blt_pixel_t                     color
        ) 
{

    status_t status = ST_SUCCESS;
    op_screen_desc *desc;
    font_ttf_glyph_t *glyph;
    point_i_t origin;
    int TAB = 0;
    int LF = 0;
    uint64_t cwch;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    desc = this;

    cwch = *(uint32_t *)&color;
    cwch <<= 32;
    cwch |= wch;

    desc->output_buf[desc->which_output_buf][desc->output_buf_index++] = cwch;

__inspect_begin:

    // Situation 0:
    // Current row can't put any characters, so we need to wrap.
    if (desc->cursor.x + family->space_advance_width >= OUTPUT_AREA_BRC_X) {
        desc->cursor.y += family->line_height;
        desc->cursor.x = (int)LSB_SIZE + OUTPUT_AREA_TLC_X;
    }
    // Situation 1:
    // scroll screen
    if ((desc->cursor.y + family->line_height) >= OUTPUT_AREA_BRC_Y)
        scroll_screen(desc, family);

    switch (wch) {
        case ' ':
            desc->cursor.x += family->space_advance_width;
            goto __draw_ch_exit;
            break;

        case '\t':
            desc->cursor.x += family->space_advance_width;
            while (TAB < TAB_SIZE) {
                TAB++;
                goto __inspect_begin;
            }
            goto __draw_ch_exit;
            break;

        case '\b':
            if (desc->cursor.x == (int)LSB_SIZE) {
                goto __draw_ch_exit;
            }
            desc->cursor.x -= family->space_advance_width;
            goto __draw_ch_exit;
            break;

        case '\n':

            desc->cursor.y += family->line_height;
            desc->cursor.x = (int)LSB_SIZE + OUTPUT_AREA_TLC_X;
            if ((desc->cursor.y + family->line_height) >= OUTPUT_AREA_BRC_Y)
                scroll_screen(desc, family);
            goto __draw_ch_exit;
            break;

        default:
            break;
    }

    // Situation 2:
    // Current row can still continue to put characters more than one.
    origin.x = desc->cursor.x;
    origin.y = desc->cursor.y;

    status = new_a_glyph(&glyph);
    if (ST_ERROR(status)) {
        return status;
    }
    glyph->init(glyph, wch, family);
    glyph->rasterize(glyph, desc, buf_id, origin, color);

    del_a_glyph(glyph);

__draw_ch_exit:

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

    for (int i = 1; i < MAX_FRAMEBUFFER; i++) {
        screen->frame_bufs[i] = (go_blt_pixel_t *)malloc(screen->frame_buf_size);
    }
    screen->frame_bufs[0] = screen->frame_buf_base;
    screen->secondary_buf = screen->frame_bufs[BACKBUFFER_INDEX];

    screen->cursor.x = (int)LSB_SIZE + OUTPUT_AREA_TLC_X;
    screen->cursor.y = OUTPUT_AREA_TLC_Y;

    status_t status = new_a_rbtree(&screen->windows);
    if (ST_ERROR(status)) {
        krnl_panic();
    }

    screen->Blt = blt;
    screen->SwapTwoBuffers = swap_framebuffer;
    screen->DrawChar = draw_ch;
    screen->DrawHollowRectangle = draw_hollow_rectangle;
    screen->DrawSecondOrderBezierCurve = draw_second_order_bezier_curve;
    screen->DrawBresenhamsLine = draw_bresenhams_line;
    screen->ClearFrameBuffers = clear_framebuffer;
    screen->clear_screen = clear_screen;
}
