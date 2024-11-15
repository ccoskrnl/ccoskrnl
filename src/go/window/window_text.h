#ifndef __OP_WINDOW_TEXT_H__
#define __OP_WINDOW_TEXT_H__

#include "../../include/types.h"
#include "../../include/libk/rbtree.h"
#include "../font/font_ttf.h"
#include "../graphics.h"
#include "./window.h"

#define OUTPUT_BUF_SIZE                                         2048
#define MAX_OUTPUT_BUFS                                         2

#define WINDOW_TEXT_CACHED_START_CODE                           0x21
#define WINDOW_TEXT_CACHED_END_CODE                             128

typedef status_t (*_window_text_register_t)(
    _in_ void                   *_this,
    _in_ font_ttf_t             *font_family,
    _in_ double                 point_size,
    _in_ int                    fix_line_gap
);

typedef status_t (*_window_text_putc_t)(
    _in_ void                                   *_this,
    _in_ wch_t                                  wch,
    _in_ go_blt_pixel_t                         color,
    _in_ boolean                                update
);

typedef status_t (*_window_text_puts_t)(
    _in_ void                                   *_this,
    _in_ const char                             *string,
    _in_ go_blt_pixel_t                         color
);

typedef status_t (*_window_text_putws_t)(
    _in_ void                                   *_this,
    _in_ const wch_t                            *wstring,
    _in_ go_blt_pixel_t                         color
);

typedef void (*_window_text_clear_window_t)(
    _in_ void                   *_this
);

typedef struct _window_text
{

    /**
     * Window
     **/
    window_t                    window;

    _window_text_register_t     Register;
    _window_show_window_t       ShowWindow;
    _window_text_clear_window_t ClearWindow;
    _window_text_putc_t         PutChar;
    _window_text_puts_t         PutString;
    _window_text_putws_t        PutWString;

    /**
     * Window use this member to tell screen which Font family
     * to display.
     */
    font_ttf_t                  *font_family;

    /**
     * @brief The font size which be displayed on this window.
     */
    double                      point_size;

    /**
     * @brief The number of characters per line. 
     */
    int                         num_of_ch_per_line;

    /**
     * (point_size * dpi) / (72 * unitsPerEm);
     */
    double                      scaling_factor;

    /**
     * @brief Size of font unitsPerEm be scaled.
     * descired_em = unitsPerEm * scaling_factor
     */
    int16_t                     desired_em;

    /**
     * @brief Space advance width.
     */
    int16_t                     advance_width;

    /**
     * @brief The member to adjust gap between rows.
     * TrueType font also has contained LineGap. But if you want to
     * use wider gap, you can set this member. In default case, the
     * value is zero.
     */
    int16_t                     fix_line_gap;

    /**
     * @brief The gap between rows.
     * line_height = (ascender - descender + lineGap * scaling_factor)
     *  + fix_gap.
     */
    int16_t                     line_height;

    /**
     * @brief The ascender of this font family.
     */
    int16_t                     ascender;

    /**
     * @brief The descender of this font family.
     */
    int16_t                     descender;

    /**
     * @brief Left side bearing.
     **/
    uint16_t                    lsb;

    /**
     * @brief Cached glyphs.
     **/
    font_ttf_glyph_t            *cached_glyphs[WINDOW_TEXT_CACHED_END_CODE - WINDOW_TEXT_CACHED_START_CODE];

    /**
     * @brief Record the position where next character
     * should be put in. 
     */
    struct _coordinates_2d_i    cursor;

    /**
     * @brief Indicate which buffer as primary output. 
     */
    uint64_t                    which_output_buf : 1;

    /**
     * @brief Index for output_buf
     */
    uint64_t                    output_buf_index : 63;

    /**
     * @brief all output buffers in current window.
     */
    uint64_t                    output_bufs[MAX_OUTPUT_BUFS][OUTPUT_BUF_SIZE];

} window_text_t;

extern window_text_t *_go_cpu_output_window[32];
extern wch_t *_go_weclome_texts[32];

#endif
