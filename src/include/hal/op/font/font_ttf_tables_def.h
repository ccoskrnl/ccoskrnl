#ifndef __FONT_TTF_TABLES_DEF_H__
#define __FONT_TTF_TABLES_DEF_H__

#include "../../../types.h"

typedef struct _font_ttf_offset_table {

    uint32_t sfntVersion;
    uint16_t numTables;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;

} __attribute__((packed))  font_ttf_offset_table_t;

typedef struct _font_ttf_table_directory {

    uint32_t tag;
    uint32_t checkSum;
    uint32_t offset;
    uint32_t length;

} font_ttf_table_directory_t;

// Define the structure of the 'head' table
typedef struct _font_ttf_head_table {

    uint16_t majorVersion;
    uint16_t minorVersion;
    int32_t fontRevision;
    uint32_t checkSumAdjustment;
    uint32_t magicNumber;
    uint16_t flags;
    uint16_t unitsPerEm;
    int64_t created;
    int64_t modified;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    uint16_t macStyle;
    uint16_t lowestRecPPEM;
    int16_t fontDirectionHint;
    int16_t indexToLocFormat;
    int16_t glyphDataFormat;

} __attribute__((packed)) font_ttf_head_table_t;

typedef struct _font_ttf_maxp_table {
    uint32_t version;
    uint16_t numGlyphs;
    uint16_t maxPoints;
    uint16_t maxContours;
    uint16_t maxCompositePoints;
    uint16_t maxCompositeContours;
    uint16_t maxZones;
    uint16_t maxTwilightPoints;
    uint16_t maxStorage;
    uint16_t maxFunctionDefs;
    uint16_t maxInstructionDefs;
    uint16_t maxStackElements;
    uint16_t maxSizeOfInstructions;
    uint16_t maxComponentElements;
    uint16_t maxComponentDepth;
} __attribute__((packed))  font_ttf_maxp_table_t;

typedef struct _font_ttf_loca_table {

    union {
        uint16_t *offsets;
        uint32_t *l_offsets;
    };
} font_ttf_loca_table_t;

typedef struct {
    uint16_t platform_id;
    uint16_t encoding_id;
    uint32_t subtable_offset;
} EncodingRecord;

typedef struct _font_ttf_cmap_table {
    uint16_t version;
    uint16_t numTables;
    EncodingRecord *encodingRecords;
} font_ttf_cmap_table_t;

typedef struct _font_ttf_cmap_subt_fmt_4 {

    uint16_t format; // Format number is set to 4.
    uint16_t length; // This is the length in bytes of the subtable.
    uint16_t
        language; // For requirements on use of the language field, see “Use of
                  // the language field in 'cmap' subtables” in this document.
    uint16_t segCountX2; // 2 × segCount.

    uint16_t
        searchRange; // Maximum power of 2 less than or equal to segCount, times 2
                     // ((2**ceil(log2(segCount))) * 2, where “**” is an
                     // exponentiation operator)

    uint16_t entrySelector; // Log2 of the maximum power of 2 less than or equal
                            // to segCount (log2(searchRange/2), which is equal to
                            // ceil(log2(segCount)))

    uint16_t rangeShift; // segCount times 2, minus searchRange
                         // ((segCount * 2) - searchRange)

    uint16_t *endCode;       // End characterCode for each segment, last=0xFFFF.
    uint16_t reservedPad;    // Set to 0.
    uint16_t *startCode;     // Start character code for each segment.
    int16_t *idDelta;        // Delta for all character codes in segment.
    uint16_t *idRangeOffset; // Offsets into glyphIdArray or 0
    uint16_t *glyphIdArray;  // Glyph index array (arbitrary length)

} font_ttf_cmap_subt_fmt_4_t;

typedef struct _font_ttf_hhea_table {
    uint16_t majorVersion; // Major version number of the horizontal header table
                           // — set to 1.
    uint16_t minorVersion; // Minor version number of the horizontal header table
                           // — set to 0.
    int16_t ascender;      // Typographic ascent—see remarks below.
    int16_t descender;     // Typographic descent—see remarks below.
    int16_t lineGap; // Typographic line gap. Negative lineGap values are treated
                     // as zero in some legacy platform implementations.
    uint16_t advanceWidthMax;    // Maximum advance width value in 'hmtx' table.
    int16_t minLeftSideBearing;  // Minimum left sidebearing value in 'hmtx' table
                                 // for glyphs with contours (empty glyphs should
                                 // be ignored).
    int16_t minRightSideBearing; // Minimum right sidebearing value; calculated as
                                 // min(aw - (lsb + xMax - xMin)) for glyphs with
                                 // contours (empty glyphs should be ignored).
    int16_t xMaxExtent;          // Max(lsb + (xMax - xMin)).
    int16_t caretSlopeRise;      // Used to calculate the slope of the cursor
                                 // (rise/run); 1 for vertical.
    int16_t caretSlopeRun;       // 0 for vertical.
    int16_t caretOffset; // The amount by which a slanted highlight on a glyph
                         // needs to be shifted to produce the best appearance.
                         // Set to 0 for non-slanted fonts

    int16_t reserved0;   // set to 0
    int16_t reserved1;   // set to 0
    int16_t reserved2;   // set to 0
    int16_t reserved3;   // set to 0
    int16_t metricDataFormat;  // 0 for current format.
    uint16_t numberOfHMetrics; // Number of hMetric entries in 'hmtx' table

}__attribute__((packed)) font_ttf_hhea_table_t;

struct _font_ttf_long_hor_metric
{
  uint16_t advanceWidth;  // Advance width, in font design units.
  int16_t lsb;            // Glyph left side bearing, in font design units.
}__attribute__((packed));

typedef struct _font_ttf_hmtx_table
{
  struct _font_ttf_long_hor_metric* hMetrics;     // Paired advance width and left side bearing values for each glyph. Records are indexed by glyph ID.
  int16_t* leftSideBearings;                      // Left side bearings for glyph IDs greater than or equal to numberOfHMetrics.
} font_ttf_hmtx_table_t ;

#endif
