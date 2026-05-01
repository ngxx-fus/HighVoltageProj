#include "OLED128x64.h"
#include <string.h>

#pragma message("OLED128x64.c is compiling for STM32F103...")

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   (OLED_HEIGHT / 8)
#define OLED_COL_OFFSET 2U
#define OLED_ROTATE_180 1U

#if (OLED_HEIGHT == 64)
#define OLED_MULTIPLEX_CFG 0x3F
#define OLED_COM_PINS_CFG  0x12
#else
#define OLED_MULTIPLEX_CFG 0x1F
#define OLED_COM_PINS_CFG  0x02
#endif

#if (OLED_ROTATE_180)
#define OLED_SEG_REMAP_CMD SSD1306_SET_SEG_REMAP_0
#define OLED_COM_SCAN_CMD  SSD1306_SET_COM_SCAN_INC
#else
#define OLED_SEG_REMAP_CMD SSD1306_SET_SEG_REMAP_127
#define OLED_COM_SCAN_CMD  SSD1306_SET_COM_SCAN_DEC
#endif

/* --- STATIC RAM BUFFER (128x32 pixels = 512 Bytes) --- */
static uint8_t s_OLED_Buffer[OLED_WIDTH * OLED_PAGES];

static ReturnCode_t OLED_SendInitSequence(uint8_t i2cAddr, const uint8_t *commands, size_t length) {
    ReturnCode_t stat = I2C1_StartTransaction(i2cAddr, 0);
    if (stat != STAT_OKE) {
        return stat;
    }

    stat = I2C1_WriteNByte(commands, (uint32_t)length);
    I2C1_StopTransaction();
    return stat;
}

ReturnCode_t OLED_Init(void) {

    /* Initialize I2C1 and send SSD1306 startup commands */
    ReturnCode_t cfgStat = I2C1_Config(I2C1_ADDR_7BIT, I2C1_MODE_100KHZ);
    if (cfgStat != STAT_OKE) {
        return cfgStat;
    }
    I2C1_Init();

    uint8_t init_commands[] = {
        SSD1306_COMMAND,
        SSD1306_DISPLAY_OFF,
        SSD1306_SET_DISPLAY_CLOCK_DIV,      0x80,
        SSD1306_SET_MULTIPLEX_RATIO,        OLED_MULTIPLEX_CFG,
        SSD1306_SET_DISPLAY_OFFSET,         0x00,
        SSD1306_SET_START_LINE(0),
        SSD1306_CHARGE_PUMP,                0x14,
        SSD1306_SET_MEMORY_MODE,            0x02,
        OLED_SEG_REMAP_CMD,
        OLED_COM_SCAN_CMD,
        SSD1306_SET_COM_PINS,               OLED_COM_PINS_CFG,
        SSD1306_SET_CONTRAST,               0xCF,
        SSD1306_SET_PRECHARGE,              0xF1,
        SSD1306_SET_VCOM_DETECT,            0x40,
        SSD1306_NORMAL_DISPLAY,
        SSD1306_ENTIRE_DISPLAY_RESUME,
        SSD1306_DEACTIVATE_SCROLL,
        SSD1306_DISPLAY_ON
    };

    ReturnCode_t stat = OLED_SendInitSequence(OLED_I2C_ADDR, init_commands, sizeof(init_commands));
    if (stat != STAT_OKE) {
        stat = OLED_SendInitSequence(/*0x3D*/ 0x7A, init_commands, sizeof(init_commands));
        if (stat != STAT_OKE) {
            return stat;
        }
    }

    OLED_FillScreen(0);
    return OLED_Flush();
}

