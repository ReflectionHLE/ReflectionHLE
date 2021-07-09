/* Copyright (C) 2014-2021 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#ifndef BE_VIDEO_H
#define BE_VIDEO_H

#include "SDL.h" // TODO: Used for g_sdlWindow, g_sdlRenderer
#include <stdbool.h>
#include <stdint.h>
#include "be_video_sdl.h"

extern bool g_sdlDoRefreshGfxOutput;
extern bool g_sdlForceGfxControlUiRefresh;
extern const uint32_t g_sdlVGADefaultBGRAScreenColors[256];
extern const uint32_t g_sdlEGABGRAScreenColors[16];
extern const uint32_t g_sdlCGAGfxBGRAScreenColors[4];

extern SDL_Window *g_sdlWindow;
extern SDL_Renderer *g_sdlRenderer;
extern BE_ST_Texture *g_sdlTexture, *g_sdlTargetTexture;

void BEL_ST_UpdateVirtualCursorPositionFromPointer(int x, int y);
void BEL_ST_RecreateMainTextures(void);

#endif

