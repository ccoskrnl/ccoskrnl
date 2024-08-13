#ifndef __OP_FONT_H__
#define __OP_FONT_H__

#include "../../types.h"
#include "../../libk/limits.h"
#include "graphics.h"

#define DEFAULT_FONT_FAMILY									0x0

#define MAX_FONT_NAME 										0x28
#define MAX_INSTALLED_FONTS									0xFF

struct _font_padding
{
	int up;
	int right;
	int down;
	int left;
};	// 0x10
struct _font_space
{
	int horizontal;
	int vertical;
};	// 0x8

// 0x70
struct _font_info
{
	// 	This is the name of the true type font.
	char face[MAX_FONT_NAME];
	// The size of the true type font.
	int size;
	// 	The font is bold.
	int bold;
	// 	The font is italic.
	int italic;
	// The name of the OEM charset used (when not unicode).
	char charset[MAX_FONT_NAME];
	// 	Set to 1 if it is the unicode charset.
	int unicode;
	// 	The font height stretch in percentage. 100% means no stretch.
	int stretchH;
	// Set to 1 if smoothing was turned on.
	int smooth;
	// The supersampling level used. 1 means no supersampling was used.
	int aa;
	// 	The padding for each character (up, right, down, left).
	struct _font_padding padding;
	// The spacing for each character (horizontal, vertical).
	struct _font_space spacing;
	// 	The outline thickness for the characters.
	int outline;

	int64_t reserved;
};

// 0x30
struct _font_common
{
	int lineHeight;
	int base;
	int scaleW;
	int scaleH;

	int pages;
	int packed;

	int alphaChnl;
	int redChnl;
	int greenChnl;
	int blueChnl;

	int64_t xadvance;

};

struct _font_page
{
	int id;
	int reserved;
	char file[MAX_FONT_NAME];
};	// 0x30

struct _char
{
	uint64_t present : 1;
	int id;

	int x;
	int y;

	int width;
	int height;

	int xoffset;
	int yoffset;
	
	int xadvance;
	int page;
	int chnl;

	uint64_t reserved : 62;

};	// 0x30


struct _ascii_font
{
	struct _font_info info;
	struct _font_common common;
	struct _font_page page_file;
	int count;
	struct _char chars[CHAR_MAX];

	struct{
		go_blt_pixel* buf_addr;
		uint16_t width;
		uint16_t height;
	} blt_buf;
};

void _op_ascii_font_init(
	_in_ char* fnt_addr, 
	_in_ char* buf_addr,
    _in_ uint16_t width,
    _in_ uint16_t height,
	_in_ _out_ struct _ascii_font* font
);

struct _installed_fonts{
    int64_t num;
    struct _ascii_font* font[MAX_INSTALLED_FONTS];
};

extern struct _installed_fonts _op_font;

#endif