/* Copyright (C) 2014-2026 NY00123
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

#include "be_cross.h"
#include "be_events_sdl.h"
#include "../audio/be_audio_main_thread.h"
#include "../input/be_input.h"
#include "../input/be_input_controller_mappings.h"
#include "../input/be_input_sdl.h"
#include "../input/be_input_ui.h"
#include "../launcher/be_st_launcher.h"
#include "be_st.h"
#include "be_st_sdl_private.h"

#include "be_events_sdl_keymap.h"

#include <stdlib.h>

void BEL_ST_ConditionallyAddJoystick(SDL_JoystickID dev_id); // Implementation-specific
void BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls(void);

extern int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;
extern float g_sdlLastReportedPixelDensity;

extern bool g_sdlAudioSubsystemUp;

static void (*g_sdlAppQuitCallback)(void) = 0;

void BE_ST_SetAppQuitCallback(void (*funcPtr)(void))
{
	g_sdlAppQuitCallback = funcPtr;
}

static void BEL_ST_HandleAxisUpdateForMapping(bool isAccum, int axis, int value)
{
	// Do nothing if some on-screen keyboard is in use
	if ((g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput) ||
	    (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys))
		return;

	int side = (value < 0) ? 0 : 1;
	/* Note: We handle BOTH sides, in case value == 0,
	 * or alternatively, the sign of value changes,
	 * so "release/clear" events can be properly sent.
	 * Ensure the release always precedes the press, though. */
	BEL_ST_AltControlScheme_HandleAnyEntry(
		&g_sdlControllerMappingActualCurr->paxes[axis][1 - side],
		0,
		&g_sdlInputbindStates.paxes[axis][1 - side], isAccum);
	if (!BEL_ST_AltControlScheme_HandleAnyEntry(
		&g_sdlControllerMappingActualCurr->paxes[axis][side],
		abs(value),
		&g_sdlInputbindStates.paxes[axis][side], isAccum))
	{
		// Special case for triggers, treated like digital buttons
		if ((axis == BE_ST_CTRL_AXIS_LTRIGGER) || (axis == BE_ST_CTRL_AXIS_RTRIGGER))
			BEL_ST_AltControlScheme_HandleAnyEntry(&g_sdlControllerMappingActualCurr->defaultMapping,
			                                       value, &g_sdlDefaultMappingBinaryState, isAccum);
	}
}

static void BEL_ST_ReorientSensorData(float (*data)[6])
{
	SDL_DisplayID displayId = SDL_GetPrimaryDisplay();
	if (!displayId)
		return;
	// 0 - No change
	// 1 - 180 degrees rotation
	// 2 - 90 degrees clockwise direction
	// 3 - 90 degrees anticlockwise direction
	const int transformsTable[4][4] = {
		{0, 1, 2, 3},
		{1, 0, 3, 2},
		{3, 2, 0, 1},
		{2, 3, 1, 0},
	};
	SDL_DisplayOrientation naturalOrientation =
	                         SDL_GetNaturalDisplayOrientation(displayId),
	                       currentOrientation =
	                         SDL_GetCurrentDisplayOrientation(displayId);
	if (((int)naturalOrientation < 1) || ((int)naturalOrientation > 4) ||
	    ((int)currentOrientation < 1) || ((int)currentOrientation > 4))
		return;
	float updated[2];
	switch (transformsTable[naturalOrientation - 1][currentOrientation - 1])
	{
	case 1:
		updated[0] = -(*data)[0];
		updated[1] = -(*data)[1];
		break;
	case 2:
		updated[0] = (*data)[1];
		updated[1] = -(*data)[0];
		break;
	case 3:
		updated[0] = -(*data)[1];
		updated[1] = (*data)[0];
		break;
	default:
		return;
	}
	(*data)[0] = updated[0];
	(*data)[1] = updated[1];
}

