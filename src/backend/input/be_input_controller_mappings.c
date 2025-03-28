/* Copyright (C) 2014-2025 NY00123
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

#include <string.h>

#include "be_input.h"
#include "be_input_controller_mappings.h"
#include "../video/be_video_ui.h"

#define NUM_OF_CONTROLLER_MAPS_IN_STACK 8

static bool g_sdlControllerSchemeNeedsCleanUp;

static BE_ST_ControllerMapping g_sdlControllerMappingDefault;

static struct {
	const BE_ST_ControllerMapping *stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];
	const BE_ST_ControllerMapping **currPtr;
	const BE_ST_ControllerMapping **endPtr;
} g_sdlControllerMappingPtrsStack;

// Current mapping, doesn't have to be *(g_sdlControllerMappingPtrsStack.currPtr) as game code can change this (e.g., helper keys)
const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr = &g_sdlControllerMappingDefault;

// HACK - These "mappings" are used for identification of on-screen keyboards (using pointers comparisons)
BE_ST_ControllerMapping g_beStControllerMappingTextInput;
BE_ST_ControllerMapping g_beStControllerMappingDebugKeys;


static void BEL_ST_AltControllerScheme_ToggleIndicators(bool enabled)
{
	for (int key = 0; key < BE_MAX_KEY_ID; ++key)
		if (g_sdlControllerMappingActualCurr->keys[key].indicator)
			*g_sdlControllerMappingActualCurr->keys[key].indicator = enabled;
}

static void BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls(void);

/* May be similar to PrepareControllerMapping, but a bit different:
   Used in order to replace controller mapping with another one internally
   (e.g., showing helper function keys during gameplay, or hiding such keys).

   Keyboard and mouse overrides, however, are not replaced. Unless it was
   the first mapping preceding a sequence of calls to this function, this
   technically modifies its overrides, so it's marked to not be used in
   any other manner later. */
