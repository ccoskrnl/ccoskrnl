#include "../../include/types.h"
#include "../graphics.h"
#include "../screen.h"
#include "../font/font_ttf.h"
#include "./window.h"
#include "./window_text.h"
#include "./window_common.h"

#include "../../include/libk/math.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/list.h"
#include "../../include/arch/lib.h"


static void clear_window(
        _in_ void                   *_this
        )
{
    window_t *this = _this;

    // background origin in window with decoration
    point_i_t win_bg_origin = { 0 };
    uint32_t window_bg_flag = WINDOW_STYLE_BG(this->style.flags);

    if (window_bg_flag == WINDOW_STYLE_BG_IMAGE) 
    {
        assert((this->style.bg.height == this->height) && (this->style.bg.width == this->width));
        for (int i = 0; i < this->height; i++) 
            memcpy(
                    this->framebuffer.buf + (win_bg_origin.y + i ) * this->framebuffer.width + win_bg_origin.x, 
                    this->style.bg.buf, 
                    this->style.bg.width * sizeof(go_blt_pixel_t)
                  );

    }
    else if (window_bg_flag == WINDOW_STYLE_BG_COLOR)
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

    this->origin.x = 0;
    this->origin.y = 0;
    this->framebuffer.buf = (go_blt_pixel_t*)malloc(sizeof(go_blt_pixel_t) * (win_height * win_width));
    assert(this->framebuffer.buf != NULL);

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

    int max_width = screen->horizontal - (WINDOW_BORDER_STROKE_SIZE << 1);
    int max_height = screen->vertical - (WINDOW_BORDER_STROKE_SIZE << 1) - WINDOW_TITLE_DECORATION_SIZE;

    uint16_t win_width = this->framebuffer.width;
    uint16_t win_height = this->framebuffer.height;
    int start_x = this->upper_left_hand.x;
    int start_y = this->upper_left_hand.y;

    boolean saved_if = intr_disable();
    spinlock_acquire(&screen->spinlock);

    // backup framebuffer into backbuffer
    screen->SwapTwoBuffers(screen, BACKBUFFER_INDEX, FRAMEBUFFER_INDEX);


    // if it has window decoration
    if (!(this->style.flags & WINDOW_STYLE_NO_BORDER)) 
    {
        // add border size and title decoration size to window size
        win_width += (WINDOW_BORDER_STROKE_SIZE << 1);
        win_height += ((WINDOW_BORDER_STROKE_SIZE << 1) + WINDOW_TITLE_DECORATION_SIZE);

        go_blt_pixel_t border_color = WINDOW_BORDER_COLOR;
        start_x = this->upper_left_hand.x - WINDOW_BORDER_STROKE_SIZE;
        start_y = this->upper_left_hand.y - WINDOW_BORDER_STROKE_SIZE - WINDOW_TITLE_DECORATION_SIZE;

        start_x = start_x < 0 ? 0 : start_x;
        start_y = start_y < 0 ? 0 : start_y;

        if (start_x + win_width > screen->horizontal)
            win_width = max_width - start_x;
        if (start_y + win_height > screen->vertical)
            win_height = max_height - start_y;

        status = screen->DrawRectangle(
                screen,
                start_x,
                start_y,
                win_width,
                win_height,
                border_color,
                BACKBUFFER_INDEX
        );

        if (ST_ERROR(status))
        {
            krnl_panic(NULL);
        }

        point_i_t origin = { WINDOW_TITLE_LSB + start_x, start_y};
        go_blt_pixel_t title_color = WINDOW_TITLE_COLOR;

        _op_text_out(
            &screen->frame_bufs[BACKBUFFER_INDEX],
            this->window_title,
            origin, 
            _go_font_ttfs.fonts[0], 
            WINDOW_TITLE_POINT_SIZE, 
            title_color
        );

        start_x += WINDOW_BORDER_STROKE_SIZE;
        start_y += WINDOW_BORDER_STROKE_SIZE + WINDOW_TITLE_DECORATION_SIZE;


    }

    status = screen->Blt(
            screen,
            this->framebuffer.buf,
            GoBltBufferToVideo,
            0,
            0,
            this->framebuffer.width,
            this->framebuffer.height,
            start_x,
            start_y,
            this->framebuffer.width,
            this->framebuffer.height,
            BACKBUFFER_INDEX
            );

    if (ST_ERROR(status)) 
    {
        krnl_panic(NULL);
    }

    screen->SwapTwoBuffers(screen, FRAMEBUFFER_INDEX, BACKBUFFER_INDEX);

    spinlock_release(&screen->spinlock);
    set_intr_state(saved_if);

    return status;;
}

