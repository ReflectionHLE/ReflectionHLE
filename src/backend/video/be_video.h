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

#ifndef BE_VIDEO_H
#define BE_VIDEO_H

#include <stdbool.h>
#include <stdint.h>

typedef struct BE_ST_Rect {
	int x, y, w, h;
} BE_ST_Rect;

extern bool g_sdlDoRefreshGfxOutput;
extern bool g_sdlForceGfxControlUiRefresh;
extern const uint32_t g_sdlVGADefaultBGRAScreenColors[256];
extern const uint32_t g_sdlEGABGRAScreenColors[16];
extern const uint32_t g_sdlCGAGfxBGRAScreenColors[4];

// NOT NECESSARILY AVAILABLE FOR ALL GAMES!
extern bool g_sdlDoAbsMouseMotion;

void BEL_ST_UpdateVirtualCursorPositionFromPointer(int x, int y);

void BEL_ST_SetGfxOutputRects(bool allowResize);
void BEL_ST_ForceHostDisplayUpdate(void);
void BEL_ST_UpdateHostDisplay(void);

// These might be implementation-dependent

void BEL_ST_RecreateWindowAndRenderer(
	int displayNo,
	int windowWidth, int windowHeight,
	int fullWidth, int fullHeight,
	bool fullScreen, bool resizable, bool vsync, int driverIndex);

void BEL_ST_DestroyWindowAndRenderer(void);

void BEL_ST_RenderClear(void);
void BEL_ST_SetDrawColor(uint32_t color);
void BEL_ST_SetDrawBlendMode(bool blend);
void BEL_ST_RenderFill(const BE_ST_Rect *rect);
void BEL_ST_UpdateWindow(void);

void BEL_ST_SetWindowFullScreenToggle(bool fullScreen);
bool BEL_ST_GetWindowFullScreenToggle(void);

void BEL_ST_SetWindowSize(int w, int h);
void BEL_ST_GetWindowSize(int *w, int *h);

void BEL_ST_GetDesktopDisplayDims(int *w, int *h);

int BEL_ST_GetWindowDisplayNum(void);

bool BEL_ST_IsConfiguredForSWRendering(void);
bool BEL_ST_IsVsyncPreferred(bool islauncherWindow);

#endif

