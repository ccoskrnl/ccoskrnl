#include "../../include/types.h"
#include "../graphics.h"
#include "../screen.h"
#include "../font/font_ttf.h"
#include "./window.h"
#include "./window_text.h"
#include "../../include/libk/math.h"
#include "../../include/libk/stdlib.h"


static void clear_window(
    _in_ void                   *_this
)
{
    window_t *this = _this;

    // background origin in window with decoration
    point_i_t win_bg_origin = { 0 };
    uint32_t window_bg_flag = this->style.flags & 0xFFFFFFFF;

    // if this window have window decoration, we need to adjust bg's origin point and size.
    if (!(this->style.flags & WINDOW_NO_WINDOW_BORDER)) 
    {
        win_bg_origin.x = WINDOW_BORDER_STROKE_SIZE;
        win_bg_origin.y = WINDOW_BORDER_STROKE_SIZE + WINDOW_TITLE_DECORATION_SIZE;
    }

    if (window_bg_flag == WINDOW_STYLE_BG) 
    {
        assert((this->style.bg.height == this->height) && (this->style.bg.width == this->width));
        for (int i = 0; i < this->height; i++) 
            memcpy(
                this->framebuffer.buf + (win_bg_origin.y + i ) * this->framebuffer.width + win_bg_origin.x, 
                this->style.bg.buf, 
                this->style.bg.width * sizeof(go_blt_pixel_t)
            );

    }
    else if (window_bg_flag == WINDOW_STYLE_COLOR)
    {
        for (int i = 0; i < this->height; i++) 
            memsetd(
                (uint32_t*)this->framebuffer.buf + (win_bg_origin.y + i ) * this->framebuffer.width + win_bg_origin.x, 
                *(uint32_t*)&this->style.color, 
               this->width 
            );
    }

}

static status_t init_window_framebuffer(
    _in_ void                                   *_this
)
{
    status_t status = ST_SUCCESS;
    window_t *this = _this;
    point_i_t win_bg_origin = { 0 };

    uint16_t win_width = this->framebuffer.width;
    uint16_t win_height = this->framebuffer.height;



    if (!(this->style.flags & WINDOW_NO_WINDOW_BORDER))
    {
        // Set window decoration
        win_width += (WINDOW_BORDER_STROKE_SIZE << 1);
        win_height += ((WINDOW_BORDER_STROKE_SIZE << 1) + WINDOW_TITLE_DECORATION_SIZE);

        this->framebuffer.buf = (go_blt_pixel_t*)malloc(sizeof(go_blt_pixel_t) * (win_height * win_width));

        assert(this->framebuffer.buf != NULL);

        // Actually, we can directly draw window decoration without the aid of any other GDI functions.
        win_bg_origin.x = WINDOW_BORDER_STROKE_SIZE;
        win_bg_origin.y = WINDOW_BORDER_STROKE_SIZE + WINDOW_TITLE_DECORATION_SIZE;

        go_blt_pixel_t border_color = WINDOW_BORDER_COLOR;
        int start_y, start_x;

        // draw top side
        start_y = 0;
        start_x = 0;
        for (int i = 0; i < (WINDOW_TITLE_DECORATION_SIZE + WINDOW_BORDER_STROKE_SIZE); i++) 
            memsetd(
        (uint32_t*)this->framebuffer.buf + (i + start_y) * win_width + start_x, 
        *(uint32_t*)&border_color, 
        win_width);

        // draw buttom side
        start_y = (WINDOW_TITLE_DECORATION_SIZE + WINDOW_BORDER_STROKE_SIZE) + this->framebuffer.height;
        for (int i = 0; i < (WINDOW_BORDER_STROKE_SIZE); i++) 
            memsetd(
        (uint32_t*)this->framebuffer.buf + (i + start_y) * win_width + start_x, 
        *(uint32_t*)&border_color, 
        win_width);

        // draw left side
        start_y = (WINDOW_TITLE_DECORATION_SIZE + WINDOW_BORDER_STROKE_SIZE);
        for (int i = 0; i < this->framebuffer.height; i++) 
            memsetd(
        (uint32_t*)this->framebuffer.buf + (i + start_y) * win_width + start_x, 
        *(uint32_t*)&border_color, 
        WINDOW_BORDER_STROKE_SIZE);

        // draw right side 
        start_x = WINDOW_BORDER_STROKE_SIZE + this->framebuffer.width;
        for (int i = 0; i < this->framebuffer.height; i++) 
            memsetd(
        (uint32_t*)this->framebuffer.buf + (i + start_y) * win_width + start_x, 
        *(uint32_t*)&border_color, 
        WINDOW_BORDER_STROKE_SIZE);

        this->framebuffer.width = win_width;
        this->framebuffer.height = win_height;

        this->origin.x = WINDOW_BORDER_STROKE_SIZE;
        this->origin.y = WINDOW_BORDER_STROKE_SIZE + WINDOW_TITLE_DECORATION_SIZE;

        // Draw window title
        point_i_t origin = { WINDOW_TITLE_LSB, 0 };
        go_blt_pixel_t title_color = WINDOW_TITLE_COLOR;
        if (this->window_title != NULL)
            _op_text_out(
        &this->framebuffer, 
        this->window_title, 
        origin, 
        _go_font_ttfs.fonts[0], 
        WINDOW_TITLE_POINT_SIZE, 
        title_color
        );
        
    }
    else 
    {
        this->origin.x = 0;
        this->origin.y = 0;
        this->framebuffer.buf = (go_blt_pixel_t*)malloc(sizeof(go_blt_pixel_t) * (win_height * win_width));
        assert(this->framebuffer.buf != NULL);
    }

    clear_window(this);



    return status;
}