status_t common_window_draw_rectangle(
        _in_ void                                   *_this,
        _in_ int                                    x,
        _in_ int                                    y,
        _in_ int                                    width,
        _in_ int                                    height,
        _in_ go_blt_pixel_t                         color
)
{

    window_t *this = _this;

    status_t status;
    go_blt_pixel_t col = color;
    uint32_t c = *(uint32_t*)&col;

    go_blt_pixel_t* buf = this->framebuffer.buf + y * this->width + x;

    if (   x >= this->width 
            || y >= this->height
            || width > this->width
            || height > this->height 
       ) 
    {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    int drawing_width = x + width <= this->width ? width : this->width - x;
    int drawing_height = y + height <= this->height ? height : this->height - y;

    for (int i = 0; i < drawing_height; i++) 
        memsetd((uint32_t*)(buf + i * this->width), c, drawing_width);

    show_window(this);

    return status;

}


static status_t register_common_window(
        _in_ void                   *_this
        )
{
    status_t status = ST_SUCCESS;

    if (_this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    window_t *this = _this;

    status = init_window_framebuffer(this);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
    }

    return status;
}











static void text_window_clear_window(
        _in_ void                   *_this
        )
{
    clear_window(_this);
    window_text_t *this = _this;
    this->cursor.x = this->lsb;
    this->cursor.y = 0;
}


static status_t text_window_scroll_screen(
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

    memzero(this->output_bufs[this->which_output_buf], OUTPUT_BUF_SIZE * sizeof(uint64_t));
    clear_window(this);

    while (*lf != 0)
    {
        go_blt_pixel_t *color = (go_blt_pixel_t*)((uint64_t)lf + sizeof(wch_t));
        wch_t wc = *lf & 0xffffffff;
        this->PutChar(this, wc, *color, false);
        if (ST_ERROR(status))
        {
            krnl_panic(NULL);
        }
        lf++;
    }

    if (this->output_bufs[0][101] == 0 || this->output_bufs[1][101] == 0)
    {
        uint32_t a = 0xffff;
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
        // this->output_bufs[this->which_output_buf][this->output_buf_index - 1] = '\n';
        this->output_bufs[this->which_output_buf][this->output_buf_index++] = '\n';
        this->ShowWindow(this);
    }

    // Situation 1:
    // We need to scroll screen.
    while ((this->cursor.y + (this->ascender - this->descender)) >= this->window.height)
        text_window_scroll_screen(this);

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
            this->ShowWindow(this);

            goto __draw_ch_exit;
            break;

        case '\r':

            this->cursor.x = this->lsb;

            goto __draw_ch_exit;
            break;

        default:
            {
                draw_at_point.x = this->cursor.x + this->window.origin.x;
                draw_at_point.y = this->cursor.y + this->window.origin.y;

                if (wch >= WINDOW_TEXT_CACHED_START_CODE
                        && wch < WINDOW_TEXT_CACHED_END_CODE) 
                {
                    glyph = this->cached_glyphs[wch - WINDOW_TEXT_CACHED_START_CODE];                
                    status = glyph->rasterize(glyph, &this->window.framebuffer, draw_at_point, this->point_size, color);
                    if (ST_ERROR(status)) 
                        krnl_panic(NULL);
                    this->cursor.x += ceil(glyph->advance_width * this->scaling_factor);
                }
                else 
                {
                    status = new_a_glyph(&glyph);
                    if (ST_ERROR(status)) 
                        krnl_panic(NULL);

                    glyph->init(glyph, wch,this->font_family);
                    status = glyph->rasterize(glyph, &this->window.framebuffer, draw_at_point, this->point_size, color);
                    if (ST_ERROR(status)) 
                        krnl_panic(NULL);

                    this->cursor.x += ceil(glyph->advance_width * this->scaling_factor);
                    del_a_glyph(glyph);
                }
            }
            break;

    }

__draw_ch_exit:

    cwch = *(uint32_t*)&color;
    cwch <<= 32;
    cwch |= wch;

    this->output_bufs[this->which_output_buf][this->output_buf_index++] = cwch;

    if (this->output_bufs[0][101] == 0 || this->output_bufs[1][101] == 0)
    {
        uint32_t a = 0xffff;
    }


    if (update) 
        this->ShowWindow(this);

    return status;

}