static void BEL_ST_ReplaceControllerMapping(BE_ST_ControllerMapping *mapping)
{
	BEL_ST_AltControlScheme_CleanUp();

	memcpy(mapping->keys, g_sdlControllerMappingActualCurr->keys, sizeof(mapping->keys));
	memcpy(mapping->mbuttons, g_sdlControllerMappingActualCurr->mbuttons, sizeof(mapping->mbuttons));

	// We generally do want g_sdlControllerMappingActualCurr to be const...
	if (!g_sdlControllerMappingActualCurr->parent)
		((BE_ST_ControllerMapping *)g_sdlControllerMappingActualCurr)->parent = g_sdlControllerMappingActualCurr;
	if (!mapping->parent)
		mapping->parent = g_sdlControllerMappingActualCurr->parent;

	if (g_sdlControllerMappingActualCurr->parent != mapping->parent)
		BE_ST_ExitWithErrorMsg("BEL_ST_ReplaceControllerMapping: Mappings share different parents!\n");

	g_sdlControllerMappingActualCurr = mapping;
	BEL_ST_AltControllerScheme_ToggleIndicators(true);

	BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

bool BEL_ST_AltControlScheme_HandleEntry(const BE_ST_ControllerSingleMap *map, int value, bool *lastBinaryStatusPtr)
{
	bool prevBinaryStatus = *lastBinaryStatusPtr;
	*lastBinaryStatusPtr = (value >= g_sdlJoystickAxisBinaryThreshold);
	switch (map->mapClass)
	{
	case BE_ST_CTRL_MAP_KEYSCANCODE:
	{
		if (*lastBinaryStatusPtr != prevBinaryStatus)
		{
			emulatedDOSKeyEvent dosKeyEvent;
			dosKeyEvent.isSpecial = false;
			dosKeyEvent.dosScanCode = map->val;
			BEL_ST_HandleEmuKeyboardEvent(*lastBinaryStatusPtr, false, dosKeyEvent);
		}
		return true;
	}
	case BE_ST_CTRL_MAP_MOUSEBUTTON:
		if (*lastBinaryStatusPtr != prevBinaryStatus)
		{
			if (*lastBinaryStatusPtr)
				g_sdlEmuMouseButtonsState |= map->val;
			else
				g_sdlEmuMouseButtonsState &= ~map->val;
		}
		return true;
	case BE_ST_CTRL_MAP_MOUSEMOTION:
		g_sdlEmuMouseMotionFromJoystick[map->val] = (value <= g_sdlJoystickAxisDeadZone) ? 0 : (value - g_sdlJoystickAxisDeadZone) * map->secondaryVal / g_sdlJoystickAxisMaxMinusDeadZone;
		return true;
	case BE_ST_CTRL_MAP_OTHERMAPPING:
		if (!prevBinaryStatus && (*lastBinaryStatusPtr))
			BEL_ST_ReplaceControllerMapping((BE_ST_ControllerMapping *)map->miscPtr);
		return true; // Confirm either way
	case BE_ST_CTRL_MAP_VALUESET:
		*(int *)(map->miscPtr) = (value <= g_sdlJoystickAxisDeadZone) ? 0 : (value - g_sdlJoystickAxisDeadZone) * map->secondaryVal / g_sdlJoystickAxisMaxMinusDeadZone;
		return true;
	}
	return false;
}


void BEL_ST_ReleasePressedKeysInTextInputUI(void);
void BEL_ST_ReleasePressedKeysInDebugKeysUI(void);

/* WARNING: In theory there may be a Clear -> HandleEntry -> Clear cycle,
 * but it can never occur since isPressed is set to false
 */
void BEL_ST_AltControlScheme_ClearBinaryStates(void)
{
	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		BEL_ST_ReleasePressedKeysInTextInputUI();
	}
	else if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		BEL_ST_ReleasePressedKeysInDebugKeysUI();
	}
	else // Otherwise simulate key releases based on the mapping
	{
		// But also don't forget this (e.g., if mouse is used)
		if (g_sdlControllerMappingActualCurr->showUi)
		{
			void BEL_ST_ReleasePressedKeysInControllerUI(void);
			BEL_ST_ReleasePressedKeysInControllerUI();
		}
		else
		{
			void BEL_ST_ReleasePressedButtonsInTouchControls(void);
			BEL_ST_ReleasePressedButtonsInTouchControls();
		}
		// Simulate binary key/button/other action "releases" and clear button states.
		// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than an unexpected key press.
		for (int key = 0; key < BE_MAX_KEY_ID; ++key)
			if (g_sdlControllerMappingActualCurr->keys[key].map.mapClass != BE_ST_CTRL_MAP_NONE)
				BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->keys[key].map, 0, &g_sdlInputbindStates.keys[key]);
		for (int but = 0; but < BE_ST_CTRL_MOUSE_BUT_MAX; ++but)
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->mbuttons[but], 0, &g_sdlInputbindStates.mbuttons[but]);
		for (int but = 0; but < BE_ST_CTRL_BUT_MAX; ++but)
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->pbuttons[but], 0, &g_sdlInputbindStates.pbuttons[but]);
		// Repeat with analog axes
		for (int axis = 0; axis < BE_ST_CTRL_AXIS_MAX; ++axis)
		{
			// Is pressed in the negative direction?
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->paxes[axis][0], 0, &g_sdlInputbindStates.paxes[axis][0]);
			// Repeat with positive
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->paxes[axis][1], 0, &g_sdlInputbindStates.paxes[axis][1]);
		}
	}

	// Check this for ALL possible mappings
	if (g_sdlDefaultMappingBinaryState)
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, 0, &g_sdlDefaultMappingBinaryState);

	memset(&g_sdlInputbindStates, 0, sizeof(g_sdlInputbindStates));
}

void BEL_ST_AltControlScheme_CleanUp(void)
{
	if (!g_sdlControllerSchemeNeedsCleanUp)
		return;

	BEL_ST_AltControlScheme_ClearBinaryStates();
	BEL_ST_AltControllerScheme_ToggleIndicators(false);

	extern void BEL_ST_HideAltInputUI(void);
	BEL_ST_HideAltInputUI();

	g_sdlControllerSchemeNeedsCleanUp = false;
}