static status_t show_window(
    _in_ void                                   *_this
)
{
    status_t status = ST_SUCCESS;
    go_screen_desc* screen;
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

    uint16_t win_width = this->framebuffer.width;
    uint16_t win_height = this->framebuffer.height;

    // TODO: acquire screen lock

    // backup framebuffer into backbuffer
    screen->SwapTwoBuffers(screen, BACKBUFFER_INDEX, FRAMEBUFFER_INDEX);

    // draw window
    if ((this->style.flags & 0xFFFFFFFF) != WINDOW_STYLE_NONE) 
    {
        if (this->upper_left_hand.x + win_width <= 0
                || this->upper_left_hand.y + win_height <= 0
                || this->upper_left_hand.x >= screen->horizontal
                || this->upper_left_hand.y >= screen->vertical
            ) 
        {
            return status;
        }

        point_i_t src_point = { 0, 0 };
        point_i_t dest_point = { 0, 0 };
        int drawing_width, drawing_height;

        if (this->upper_left_hand.x < 0) 
        {
            src_point.x = abs(this->upper_left_hand.x);
            drawing_width = win_width + this->upper_left_hand.x;
            dest_point.x = 0;
        }
        else 
        {
            dest_point.x = this->upper_left_hand.x;
            drawing_width = win_width;
        }
        drawing_width = (dest_point.x + drawing_width < screen->horizontal ? 
                drawing_width : screen->horizontal - dest_point.x);

        if (this->upper_left_hand.y < 0)
        {
            src_point.y = abs(this->upper_left_hand.y);
            drawing_height = win_height + this->upper_left_hand.y;
            dest_point.y = 0;
        }
        else
        {
            dest_point.y = this->upper_left_hand.y;
            drawing_height = win_height;
        }

        drawing_height = (dest_point.y + drawing_height < screen->vertical ?
            drawing_height : screen->vertical - dest_point.y);

        status = screen->Blt(
                screen,
                this->framebuffer.buf,
                GoBltBufferToVideo,
                src_point.x,
                src_point.y,
                win_width,
                win_height,
                dest_point.x,
                dest_point.y,
                drawing_width,
                drawing_height,
                BACKBUFFER_INDEX
        );
        if (ST_ERROR(status))
        {
            krnl_panic();
        }

    } 


    screen->SwapTwoBuffers(screen, FRAMEBUFFER_INDEX, BACKBUFFER_INDEX);
    // TODO: release screen lock

    return status;;
}

static status_t scroll_screen(
    _in_ void                                   *_this
)
{
    status_t status = ST_SUCCESS;
    window_text_t *this = _this;
    uint64_t *lf = this->output_bufs[this->which_output_buf];
    for(; *(wch_t*)lf != '\n'; ++lf);
    lf++;

    this->which_output_buf ^= 1;
    this->cursor.x = this->lsb;
    this->cursor.y = 0;
    this->output_buf_index = 0;

    memzero(this->output_bufs[this->which_output_buf], OUTPUT_BUF_SIZE * sizeof(wch_t));
    clear_window(this);

    while (*lf != 0)
    {
        go_blt_pixel_t *color = (go_blt_pixel_t*)((uint64_t)lf + sizeof(wch_t));
        wch_t wc = *lf & 0xffffffff;
        this->PutChar(this, wc, *color, false);
        if (ST_ERROR(status))
        {
            krnl_panic();
        }
        lf++;
    }

    this->ShowWindow(this);
    return status;
}

static status_t text_window_putc( 
    _in_ void                                   *_this,
    _in_ wch_t                                  wch,
    _in_ go_blt_pixel_t                         color,
    _in_ boolean                                update
)
{
    status_t status = ST_SUCCESS;
    int TAB = 0;
    uint64_t cwch;
    window_text_t *this = _this;
    font_ttf_glyph_t *glyph;
    point_i_t draw_at_point;


    if (this == NULL)
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }



