/* Copyright (C) 2015-2021 NY00123
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

// This header should be used by be_st_sdl* code internally

#ifndef BE_ST_SDL_PRIVATE_H
#define BE_ST_SDL_PRIVATE_H

#include "SDL.h"

#ifdef REFKEEN_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#define BEL_ST_SleepMS emscripten_sleep
#else
#define BEL_ST_SleepMS SDL_Delay
#endif

#define BEL_ST_GetTicksMS SDL_GetTicks

#define BE_ST_MAXJOYSTICKS 8

#define BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS 500
#define BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS 40

// Hack for compilation against SDL pre-2.0.4
#ifndef SDL_RENDER_DEVICE_RESET
#define SDL_RENDER_DEVICE_RESET 0x2001
#endif

#define BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO 1

typedef struct {
	bool isSpecial; // Scancode of 0xE0 sent?
	uint8_t dosScanCode;
} emulatedDOSKeyEvent;

typedef enum {
	BE_ST_MOUSEMODE_ABS_WITH_CURSOR, BE_ST_MOUSEMODE_ABS_WITHOUT_CURSOR, BE_ST_MOUSEMODE_REL
} BESDLMouseModeEnum;

void BEL_ST_ForceHostDisplayUpdate(void);
void BEL_ST_SetGfxOutputRects(bool allowResize);
void BEL_ST_UpdateHostDisplay(void);
void BEL_ST_HandleEmuKeyboardEvent(bool isPressed, bool isRepeated, emulatedDOSKeyEvent keyEvent);
void BEL_ST_ConditionallyShowAltInputPointer(void);
void BEL_ST_SetMouseMode(BESDLMouseModeEnum mode);

extern SDL_Surface *g_be_sdl_windowIconSurface;
// NOT NECESSARILY AVAILABLE FOR ALL GAMES!
extern bool g_sdlDoAbsMouseMotion;

#endif // BE_ST_SDL_PRIVATE_H
