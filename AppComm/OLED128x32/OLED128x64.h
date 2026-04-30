/// @file   OLED128x64.h
/// @brief  Bare-metal OLED SSD1306 128x64 driver using I2C1.

#ifndef __OLED128X64_H__
#define __OLED128X64_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../../AppUtils/All.h"
#include "../../AppFonts/fontUtils.h"
#include "../I2C1/I2C1.h"
#include "Commands.h"
#include <stdbool.h>

// #define OLED_I2C_ADDR       0x3C /*0011_1100*/
#define OLED_I2C_ADDR          0x78 /*0111_1000*/

typedef int32_t     xy_t;
typedef uint8_t     color_t;
typedef uint32_t    flag_t;

/*
      0 1 2  y    127
    0 ###############
    1 ###############
    x #######p#######
    7 ###############

    For 1D buffer:
        + Byte access:      __byte_at(x, y): (integer(x/8)*128 + y)
        + Read operation:   __read_at(x, y):  (__byte_at(x, y) & (1<<(x%8)))
        + Write 1-bit op:   __write_1_at(x, y):  (__byte_at(x, y) |= (1<<(x%8)))
        + Write 0-bit op:   __write_0_at(x, y):  (__byte_at(x, y) &= (~(1<<(x%8))))
*/

/// @brief Initialize and start up the OLED device (send initialization sequence).
/// @param None
/// @return STAT_OKE on success, or error code if initialization fails.
ReturnCode_t OLED_Init(void);

/// @brief Reset the OLED control registers to default state (top-left = (0,0)).
/// @param None
/// @return STAT_OKE on success.
ReturnCode_t OLED_ResetView(void);

/// @brief Turn on all pixels on the OLED display (override RAM content).
/// @param None
/// @return STAT_OKE on success.
ReturnCode_t OLED_TurnOnAllPixels(void);

/// @brief Display the content stored in OLED RAM (resume from “all pixels on” mode).
/// @param None
/// @return STAT_OKE on success.
ReturnCode_t OLED_ShowRAMContent(void);

/// @brief Flush the current framebuffer (canvas) to the OLED display.
/// @param None
/// @return STAT_OKE on success.
ReturnCode_t OLED_Flush(void);

/// @brief Fill the entire screen buffer with a uniform color.
/// @param color Pixel value (1 = ON, 0 = OFF).
/// @return STAT_OKE on success.
ReturnCode_t OLED_FillScreen(color_t color);

/// @brief Set a pixel in the OLED canvas to a given color (1 = ON, 0 = OFF).
/// @param row Pixel row index (0-63).
/// @param col Pixel column index (0-127).
/// @param c Color value (1 = ON, 0 = OFF).
/// @return STAT_OKE on success, STAT_ERR_OUT_OF_RANGE if coordinates are out of bounds.
ReturnCode_t OLED_SetPixel(xy_t row, xy_t col, color_t c);

/// @brief Draw an empty (border-only) rectangle on the OLED canvas.
/// @param rTopLeft     Top-left corner row index.
/// @param cTopLeft     Top-left corner column index.
/// @param rBottomRight Bottom-right corner row index.
/// @param cBottomRight Bottom-right corner column index.
/// @param edgeSize     Thickness of rectangle border (in pixels).
/// @param color        Color value (1 = ON, 0 = OFF).
/// @return STAT_OKE on success.
ReturnCode_t OLED_DrawEmptyRect(xy_t rTopLeft, xy_t cTopLeft, xy_t rBottomRight, xy_t cBottomRight, xy_t edgeSize, color_t color);

/// @brief Draw a single character on the OLED display.
/// @param r Starting row index.
/// @param c Starting column index.
/// @param ch Character to draw.
/// @param cl Text color (1 = ON, 0 = OFF).
/// @return STAT_OKE on success.
ReturnCode_t OLED_DrawChar(xy_t r, xy_t c, char ch, color_t cl);

/// @brief Draw a text string on the OLED display using the active font.
/// @param r    Starting row (baseline position).
/// @param c    Starting column position.
/// @param str  Pointer to the null-terminated text string to render.
/// @param cl   Text color (1 = ON, 0 = OFF).
/// @param wrap Enable word wrapping (true = wrap at screen edge).
/// @param lineHeight Line spacing (in pixels); 0 to use font default.
/// @return STAT_OKE on success, STAT_ERR_INVALID_ARG if string is NULL.
ReturnCode_t OLED_DrawText(xy_t r, xy_t c, const char *str, color_t cl, bool wrap, uint8_t lineHeight);

/// @brief Draw text with line param instead of row index of baseline.
/// @param text Text to be draw
/// @param conf Config lineH, mode. 
/// @note  conf[7:0]:  lineIndex; Range: [0, (screenHeight/lineHeight)].
/// @note  conf[15:08]:  lineHeight;                Set 0 for auto determine.
/// @note  conf[16]: 0 - Call oledResetView(),      1 - Skip oledResetView()
/// @note  conf[17]: 0 - Call oledFillScreen(),     1 - Skip oledFillScreen()
/// @note  conf[18]: 0 - Call oledShowRAMContent(), 1 - Skip oledShowRAMContent()
/// @note  conf[19]: 0 - Call oledFlush(),          1 - Skip oledFlush()
void OLED_DrawLineText(const char *text, flag_t conf);

#ifdef __cplusplus
}
#endif

#endif /* __OLED128X64_H__ */