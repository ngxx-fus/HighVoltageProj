#ifndef _GFFFONT_H_
#define _GFFFONT_H_
#include <stdint.h>

#define PROGMEM

typedef struct { // Data stored PER GLYPH
	uint16_t bitmapOffset;     // Pointer into GFXfont->bitmap
	uint8_t  width, height;    // Bitmap dimensions in pixels
	uint8_t  xAdvance;         // Distance to advance cursor (x axis)
	int8_t   xOffset, yOffset; // Dist from cursor pos to UL (upper-left) corner 
} GFXglyph;

typedef struct { // Data stored for FONT AS A WHOLE:
	const uint8_t  *bitmap;      // Glyph bitmaps, concatenated
	const GFXglyph *glyph;       // Glyph array
	uint8_t   first, last; // ASCII extents
	uint8_t   yAdvance;    // Newline distance (y axis)
} GFXfont;

#endif