ReturnCode_t OLED_ResetView(void) {

    /* Reset window boundary registers to cover full screen */
    uint8_t cmds[] = {
        SSD1306_COMMAND,
        SSD1306_DEACTIVATE_SCROLL,
        SSD1306_SET_MEMORY_MODE,    0x02,
        0xB0,
        (uint8_t)(OLED_COL_OFFSET & 0x0F),
        (uint8_t)(0x10 | ((OLED_COL_OFFSET >> 4) & 0x0F)),
        SSD1306_SET_START_LINE(0),
        OLED_SEG_REMAP_CMD,
        OLED_COM_SCAN_CMD,
        SSD1306_SET_DISPLAY_OFFSET, 0x00,
        SSD1306_ENTIRE_DISPLAY_RESUME,
        SSD1306_DISPLAY_ON
    };

    if (I2C1_StartTransaction(OLED_I2C_ADDR, 0) == STAT_OKE) {
        I2C1_WriteNByte(cmds, sizeof(cmds));
        I2C1_StopTransaction();
        return STAT_OKE;
    }
    else {
        return STAT_ERR_IO;
    }
}

ReturnCode_t OLED_TurnOnAllPixels(void) {

    /* Trigger hardware force all pixels on */
    if (I2C1_StartTransaction(OLED_I2C_ADDR, 0) == STAT_OKE) {
        I2C1_WriteByte(SSD1306_COMMAND);
        I2C1_WriteByte(SSD1306_ENTIRE_DISPLAY_ON);
        I2C1_StopTransaction();
        return STAT_OKE;
    }
    else {
        return STAT_ERR_IO;
    }
}

ReturnCode_t OLED_ShowRAMContent(void) {
    SysEntry("OLED_ShowRAMContent");

    /* Disable hardware force pixel and show RAM */
    if (I2C1_StartTransaction(OLED_I2C_ADDR, 0) == STAT_OKE) {
        I2C1_WriteByte(SSD1306_COMMAND);
        I2C1_WriteByte(SSD1306_ENTIRE_DISPLAY_RESUME);
        I2C1_StopTransaction();
        SysExit("OLED_ShowRAMContent:CODE:STAT_OKE");
        return STAT_OKE;
    }
    else {
        SysExit("OLED_ShowRAMContent:CODE:STAT_ERR_IO");
        return STAT_ERR_IO;
    }
}

ReturnCode_t OLED_Flush(void) {
    SysEntry("OLED_Flush");
    /* Stream framebuffer page-by-page for wider controller compatibility */
    if (OLED_ResetView() != STAT_OKE) {
        SysExit("OLED_Flush: CODE: STAT_ERR_IO");
        return STAT_ERR_IO;
    }

    for (uint8_t page = 0; page < OLED_PAGES; ++page) {
        uint8_t set_page_cmd[] = {
            SSD1306_COMMAND,
            (uint8_t)(0xB0 | page),
            (uint8_t)(OLED_COL_OFFSET & 0x0F),
            (uint8_t)(0x10 | ((OLED_COL_OFFSET >> 4) & 0x0F))
        };

        if (I2C1_StartTransaction(OLED_I2C_ADDR, 0) != STAT_OKE) {
            SysExit("OLED_Flush: CODE: STAT_ERR_IO");
            return STAT_ERR_IO;
        }

        if (I2C1_WriteNByte(set_page_cmd, sizeof(set_page_cmd)) != STAT_OKE) {
            I2C1_StopTransaction();
            SysExit("OLED_Flush: CODE: STAT_ERR_IO");
            return STAT_ERR_IO;
        }
        I2C1_StopTransaction();

        if (I2C1_StartTransaction(OLED_I2C_ADDR, 0) != STAT_OKE) {
            SysExit("OLED_Flush: CODE: STAT_ERR_IO");
            return STAT_ERR_IO;
        }

        if (I2C1_WriteByte(SSD1306_DATA) != STAT_OKE) {
            I2C1_StopTransaction();
            SysExit("OLED_Flush: CODE: STAT_ERR_IO");
            return STAT_ERR_IO;
        }

        if (I2C1_WriteNByte(&s_OLED_Buffer[(uint32_t)page * OLED_WIDTH], OLED_WIDTH) != STAT_OKE) {
            I2C1_StopTransaction();
            SysExit("OLED_Flush: CODE: STAT_ERR_IO");
            return STAT_ERR_IO;
        }

        I2C1_StopTransaction();
    }

    SysExit("OLED_Flush: CODE: STAT_OKE");
    return STAT_OKE;
}

