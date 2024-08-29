#include "../../include/types.h"
#include "../../include/hal/op/graphics.h"
#include "../../include/hal/op/screen.h"
#include "../../include/hal/op/font/font_ttf.h"
#include "../../include/hal/op/window.h"
#include "../../include/hal/op/window.h"
#include "../../include/libk/math.h"
#include "../../include/libk/stdlib.h"

static status_t draw_window_border(
    _in_ void                                   *_this
)
{
    status_t status = ST_SUCCESS;
    op_screen_desc* screen;
    window_t* this;
    this = _this;
    if (this->screen == NULL)
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    screen = this->screen;
    point_i_t border_origin = 
        { this->upper_left_hand.x - WINDOW_BORDER_STROKE_SIZE, 
        this->upper_left_hand.y - WINDOW_BORDER_STROKE_SIZE };

    uint32_t width = this->width + (WINDOW_BORDER_STROKE_SIZE << 1);
    uint32_t height = this->height + (WINDOW_BORDER_STROKE_SIZE << 1);

    go_blt_pixel_t border_color = WINDOW_BORDER_COLOR;

    screen->DrawHollowRectangle(
        screen,
        border_origin.x,
        border_origin.y,
        width,
        height,
        WINDOW_BORDER_STROKE_SIZE,
        border_color,
        BACKBUFFER_INDEX
    );

    border_origin.y -= WINDOW_TITLE_DECORATION_SIZE;
    screen->DrawRectangle(
        screen,
        border_origin.x,
        border_origin.y,
        width,
        WINDOW_TITLE_DECORATION_SIZE,
        border_color,
        BACKBUFFER_INDEX
    );

    go_blt_pixel_t title_color = WINDOW_TITLE_COLOR;
    screen->DrawString(
        screen,
        this->window_title,
        border_origin,
        _op_font_ttfs.fonts[0],
        18,
        title_color,
        BACKBUFFER_INDEX
    );

    return status;
}

static status_t show_window(
    _in_ void                                   *_this
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
    if (this->style.flags != WINDOW_STYLE_NONE) 
    {

        go_blt_pixel_t* win_start = (screen->frame_bufs[BACKBUFFER_INDEX] 
                + this->upper_left_hand.y * screen->horizontal 
                + this->upper_left_hand.x);

        // set background color
        for (int i = 0; i < this->height ; i++) 
            memcpy(
            win_start + i * screen->horizontal, 
            this->framebuffer + i * this->width, 
            this->width * sizeof(go_blt_pixel_t));

    } 

    draw_window_border(this);

    screen->SwapTwoBuffers(screen, FRAMEBUFFER_INDEX, BACKBUFFER_INDEX);
    // TODO: release screen lock

    return status;;
}


static status_t register_text_window(
    _in_ void                   *_this,
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

    window_text_t *this = _this;



    this->font_family = font_family;
    this->point_size = point_size;

    this->scaling_factor = (point_size * DPI) / (72 * font_family->head.table.unitsPerEm);
    this->desired_em = ceil(this->scaling_factor * font_family->head.table.unitsPerEm);
    this->advance_width = this->desired_em >> 1;
    this->num_of_ch_per_line = floor((double)(this->window.width) / this->desired_em);
    this->fix_line_gap = fix_line_gap;
    this->ascender = ceil(this->scaling_factor * font_family->hhea.table.ascender);
    this->descender = ceil(this->scaling_factor * font_family->hhea.table.descender);
    this->line_height = ceil(
            (font_family->hhea.table.ascender
            - font_family->hhea.table.descender
            + font_family->hhea.table.lineGap) * this->scaling_factor) + this->fix_line_gap;

    return status;
}


status_t new_a_window(
    _in_ uint64_t                           tag,
    _in_ WindowType                         type,
    _in_ window_t                           *parent_window,
    _in_ void                               *screen,
    _in_ wch_t                              *window_title,
    _in_ window_style_t                     style,
    _in_ int                                x_of_upper_left_hand,
    _in_ int                                y_of_upper_left_hand,
    _in_ int                                width,
    _in_ int                                height,
    _in_ _out_ void                         **window
)
{
    status_t status = ST_SUCCESS;

    if (screen == NULL)
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }
    op_screen_desc *screen_desc = screen;
    window_t* win = NULL;

    switch (type) 
    {
        case WindowNone:
            win = malloc(sizeof(window_t));
            assert(win != NULL);
            memzero(win, sizeof(*win));
            break;
        case WindowText:
            win = malloc(sizeof(window_text_t));
            assert(win != NULL);
            memzero(win, sizeof(*win));
            *(_window_text_register_t*)((uint64_t)win + element_offset(window_text_t, Register)) = register_text_window;
            *(_window_show_window_t*)((uint64_t)win + element_offset(window_text_t, ShowWindow)) = show_window;
            break;
        default:
            break;
    }


    win->node.key = tag;
    win->screen = screen;
    win->parent_window = parent_window;
    win->window_title = window_title;
    win->height = height;
    win->width = width;
    win->style = style;
    win->upper_left_hand.x = x_of_upper_left_hand;
    win->upper_left_hand.y = y_of_upper_left_hand;

    if (style.flags == WINDOW_STYLE_BG) 
    {
        assert((style.bg.height == height) && (style.bg.width == width));
        win->framebuffer = style.bg.buf;
    }
    else if (style.flags == WINDOW_STYLE_COLOR)
    {
        win->framebuffer = (go_blt_pixel_t*)malloc(sizeof(go_blt_pixel_t) * (width * height));
        uint32_t color = *(uint32_t*)&win->style.color;
        assert(win->framebuffer != NULL);
        memsetd((uint32_t*)win->framebuffer, color, (width * height));
    }
    *window = win;
    screen_desc->windows->Insert(screen_desc->windows, (rbtree_node_t*)((uint64_t)win + element_offset(window_t, node)));

    return status;
}