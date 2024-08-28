#ifndef __GO_GRAPHICS_H__
#define __GO_GRAPHICS_H__

#include "../../types.h"
// #include "font.h"

typedef struct
{
	uint32_t RedMask;
	uint32_t GreenMask;
	uint32_t BlueMask;
	uint32_t ReservedMask;
} EFI_PIXEL_BITMASK;

typedef enum _GRAPHICS_PIXEL_FORMAT
{
	///
	/// A pixel is 32-bits and byte zero represents red, byte one represents green,
	/// byte two represents blue, and byte three is reserved. This is the definition
	/// for the physical frame buffer. The byte values for the red, green, and blue
	/// components represent the color intensity. This color intensity value range
	/// from a minimum intensity of 0 to maximum intensity of 255.
	///
	PixelRedGreenBlueReserved8BitPerColor,
	///
	/// A pixel is 32-bits and byte zero represents blue, byte one represents green,
	/// byte two represents red, and byte three is reserved. This is the definition
	/// for the physical frame buffer. The byte values for the red, green, and blue
	/// components represent the color intensity. This color intensity value range
	/// from a minimum intensity of 0 to maximum intensity of 255.
	///
	PixelBlueGreenRedReserved8BitPerColor,
	///
	/// The Pixel definition of the physical frame buffer.
	///
	PixelBitMask,
	///
	/// This mode does not support a physical frame buffer.
	///
	PixelBltOnly,
	///
	/// Valid EFI_GRAPHICS_PIXEL_FORMAT enum values are less than this value.
	///
	PixelFormatMax
} GRAPHICS_PIXEL_FORMAT;

typedef enum _GO_BLT_OPERATIONS
{
	// Fills a rectangle with a specified color.
	GoBltVideoFill,
	// Copies a rectangle from the video frame buffer to a buffer.
	GoBltVideoToBltBuffer,
	// Copies a rectangle from a buffer to the video frame buffer.
	GoBltBufferToVideo,
	// Copies a rectangle from one location in the video frame buffer to another.
	GoBltVideoToVideo
} GO_BLT_OPERATIONS;

typedef struct _go_blt_pixel
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Reserved;
} go_blt_pixel_t;


typedef struct _coordinates_2d_i
{
	int x;
	int y;
} point_i_t;

typedef struct _coordinates_2d_f
{
	float x;
	float y;
} point_f_t;


struct _go_image_output
{
	go_blt_pixel_t* buf;
	uint16_t width;
	uint16_t height;
	uint32_t size;
};



#define POINT_SIZE  						        18.0f
#define LSB_SIZE  							5.0f
#define LINE_SPACE							2.0f
#define DPI  								96.0f


#endif
