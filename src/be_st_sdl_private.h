/* Copyright (C) 2015-2016 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// This header should be used by be_st_sdl* code internally

#ifndef	_BE_ST_SDL_PRIVATE_
#define _BE_ST_SDL_PRIVATE_

#include "SDL_surface.h"

#define BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS 500
#define BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS 40

typedef struct {
	bool isSpecial; // Scancode of 0xE0 sent?
	uint8_t dosScanCode;
} emulatedDOSKeyEvent;

void BEL_ST_ForceHostDisplayUpdate(void);
void BEL_ST_SetGfxOutputRects(bool allowResize);
void BEL_ST_UpdateHostDisplay(void);
void BEL_ST_HandleEmuKeyboardEvent(bool isPressed, bool isRepeated, emulatedDOSKeyEvent keyEvent);

extern SDL_Surface *g_be_sdl_windowIconSurface;

#endif
