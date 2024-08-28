#ifndef __FONT_TTF_H__
#define __FONT_TTF_H__

#include "../../../types.h"
#include "../graphics.h"
#include "font_ttf_tables_def.h"

#define MAX_INSTALLED_FONTS									0xFF

// 0	If set, the point is on the curve; Otherwise, it is off the curve.
#define GLYPH_FLAG_On_Curve 0x1

// Vector	1	If set, the corresponding x-coordinate is 1 byte long;
// Otherwise, the corresponding x-coordinate is 2 bytes long
#define GLYPH_FLAG_x_Short 0x2

// Vector	2	If set, the corresponding y-coordinate is 1 byte long;
// Otherwise, the corresponding y-coordinate is 2 bytes long
#define GLYPH_FLAG_y_Short 0x4

// 3	If set, the next byte specifies the number of additional times this set
// of flags is to be repeated. In this way, the number of flags listed can
// be smaller than the number of points in a character.
#define GLYPH_FLAG_Repeat 0x8

// This x is same (Positive x-Short vector)
// 4	This flag has one of two meanings,
// depending on how the x-Short Vector flag is set. If the x-Short Vector bit is
// set, this bit describes the sign of the value, with a value of 1 equalling
// positive and a zero value negative. If the x-short Vector bit is not set, and
// this bit is set, then the current x-coordinate is the same as the previous
// x-coordinate. If the x-short Vector bit is not set, and this bit is not set,
// the current x-coordinate is a signed 16-bit delta vector. In this case, the
// delta vector is the change in x
#define GLYPH_FLAG_x_Dual 0x10

// This y is same (Positive y-Short vector)
// 5	This flag has one of two meanings,
// depending on how the y-Short Vector flag is set. If the y-Short Vector bit is
// set, this bit describes the sign of the value, with a value of 1 equalling
// positive and a zero value negative. If the y-short Vector bit is not set, and
// this bit is set, then the current y-coordinate is the same as the previous
// y-coordinate. If the y-short Vector bit is not set, and this bit is not set,
// the current y-coordinate is a signed 16-bit delta vector. In this case, the
// delta vector is the change in y
#define GLYPH_FLAG_y_Dual 0x20

#define __glyph_on_curve(flag) ((flag & GLYPH_FLAG_On_Curve) != 0)
#define __glyph_off_curve(flag) ((flag & GLYPH_FLAG_On_Curve) == 0)
#define __glyph_x_short(flag) ((flag & GLYPH_FLAG_x_Short) != 0)
#define __glyph_y_short(flag) ((flag & GLYPH_FLAG_y_Short) != 0)
#define __glyph_repeat(flag) ((flag & GLYPH_FLAG_Repeat) != 0)
#define __glyph_x_Dual(flag) ((flag & GLYPH_FLAG_x_Dual) != 0)
#define __glyph_y_Dual(flag) ((flag & GLYPH_FLAG_y_Dual) != 0)

#define POINT_STATE_NOT_PROCESS 0x0
#define POINT_STATE_IS_CONTOUR 0x2
#define POINT_STATE_ON_CONTOUR 0x4
#define POINT_STATE_OFF_CONTOUR 0x8

// typedef uint8_t point_state_t;

typedef struct _bezier_curve {

    point_f_t p0;
    point_f_t p1;
    point_f_t p2;

    int is_curve;
    int count;

    struct _bezier_curve *flink;
    struct _bezier_curve *blink;

} bezier_curve_t;

typedef bezier_curve_t *curves_t;

typedef uint8_t GlyphFlag;

typedef status_t (*_font_ttf_destroy)(void *_this);
typedef status_t (*_font_ttf_init)(void *_this, uint8_t *font_bin_data);
typedef uint16_t (*_font_ttf_get_glyph_id)(void *_this, wch_t ch_code);

typedef struct _font_ttf {
    font_ttf_offset_table_t offset_t;

    struct {
        font_ttf_table_directory_t dir;
        font_ttf_head_table_t table;
    } head;

    struct {
        font_ttf_table_directory_t dir;
        font_ttf_maxp_table_t table;
    } maxp;

    struct {
        font_ttf_table_directory_t dir;
        uint64_t start;
    } glyp;

    struct {
        font_ttf_table_directory_t dir;
        font_ttf_loca_table_t table;
    } loca;

    struct {
        font_ttf_table_directory_t dir;
        font_ttf_cmap_table_t table;
        font_ttf_cmap_subt_fmt_4_t fmt4;
        uint16_t fmt4_segcount;
    } cmap;

    struct {
        font_ttf_table_directory_t dir;
        font_ttf_hhea_table_t table;
    } hhea;

    struct {
        font_ttf_table_directory_t dir;
        font_ttf_hmtx_table_t table;
    } hmtx;

    double scaling_factor;
    double point_size;

    int16_t desired_em;

    int16_t line_space;
    int16_t line_height;
    int16_t ascender;
    int16_t descender;

    double dpi;
    
    int space_advance_width;

    _font_ttf_init init;
    _font_ttf_get_glyph_id get_glyph_id;
    _font_ttf_destroy destroy;

} font_ttf_t;



typedef status_t (*_font_ttf_glyph_destroy)(void *_this);
typedef status_t (*_font_ttf_glyph_init)(void* _this, wch_t wch_code, font_ttf_t* family);
typedef status_t (*_font_ttf_glyph_rasterize)(void *_this, void* screen, uint8_t buf_id, point_i_t origin, go_blt_pixel_t color);

typedef struct _font_ttf_glyph {

    // If the number of contours is greater than or
    // equal to zero, this is a simple glyph. If
    // negative, this is a composite glyph — the value
    // -1 should be used for composite glyphs.
    int16_t numberOfContours;

    int16_t xMin; // Minimum x for coordinate data.
    int16_t yMin; // Minimum y for coordinate data.
    int16_t xMax; // Maximum x for coordinate data.
    int16_t yMax; // Maximum y for coordinate data.

    int16_t advance_width;
    int16_t lsb;    // left side bearing.
    int16_t rsb;    // right side bearing.

    // Array of last points of each contour; n is the number
    // of contours; array entries are point indices
    uint16_t *endPtsOfContours;

    uint16_t **point_index_in_contour;
    uint16_t *contour_index_for_point;

    curves_t *contour_list;

    int32_t numberOfTriangles;
    int32_t numberOfPoints;

    GlyphFlag *flags;

    point_f_t *point_2d_coordinates;

    uint16_t glyph_index;

    font_ttf_t* font_family;

    _font_ttf_glyph_init init;
    _font_ttf_glyph_destroy destroy;
    _font_ttf_glyph_rasterize rasterize;

} font_ttf_glyph_t;

status_t new_a_font(font_ttf_t **font);
status_t del_a_font(font_ttf_t *font);

status_t new_a_glyph(font_ttf_glyph_t **glyph);
status_t del_a_glyph(font_ttf_glyph_t *glyph);

struct _installed_font_ttfs{
    int64_t num;
    struct _font_ttf* fonts[MAX_INSTALLED_FONTS];
};

extern struct _installed_font_ttfs _op_font_ttfs;
#endif
