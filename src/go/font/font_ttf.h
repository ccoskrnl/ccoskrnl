#ifndef __FONT_TTF_H__
#define __FONT_TTF_H__

#include "../../include/types.h"
#include "../graphics.h"
#include "font_ttf_tables_def.h"

#define MAX_INSTALLED_FONTS									0x10

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

    _font_ttf_init init;
    _font_ttf_get_glyph_id get_glyph_id;
    _font_ttf_destroy destroy;

} font_ttf_t;



typedef status_t (*_font_ttf_glyph_destroy)(void *_this);
typedef status_t (*_font_ttf_glyph_init)(void* _this, wch_t wch_code, font_ttf_t* family);
typedef status_t (*_font_ttf_glyph_rasterize)(void *_this, go_buf_t* buffer, point_i_t origin, double point_size, go_blt_pixel_t color);

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

status_t _op_text_out(
    _in_ _out_ go_buf_t                         *buf,
    _in_ wch_t                                  *string,
    _in_ point_i_t                              origin,
    _in_ font_ttf_t                             *font_family,
    _in_ double                                 point_size,
    _in_ go_blt_pixel_t                         color
);

struct _installed_font_ttfs{
    int64_t num;
    struct _font_ttf* fonts[MAX_INSTALLED_FONTS];
};


#define TAB_SIZE                                        4



extern struct _installed_font_ttfs _go_font_ttfs;
#endif
