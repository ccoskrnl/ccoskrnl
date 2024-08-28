#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/hal/op/window.h"
#include "../../include/hal/op/window.h"
#include "../../include/libk/math.h"
#include "../../include/libk/stdlib.h"

static status_t show_window(
        _in_ void                               *_this
)
{
    status_t status = ST_SUCCESS;
    op_screen_desc* screen;
    window_t* this;

    if (_this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    this = _this;
    if (this->screen == NULL)
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    screen = this->screen;

    // TODO: acquire screen lock

    // backup framebuffer into backbuffer
    screen->SwapTwoBuffers(screen, 1, 0);

    // draw window
    if (this->style.flags == WINDOW_STYLE_COLOR) 
    {
        uint32_t color = this->style.color.Blue;
        color <<= 8;
        color |= this->style.color.Green;
        color <<= 8;
        color |= this->style.color.Red;
        color <<= 8;
        go_blt_pixel_t* win_start = (screen->frame_bufs[BACKBUFFER_INDEX] 
                + this->upper_left_hand.y * screen->horizontal 
                + this->upper_left_hand.x);

        // set background color
        for (int i = 0; i < (this->buttom_right_hand.y - this->buttom_right_hand.y); i++) 
            memsetd(
                (uint32_t*)(win_start + i * screen->horizontal), 
                color, 
                this->buttom_right_hand.x - this->upper_left_hand.x);
    } 
    else if (this->style.flags == WINDOW_STYLE_BG)
    {
        // TODO:        
    }

    // TODO: release screen lock

    return status;;
}


static status_t register_window(
    _in_ void                   *_this,
    _in_ wch_t                  *window_title,
    _in_ window_style_t         style,
    _in_ int                    x_of_upper_left_hand,
    _in_ int                    y_of_upper_left_hand,
    _in_ int                    x_of_buttom_right_hand,
    _in_ int                    y_of_buttom_right_hand,
    _in_ font_ttf_t             *font_family,
    _in_ double                 point_size,
    _in_ int                    fix_line_gap
)
{
    status_t status = ST_SUCCESS;

    if (_this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    window_t *window = _this;


    window->window_title = window_title;
    window->style = style;

    window->upper_left_hand.x = x_of_upper_left_hand;
    window->upper_left_hand.y = y_of_upper_left_hand;
    window->buttom_right_hand.x = x_of_buttom_right_hand;
    window->buttom_right_hand.y = y_of_buttom_right_hand;

    window->font_family = font_family;
    window->point_size = point_size;

    window->scaling_factor = (point_size * DPI) / (72 * font_family->head.table.unitsPerEm);
    window->desired_em = ceil(window->scaling_factor * font_family->head.table.unitsPerEm);
    window->num_of_ch_per_line = floor((double)(x_of_buttom_right_hand - x_of_upper_left_hand) / window->desired_em);
    window->fix_line_gap = fix_line_gap;
    window->ascender = ceil(window->scaling_factor * font_family->hhea.table.ascender);
    window->descender = ceil(window->scaling_factor * font_family->hhea.table.descender);
    window->line_height = ceil(
            (font_family->hhea.table.ascender
            - font_family->hhea.table.descender
            + font_family->hhea.table.lineGap) * window->scaling_factor) + window->fix_line_gap;


    return status;
}


status_t new_a_window(
    _in_ uint64_t                           tag,
    _in_ window_t                           *parent_window,
    _in_ void                               *screen,
    _in_ _out_ window_t                     **window
)
{
    status_t status = ST_SUCCESS;

    if (screen == NULL)
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    op_screen_desc *screen_desc = screen;

    *window = (window_t*)malloc(sizeof(window_t));
    if (*window == NULL) {
        status = ST_FAILED;
        return status;
    }
    memzero(*window, sizeof(window_t));

    (*window)->rbnode.key = tag;
    (*window)->screen = screen;
    (*window)->parent_window = parent_window;
    (*window)->Register = register_window;

    screen_desc->windows->Insert(screen_desc->windows, &(*window)->rbnode);

    return status;
}
