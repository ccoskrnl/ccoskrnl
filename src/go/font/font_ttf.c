#include "./font_ttf.h"
#include "./font_ttf_tables_def.h"
#include "../graphics.h"
#include "../screen.h"
#include "../../include/libk/math.h"
#include "../../include/libk/stdlib.h"
#include "../../include/libk/list.h"
#include "../../include/libk/string.h"
#include "../../include/types.h"

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



static status_t font_tty_destroy(void *this) {
    status_t status = ST_SUCCESS;
    font_ttf_t *font;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    font = this;

    free(font->cmap.fmt4.endCode);
    free(font->cmap.fmt4.startCode);
    free(font->cmap.fmt4.idDelta);

    return status;
}

static status_t font_tty_init(void *this, uint8_t *font_bin_data) {
    status_t status = ST_SUCCESS;
    if (this == NULL)
        return ST_INVALID_PARAMETER;

    font_ttf_t *font = this;
    uint64_t font_bin_offset = (uint64_t)font_bin_data;

    font->offset_t.sfntVersion = swap_endian_32(*(uint32_t *)font_bin_offset);
    font_bin_offset += sizeof(uint32_t);
    font->offset_t.numTables = swap_endian_16(*(uint16_t *)(font_bin_offset));
    font_bin_offset += sizeof(uint16_t);
    font->offset_t.searchRange = swap_endian_16(*(uint16_t *)(font_bin_offset));
    font_bin_offset += sizeof(uint16_t);
    font->offset_t.entrySelector = swap_endian_16(*(uint16_t *)(font_bin_offset));
    font_bin_offset += sizeof(uint16_t);
    font->offset_t.rangeShift = swap_endian_16(*(uint16_t *)(font_bin_offset));

    font_bin_offset = (uint64_t)(font_bin_data + sizeof(font_ttf_offset_table_t));

    font_ttf_table_directory_t *table_dirs =
        (font_ttf_table_directory_t *)font_bin_offset;

    for (int i = 0; i < font->offset_t.numTables; i++) 
    {

        // head 0x68656164
        if (table_dirs[i].tag == 0x64616568) {

            font->head.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->head.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->head.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->head.dir.length = swap_endian_32(table_dirs[i].length);

            uint8_t *head = (uint8_t *)(font_bin_data + font->head.dir.offset);
            font->head.table.magicNumber = swap_endian_32(*(uint32_t *)(head + 12));

            if (font->head.table.magicNumber != 0x5F0F3CF5)
                krnl_panic(NULL);

            font->head.table.majorVersion = swap_endian_16(*(uint16_t *)(head));
            font->head.table.minorVersion = swap_endian_16(*(uint16_t *)(head + 2));
            font->head.table.fontRevision = swap_endian_32(*(int32_t *)(head + 4));
            font->head.table.checkSumAdjustment =
                swap_endian_32(*(uint32_t *)(head + 8));
            font->head.table.flags = swap_endian_16(*(uint16_t *)(head + 0x10));
            font->head.table.unitsPerEm = swap_endian_16(*(uint32_t *)(head + 0x12));
            font->head.table.created = swap_endian_64(*(int64_t *)(head + 0x14));
            font->head.table.modified = swap_endian_64(*(int64_t *)(head + 0x1a));

            font->head.table.xMax = swap_endian_16(*(int16_t *)head + 0x24);
            font->head.table.xMin = swap_endian_16(*(int16_t *)head + 0x26);
            font->head.table.yMax = swap_endian_16(*(int16_t *)head + 0x28);
            font->head.table.yMin = swap_endian_16(*(int16_t *)head + 0x2a);

            font->head.table.macStyle = swap_endian_16(
                    *(uint16_t *)(font_bin_data + font->head.dir.offset + 0x2C));
            font->head.table.lowestRecPPEM = swap_endian_16(
                    *(uint16_t *)(font_bin_data + font->head.dir.offset + 0x2E));
            font->head.table.fontDirectionHint = swap_endian_16(
                    *(int16_t *)(font_bin_data + font->head.dir.offset + 0x30));
            font->head.table.indexToLocFormat = swap_endian_16(
                    *(int16_t *)(font_bin_data + font->head.dir.offset + 0x32));
        }

        // maxp 0x6d617870
        else if (table_dirs[i].tag == 0x7078616d) {
            font->maxp.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->maxp.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->maxp.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->maxp.dir.length = swap_endian_32(table_dirs[i].length);

            font_ttf_maxp_table_t *maxp =
                (font_ttf_maxp_table_t *)(font_bin_data + font->maxp.dir.offset);
            font->maxp.table.version = swap_endian_32(maxp->version);
            font->maxp.table.numGlyphs = swap_endian_16(maxp->numGlyphs);
        }
        // glyp 0x676c7966
        else if (table_dirs[i].tag == 0x66796c67) {
            // glyp = (uint8_t*)(addr + table_dirs[i].offset);
            font->glyp.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->glyp.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->glyp.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->glyp.dir.length = swap_endian_32(table_dirs[i].length);

            font->glyp.start = (uint64_t)font_bin_data + font->glyp.dir.offset;
        }

        // loca 0x6c6f6361
        else if (table_dirs[i].tag == 0x61636f6c) {
            font->loca.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->loca.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->loca.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->loca.dir.length = swap_endian_32(table_dirs[i].length);

            font->loca.table.offsets =
                (uint16_t *)(font_bin_data + font->loca.dir.offset);
        }

        // hhea 0x68686561
        else if (table_dirs[i].tag == 0x61656868)
        {
            font->hhea.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->hhea.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->hhea.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->hhea.dir.length = swap_endian_32(table_dirs[i].length);

            font_ttf_hhea_table_t* hhea = (font_ttf_hhea_table_t*)(font_bin_data + font->hhea.dir.offset);
            font->hhea.table.advanceWidthMax = swap_endian_16(hhea->advanceWidthMax);
            font->hhea.table.ascender = swap_endian_16(hhea->ascender);
            font->hhea.table.descender = swap_endian_16(hhea->descender);
            font->hhea.table.lineGap = swap_endian_16(hhea->lineGap);
            font->hhea.table.numberOfHMetrics = swap_endian_16(hhea->numberOfHMetrics);
        }

        // hmtx 0x686d7478
        else if (table_dirs[i].tag == 0x78746d68)
        {
            font->hmtx.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->hmtx.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->hmtx.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->hmtx.dir.length = swap_endian_32(table_dirs[i].length);

            
        }

        // cmap 0x636d6170
        else if (table_dirs[i].tag == 0x70616d63) {
            font->cmap.dir.tag = swap_endian_32(table_dirs[i].tag);
            font->cmap.dir.checkSum = swap_endian_32(table_dirs[i].checkSum);
            font->cmap.dir.offset = swap_endian_32(table_dirs[i].offset);
            font->cmap.dir.length = swap_endian_32(table_dirs[i].length);

            uint16_t *tm;
            font_ttf_cmap_table_t *cmap =
                (font_ttf_cmap_table_t *)(font_bin_data + font->cmap.dir.offset);
            font->cmap.table.version = swap_endian_16(cmap->version);
            font->cmap.table.numTables = swap_endian_16(cmap->numTables);
            font->cmap.table.encodingRecords = (EncodingRecord *)((uint64_t)cmap + 4);

            for (int m = 0; m < font->cmap.table.numTables; m++) {
                uint16_t platform_id =
                    swap_endian_16(font->cmap.table.encodingRecords[m].platform_id);
                uint16_t encoding_id =
                    swap_endian_16(font->cmap.table.encodingRecords[m].encoding_id);
                uint32_t subtable_offset =
                    swap_endian_32(font->cmap.table.encodingRecords[m].subtable_offset);

                if (platform_id == 0 && encoding_id == 3) {
                    font_ttf_cmap_subt_fmt_4_t *subt_fmt4 =
                        (font_ttf_cmap_subt_fmt_4_t *)(subtable_offset + (uint64_t)cmap);
                    font->cmap.fmt4.format = swap_endian_16(subt_fmt4->format);
                    font->cmap.fmt4.length = swap_endian_16(subt_fmt4->length);
                    font->cmap.fmt4.language = swap_endian_16(subt_fmt4->language);
                    font->cmap.fmt4.segCountX2 = swap_endian_16(subt_fmt4->segCountX2);
                    font->cmap.fmt4_segcount = font->cmap.fmt4.segCountX2 >> 1;

                    tm = (uint16_t *)((uint64_t)subt_fmt4 + 7 * sizeof(uint16_t));
                    font->cmap.fmt4.endCode =
                        (uint16_t *)malloc(font->cmap.fmt4_segcount * sizeof(uint16_t));

                    if (font->cmap.fmt4.endCode == NULL)
                        krnl_panic(NULL);
                    for (size_t i = 0; i < font->cmap.fmt4_segcount; i++)
                        font->cmap.fmt4.endCode[i] = swap_endian_16(tm[i]);

                    font->cmap.fmt4.reservedPad =
                        swap_endian_16(*(tm + font->cmap.fmt4_segcount));

                    tm = (uint16_t *)(tm + font->cmap.fmt4_segcount + 1);
                    font->cmap.fmt4.startCode =
                        (uint16_t *)malloc(font->cmap.fmt4_segcount * sizeof(uint16_t));

                    if (font->cmap.fmt4.startCode == NULL)
                        krnl_panic(NULL);
                    for (size_t i = 0; i < font->cmap.fmt4_segcount; i++)
                        font->cmap.fmt4.startCode[i] = swap_endian_16(tm[i]);

                    tm = (uint16_t *)(tm + font->cmap.fmt4_segcount);
                    font->cmap.fmt4.idDelta =
                        (int16_t *)malloc(font->cmap.fmt4_segcount * sizeof(int16_t));

                    if (font->cmap.fmt4.idDelta == NULL)
                        krnl_panic(NULL);
                    for (size_t i = 0; i < font->cmap.fmt4_segcount; i++)
                        font->cmap.fmt4.idDelta[i] = swap_endian_16(tm[i]);

                    tm = (uint16_t *)(tm + font->cmap.fmt4_segcount);
                    font->cmap.fmt4.idRangeOffset = tm;

                    font->cmap.fmt4.glyphIdArray =
                        (uint16_t *)(font->cmap.fmt4.idRangeOffset +
                                font->cmap.fmt4_segcount);
                }
            }
        }
    }

    font->hmtx.table.hMetrics = (struct _font_ttf_long_hor_metric*)(font_bin_data + font->hmtx.dir.offset);
    font->hmtx.table.leftSideBearings = (int16_t*)
        ((uint64_t)font->hmtx.table.hMetrics + sizeof(struct _font_ttf_long_hor_metric) * font->hhea.table.numberOfHMetrics);

    return status;
}