void BE_ST_PollEvents(void)
{
	SDL_Event event;
	g_sdlLastPollEventsTime = BEL_ST_GetTicksMS();
	while (SDL_PollEvent(&event))
	{

		switch (event.type)
		{
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			SDL_Scancode scancode = event.key.scancode;
			if ((scancode < 0) || (scancode >= SDL_SCANCODE_COUNT))
				break;
			bool isPressed = (event.type == SDL_EVENT_KEY_DOWN);
			if (isPressed)
			{
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
				if (((scancode == SDL_SCANCODE_RETURN) ||
				     (scancode == SDL_SCANCODE_KP_ENTER)) &&
				    !event.key.repeat &&
				    ((event.key.mod & (SDL_KMOD_LALT|SDL_KMOD_RALT))
#ifdef REFKEEN_PLATFORM_MACOS
				     || (event.key.mod & (SDL_KMOD_LGUI|SDL_KMOD_RGUI))
#endif
				))
				{
					BE_ST_HostGfx_ToggleFullScreen();
					break;
				}
#endif
				if (event.key.repeat)
					break; // Ignore (we emulate key repeat on our own)

				if ((g_refKeenCfg.touchInputToggle == TOUCHINPUT_AUTO) && g_sdlShowTouchUI)
				{
					// Ignore a few specific scancodes on Android
					if (sdlKeyMappings[scancode].dosScanCode && (scancode != SDL_SCANCODE_SELECT) && (scancode != SDL_SCANCODE_AC_BACK))
						BEL_ST_DoHideTouchUI();
				}
			}

			// Note that this translates scancode and processes it as usual even if code is mapped.
			BEL_ST_HandleEmuKeyboardEvent(isPressed, false, sdlKeyMappings[scancode]);

			if (((int)scancode < BE_MAX_KEY_ID) &&
			    (g_sdlControllerMappingActualCurr->keys[scancode].map.mapClass != BE_ST_CTRL_MAP_NONE))
			{
				if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->keys[scancode].map,
				    g_sdlJoystickAxisMax*isPressed, &g_sdlInputbindStates.keys[scancode]))
					BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax*isPressed, &g_sdlDefaultMappingBinaryState);
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			bool isPressed = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
			if ((event.button.which == SDL_TOUCH_MOUSEID) || (event.button.button < 1))
				break;

			int bit = 0, button = event.button.button - 1;
			if (button < 3)
			{
				if (g_sdlMouseButtonsStates[button] == isPressed)
					break; // Ignore (used in case pointer is shown/hidden)
				g_sdlMouseButtonsStates[button] = isPressed;
			}

			int x = event.button.x * g_sdlLastReportedPixelDensity,
			    y = event.button.y * g_sdlLastReportedPixelDensity;
			if (isPressed)
			{
				BEL_ST_CheckForHidingTouchUI();

				if (BEL_ST_CheckCommonPointerPressCases(BE_ST_MouseTouchID, 0, x, y))
					break;
			}
			else
				if (BEL_ST_CheckCommonPointerReleaseCases(BE_ST_MouseTouchID, 0, x, y))
					break;

			if ((button < BE_ST_CTRL_MOUSE_BUT_MAX) &&
			    (g_sdlControllerMappingActualCurr->mbuttons[button].mapClass != BE_ST_CTRL_MAP_NONE))
			{
				if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->mbuttons[button],
				    g_sdlJoystickAxisMax*isPressed, &g_sdlInputbindStates.mbuttons[button]))
					BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax*isPressed, &g_sdlDefaultMappingBinaryState);
				break;
			}

			// If there's no mapping, translate button and process as usual
			if (event.button.button == SDL_BUTTON_LEFT)
				bit = 1;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				bit = 2;
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				bit = 4;

			if (isPressed)
				g_sdlEmuMouseButtonsState |= bit;
			else
				g_sdlEmuMouseButtonsState &= ~bit;
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
		{
			if (event.motion.which == SDL_TOUCH_MOUSEID)
				break;

			int x = event.motion.x * g_sdlLastReportedPixelDensity,
			    y = event.motion.y * g_sdlLastReportedPixelDensity;
			if (BEL_ST_CheckCommonPointerMoveCases(BE_ST_MouseTouchID, 0, x, y))
				break;

			if (g_sdlDoAbsMouseMotion && g_sdlControllerMappingActualCurr->absoluteFingerPositioning)
			{
				void BEL_ST_UpdateVirtualCursorPositionFromPointer(int x, int y);
				BEL_ST_UpdateVirtualCursorPositionFromPointer(x, y);
				// Update cursor shown in black bars
				extern int g_sdlHostVirtualMouseCursorState[2];
				g_sdlHostVirtualMouseCursorState[0] = x;
				g_sdlHostVirtualMouseCursorState[1] = y;
				g_sdlForceGfxControlUiRefresh = true;
			}
			else
			{
				g_sdlEmuMouseMotionAccumulatedState[0] += event.motion.xrel * g_sdlLastReportedPixelDensity;
				g_sdlEmuMouseMotionAccumulatedState[1] += event.motion.yrel * g_sdlLastReportedPixelDensity;
			}
			break;
		}

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
		case SDL_EVENT_FINGER_DOWN:
			if ((g_refKeenCfg.touchInputToggle == TOUCHINPUT_AUTO) && !g_sdlShowTouchUI)
			{
				g_sdlShowTouchUI = true;
				g_sdlForceGfxControlUiRefresh = true;
				BEL_ST_ConditionallyShowAltInputPointer();
				if (!g_sdlShowControllerUI ||
				    (
				      (g_sdlControllerMappingActualCurr != &g_beStControllerMappingTextInput)
				      && (g_sdlControllerMappingActualCurr != &g_beStControllerMappingDebugKeys)
				      && !(g_sdlControllerMappingActualCurr->showUi)
				    )
				)
					BEL_ST_AltControlScheme_ClearBinaryStates();
				// Do take care of mouse cursor if there's a need to
				if (g_sdlControllerMappingActualCurr->absoluteFingerPositioning)
					BEL_ST_CheckPressedPointerInTouchControls(event.tfinger.touchID, event.tfinger.fingerID, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, true);
				// Let's not trigger an immediate action when on-screen controls are shown, *if* there are any, which were *not* previously shown
				if (g_sdlControllerMappingActualCurr->touchMappings || (!g_sdlShowControllerUI && g_sdlControllerMappingActualCurr->showUi))
					break;
			}

			BEL_ST_CheckCommonPointerPressCases(event.tfinger.touchID, event.tfinger.fingerID, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
			break;
		case SDL_EVENT_FINGER_UP:
			BEL_ST_CheckCommonPointerReleaseCases(event.tfinger.touchID, event.tfinger.fingerID, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
			break;
		case SDL_EVENT_FINGER_MOTION:
			BEL_ST_CheckCommonPointerMoveCases(event.tfinger.touchID, event.tfinger.fingerID, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
			break;
#endif // REFKEEN_CONFIG_ENABLE_TOUCHINPUT

		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
			for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			{
				if (g_sdlJoysticks[i] && (g_sdlJoysticksInstanceIds[i] == event.jaxis.which))
				{
					if (event.jaxis.value >= 0)
						g_sdlEmuJoyMotionState[(event.jaxis.axis + 2*i) % 4] = event.jaxis.value*(BE_ST_EMU_JOYSTICK_RANGEMAX-BE_ST_EMU_JOYSTICK_RANGECENTER)/32767 + BE_ST_EMU_JOYSTICK_RANGECENTER;
					else
						g_sdlEmuJoyMotionState[(event.jaxis.axis + 2*i) % 4] = (event.jaxis.value+32768)*(BE_ST_EMU_JOYSTICK_RANGECENTER-BE_ST_EMU_JOYSTICK_RANGEMIN)/32768 + BE_ST_EMU_JOYSTICK_RANGEMIN;
					break;
				}
			}
			break;
		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			BEL_ST_CheckForHidingTouchUI();
			// Fall-through
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
			for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			{
				if (g_sdlJoysticks[i] && (g_sdlJoysticksInstanceIds[i] == event.jaxis.which))
				{
					int mask = (1 << ((event.jbutton.button + 2*i) % 4));
					if (event.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN)
						g_sdlEmuJoyButtonsState |= mask;
					else
						g_sdlEmuJoyButtonsState &= ~mask;
					break;
				}
			}
			break;

		/* Don't use SDL_EVENT_GAMEPAD_ADDED with alternative controller schemes, and for the sake of consistency avoid SDL_EVENT_GAMEPAD_REMOVED as well.
		 * Reason is that on init, there is a problem handling controller mappings loaded from the database using SDL_EVENT_GAMEPAD_ADDED
		 * (if loaded before init, the mappings seem to be deleted, otherwise SDL_EVENT_GAMEPAD_ADDED is just not spawned for these).
		 */
		case SDL_EVENT_JOYSTICK_ADDED:
			BEL_ST_ConditionallyAddJoystick(event.jdevice.which);
			break;
		case SDL_EVENT_JOYSTICK_REMOVED:
			if (!g_refKeenCfg.altControlScheme)
			{
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
				{
					if (g_sdlJoysticks[i] && (g_sdlJoysticksInstanceIds[i] == event.jdevice.which))
					{
						SDL_CloseJoystick(g_sdlJoysticks[i]);
						BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BE_ST_PollEvents: Closed joystick %u\n", event.jdevice.which);
						g_sdlJoysticks[i] = NULL;
					}
				}
			}
			else
			{
				bool isAnyControllerStillConnected = false;
				bool isAnyControllerDisconnected = false;
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
				{
					if (g_sdlControllers[i])
					{
						if (g_sdlJoysticksInstanceIds[i] == event.jdevice.which)
						{
							SDL_CloseGamepad(g_sdlControllers[i]);
							BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BE_ST_PollEvents: Closed gamepad %u\n", event.jdevice.which);
							g_sdlControllers[i] = NULL;
							isAnyControllerDisconnected = true;
						}
						else
						{
							isAnyControllerStillConnected = true;
						}
					}
				}

				if (isAnyControllerDisconnected)
					BEL_ST_AltControlScheme_ClearBinaryStates();

				if (!isAnyControllerStillConnected)
				{
					g_sdlShowControllerUI = false;
					g_sdlForceGfxControlUiRefresh = true;
					BEL_ST_ConditionallyShowAltInputPointer();
				}
			}
			break;

		case SDL_EVENT_SENSOR_UPDATE:
		case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
		{
			bool isGamepad = (event.type == SDL_EVENT_GAMEPAD_SENSOR_UPDATE);
			int32_t sensorType = isGamepad ? event.gsensor.sensor :
			                     SDL_GetSensorTypeForID(event.sensor.which);
			float *data = isGamepad ? event.gsensor.data : event.sensor.data;
			switch (sensorType)
			{
			case SDL_SENSOR_ACCEL:
			case SDL_SENSOR_ACCEL_L:
			case SDL_SENSOR_ACCEL_R:
			case SDL_SENSOR_GYRO:
			case SDL_SENSOR_GYRO_L:
			case SDL_SENSOR_GYRO_R:
			{
				const bool isGyro = (sensorType == SDL_SENSOR_GYRO) ||
				                    (sensorType == SDL_SENSOR_GYRO_L) ||
				                    (sensorType == SDL_SENSOR_GYRO_R);
				const int axisX =
				        3 * (sensorType - SDL_SENSOR_ACCEL) +
				        BE_ST_CTRL_FULL_AXIS_ACCEL_X;
				for (int i = 0; i < 3; ++i)
				{
					if (!isGyro)
						data[i] /= SDL_STANDARD_GRAVITY;
#if 0
					// Add more to the usual deadzone used
					// in BEL_ST_AltControlScheme_HandleEntry
					if (fabs(data[i]) <= 0.25f)
						data[i] = 0;
					else if (data[i] < 0.f)
						data[i] = (data[i]+0.25f)/0.75f;
					else
						data[i] = (data[i]-0.25f)/0.75f;
#endif
				}
				if (!isGamepad)
					BEL_ST_ReorientSensorData(&event.sensor.data);

				BEL_ST_HandleAxisUpdateForMapping(isGyro, axisX,
				                                  g_sdlJoystickAxisMax * data[0]);
				BEL_ST_HandleAxisUpdateForMapping(isGyro, axisX + 1,
				                                  g_sdlJoystickAxisMax * data[1]);
				BEL_ST_HandleAxisUpdateForMapping(isGyro, axisX + 2,
				                                  g_sdlJoystickAxisMax * data[2]);
				break;
			}
			default:
				break;
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			if ((event.gaxis.axis < 0) ||
			    (event.gaxis.axis >= BE_ST_CTRL_AXIS_MAX))
				break;

			BEL_ST_HandleAxisUpdateForMapping(false, event.gaxis.axis,
			                                  event.gaxis.value);
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			BEL_ST_CheckForHidingTouchUI();
			// Fall-through
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			bool isPressed = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
			int but = event.gbutton.button;
			if ((but < 0) || (but >= BE_ST_CTRL_BUT_MAX))
				break;

			if (g_refKeenCfg.swapConfirmCancel &&
			    !g_sdlControllerMappingActualCurr->ignoreConfirmCancelSwap)
				but = BE_ST_AltControlScheme_SwapPadButton(but);

			// Special handling for text input / debug keys
			if ((g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput) || (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys))
			{
				// Usually done from BEL_ST_AltControlScheme_HandleEntry
				if (isPressed == g_sdlInputbindStates.pbuttons[but])
					break;
				g_sdlInputbindStates.pbuttons[but] = isPressed;

				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
					BEL_ST_AltControlScheme_HandleTextInputEvent(but, isPressed);
				else
					BEL_ST_AltControlScheme_HandleDebugKeysEvent(but, isPressed);

				if (isPressed)
				{
					switch (but)
					{
					case BE_ST_CTRL_BUT_DPAD_UP:
					case BE_ST_CTRL_BUT_DPAD_DOWN:
					case BE_ST_CTRL_BUT_DPAD_LEFT:
					case BE_ST_CTRL_BUT_DPAD_RIGHT:
						g_sdlOnScreenKeyboardLastPressedDirButton = but;
						g_sdlOnScreenKeyboardLastDirButtonPressTime = g_sdlLastPollEventsTime;
						g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay = BE_ST_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
						break;
					default:
						g_sdlOnScreenKeyboardLastPressedDirButton = BE_ST_CTRL_BUT_INVALID;
					}
				}
				else
				{
					if (but == g_sdlOnScreenKeyboardLastPressedDirButton)
						g_sdlOnScreenKeyboardLastPressedDirButton = BE_ST_CTRL_BUT_INVALID;
				}
			}
			// Try the usual otherwise (similar, but not identical, handling done with analog axes, triggers included)
			else if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->pbuttons[but], g_sdlJoystickAxisMax*isPressed, &g_sdlInputbindStates.pbuttons[but]))
			{
				BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax*isPressed, &g_sdlDefaultMappingBinaryState);
			}
			break;
		}

		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			BEL_ST_SetGfxOutputRects(false);
			// Fall-through
		case SDL_EVENT_WINDOW_EXPOSED:
			BEL_ST_ForceHostDisplayUpdate();
			break;
		case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
			BEL_ST_HandleFullScreenChange(true);
			break;
		case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
			BEL_ST_HandleFullScreenChange(false);
			break;

		case SDL_EVENT_RENDER_TARGETS_RESET:
		case SDL_EVENT_RENDER_DEVICE_RESET:
			BEL_ST_DestroyAllTextures();
			BEL_ST_RecreateMainTextures();
			BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls();
			break;

		case SDL_EVENT_QUIT:
			if (g_sdlAppQuitCallback)
				g_sdlAppQuitCallback();
			BE_ST_QuickExit();
			break;
		default: ;
		}

	}

