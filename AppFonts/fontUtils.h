#ifndef __FONT_UTILS_H__
#define __FONT_UTILS_H__

#include "gfxfont.h"
#include "localFonts/FreeSerif9pt7b.h"

#ifndef defaultTextH
    #define defaultTextH(__GFXfont) (__GFXfont.yAdvance)
#endif

extern const GFXfont    mainFont;

#endif 
