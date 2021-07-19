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

#include "be_features.h"
#include "be_input.h"
#include "be_st.h"

uint32_t g_sdlLastPollEventsTime;

bool g_sdlControllersButtonsStates[BE_ST_CTRL_BUT_MAX];
bool g_sdlControllersAxesStates[BE_ST_CTRL_AXIS_MAX][2];

bool g_sdlMouseButtonsStates[3];

bool g_sdlDefaultMappingBinaryState;

int g_sdlEmuMouseButtonsState;
int16_t g_sdlEmuMouseMotionAccumulatedState[2];
int16_t g_sdlEmuMouseCursorPos[2];
int16_t g_sdlEmuMouseMotionFromJoystick[2];
int16_t g_sdlVirtualMouseCursorState[2];
int g_sdlEmuJoyButtonsState;
int16_t g_sdlEmuJoyMotionState[4];

int g_sdlEmuKeyboardLastPressedScanCode;
bool g_sdlEmuKeyboardLastPressedIsSpecial;
uint32_t g_sdlEmuKeyboardLastScanCodePressTime;
uint32_t g_sdlEmuKeyboardLastScanCodePressTimeDelay;

int g_sdlOnScreenKeyboardLastPressedDirButton;
uint32_t g_sdlOnScreenKeyboardLastDirButtonPressTime;
uint32_t g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay;

bool g_sdlEmuKeyboardStateByScanCode[BE_ST_SC_MAX];

static void (*g_sdlKeyboardInterruptFuncPtr)(uint8_t) = 0;

static uint8_t g_sdlLastKeyScanCodeBeforeAnyReset; // May be reset by BE_ST_BiosScanCode

void BE_ST_StartKeyboardService(void (*funcPtr)(uint8_t))
{
	g_sdlKeyboardInterruptFuncPtr = funcPtr;
}

void BE_ST_StopKeyboardService(void)
{
	g_sdlKeyboardInterruptFuncPtr = 0;
}

void BE_ST_ResetEmuMouse(void)
{
	g_sdlEmuMouseCursorPos[0] = BE_ST_EMU_MOUSE_XRANGE/2;
	g_sdlEmuMouseCursorPos[1] = BE_ST_EMU_MOUSE_YRANGE/2;
}

void BE_ST_SetEmuMousePos(int16_t x, int16_t y)
{
	g_sdlEmuMouseCursorPos[0] = x;
	g_sdlEmuMouseCursorPos[1] = y;

	if (g_sdlEmuMouseCursorPos[0] < 0)
		g_sdlEmuMouseCursorPos[0] = 0;
	else if (g_sdlEmuMouseCursorPos[0] >= BE_ST_EMU_MOUSE_XRANGE)
		g_sdlEmuMouseCursorPos[0] = BE_ST_EMU_MOUSE_XRANGE - 1;

	if (g_sdlEmuMouseCursorPos[1] < 0)
		g_sdlEmuMouseCursorPos[1] = 0;
	else if (g_sdlEmuMouseCursorPos[1] >= BE_ST_EMU_MOUSE_YRANGE)
		g_sdlEmuMouseCursorPos[1] = BE_ST_EMU_MOUSE_YRANGE - 1;
}

void BE_ST_GetEmuMousePos(int16_t *x, int16_t *y)
{
	*x = g_sdlEmuMouseCursorPos[0];
	*y = g_sdlEmuMouseCursorPos[1];
}

void BE_ST_GetEmuAccuMouseMotion(int16_t *optX, int16_t *optY)
{
	BE_ST_PollEvents();

	int16_t dx = g_sdlEmuMouseMotionAccumulatedState[0] + g_sdlEmuMouseMotionFromJoystick[0];
	int16_t dy = g_sdlEmuMouseMotionAccumulatedState[1] + g_sdlEmuMouseMotionFromJoystick[1];
	if (g_refKeenCfg.novert)
		dy = 0;

	BE_ST_SetEmuMousePos(g_sdlEmuMouseCursorPos[0] + dx, g_sdlEmuMouseCursorPos[1] + dy);

	if (optX)
		*optX = dx;
	if (optY)
		*optY = dy;

	g_sdlEmuMouseMotionAccumulatedState[0] = g_sdlEmuMouseMotionAccumulatedState[1] = 0;
}