#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK
	BEL_ST_CheckForExitFromEventsCallback();
#endif

	// Key repeat emulation
	if (g_sdlEmuKeyboardLastPressedScanCode && (g_sdlLastPollEventsTime - g_sdlEmuKeyboardLastScanCodePressTime >= g_sdlEmuKeyboardLastScanCodePressTimeDelay))
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = g_sdlEmuKeyboardLastPressedIsSpecial;
		dosKeyEvent.dosScanCode = g_sdlEmuKeyboardLastPressedScanCode;

		BEL_ST_HandleEmuKeyboardEvent(true, true, dosKeyEvent);

		g_sdlEmuKeyboardLastScanCodePressTime += g_sdlEmuKeyboardLastScanCodePressTimeDelay;
		g_sdlEmuKeyboardLastScanCodePressTimeDelay = BE_ST_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;
	}

	// Similar repeat for on-screen keyboard (directional movement only)
	if ((g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput) || (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys))
	{
		if ((g_sdlOnScreenKeyboardLastPressedDirButton != BE_ST_CTRL_BUT_INVALID) && (g_sdlLastPollEventsTime - g_sdlOnScreenKeyboardLastDirButtonPressTime >= g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay))
		{
			if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
				BEL_ST_AltControlScheme_HandleTextInputEvent(g_sdlOnScreenKeyboardLastPressedDirButton, true);
			else
				BEL_ST_AltControlScheme_HandleDebugKeysEvent(g_sdlOnScreenKeyboardLastPressedDirButton, true);

			g_sdlOnScreenKeyboardLastDirButtonPressTime += g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay;
			g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay = BE_ST_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;
		
		}
	}