static void BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls(void)
{
	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		extern void BEL_ST_PrepareToShowTextInputUI(void);
		BEL_ST_PrepareToShowTextInputUI();
		g_sdlOnScreenKeyboardLastPressedDirButton = BE_ST_CTRL_BUT_INVALID;
	}
	else if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		extern void BEL_ST_PrepareToShowDebugKeysUI(void);
		BEL_ST_PrepareToShowDebugKeysUI();
		g_sdlOnScreenKeyboardLastPressedDirButton = BE_ST_CTRL_BUT_INVALID;
	}
	else if (g_sdlControllerMappingActualCurr->showUi)
	{
		extern void BEL_ST_PrepareToShowControllerUI(const BE_ST_ControllerMapping *mapping);
		BEL_ST_PrepareToShowControllerUI(g_sdlControllerMappingActualCurr);
	}
	else if ((g_refKeenCfg.touchInputToggle != TOUCHINPUT_OFF) && g_sdlControllerMappingActualCurr->touchMappings)
	{
		extern void BEL_ST_PrepareToShowTouchControls(const BE_ST_ControllerMapping *mapping);
		BEL_ST_PrepareToShowTouchControls(g_sdlControllerMappingActualCurr);
	}

	BEL_ST_ConditionallyShowAltInputPointer();
}


static void BEL_ST_AltControlScheme_AbortIfHasParent(const char *msg)
{
	if (g_sdlControllerMappingActualCurr->parent &&
	    g_sdlControllerMappingActualCurr->parent != g_sdlControllerMappingActualCurr)
		BE_ST_ExitWithErrorMsg(msg);
}

void BE_ST_AltControlScheme_Push(void)
{
	//if (!g_refKeenCfg.altControlScheme && (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF))
	//	return;

	BEL_ST_AltControlScheme_CleanUp();

	++g_sdlControllerMappingPtrsStack.currPtr;
	if (g_sdlControllerMappingPtrsStack.currPtr == g_sdlControllerMappingPtrsStack.endPtr)
		BE_ST_ExitWithErrorMsg("BE_ST_AltControlScheme_Push: Out of stack bounds!\n");
}