static uint16_t get_glyph_id(void *this, wch_t ch_code) {
    uint16_t glyph_id = 0;
    uint64_t glyf_offset;
    font_ttf_t *font;

    if (this == NULL)
        return -1;

    font = this;

    for (int i = 0; i < font->cmap.fmt4_segcount; i++) {

        if ((font->cmap.fmt4.startCode[i] <= ch_code) &&
                (ch_code <= font->cmap.fmt4.endCode[i])) {

            // If the idRangeOffset value for the segment is not 0, the mapping of
            // character codes relies on glyphIdArray. The character code offset from
            // startCode is added to the idRangeOffset value. This sum is used as an
            // offset from the current location within idRangeOffset itself to index
            // out the correct glyphIdArray value. This obscure indexing trick works
            // because glyphIdArray immediately follows idRangeOffset in the font
            // file.
            if (font->cmap.fmt4.idRangeOffset[i] != 0) {

                glyph_id = swap_endian_16(
                        *(&font->cmap.fmt4.idRangeOffset[i] +
                            (swap_endian_16(font->cmap.fmt4.idRangeOffset[i]) / 2) +
                            (ch_code - font->cmap.fmt4.startCode[i])));

                // If the value obtained from the indexing operation is not 0 (which
                // indicates missingGlyph), idDelta[i] is added to it to get the glyph
                // index.
                if (glyph_id != 0) {
                    glyph_id = (glyph_id + font->cmap.fmt4.idDelta[i]) % 65536;
                }
            }

            // If the idRangeOffset is 0, the idDelta value is added directly to
            // the character code offset (i.e. idDelta[i] + c) to get the
            // corresponding glyph index. Again, the idDelta arithmetic is modulo
            // 65536. If the result after adding idDelta[i] + c is less than zero, add
            // 65536 to obtain a valid glyph ID.
            else {
                glyph_id = font->cmap.fmt4.idDelta[i] + ch_code;
                if (glyph_id < 0) {
                    glyph_id += 65536;
                }
            }
            break;
        }
    }

    return glyph_id;
}