ReturnCode_t OLED_FillScreen(color_t color) {
    SysEntry("OLED_FillScreen");

    /* Erase or fill the local RAM buffer array */
    // memset(s_OLED_Buffer, (color != 0) ? 0xFF : 0x00, sizeof(s_OLED_Buffer));
    memset(s_OLED_Buffer, color, sizeof(s_OLED_Buffer));
    
    SysExit("OLED_FillScreen:CODE:STAT_OKE");
    return STAT_OKE;
}

ReturnCode_t OLED_SetPixel(xy_t row, xy_t col, color_t c) {

    /* Determine byte offset and manipulate bitmask */
    if (col < 0 || col >= OLED_WIDTH || row < 0 || row >= OLED_HEIGHT) {
        return STAT_ERR_INVALID_ARG;
    }

    xy_t pageIndex  = row >> 3;
    xy_t bitIndex   = row & 0x7;
    xy_t canvasIndex = pageIndex * OLED_WIDTH + col;

    if (c) {
        s_OLED_Buffer[canvasIndex] |= (1 << bitIndex);
    }
    else {
        s_OLED_Buffer[canvasIndex] &= ~(1 << bitIndex);
    }

    return STAT_OKE;
}

ReturnCode_t OLED_DrawEmptyRect(xy_t rTopLeft, xy_t cTopLeft, xy_t rBottomRight, xy_t cBottomRight, xy_t edgeSize, color_t color) {
    SysEntry("OLED_DrawEmptyRect");

    /* Draw outline borders iteratively */
    if (edgeSize < 1) {
        SysExit("OLED_DrawEmptyRect:CODE:STAT_OKE");
        return STAT_OKE;
    }

    if (rTopLeft < 0 || rTopLeft >= OLED_HEIGHT || cTopLeft < 0 || cTopLeft >= OLED_WIDTH ||
        rBottomRight < 0 || rBottomRight >= OLED_HEIGHT || cBottomRight < 0 || cBottomRight >= OLED_WIDTH) {
        SysExit("OLED_DrawEmptyRect:CODE:STAT_ERR_INVALID_ARG");
        return STAT_ERR_INVALID_ARG;
    }

    if (rTopLeft > rBottomRight || cTopLeft > cBottomRight) {
        SysExit("OLED_DrawEmptyRect:CODE:STAT_ERR_INVALID_ARG");
        return STAT_ERR_INVALID_ARG;
    }

    if (rBottomRight - rTopLeft <= 1 || cBottomRight - cTopLeft <= 1) {
        SysExit("OLED_DrawEmptyRect:CODE:STAT_OKE");
        return STAT_OKE;
    }

    for (xy_t x = rTopLeft; x <= rBottomRight; x++) {
        OLED_SetPixel(x, cTopLeft, color);
        OLED_SetPixel(x, cBottomRight, color);
    }

    for (xy_t y = cTopLeft; y <= cBottomRight; y++) {
        OLED_SetPixel(rTopLeft, y, color);
        OLED_SetPixel(rBottomRight, y, color);
    }

    return OLED_DrawEmptyRect(rTopLeft + 1, cTopLeft + 1, rBottomRight - 1, cBottomRight - 1, edgeSize - 1, color);
}