void BE_ST_AltControlScheme_Pop(void)
{
	//if (!g_refKeenCfg.altControlScheme && (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF))
	//	return;

	BEL_ST_AltControlScheme_CleanUp();

	if (g_sdlControllerMappingPtrsStack.currPtr == &g_sdlControllerMappingPtrsStack.stack[0])
		BE_ST_ExitWithErrorMsg("BE_ST_AltControlScheme_Pop: Popped more than necessary!\n");
	--g_sdlControllerMappingPtrsStack.currPtr;

	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr;
	BEL_ST_AltControlScheme_AbortIfHasParent("BE_ST_AltControlScheme_Pop: Popped a mapping with a parent!\n");
	BEL_ST_AltControllerScheme_ToggleIndicators(true);

	BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_Reset(void)
{
	BEL_ST_AltControlScheme_CleanUp();

	g_sdlControllerMappingPtrsStack.stack[0] = &g_sdlControllerMappingDefault;
	g_sdlControllerMappingPtrsStack.currPtr = &g_sdlControllerMappingPtrsStack.stack[0];
	g_sdlControllerMappingPtrsStack.endPtr = &g_sdlControllerMappingPtrsStack.stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];
	g_sdlControllerMappingActualCurr = g_sdlControllerMappingPtrsStack.stack[0];
	BEL_ST_AltControlScheme_AbortIfHasParent("BE_ST_AltControlScheme_Reset: Reset to a mapping with a parent!\n");
	BEL_ST_AltControllerScheme_ToggleIndicators(true);

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_PrepareControllerMapping(const BE_ST_ControllerMapping *mapping)
{
	//if (!g_refKeenCfg.altControlScheme && (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF))
	//	return;

	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr = mapping;
	BEL_ST_AltControlScheme_AbortIfHasParent("BE_ST_AltControlScheme_PrepareControllerMapping: Set a mapping with a parent!\n");
	BEL_ST_AltControllerScheme_ToggleIndicators(true);

	BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_UpdateVirtualMouseCursor(int x, int y)
{
	g_sdlVirtualMouseCursorState[0] = x;
	g_sdlVirtualMouseCursorState[1] = y;
}


/*** A couple of special handlers call from BE_ST_PollEvents ***/

void BEL_ST_AltControlScheme_HandleTextInputEvent(int but, bool isPressed)
{
	extern void BEL_ST_MoveUpInTextInputUI(void);
	extern void BEL_ST_MoveDownInTextInputUI(void);
	extern void BEL_ST_MoveLeftInTextInputUI(void);
	extern void BEL_ST_MoveRightInTextInputUI(void);
	extern void BEL_ST_ToggleShiftStateInTextInputUI(void);
	extern void BEL_ST_ToggleKeyPressInTextInputUI(bool toggle);
	switch (but)
	{
	case BE_ST_CTRL_BUT_DPAD_UP:
		if (isPressed)
			BEL_ST_MoveUpInTextInputUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_DOWN:
		if (isPressed)
			BEL_ST_MoveDownInTextInputUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_LEFT:
		if (isPressed)
			BEL_ST_MoveLeftInTextInputUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_MoveRightInTextInputUI();
		break;
	// A few other special cases
	case BE_ST_CTRL_BUT_START:
		if (isPressed) // No need for !isPressed case since pause key send scancodes on release only (under DOS)
		{
			emulatedDOSKeyEvent dosKeyEvent;
			dosKeyEvent.isSpecial = false;
			dosKeyEvent.dosScanCode = BE_ST_SC_PAUSE;
			BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
		}
		break;
	case BE_ST_CTRL_BUT_B:
	case BE_ST_CTRL_BUT_BACK:
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax*isPressed, &g_sdlDefaultMappingBinaryState);
		break;
	case BE_ST_CTRL_BUT_X:
		// Change shift state (or at least try to).
		if (isPressed)
			BEL_ST_ToggleShiftStateInTextInputUI();
		break;
	default:
		// Select key from UI.
		BEL_ST_ToggleKeyPressInTextInputUI(isPressed);
	}
}

void BEL_ST_AltControlScheme_HandleDebugKeysEvent(int but, bool isPressed)
{
	extern void BEL_ST_MoveUpInDebugKeysUI(void);
	extern void BEL_ST_MoveDownInDebugKeysUI(void);
	extern void BEL_ST_MoveLeftInDebugKeysUI(void);
	extern void BEL_ST_MoveRightInDebugKeysUI(void);
	extern void BEL_ST_ToggleKeyPressInDebugKeysUI(void);
	switch (but)
	{
	case BE_ST_CTRL_BUT_DPAD_UP:
		if (isPressed)
			BEL_ST_MoveUpInDebugKeysUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_DOWN:
		if (isPressed)
			BEL_ST_MoveDownInDebugKeysUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_LEFT:
		if (isPressed)
			BEL_ST_MoveLeftInDebugKeysUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_MoveRightInDebugKeysUI();
		break;
	// A few other special cases
	case BE_ST_CTRL_BUT_START:
		if (isPressed) // No need for !isPressed case since pause key send scancodes on release only (under DOS)
		{
			emulatedDOSKeyEvent dosKeyEvent;
			dosKeyEvent.isSpecial = false;
			dosKeyEvent.dosScanCode = BE_ST_SC_PAUSE;
			BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
		}
		break;
	case BE_ST_CTRL_BUT_B:
	case BE_ST_CTRL_BUT_BACK:
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax*isPressed, &g_sdlDefaultMappingBinaryState);
		return;
	default:
		// Select or deselect key from UI, IF actual button is pressed.
		if (isPressed)
			BEL_ST_ToggleKeyPressInDebugKeysUI();
	}
}