__inspect_begin:

    // Situation 0:
    // Current row can't put ant characters, so we need to wrap.
    if (this->cursor.x + this->desired_em >= this->window.width && wch != '\n')
    {
        this->cursor.y += this->line_height;
        this->cursor.x = LSB_SIZE;
        this->output_bufs[this->which_output_buf][this->output_buf_index - 1] = '\n';
    }

    // Situation 1:
    // We need to scroll screen.
    if ((this->cursor.y + (this->ascender - this->descender)) >= this->window.height)
        scroll_screen(this);
    
    switch(wch)
    {
        case ' ':
            this->cursor.x += this->advance_width;
            break;
        case '\t':
            this->cursor.x += this->advance_width;
            while (TAB < TAB_SIZE)
            {
                TAB++;
                goto __inspect_begin;
            }
            goto __draw_ch_exit;
            break;
        case '\n':

            this->cursor.y += this->line_height;
            this->cursor.x = this->lsb;

            // Situation 1:
            // We need to scroll screen.
            // if ((this->cursor.y + (this->ascender - this->descender)) >= this->window.height)
            //     scroll_screen(this);

            goto __draw_ch_exit;
            break;

        default:
            status = new_a_glyph(&glyph);
            if (ST_ERROR(status)) 
                krnl_panic();

            glyph->init(glyph, wch,this->font_family);
            draw_at_point.x = this->cursor.x + this->window.origin.x;
            draw_at_point.y = this->cursor.y + this->window.origin.y;
            status = glyph->rasterize(glyph, &this->window.framebuffer, draw_at_point, this->point_size, color);
            if (ST_ERROR(status)) 
                krnl_panic();

            this->cursor.x += ceil(glyph->advance_width * this->scaling_factor);
            del_a_glyph(glyph);
            break;
            
    }

__draw_ch_exit:

    cwch = *(uint32_t*)&color;
    cwch <<= 32;
    cwch |= wch;

    this->output_bufs[this->which_output_buf][this->output_buf_index++] = cwch;


    if (update) 
        this->ShowWindow(this);

    return status;

}

static status_t text_window_puts(
    _in_ void                                   *_this,
    _in_ const wch_t                            *wstring,
    _in_ go_blt_pixel_t                         color
)
{
    status_t status = ST_SUCCESS;
    window_text_t *this = _this;


    if (this == NULL)
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    for (size_t i = 0; wstring[i] != '\0'; i++) 
        text_window_putc(this, wstring[i], color, false);

    this->ShowWindow(this);
    return status;
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


    this->lsb = LSB_SIZE;
    this->scaling_factor = (point_size * DPI) / (72 * font_family->head.table.unitsPerEm);
    this->desired_em = ceil(this->scaling_factor * font_family->head.table.unitsPerEm);
    this->advance_width = this->desired_em >> 1;
    this->num_of_ch_per_line = floor((double)(this->window.framebuffer.width) / this->desired_em);
    this->fix_line_gap = fix_line_gap;
    this->ascender = ceil(this->scaling_factor * font_family->hhea.table.ascender);
    this->descender = ceil(this->scaling_factor * font_family->hhea.table.descender);
    this->line_height = ceil(
            (font_family->hhea.table.ascender
            - font_family->hhea.table.descender
            + font_family->hhea.table.lineGap) * this->scaling_factor) + this->fix_line_gap;

    this->cursor.x = this->lsb;
    this->cursor.y = 0;

    status = init_window_framebuffer(this);
    if (ST_ERROR(status)) {
        krnl_panic();
    }
    clear_window(this);

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
    go_screen_desc *screen_desc = screen;
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
            *(_window_text_clear_window_t*)((uint64_t)win + element_offset(window_text_t, ClearWindow)) = clear_window;
            *(_window_text_putc_t*)((uint64_t)win + element_offset(window_text_t, PutChar)) = text_window_putc;
            *(_window_text_puts_t*)((uint64_t)win + element_offset(window_text_t, PutString)) = text_window_puts;
            break;
        default:
            break;
    }


    win->node.key = tag;
    win->screen = screen;
    win->parent_window = parent_window;
    win->window_title = window_title;
    win->width = width;
    win->height = height;
    win->framebuffer.height = height;
    win->framebuffer.width = width;
    win->style = style;
    win->upper_left_hand.x = x_of_upper_left_hand;
    win->upper_left_hand.y = y_of_upper_left_hand;

    *window = win;
    screen_desc->windows->Insert(screen_desc->windows, (rbtree_node_t*)((uint64_t)win + element_offset(window_t, node)));

    return status;
}
