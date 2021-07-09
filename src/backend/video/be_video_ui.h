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

