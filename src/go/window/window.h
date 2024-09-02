#ifndef __OP_WINDOW_H__
#define __OP_WINDOW_H__

#include "../../include/types.h"
#include "../../include/libk/rbtree.h"
#include "../font/font_ttf.h"
#include "../graphics.h"

typedef enum
{
    WindowNone = 0,
    WindowText = 1

} WindowType;

/**
 * @brief Window Style Flags
 */

//  This flag indicate the window not has background. Whether the bg member
//  or color member has been set. We don't use them. In earily designing, this
//  flag is used to create no background window. We discard now.
#define WINDOW_STYLE_NONE                                       0

//  This flag indicate the window background is prue RGB color.
//  When this flag is set. color member will be used. But bg member
//  will be ignored.
#define WINDOW_STYLE_COLOR                                      1

//  This flag indicate the window background is determined by bg member.
//  In this case, color member will be ignored.
#define WINDOW_STYLE_BG                                         2

//  This flag indicate the window not have window border.
#define WINDOW_NO_WINDOW_BORDER                                 0x100000000

typedef struct _window_style
{
    uint64_t flags;
    go_blt_pixel_t color;
    struct _go_buffer bg;

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
     * The member to indicate window origin point in framebuffer.
     * */
    point_i_t                   origin;

    /**
     * These members record original window size from creating window object.
     **/
    uint16_t                    width;
    uint16_t                    height;

    /**
     * The window's framebuffer. 
     * There are one point about window's framebuffer need to pay attention.
     * The framebuffer.width and height depending on whether 
     * WINDOW_NO_WINDOW_BORDER is marked or not. If WINDOW_NO_WINDOW_BORDER
     * is marked, width and height describe the framebuffer size of window
     * with window decoration. If it isn't, width and height record original
     * window size, which means their values are equal to original values from
     * creating window object.
     * */
    go_buf_t                    framebuffer;

} __attribute__((packed)) window_t;



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

#define WINDOW_TITLE_POINT_SIZE                     18
#define WINDOW_TITLE_LSB                            20
#define WINDOW_TITLE_DECORATION_SIZE                25
#define WINDOW_BORDER_STROKE_SIZE                   5
#define WINDOW_BORDER_COLOR                         { 0x20, 0x20, 0x20, 0 }
#define WINDOW_TITLE_COLOR                          { 0xff, 0xff, 0xff, 0 }
#define WINDOW_BG_COLOR                             { 0xc0, 0xc0, 0xc0, 0 }


#endif