uint16_t BE_ST_GetEmuMouseButtons(void)
{
	BE_ST_PollEvents();

	return g_sdlEmuMouseButtonsState;
}

void BE_ST_GetEmuJoyAxes(uint16_t joy, uint16_t *optX, uint16_t *optY)
{
	BE_ST_PollEvents();

	if (optX)
		*optX = g_sdlEmuJoyMotionState[2*joy];
	if (optY)
		*optY = g_sdlEmuJoyMotionState[2*joy+1];
}

uint16_t BE_ST_GetEmuJoyButtons(uint16_t joy)
{
	BE_ST_PollEvents();

	return ((g_sdlEmuJoyButtonsState >> (2*joy)) & 3);
}

int16_t BE_ST_KbHit(void)
{
	return g_sdlLastKeyScanCodeBeforeAnyReset;
}

int16_t BE_ST_BiosScanCode(int16_t command)
{
	if (command == 1)
	{
		return g_sdlLastKeyScanCodeBeforeAnyReset;
	}

	while (!g_sdlLastKeyScanCodeBeforeAnyReset)
	{
		BE_ST_ShortSleep();
	}
	int16_t result = g_sdlLastKeyScanCodeBeforeAnyReset;
	g_sdlLastKeyScanCodeBeforeAnyReset = 0;
	return result;
}

void BEL_ST_HandleEmuKeyboardEvent(bool isPressed, bool isRepeated, emulatedDOSKeyEvent keyEvent)
{
	if (keyEvent.dosScanCode == BE_ST_SC_PAUSE)
	{
		if (isPressed && g_sdlKeyboardInterruptFuncPtr)
		{
			// SPECIAL: 6 scancodes sent on key press ONLY
			g_sdlKeyboardInterruptFuncPtr(0xe1);
			g_sdlKeyboardInterruptFuncPtr(0x1d);
			g_sdlKeyboardInterruptFuncPtr(0x45);
			g_sdlKeyboardInterruptFuncPtr(0xe1);
			g_sdlKeyboardInterruptFuncPtr(0x9d);
			g_sdlKeyboardInterruptFuncPtr(0xc5);

			g_sdlEmuKeyboardLastPressedScanCode = 0; // Reset this
		}
	}
	else
	{
		if ((isPressed == g_sdlEmuKeyboardStateByScanCode[keyEvent.dosScanCode]) && !isRepeated)
			return;

		if (g_sdlKeyboardInterruptFuncPtr)
		{
			if (keyEvent.isSpecial)
			{
				g_sdlKeyboardInterruptFuncPtr(0xe0);
			}
			g_sdlKeyboardInterruptFuncPtr(keyEvent.dosScanCode | (isPressed ? 0 : 0x80));
		}
		else if (isPressed)
		{
			g_sdlLastKeyScanCodeBeforeAnyReset = keyEvent.dosScanCode;
		}

		// Key repeat emulation
		g_sdlEmuKeyboardStateByScanCode[keyEvent.dosScanCode] = isPressed;
		if (isPressed)
		{
			if (!isRepeated)
			{
				g_sdlEmuKeyboardLastPressedScanCode = keyEvent.dosScanCode;
				g_sdlEmuKeyboardLastPressedIsSpecial = keyEvent.isSpecial;
				g_sdlEmuKeyboardLastScanCodePressTime = g_sdlLastPollEventsTime;
				g_sdlEmuKeyboardLastScanCodePressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
			}
		}
		else
		{
			g_sdlEmuKeyboardLastPressedScanCode = 0; // Reset this
		}
	}
}

#ifdef BE_CROSS_ENABLE_FARPTR_CFG
uint16_t BE_ST_Compat_GetFarPtrRelocationSegOffset(void)
{
	return g_refKeenCfg.farPtrSegOffset;
}
#endif