ReturnCode_t OLED_DrawChar(xy_t r, xy_t c, char ch, color_t cl) {

    /* Map character to font array and render pixel bits */
    if (ch < 0x20 || ch > 0x7E) {
        return STAT_ERR_INVALID_ARG;
    }

    if (mainFont.glyph == NULL || mainFont.bitmap == NULL) {
        return STAT_ERR_IO;
    }

    GFXglyph glyph = mainFont.glyph[ch - 0x20];

    if (glyph.width == 0 || glyph.height == 0 || glyph.width > OLED_WIDTH || glyph.height > OLED_HEIGHT) {
        return STAT_ERR_INVALID_ARG;
    }

    uint16_t bo = glyph.bitmapOffset;
    xy_t     w  = glyph.width;
    xy_t     h  = glyph.height;
    xy_t     xo = glyph.xOffset;
    xy_t     yo = glyph.yOffset;

    xy_t bitmap_top_x = r + yo;
    xy_t bitmap_left_y = c + xo;
    uint8_t bits = 0;
    uint8_t bitCount = 0;

    for (xy_t yy = 0; yy < h; ++yy) {
        for (xy_t xx = 0; xx < w; ++xx) {
            if (bitCount == 0) {
                bits = mainFont.bitmap[bo++];
                bitCount = 8;
            }
            if (bits & 0x80) {
                xy_t pixel_x = bitmap_top_x + yy;
                xy_t pixel_y = bitmap_left_y + xx;

                if (pixel_x >= 0 && pixel_x < OLED_HEIGHT && pixel_y >= 0 && pixel_y < OLED_WIDTH) {
                    OLED_SetPixel(pixel_x, pixel_y, cl);
                }
            }
            bits <<= 1;
            bitCount--;
        }
    }

    return STAT_OKE;
}

ReturnCode_t OLED_DrawText(xy_t r, xy_t c, const char *str, color_t color, bool wrap, uint8_t lineHeight) {
    SysEntry("OLED_DrawText");
    /* Render string and calculate screen wrapping boundaries */
    if (!str) {
        SysExit("OLED_DrawText:CODE:STAT_ERR_INVALID_ARG");
        return STAT_ERR_INVALID_ARG;
    }

    if (mainFont.glyph == NULL || mainFont.bitmap == NULL) {
        SysExit("OLED_DrawText:CODE:STAT_ERR_IO");
        return STAT_ERR_IO;
    }

    xy_t cursor_r = r;
    xy_t cursor_c = c;
    uint8_t finalLineHeight = (lineHeight > 0) ? lineHeight : defaultTextH(mainFont);
    uint32_t charCount = 0;

    while (*str) {
        if (++charCount > 128U) {
            SysExit("OLED_DrawText:CODE:STAT_ERR_IO");
            return STAT_ERR_IO;
        }

        char ch = *str++;

        if (ch == '\n') {
            cursor_c = 0;
            cursor_r += finalLineHeight;
            continue;
        }

        if (ch < 0x20 || ch > 0x7E) {
            continue;
        }

        GFXglyph glyph = mainFont.glyph[ch - 0x20];

        if (wrap && (cursor_c + glyph.xAdvance > OLED_WIDTH)) {
            cursor_c = 0;
            cursor_r += finalLineHeight;
        }

        (void)OLED_DrawChar(cursor_r, cursor_c, ch, color);
        cursor_c += glyph.xAdvance;
    }

    SysExit("OLED_DrawText:CODE:STAT_OKE");
    return STAT_OKE;
}

void OLED_DrawLineText(const char *text, flag_t conf) {

    /* Draw formatted text line based on system configuration bitflags */
    if (text == NULL) {
        return;
    }

    if (!(conf & (1 << 16))) {
        OLED_ResetView();
    }

    if (!(conf & (1 << 17))) {
        OLED_FillScreen(0);
    }

    if (!(conf & (1 << 18))) {
        OLED_ShowRAMContent();
    }

    uint8_t lineIdx = conf & 0xFF;
    uint8_t lineH   = (conf >> 8) & 0xFF;
    
    if (lineH == 0) {
        lineH = defaultTextH(mainFont);
    }

    OLED_DrawText(((lineIdx + 1) * lineH) + (lineIdx * 1), 0, text, 1, true, lineH);

    if (!(conf & (1 << 19))) {
        OLED_Flush();
    }
}