static status_t text_window_puts(
        _in_ void                                   *_this,
        _in_ const char                             *string,
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

    for (size_t i = 0; string[i] != '\0'; i++) 
        text_window_putc(this, string[i], color, false);

    this->ShowWindow(this);
    return status;
}

static status_t text_window_putws(
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

    int number_of_cached_glyphs = (WINDOW_TEXT_CACHED_END_CODE - WINDOW_TEXT_CACHED_START_CODE);
    wch_t start_code = WINDOW_TEXT_CACHED_START_CODE;
    font_ttf_glyph_t *glyph = NULL;

    if (this->has_been_register) {

        for (int i = 0; i < number_of_cached_glyphs; i++) 
        {
            if (this->cached_glyphs[i] == NULL)
                continue;
            glyph = this->cached_glyphs[i];
            del_a_glyph(glyph);
            this->cached_glyphs[i] = NULL;
        }
    }

    for (int i = 0; i < number_of_cached_glyphs; i++) 
    {
        status = new_a_glyph(&glyph);
        if (ST_ERROR(status)) 
            krnl_panic(NULL);

        this->cached_glyphs[i] = glyph;
        status = glyph->init(glyph, start_code, this->font_family);
        if (ST_ERROR(status)) 
            krnl_panic(NULL);

        start_code++;
    }

    status = init_window_framebuffer(this);
    if (ST_ERROR(status)) {
        krnl_panic(NULL);
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
        case WindowCommon:
            win = malloc(sizeof(window_common_t));
            assert(win != NULL);
            memzero(win, sizeof(*win));
            *(_window_register_t*)((uint64_t)win + element_offset(window_common_t, Register)) = register_common_window;
            *(_window_show_t*)((uint64_t)win + element_offset(window_common_t, ShowWindow)) = show_window;
            *(_window_clear_t*)((uint64_t)win + element_offset(window_common_t, ClearWindow)) = clear_window;
            *(_window_draw_rectangle_t*)((uint64_t)win + element_offset(window_common_t, DrawRectangle)) = common_window_draw_rectangle;
            break;
        case WindowText:
            win = malloc(sizeof(window_text_t));
            assert(win != NULL);
            memzero(win, sizeof(*win));
            *(_window_text_register_t*)((uint64_t)win + element_offset(window_text_t, Register)) = register_text_window;
            *(_window_text_show_t*)((uint64_t)win + element_offset(window_text_t, ShowWindow)) = show_window;
            *(_window_text_clear_window_t*)((uint64_t)win + element_offset(window_text_t, ClearWindow)) = text_window_clear_window;
            *(_window_text_putc_t*)((uint64_t)win + element_offset(window_text_t, PutChar)) = text_window_putc;
            *(_window_text_puts_t*)((uint64_t)win + element_offset(window_text_t, PutString)) = text_window_puts;
            *(_window_text_putws_t*)((uint64_t)win + element_offset(window_text_t, PutWString)) = text_window_putws;
            break;
        default:
            break;
    }


    win->tag = tag;
    win->screen = screen;
    win->parent_window = parent_window;
    win->window_title = window_title;
    win->style = style;
    win->upper_left_hand.x = x_of_upper_left_hand;
    win->upper_left_hand.y = y_of_upper_left_hand;

    if (style.flags & WINDOW_STYLE_NO_BORDER)
    {
        win->width = width > screen_desc->horizontal ? screen_desc->horizontal : width;
        win->height = height > screen_desc->vertical ? screen_desc->vertical : height;
        win->framebuffer.height = win->height;
        win->framebuffer.width = win->width;
    }
    else 
    {
        int max_width = screen_desc->horizontal - (WINDOW_BORDER_STROKE_SIZE << 1);
        int max_height = screen_desc->vertical - (WINDOW_BORDER_STROKE_SIZE << 1) - WINDOW_TITLE_DECORATION_SIZE;

        win->width = width > max_width ? max_width : width;
        win->height = height > max_height ? max_height : height;
        win->framebuffer.height = height;
        win->framebuffer.width = width;
    }

    *window = win;

    // screen_desc->windows->Insert(screen_desc->windows, (rbtree_node_t*)((uint64_t)win + element_offset(window_t, node)));
    _list_push(&screen_desc->windows, (list_node_t*)((uint64_t)win + element_offset(window_t, node)));

    return status;
}
