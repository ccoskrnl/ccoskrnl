#ifndef __OP_WINDOW_H__
#define __OP_WINDOW_H__

#include "../../types.h"
#include "../../libk/rbtree.h"
#include "./font/font_ttf.h"
#include "./graphics.h"

#define OUTPUT_BUF_SIZE                                         2048
#define MAX_OUTPUT_BUFS                                         2


typedef enum
{
    WindowNone = 0,
    WindowText = 1

} WindowType;

/**
 * @brief Window Style Flags
 */

//  This flag indicate the window not has background. Whether the bg member
//  or color member has been set. We don't use them.
#define WINDOW_STYLE_NONE                                       0

//  This flag indicate the window background is prue RGB color.
//  When this flag is set. color member will be used. But bg member
//  will be ignored.
#define WINDOW_STYLE_COLOR                                      1

//  This flag indicate the window background is determined by bg member.
//  In this case, color member will be ignored.
#define WINDOW_STYLE_BG                                         2


typedef struct _window_style
{
    uint32_t flags;
    go_blt_pixel_t color;
    struct _go_image_output bg;

} window_style_t;

typedef status_t (*_window_show_window_t)(
        _in_ void                               *_this
);

typedef struct _window
{
    /**
     * @brief tag to mark this window instance.
     */
    rbtree_node_t               node;

    /**
     * @brief Mark the window type.
     */
    WindowType                  type;

    /**
     * @brief Pointer that points default screen.
     */
    void			            *screen;

    /**
     * @brief Parent Window, can be NULL.
     */
    struct _window 		        *parent_window;

    /**
     * @brief Window Title 
     */
    wch_t*                      window_title;

    /**
     * @brief To describe the window style.
     */
    window_style_t              style;

    /**
     * The upper left-hand corner in screen it belonged.
     */
    point_i_t                   upper_left_hand;

    /**
     * The window's widht and height.
     */
    int                         width;
    int                         height;

    /**
     * The window's framebuffer. 
     * */
    go_blt_pixel_t              *framebuffer;

} __attribute__((packed)) window_t;



typedef status_t (*_window_text_register_t)(
    _in_ void                   *_this,
    _in_ font_ttf_t             *font_family,
    _in_ double                 point_size,
    _in_ int                    fix_line_gap
);

typedef struct _window_text
{

    /**
     * Window
     **/
    window_t                    window;

    _window_text_register_t     Register;
    _window_show_window_t       ShowWindow;

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
     * @brief Record the position where next character
     * should be put in. 
     */
    struct _coordinates_2d_i    cursor;

    /**
     * @brief Indicate which buffer as primary output. 
     */
    int                         which_output_buf;

    /**
     * @brief Index for output_buf
     */
    size_t                      output_buf_index;

    /**
     * @brief all output buffers in current window.
     */
    uint64_t                    output_bufs[MAX_OUTPUT_BUFS][OUTPUT_BUF_SIZE];

} window_text_t;

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
);


#define WINDOW_TITLE_DECORATION_SIZE                25
#define WINDOW_BORDER_STROKE_SIZE                   5
#define WINDOW_BORDER_COLOR                         { 0x20, 0x20, 0x20, 0 }
#define WINDOW_TITLE_COLOR                          { 0xff, 0xff, 0xff, 0 }
#define WINDOW_BG_COLOR                             { 0xc0, 0xc0, 0xc0, 0 }


#endif