#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
	BE_ST_PrepareForManualAudioCallbackCall();
#else
	// HACK - If audio subsystem is disabled we still want to at least
	// make the sound callback run (so e.g., no loop gets stuck waiting
	// for sound playback to complete)
	if (!g_sdlAudioSubsystemUp)
		BE_ST_PrepareForManualAudioCallbackCall();
#endif
}

#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK

SDL_Semaphore *g_sdlEventsCallbackToMainSem, *g_sdlMainToEventsCallbackSem;

void BEL_ST_SaveConfigFiles(void);

void BEL_ST_CheckForExitFromEventsCallback(void)
{
	if (SDL_TryWaitSemaphore(g_sdlEventsCallbackToMainSem))
	{
		BEL_ST_SaveConfigFiles(); // From BE_ST_QuickExit
		SDL_SignalSemaphore(g_sdlMainToEventsCallbackSem);
		SDL_WaitSemaphore(g_sdlEventsCallbackToMainSem); // Wait here "forever"
	}
}

// Use this to catch a few special events here when required
static uint32_t g_sdl_eventCallback_EnterBackgroundLastTicks;
static bool g_sdl_eventCallback_destroyedTextures = false;

bool BEL_ST_EventsCallback(void *userdata, SDL_Event *event)
{
	extern SDL_AudioStream *g_sdlAudioStream;
	extern uint32_t g_be_audioMainThread_lastCallTicks;

	switch (event->type)
	{
	case SDL_EVENT_TERMINATING:
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Callback SDL_EVENT_TERMINATING\n");
		SDL_SignalSemaphore(g_sdlEventsCallbackToMainSem);
		SDL_WaitSemaphore(g_sdlMainToEventsCallbackSem);
		if (event->type != SDL_EVENT_TERMINATING)
			exit(0);
		return false;
	case SDL_EVENT_LOW_MEMORY:
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Callback SDL_EVENT_LOW_MEMORY\n");
		g_sdl_eventCallback_destroyedTextures = true;
		BEL_ST_DestroyAllTextures();
		return false;
	case SDL_EVENT_WILL_ENTER_BACKGROUND:
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Callback SDL_EVENT_WILL_ENTER_BACKGROUND\n");
		if (g_sdlAudioSubsystemUp) // FIXME - Hope this works well
			SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(g_sdlAudioStream));
		return false;
	case SDL_EVENT_DID_ENTER_BACKGROUND:
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Callback SDL_EVENT_DID_ENTER_BACKGROUND\n");
		g_sdl_eventCallback_EnterBackgroundLastTicks = BEL_ST_GetTicksMS();
		return false;
	case SDL_EVENT_WILL_ENTER_FOREGROUND:
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Callback SDL_EVENT_WILL_ENTER_FOREGROUND\n");
		if (g_sdl_eventCallback_destroyedTextures)
		{
			// Force recreation of textures from main thread
			SDL_Event event = {0};
			event.type = SDL_EVENT_RENDER_DEVICE_RESET;
			// We have just one window to which access was
			// restricted to one compilation unit at the time.
			//event.render.windowID = g_sdlWindow;
			SDL_PushEvent(&event);
			g_sdl_eventCallback_destroyedTextures = false;
		}
		// FIXME!!! - Hope this works well
		g_be_audioMainThread_lastCallTicks += (BEL_ST_GetTicksMS() - g_sdl_eventCallback_EnterBackgroundLastTicks);
		return false;
	case SDL_EVENT_DID_ENTER_FOREGROUND:
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Callback SDL_EVENT_DID_ENTER_FOREGROUND\n");
		// HACK - These may be done from a different thread,
		// but should be relatively simple anyway
		BEL_ST_ForceHostDisplayUpdate();
#ifdef REFKEEN_ENABLE_LAUNCHER
		BE_ST_Launcher_MarkGfxCache();
#endif
		if (g_sdlAudioSubsystemUp) // FIXME - Hope this works well
			SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(g_sdlAudioStream));
		return false;
	default:
		return true; // Just send to SDL_PollEvent as usual
	}
}

#endif // REFKEEN_CONFIG_EVENTS_CALLBACK