static status_t glyph_destroy(void *this) {
    status_t status = ST_SUCCESS;
    font_ttf_glyph_t *glyph;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    glyph = this;

    if (glyph->endPtsOfContours != NULL)
        free(glyph->endPtsOfContours);

    if (glyph->point_index_in_contour != NULL) {
        for (size_t i = 0; i < glyph->numberOfContours; i++)
            if (glyph->point_index_in_contour[i] != NULL)
                free(glyph->point_index_in_contour[i]);

        free(glyph->point_index_in_contour);
    }

    if (glyph->contour_index_for_point != NULL)
        free(glyph->contour_index_for_point);

    if (glyph->flags != NULL)
        free(glyph->flags);

    if (glyph->point_2d_coordinates != NULL)
        free(glyph->point_2d_coordinates);

    if (glyph->contour_list != NULL) {
        for (bezier_curve_t *curve = glyph->contour_list[0]; curve != NULL;) {
            bezier_curve_t *prev = curve;
            curve = prev->flink;
            free(prev);
        }
        free(glyph->contour_list);
    }

    return status;
}



static status_t glyph_init(void *this, wch_t wch_code, font_ttf_t *family) {
    status_t status = ST_SUCCESS;
    font_ttf_glyph_t *glyph;
    uint64_t glyf_offset;
    uint16_t glyph_id;

    if (this == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    glyph = this;

    glyph_id = family->get_glyph_id(family, wch_code);

__glyph_init:

    if (family->head.table.indexToLocFormat) {
        glyf_offset = swap_endian_32(family->loca.table.l_offsets[glyph_id]);
    } else {
        uint16_t offset = swap_endian_16(family->loca.table.offsets[glyph_id]);
        glyf_offset = offset;
        glyf_offset <<= 1;
    }

    glyf_offset += (uint64_t)family->glyp.start;

    glyph->glyph_index = glyph_id;
    glyph->font_family = family;

    uint16_t *glyp_uint16_offset = (uint16_t *)glyf_offset;
    uint8_t *glyp_uint8_offset = (uint8_t *)glyf_offset;

    glyph->numberOfContours = swap_endian_16(*(glyp_uint16_offset++));
    glyph->xMin = swap_endian_16(*(glyp_uint16_offset++));
    glyph->yMin = swap_endian_16(*(glyp_uint16_offset++));
    glyph->xMax = swap_endian_16(*(glyp_uint16_offset++));
    glyph->yMax = swap_endian_16(*(glyp_uint16_offset++));

    glyph->advance_width = swap_endian_16(family->hmtx.table.hMetrics[glyph_id].advanceWidth);
    glyph->lsb = swap_endian_16(family->hmtx.table.hMetrics[glyph_id].lsb);
    glyph->rsb = glyph->advance_width - (glyph->lsb + glyph->xMax - glyph->xMin);

    // point_f_t glyph_center;
    // glyph_center.x = (glyph->xMin + glyph->xMax) / 2.0f;
    // glyph_center.y = (glyph->yMin + glyph->yMax) / 2.0f;

    if (glyph->numberOfContours > 0) {

        // Array of last points of each contour;
        glyph->endPtsOfContours =
            (uint16_t *)malloc(glyph->numberOfContours * sizeof(uint16_t));

        glyph->contour_list =
            (curves_t *)malloc(sizeof(curves_t) * glyph->numberOfContours);

        for (int i = 0; i < glyph->numberOfContours; i++) {
            glyph->contour_list[i] = NULL;
        }

        // read the last point of each contour.
        for (int i = 0; i < glyph->numberOfContours; i++) {
            glyph->endPtsOfContours[i] = swap_endian_16(*(glyp_uint16_offset++));
        }

        // calculate the number of all points based on the last item of
        // endPtsOfContours.
        glyph->numberOfPoints =
            glyph->endPtsOfContours[glyph->numberOfContours - 1] + 1;

        // Total number of bytes needed for instructions
        uint16_t instruction_num;

        glyph->contour_index_for_point =
            (uint16_t *)malloc(sizeof(uint16_t) * glyph->numberOfPoints);

        instruction_num = swap_endian_16(*(glyp_uint16_offset++));

        // uint8	instructions[instructionLength]	Array of instructions for this
        // glyph skip instructions
        glyp_uint16_offset =
            (uint16_t *)((uint64_t)glyp_uint16_offset + instruction_num);

        // every point conresponding one flag
        glyph->flags =
            (GlyphFlag *)malloc(sizeof(GlyphFlag) * glyph->numberOfPoints);

        // record all points which be contained in each contour;
        glyph->point_index_in_contour =
            (uint16_t **)malloc(glyph->numberOfContours * sizeof(uint16_t *));

        if (glyph->point_index_in_contour == NULL) {
            krnl_panic(NULL);
        }

        glyph->point_index_in_contour[0] =
            (uint16_t *)malloc(sizeof(uint16_t) * (glyph->endPtsOfContours[0] + 1));

        for (int i = 1; i < glyph->numberOfContours; ++i) {
            glyph->point_index_in_contour[i] = (uint16_t *)malloc(
                    (glyph->endPtsOfContours[i] - glyph->endPtsOfContours[i - 1]) *
                    sizeof(uint16_t));
        }

        glyp_uint8_offset = (uint8_t *)(glyp_uint16_offset);

        uint8_t repeat = 0;
        for (int i = 0, point_index = 0, contour_index = 0;
                i < glyph->numberOfPoints; i++, point_index++) {
            if (repeat == 0) {
                uint8_t flag;

                flag = *(glyp_uint8_offset++);

                if (__glyph_repeat(flag))
                    repeat = *(glyp_uint8_offset++);

                glyph->flags[i] = flag;

            } else {
                glyph->flags[i] = glyph->flags[i - 1];
                repeat--;
            }

            if (point_index >= (contour_index
                        ? (glyph->endPtsOfContours[contour_index] -
                            glyph->endPtsOfContours[contour_index - 1])
                        : (glyph->endPtsOfContours[0] + 1))) {
                point_index = 0;
                contour_index++;
            }

            glyph->contour_index_for_point[i] = contour_index;
            glyph->point_index_in_contour[contour_index][point_index] = i;
        }

        glyp_uint16_offset = (uint16_t *)glyp_uint8_offset;

        // Point 2d-coordinates
        glyph->point_2d_coordinates =
            (point_f_t *)malloc(sizeof(point_f_t) * glyph->numberOfPoints);

        // If the y-short Vector bit is not set, and this bit is set,
        // then the current y-coordinate is the same as the previous y-coordinate.

        for (int i = 0; i < glyph->numberOfPoints; i++) {
            int16_t i16 = 0;
            uint8_t u8 = 0;
            uint8_t flag = glyph->flags[i];

            if (__glyph_x_short(flag)) {
                u8 = *(glyp_uint8_offset);
                glyp_uint8_offset += 1;

                glyph->point_2d_coordinates[i].x = __glyph_x_Dual(flag) ? u8 : -u8;

            } else {
                if (__glyph_x_Dual(flag))
                    glyph->point_2d_coordinates[i].x =
                        i ? glyph->point_2d_coordinates[i - 1].y : 0;
                else {
                    i16 = swap_endian_16(*(uint16_t *)glyp_uint8_offset);
                    glyp_uint8_offset += 2;
                    glyph->point_2d_coordinates[i].x = i16;
                }
            }
        }

        for (int i = 0; i < glyph->numberOfPoints; i++) {
            int16_t i16 = 0;
            uint8_t u8 = 0;
            uint8_t flag = glyph->flags[i];

            if (__glyph_y_short(flag)) {
                u8 = *(glyp_uint8_offset);
                glyp_uint8_offset += 1;

                glyph->point_2d_coordinates[i].y = __glyph_y_Dual(flag) ? u8 : -u8;

            } else {
                if (__glyph_y_Dual(flag))
                    glyph->point_2d_coordinates[i].y =
                        i ? glyph->point_2d_coordinates[i - 1].y : 0;
                else {
                    i16 = swap_endian_16(*(uint16_t *)glyp_uint8_offset);
                    glyp_uint8_offset += 2;
                    glyph->point_2d_coordinates[i].y = i16;
                }
            }
        }

        for (int i = 1; i < glyph->numberOfPoints; i++) {
            uint8_t flag = glyph->flags[i];
            if (!__glyph_x_short(flag) && __glyph_x_Dual(flag)) {
                glyph->point_2d_coordinates[i].x = glyph->point_2d_coordinates[i - 1].x;
            } else {
                glyph->point_2d_coordinates[i].x +=
                    glyph->point_2d_coordinates[i - 1].x;
            }
            if (!__glyph_y_short(flag) && __glyph_y_Dual(flag)) {
                glyph->point_2d_coordinates[i].y = glyph->point_2d_coordinates[i - 1].y;
            } else {
                glyph->point_2d_coordinates[i].y +=
                    glyph->point_2d_coordinates[i - 1].y;
            }
        }

        for (int i = 0; i < glyph->numberOfContours; i++) {

            uint16_t num_of_points_on_contour =
                i ? glyph->endPtsOfContours[i] - glyph->endPtsOfContours[i - 1]
                : glyph->endPtsOfContours[i] + 1;
            uint16_t first_point_in_contour = glyph->point_index_in_contour[i][0];

            uint8_t first_point_flag = glyph->flags[first_point_in_contour];
            point_f_t pre_point = {0};

            if (__glyph_off_curve(first_point_flag)) {
                uint16_t last_point_in_contour =
                    glyph->point_index_in_contour[i][num_of_points_on_contour - 1];
                uint8_t last_point_flag = glyph->flags[last_point_in_contour];

                if (__glyph_off_curve(last_point_flag)) {

                    pre_point.x = (glyph->point_2d_coordinates[first_point_in_contour].x +
                            glyph->point_2d_coordinates[last_point_in_contour].x) /
                        2.0f;
                    pre_point.y = (glyph->point_2d_coordinates[first_point_in_contour].y +
                            glyph->point_2d_coordinates[last_point_in_contour].y) /
                        2.0f;
                } else {
                    pre_point.x = glyph->point_2d_coordinates[last_point_in_contour].x;
                    pre_point.y = glyph->point_2d_coordinates[last_point_in_contour].y;
                }
            }

            bezier_curve_t *prev_curve = NULL;
            for (int j = 0; j < num_of_points_on_contour; j++) {

                uint16_t p0_index =
                    glyph->point_index_in_contour[i][j % num_of_points_on_contour];
                uint16_t p1_index =
                    glyph
                    ->point_index_in_contour[i][(j + 1) % num_of_points_on_contour];

                uint8_t p0_flag = glyph->flags[p0_index];
                uint8_t p1_flag = glyph->flags[p1_index];

                point_f_t p0 = glyph->point_2d_coordinates[p0_index];
                point_f_t p1 = glyph->point_2d_coordinates[p1_index];

                bezier_curve_t *curve =
                    (bezier_curve_t *)malloc(sizeof(bezier_curve_t));
                memzero(curve, sizeof(bezier_curve_t));

                if (__glyph_on_curve(p0_flag)) {
                    curve->p0.x = p0.x;
                    curve->p0.y = p0.y;

                    // check if the curve is line.
                    if (__glyph_on_curve(p1_flag)) {
                        curve->p1.x = p1.x;
                        curve->p1.y = p1.y;
                        curve->is_curve = 0;

                        pre_point.x = p1.x;
                        pre_point.y = p1.y;

                        goto __inspect_end;
                    } else {
                        pre_point.x = p0.x;
                        pre_point.y = p0.y;

                        curve->p1.x = p1.x;
                        curve->p1.y = p1.y;

                        uint16_t p2_index =
                            glyph->point_index_in_contour[i][(j + 2) %
                            num_of_points_on_contour];
                        uint8_t p2_flag = glyph->flags[p2_index];
                        point_f_t p2 = glyph->point_2d_coordinates[p2_index];

                        // check if the curve is bezier curve.
                        if (__glyph_on_curve(p2_flag)) {
                            curve->p2.x = p2.x;
                            curve->p2.y = p2.y;

                            pre_point.x = p2.x;
                            pre_point.y = p2.y;

                            curve->is_curve = 1;
                            j++;

                            goto __inspect_end;
                        } else {
                            pre_point.x = (p1.x + p2.x) / 2.0f;
                            pre_point.y = (p1.y + p2.y) / 2.0f;

                            curve->p2.x = pre_point.x;
                            curve->p2.y = pre_point.y;

                            curve->is_curve = 1;

                            goto __inspect_end;
                        }
                    }

                } else {
                    if (__glyph_on_curve(p1_flag)) {
                        curve->p0.x = pre_point.x;
                        curve->p0.y = pre_point.y;

                        curve->p1.x = p0.x;
                        curve->p1.y = p0.y;

                        curve->p2.x = p1.x;
                        curve->p2.y = p1.y;

                        curve->is_curve = 1;

                        pre_point.x = p1.x;
                        pre_point.y = p1.y;

                        goto __inspect_end;
                    } else {
                        curve->p0.x = pre_point.x;
                        curve->p0.y = pre_point.y;

                        curve->p1.x = p0.x;
                        curve->p1.y = p0.y;

                        pre_point.x = (p0.x + p1.x) / 2.0f;
                        pre_point.y = (p0.y + p1.y) / 2.0f;

                        curve->p2.x = pre_point.x;
                        curve->p2.y = pre_point.y;

                        curve->is_curve = 1;

                        goto __inspect_end;
                    }
                }

__inspect_end:

                if (glyph->contour_list[i] == NULL && prev_curve == NULL) {
                    glyph->contour_list[i] = curve;
                    curve->count = 1;
                    prev_curve = curve;
                } else {
                    bezier_curve_t *head = glyph->contour_list[i];
                    head->count++;

                    prev_curve->flink = curve;
                    curve->blink = prev_curve;

                    prev_curve = curve;
                }
                glyph->numberOfTriangles++;
            }
        }
    } 
    else 
    {
        glyph_id = 0;
        goto __glyph_init;
    }

    return status;
}

// #define FILL_CHAR

#ifdef FILL_CHAR
typedef enum _point_state
{
    NotBeProcessed = 0,
    IsContour,
    InContour,
    OutOfContour

} point_state_t;

static void fill_neighbor(int start_x, int start_y, point_state_t state, point_state_t** flags, int width, int height);
#endif

status_t glyph_rasterize(void *_this, go_buf_t* buffer, point_i_t origin, double point_size, go_blt_pixel_t color) 
{
    status_t status = ST_SUCCESS;
    font_ttf_glyph_t *this;

    if (_this == NULL || buffer == NULL) {
        status = ST_INVALID_PARAMETER;
        return status;
    }

    this = _this;


    int contour_num = this->numberOfContours;
    double scaling_factor = (point_size * DPI) / (72 * this->font_family->head.table.unitsPerEm);

#ifdef FILL_CHAR

    int width, height;
    width = ceil(this->font_family->head.table.unitsPerEm * scaling_factor);
    height = ceil((this->font_family->hhea.table.ascender - this->font_family->hhea.table.descender) * scaling_factor);

    point_state_t** flags = (point_state_t**)malloc(sizeof(point_state_t*) * height); 

    for (size_t i = 0; i < height; i++) 	
    {
        flags[i] = (point_state_t*)malloc(sizeof(point_state_t) * width);
        memzero(flags[i], sizeof(point_state_t) * width);
    }

#endif

    origin.x += ceil(this->lsb * scaling_factor);
    origin.y -= ceil(this->yMin * scaling_factor);

    for (int i = 0; i < contour_num; i++) {

        bezier_curve_t *curve = this->contour_list[i];
        int curve_num = curve->count;

        for (int c = 0; curve != NULL; c++) {

            point_i_t p;
            bezier_curve_t *next = curve->flink;
            point_f_t p0, p1, p2;
            p0 = curve->p0;
            p1 = curve->p1;
            p2 = curve->p2;

            p0.x += (-this->xMin);
            p1.x += (-this->xMin);
            p2.x += (-this->xMin);

            p0.y += (-this->yMin);
            p1.y += (-this->yMin);
            p2.y += (-this->yMin);

            p0.y = this->font_family->hhea.table.ascender - p0.y;
            p1.y = this->font_family->hhea.table.ascender - p1.y;
            p2.y = this->font_family->hhea.table.ascender - p2.y;

            if (curve->is_curve) 
            {
                p0.x *= scaling_factor;
                p0.y *= scaling_factor;

                p1.x *= scaling_factor;
                p1.y *= scaling_factor;

                p2.x *= scaling_factor;
                p2.y *= scaling_factor;

                // const float delta = 1 / ((this->yMax - this->yMin) * scaling_factor);
                const float delta = 1 / ((((float)(this->yMax - this->yMin) / (float)4)) * scaling_factor);

                float i = 0.0f;
                point_i_t p_i;
                point_f_t t1, t2, p;


                do {

                    t1.x = __lerp_point_x_f(p0, p1, i);
                    t1.y = __lerp_point_y_f(p0, p1, i);
                    t2.x = __lerp_point_x_f(p1, p2, i);
                    t2.y = __lerp_point_y_f(p1, p2, i);
                    p.x = __lerp_point_x_f(t1, t2, i);
                    p.y = __lerp_point_y_f(t1, t2, i);

                    p_i.x = round(p.x);
                    p_i.y = round(p.y);

#ifdef FILL_CHAR
                    flags[p_i.y][p_i.x] = IsContour;
#endif
                    p_i.x += origin.x;
                    p_i.y += origin.y;

                    buffer->buf[(uint16_t)p_i.y * buffer->width + (uint16_t)p_i.x] = color;
                    i += delta; 

                } while (i <= 1.0f);

            } 
            else 
            {

                point_i_t p_i0, p_i1;

                p_i0.x = round(p0.x * scaling_factor);
                p_i0.y = round(p0.y * scaling_factor);

                p_i1.x = round(p1.x * scaling_factor);
                p_i1.y = round(p1.y * scaling_factor);

                int64_t dx = abs(p_i1.x - p_i0.x);
                int64_t dy = abs(p_i1.y - p_i0.y);
                int sx = (p_i0.x < p_i1.x) ? 1 : -1;
                int sy = (p_i0.y < p_i1.y) ? 1 : -1;
                int err = dx - dy;
                point_i_t p;

                while (1) 
                {
                    p.x = origin.x + p_i0.x;
                    p.y = origin.y + p_i0.y;

#ifdef FILL_CHAR
                    flags[p_i0.y][p_i0.x] = IsContour;
#endif
                    buffer->buf[(uint16_t)p.y * buffer->width + (uint16_t)p.x] = color;

                    if (p_i0.x == p_i1.x && p_i0.y == p_i1.y) break;
                    int e2 = 2 * err;
                    if (e2 > -dy) {
                        err -= dy;
                        p_i0.x += sx;
                    }
                    if (e2 < dx) {
                        err += dx;
                        p_i0.y += sy;
                    }
                }
            }

            curve = next;
        }
    }


#ifdef FILL_CHAR
    for (int i = 0; i < height; i++) 
    {
        for (int j = 0; j < width; j++) 
        {
            if (flags[i][j] == NotBeProcessed) 
            {
                int count = 0;
                for (int k = 0; k < j; k++) 
                {
                    if (flags[i][k] == IsContour && flags[i][k+1] != IsContour) 
                        count++;
                }
                if (count % 2)
                {
                    flags[i][j] = InContour;
                    fill_neighbor(j, i, InContour, flags, width, height);
                }
                else 
                {

                    flags[i][j] = OutOfContour;
                    fill_neighbor(j, i, OutOfContour, flags, width, height);
                }
            }
        }
    }
        
    point_i_t p_fill_em;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (flags[i][j] == InContour) {
                p_fill_em.x = j + origin.x; 
                p_fill_em.y = i + origin.y; 
                buffer->buf[(uint16_t)p_fill_em.y * buffer->width + (uint16_t)p_fill_em.x] = color;
            }
        }
    }



    for (size_t i = 0; i < height; i++)
    	free(flags[i]);
    free(flags);

