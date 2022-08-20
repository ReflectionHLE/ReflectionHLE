/* Copyright (C) 2014-2022 NY00123
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

#include "be_st.h"
#include "be_input_controller_mappings.h"
#include "../video/be_video_ui.h"

void BEL_ST_DoHideTouchUI(void)
{
	g_sdlShowTouchUI = false;
	g_sdlForceGfxControlUiRefresh = true;
	BEL_ST_ConditionallyShowAltInputPointer();
	if (g_sdlControllerMappingActualCurr->touchMappings
	    || (!g_sdlShowControllerUI && (
	         (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	         || (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	         || g_sdlControllerMappingActualCurr->showUi
	       ))
	)
		BEL_ST_AltControlScheme_ClearBinaryStates();
}

void BEL_ST_CheckForHidingTouchUI(void)
{
	if ((g_refKeenCfg.touchInputToggle != TOUCHINPUT_AUTO) || !g_sdlShowTouchUI)
		return;
	BEL_ST_DoHideTouchUI();
}

/* Shared with mouse and touch inputs, where mouse's touchId is 0
 * (documentation of SDL_GetTouchDevice says 0 is returned for an invalid input index)
 */

void BEL_ST_CheckPressedPointerInTextInputUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckMovedPointerInTextInputUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInTextInputUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckPressedPointerInDebugKeysUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckMovedPointerInDebugKeysUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInDebugKeysUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckPressedPointerInControllerUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckMovedPointerInControllerUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInControllerUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId);
void BEL_ST_CheckPressedPointerInTouchControls(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y, bool forceAbsoluteFingerPositioning);
void BEL_ST_CheckMovedPointerInTouchControls(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInTouchControls(BE_ST_TouchID touchId, BE_ST_FingerID fingerId);

bool BEL_ST_CheckCommonPointerPressCases(
	BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	if (!g_sdlShowControllerUI && !g_sdlShowTouchUI && !g_sdlDebugKeysUIIsShown)
		return false;

	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		BEL_ST_CheckPressedPointerInTextInputUI(touchId, fingerId, x, y);
		return true;
	}
	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		BEL_ST_CheckPressedPointerInDebugKeysUI(touchId, fingerId, x, y);
		return true;
	}
	if (g_sdlControllerMappingActualCurr->showUi)
	{
		BEL_ST_CheckPressedPointerInControllerUI(touchId, fingerId, x, y);
		return true;
	}

	if (g_sdlShowTouchUI)
	{
		BEL_ST_CheckPressedPointerInTouchControls(touchId, fingerId, x, y, false);
		return true;
	}

	return false;
}

bool BEL_ST_CheckCommonPointerReleaseCases(
	BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	if (!g_sdlShowControllerUI && !g_sdlShowTouchUI && !g_sdlDebugKeysUIIsShown)
		return false;

	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		BEL_ST_CheckReleasedPointerInTextInputUI(touchId, fingerId, x, y);
		return true;
	}
	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		BEL_ST_CheckReleasedPointerInDebugKeysUI(touchId, fingerId, x, y);
		return true;
	}
	if (g_sdlControllerMappingActualCurr->showUi)
	{
		BEL_ST_CheckReleasedPointerInControllerUI(touchId, fingerId);
		return true;
	}

	if (g_sdlShowTouchUI)
	{
		BEL_ST_CheckReleasedPointerInTouchControls(touchId, fingerId);
		return true;
	}

	return false;
}

bool BEL_ST_CheckCommonPointerMoveCases(
	BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	if (!g_sdlShowControllerUI && !g_sdlShowTouchUI && !g_sdlDebugKeysUIIsShown)
		return false;

	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		BEL_ST_CheckMovedPointerInTextInputUI(touchId, fingerId, x, y);
		return true;
	}
	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		BEL_ST_CheckMovedPointerInDebugKeysUI(touchId, fingerId, x, y);
		return true;
	}
	if (g_sdlControllerMappingActualCurr->showUi)
	{
		BEL_ST_CheckMovedPointerInControllerUI(touchId, fingerId, x, y);
		return true;
	}

	if (g_sdlShowTouchUI)
	{
		BEL_ST_CheckMovedPointerInTouchControls(touchId, fingerId, x, y);
		return true;
	}

	return false;
}
