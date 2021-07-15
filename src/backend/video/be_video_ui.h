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

#ifndef BE_VIDEO_UI_H
#define BE_VIDEO_UI_H

#include <stdbool.h>
#include "be_video_sdl.h"

typedef struct {
	BE_ST_TouchID touchId;
	BE_ST_FingerID fingerId;
	int lastX, lastY;
	// Used in multiple mappings, but not necessarily all of them
	bool isDefaultBinaryStateToggle;
	// This one depends on current mapping in use
	union {
		struct {
			int x, y;
		} key;
		int padButtonScanCode;
		int touchMappingIndex;
	} miscData;
} BESDLTrackedFinger;

extern BESDLTrackedFinger g_sdlTrackedFingers[];
extern int g_nOfTrackedFingers;

// With alternative game controllers scheme, all UI is hidden
// if no controller is connected. Similar handling is done for touch input.
// Furthermore, some UI is shared.
extern bool g_sdlShowControllerUI;
extern bool g_sdlShowTouchUI;

extern BE_ST_Rect g_sdlControllerFaceButtonsRect, g_sdlControllerDpadRect, g_sdlControllerTextInputRect, g_sdlControllerDebugKeysRect;
extern BE_ST_Texture *g_sdlFaceButtonsTexture, *g_sdlDpadTexture, *g_sdlTextInputTexture, *g_sdlDebugKeysTexture;
extern bool g_sdlFaceButtonsAreShown, g_sdlDpadIsShown, g_sdlTextInputUIIsShown, g_sdlDebugKeysUIIsShown, g_sdlTouchControlsAreShown;

extern BE_ST_Rect g_sdlOnScreenTouchControlsRects[];
extern BE_ST_Texture *g_sdlOnScreenTouchControlsTextures[];
extern int g_sdlNumOfOnScreenTouchControls;

void BEL_ST_SetCommonUIRects(void);
void BEL_ST_SetTouchControlsRects(void);

#endif