#endif
    return status;
}


#ifdef FILL_CHAR
static void fill_neighbor(int start_x, int start_y, point_state_t state, point_state_t** flags, int width, int height)
{
    typedef struct _point_node
    {
        point_i_t p;
        list_node_t node;
    } point_node_t ;

    list_node_t queue = { NULL, NULL};
    int count_node = 0;
    point_node_t* s = (point_node_t*)malloc(sizeof(point_node_t));
    s->p.x = start_x; 
    s->p.y = start_y;
    queue.flink = &s->node;
    queue.blink = queue.flink;
    count_node++;

    int dx[4] = { -1, 1, 0, 0};
    int dy[4] = { 0, 0, 1, 1};
    while (count_node > 0) {

        point_node_t* top = struct_base(point_node_t, node, queue.flink);
        _list_dequeue(&queue);
        count_node--;

        for (int i = 0; i < 4; i++) {
            int new_x = top->p.x + dx[i];
            int new_y = top->p.y + dy[i];

            if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height || flags[new_y][new_x] != NotBeProcessed) continue;
            point_node_t* new_node = (point_node_t*)malloc(sizeof(point_node_t) * 2);
            new_node->p.x = new_x; 
            new_node->p.y = new_y;
            _list_push(&queue, &new_node->node);
            count_node++;
            flags[new_y][new_x] = state;

        }
        free(top);

    }
}
#endif

