/* Copyright (C) 2014-2021 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BE_VIDEO_EMU_H
#define BE_VIDEO_EMU_H

#include "stdint.h"

#define GFX_TEX_WIDTH 320
#define GFX_TEX_HEIGHT 200
#define VGA_TXT_TEX_WIDTH 720
#define VGA_TXT_TEX_HEIGHT 400
//#define EGACGA_TXT_TEX_WIDTH 640
//#define EGACGA_TXT_TEX_HEIGHT 200

// Overscan border dimensions (for each side of the screen)
#define ENGINE_VGA_16COLORGFX_OVERSCAN_LEFT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_16COLORGFX_OVERSCAN_RIGHT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_256COLORGFX_OVERSCAN_LEFT_AFTER_DOUBLING 8 // Doubling from 4 for us
#define ENGINE_VGA_256COLORGFX_OVERSCAN_RIGHT_AFTER_DOUBLING 8 // Doubling from 4 for us
#define ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING 7    // 200-line doubling
#define ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING 7 // 200-line doubling
#define ENGINE_VGA_TXT_OVERSCAN_LEFT 9
#define ENGINE_VGA_TXT_OVERSCAN_RIGHT 9
#define ENGINE_VGA_TXT_OVERSCAN_TOP 7
#define ENGINE_VGA_TXT_OVERSCAN_BOTTOM 7

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16

// We can use a union because the memory contents are refreshed on screen mode change
// (well, not on change between modes 0xD and 0xE, both sharing planar A000:0000)
typedef union BEVideoEmuMem {
	uint64_t egaGfx[0x10000]; // Contents of A000:0000, de-planed (1 byte per pixel)
	uint32_t vgaGfx[0x10000]; // Same but for 256-colors. Similarly linear.
	uint8_t gfxByByte[0x80000]; // Same graphics data, accessible per byte.
	uint8_t text[TXT_COLS_NUM*TXT_ROWS_NUM*2]; // Textual contents of B800:0000
} BEVideoEmuMem;

// Used for simple caching of EGA/VGA graphics (due to page flipping and more)
// and similarly CGA graphics (modified only at one place)
typedef union BEVideoEmuHostScrMem {
	uint8_t egaGfx[2*GFX_TEX_WIDTH*GFX_TEX_HEIGHT]; // Support 640x200 mode for Catacomb Abyss
	uint8_t cgaGfx[GFX_TEX_WIDTH*GFX_TEX_HEIGHT];
} BEVideoEmuHostScrMem;

extern BEVideoEmuMem g_sdlVidMem;

extern BEVideoEmuHostScrMem g_sdlHostScrMem, g_sdlHostScrMemCache;

extern uint16_t g_sdlScreenStartAddress;
extern int g_sdlScreenMode;
extern int g_sdlTexWidth, g_sdlTexHeight;
extern uint8_t g_sdlPelPanning;
extern int g_sdlPixLineWidth; // Originally stored a byte, while measuring this in bytes instead of pixels
extern int16_t g_sdlSplitScreenLine;
extern int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
extern bool g_sdlTxtCursorEnabled;
extern int g_sdlTxtColor, g_sdlTxtBackground;

extern uint32_t g_sdlEGACurrBGRAPalette[256], g_sdlEGACurrBGRAPaletteCache[256];
extern uint32_t g_sdlEGALastBGRABorderColor;
extern uint8_t g_overscanBorderColorIndex, g_overscanBorderColorIndexCache;

#endif
