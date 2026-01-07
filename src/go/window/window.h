#ifndef __OP_WINDOW_H__
#define __OP_WINDOW_H__

#include "../../include/types.h"
#include "../../include/libk/rbtree.h"
#include "../font/font_ttf.h"
#include "../graphics.h"

typedef enum
{
    WindowCommon = 0,
    WindowText = 1

} WindowType;

/**
 * @brief Window Style Flags
 */

//  This flag indicates the window not has background. Whether the bg member
//  or color member has been set. We don't use them. In earily designing, this
//  flag is used to create no background window. We discard now.
#define WINDOW_STYLE_NONE                                       0


//  This flag indicates the window bg is transparent. 
#define WINDOW_STYLE_BG_NONE                                      0x00


//  This flag indicates the window background is prue RGB color.
//  When this flag is set. color member will be used. But bg member
//  will be ignored.
#define WINDOW_STYLE_BG_COLOR                                      0x01


//  This flag indicates the window background is determined by bg member.
//  In this case, color member will be ignored.
#define WINDOW_STYLE_BG_IMAGE                                         0x02


//  This flag indicates the window not have window border. And we don't paint the window title bar.
#define WINDOW_STYLE_NO_BORDER                                  0x100

// // This flag indicates that the window will not have a title.
// #define WINDOW_STYLE_NO_TITLE                                   0x1000
//

#define WINDOW_STYLE_BG(flag)                                   (flag & 0x3)


typedef struct _window_style
{
    uint64_t flags;
    go_blt_pixel_t color;
    struct _go_buffer bg;

} window_style_t;

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



/**
 * @brief Create a new window.
 * 
 * @param tag 
 * @param type 
 * @param parent_window 
 * @param screen 
 * @param window_title 
 * @param style 
 * @param x_of_upper_left_hand 
 * @param y_of_upper_left_hand 
 * @param width 
 * @param height 
 * @param window 
 * @return status_t 
 */
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