status_t new_a_glyph(font_ttf_glyph_t **glyph) {
    font_ttf_glyph_t *obj = NULL;
    obj = (font_ttf_glyph_t *)malloc(sizeof(font_ttf_glyph_t));

    if (obj == NULL)
        krnl_panic(NULL);
    else
        memzero(obj, sizeof(font_ttf_glyph_t));

    if (obj == NULL) {
        return ST_OUT_OF_RESOURCES;
    }

    obj->init = glyph_init;
    obj->destroy = glyph_destroy;
    obj->rasterize = glyph_rasterize;

    *glyph = obj;
    return ST_SUCCESS;
}

status_t del_a_glyph(font_ttf_glyph_t *glyph) {
    if (ST_ERROR(glyph->destroy(glyph))) {
        return ST_FAILED;
    }
    free(glyph);
    return ST_SUCCESS;
}

status_t new_a_font(font_ttf_t **font) {
    font_ttf_t *obj;
    obj = (font_ttf_t *)malloc(sizeof(font_ttf_t));

    if (obj == NULL)
        krnl_panic(NULL);
    else
        memzero(obj, sizeof(*obj));

    if (obj == NULL) {
        return ST_OUT_OF_RESOURCES;
    }


    obj->init = font_tty_init;
    obj->destroy = font_tty_destroy;
    obj->get_glyph_id = get_glyph_id;

    *font = obj;

    return ST_SUCCESS;
}

status_t del_a_font(font_ttf_t *font) {
    if (ST_ERROR(font->destroy(font))) {
        return ST_FAILED;
    }
    free(font);
    return ST_SUCCESS;
}

status_t _op_text_out(
    _in_ _out_ go_buf_t                         *buf,
    _in_ wch_t                                  *string,
    _in_ point_i_t                              origin,
    _in_ font_ttf_t                             *font_family,
    _in_ double                                 point_size,
    _in_ go_blt_pixel_t                         color
)
{
    status_t status = ST_SUCCESS;
    font_ttf_glyph_t *glyph;
    double scaling_factor;
    int16_t desired_em;
    int16_t ascender;
    int16_t descender;
    int16_t advance_width; 

    scaling_factor = (point_size * DPI) / (72 * font_family->head.table.unitsPerEm);
    desired_em = ceil(scaling_factor * font_family->head.table.unitsPerEm);
    ascender = ceil(scaling_factor * font_family->hhea.table.ascender);
    descender = ceil(scaling_factor * font_family->hhea.table.descender);
    advance_width = desired_em >> 1;

    for (size_t i = 0; string[i] != 0; i++) 
    {
        switch (string[i]) 
        {

        case ' ':
            origin.x += advance_width;
            break;

        case '\t':
            origin.x += (advance_width) * TAB_SIZE;
            break;

        case '\b':
            // origin.x -= advance_width;
            break;

        default:
            status = new_a_glyph(&glyph);
            if (ST_ERROR(status)) 
                krnl_panic(NULL);

            glyph->init(glyph, string[i], font_family);
            status = glyph->rasterize(glyph, buf, origin, point_size, color);
            if (ST_ERROR(status)) 
                krnl_panic(NULL);

            origin.x += ceil(glyph->advance_width * scaling_factor);
            del_a_glyph(glyph);

            break;
        }

        if (origin.x >= buf->width) 
            break;
        
    }

    return status;
}

