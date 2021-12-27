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

void BEL_ST_ConditionallyAddJoystick(int device_index); // Implementation-specific

extern int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;

extern bool g_sdlAudioSubsystemUp;

static void (*g_sdlAppQuitCallback)(void) = 0;

void BE_ST_SetAppQuitCallback(void (*funcPtr)(void))
{
	g_sdlAppQuitCallback = funcPtr;
}


void BE_ST_PollEvents(void)
{
	SDL_Event event;
	g_sdlLastPollEventsTime = BEL_ST_GetTicksMS();
	while (SDL_PollEvent(&event))
	{

		switch (event.type)
		{
		case SDL_KEYDOWN:
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
			if (((event.key.keysym.scancode == SDL_SCANCODE_RETURN) ||
			     (event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)) &&
			    !event.key.repeat &&
			    ((event.key.keysym.mod & (KMOD_LALT|KMOD_RALT))
#ifdef REFKEEN_PLATFORM_MACOS
			     || (event.key.keysym.mod & (KMOD_LGUI|KMOD_RGUI))
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
				if (sdlKeyMappings[event.key.keysym.scancode].dosScanCode && (event.key.keysym.scancode != SDL_SCANCODE_SELECT) && (event.key.keysym.scancode != SDL_SCANCODE_AC_BACK))
					BEL_ST_DoHideTouchUI();
			}
			// Fall-through
		case SDL_KEYUP:
		{
			bool isPressed = (event.type == SDL_KEYDOWN);
			SDL_Scancode scancode = event.key.keysym.scancode;
			if (scancode >= SDL_NUM_SCANCODES)
				break;

			// If there's no mapping, translate scancode and process as usual
			if (g_sdlControllerMappingActualCurr->keys[scancode].mapClass == BE_ST_CTRL_MAP_NONE)
			{
				BEL_ST_HandleEmuKeyboardEvent(isPressed, false, sdlKeyMappings[scancode]);
				break;
			}

			// Special handling for text input / debug keys
			if ((g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput) || (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys))
				break; // FIXME: This currently does nothing

			if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->keys[scancode],
			    g_sdlJoystickAxisMax*isPressed, &g_sdlInputbindStates.keys[scancode]))
				BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax*isPressed, &g_sdlDefaultMappingBinaryState);
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			bool isPressed = (event.type == SDL_MOUSEBUTTONDOWN);
			int bit = 0;
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if ((event.button.button >= 1) && (event.button.button <= 3))
			{
				if (g_sdlMouseButtonsStates[event.button.button-1] == isPressed)
					break; // Ignore (used in case pointer is shown/hidden)
				g_sdlMouseButtonsStates[event.button.button-1] = isPressed;
			}

			if (isPressed)
			{
				BEL_ST_CheckForHidingTouchUI();

				if (BEL_ST_CheckCommonPointerPressCases(BE_ST_MouseTouchID, 0, event.button.x, event.button.y))
					break;
			}
			else
				if (BEL_ST_CheckCommonPointerReleaseCases(BE_ST_MouseTouchID, 0, event.button.x, event.button.y))
					break;

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

		case SDL_MOUSEMOTION:
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if (BEL_ST_CheckCommonPointerMoveCases(BE_ST_MouseTouchID, 0, event.motion.x, event.motion.y))
				break;

			if (g_sdlDoAbsMouseMotion && g_sdlControllerMappingActualCurr->absoluteFingerPositioning)
			{
				void BEL_ST_UpdateVirtualCursorPositionFromPointer(int x, int y);
				BEL_ST_UpdateVirtualCursorPositionFromPointer(event.motion.x, event.motion.y);
				// Update cursor shown in black bars
				extern int g_sdlHostVirtualMouseCursorState[2];
				g_sdlHostVirtualMouseCursorState[0] = event.motion.x;
				g_sdlHostVirtualMouseCursorState[1] = event.motion.y;
				g_sdlForceGfxControlUiRefresh = true;
			}
			else
			{
				g_sdlEmuMouseMotionAccumulatedState[0] += event.motion.xrel;
				g_sdlEmuMouseMotionAccumulatedState[1] += event.motion.yrel;
			}
			break;

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
		case SDL_FINGERDOWN:
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
					BEL_ST_CheckPressedPointerInTouchControls(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, true);
				// Let's not trigger an immediate action when on-screen controls are shown, *if* there are any, which were *not* previously shown
				if (g_sdlControllerMappingActualCurr->touchMappings || (!g_sdlShowControllerUI && g_sdlControllerMappingActualCurr->showUi))
					break;
			}

			BEL_ST_CheckCommonPointerPressCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
			break;
		case SDL_FINGERUP:
			BEL_ST_CheckCommonPointerReleaseCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
			break;
		case SDL_FINGERMOTION:
			BEL_ST_CheckCommonPointerMoveCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
			break;
#endif // REFKEEN_CONFIG_ENABLE_TOUCHINPUT

		case SDL_JOYAXISMOTION:
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
		case SDL_JOYBUTTONDOWN:
			BEL_ST_CheckForHidingTouchUI();
			// Fall-through
		case SDL_JOYBUTTONUP:
			for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			{
				if (g_sdlJoysticks[i] && (g_sdlJoysticksInstanceIds[i] == event.jaxis.which))
				{
					int mask = (1 << ((event.jbutton.button + 2*i) % 4));
					if (event.type == SDL_JOYBUTTONDOWN)
						g_sdlEmuJoyButtonsState |= mask;
					else
						g_sdlEmuJoyButtonsState &= ~mask;
					break;
				}
			}
			break;

		/* Don't use SDL_CONTROLLERDEVICEADDED with alternative controller schemes, and for the sake of consistency avoid SDL_CONTROLLERDEVICEREMOVED as well.
		 * Reason is that on init, there is a problem handling controller mappings loaded from the database using SDL_CONTROLLERDEVICEADDED
		 * (if loaded before init, the mappings seem to be deleted, otherwise SDL_CONTROLLERDEVICEADDED is just not spawned for these).
		 */
		case SDL_JOYDEVICEADDED:
			BEL_ST_ConditionallyAddJoystick(event.jdevice.which);
			break;
		case SDL_JOYDEVICEREMOVED:
			if (!g_refKeenCfg.altControlScheme)
			{
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
				{
					if (g_sdlJoysticks[i] && (g_sdlJoysticksInstanceIds[i] == event.jdevice.which))
					{
						SDL_JoystickClose(g_sdlJoysticks[i]);
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
							SDL_GameControllerClose(g_sdlControllers[i]);
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

		case SDL_CONTROLLERAXISMOTION:
		{
			// Do nothing if some on-screen keyboard is in use
			if ((g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput) || (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys))
				break;

			int axis = event.caxis.axis;
			int axisVal = event.caxis.value;
			int side = (axisVal < 0) ? 0 : 1;
			if ((axis < 0) || (axis >= BE_ST_CTRL_AXIS_MAX))
				break;
			/* Note: We handle BOTH sides, in case axisVal == 0,
			 * or alternatively, the sign of axisVal changes, so
			 * "release/clear" events can be properly sent.
			 * Ensure the release always precedes the press, though. */
			BEL_ST_AltControlScheme_HandleEntry(
				&g_sdlControllerMappingActualCurr->paxes[axis][1 - side],
				0,
				&g_sdlInputbindStates.paxes[axis][1 - side]);
			if (!BEL_ST_AltControlScheme_HandleEntry(
				&g_sdlControllerMappingActualCurr->paxes[axis][side],
				abs(axisVal),
				&g_sdlInputbindStates.paxes[axis][side]))
				{
					// Special case for triggers, treated like digital buttons
					if ((axis == BE_ST_CTRL_AXIS_LTRIGGER) || (axis == BE_ST_CTRL_AXIS_RTRIGGER))
						BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, axisVal, &g_sdlDefaultMappingBinaryState);
				}

			break;
		}

		case SDL_CONTROLLERBUTTONDOWN:
			BEL_ST_CheckForHidingTouchUI();
			// Fall-through
		case SDL_CONTROLLERBUTTONUP:
		{
			bool isPressed = (event.type == SDL_CONTROLLERBUTTONDOWN);
			int but = event.cbutton.button;
			if ((but < 0) || (but >= BE_ST_CTRL_BUT_MAX))
				break;

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
						g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
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

		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				BEL_ST_SetGfxOutputRects(false);
				// Fall-through
			case SDL_WINDOWEVENT_EXPOSED:
				BEL_ST_ForceHostDisplayUpdate();
				break;
			}
			break;

		case SDL_RENDER_TARGETS_RESET:
		case SDL_RENDER_DEVICE_RESET:
			BEL_ST_RecreateAllTextures();
			break;

		case SDL_QUIT:
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
		g_sdlEmuKeyboardLastScanCodePressTimeDelay = BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;
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
			g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay = BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;
		
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

SDL_sem *g_sdlEventsCallbackToMainSem, *g_sdlMainToEventsCallbackSem;

void BEL_ST_SaveConfigFiles(void);

void BEL_ST_CheckForExitFromEventsCallback(void)
{
	if (SDL_SemTryWait(g_sdlEventsCallbackToMainSem) == 0)
	{
		BEL_ST_SaveConfigFiles(); // From BE_ST_QuickExit
		SDL_SemPost(g_sdlMainToEventsCallbackSem);
		SDL_SemWait(g_sdlEventsCallbackToMainSem); // Wait here "forever"
	}
}

// Use this to catch a few special events here when required
static uint32_t g_sdl_eventCallback_EnterBackgroundLastTicks;

int BEL_ST_EventsCallback(void *userdata, SDL_Event *event)
{
	extern SDL_AudioDeviceID g_sdlAudioDevice;
	extern uint32_t g_be_audioMainThread_lastCallTicks;

	switch (event->type)
	{
	case SDL_APP_TERMINATING:
	case SDL_APP_LOWMEMORY: // Let's just terminate the app in such a case
		SDL_SemPost(g_sdlEventsCallbackToMainSem);
		SDL_SemWait(g_sdlMainToEventsCallbackSem);
		if (event->type != SDL_APP_TERMINATING)
			exit(0);
		return 0;
	case SDL_APP_WILLENTERBACKGROUND:
		if (g_sdlAudioSubsystemUp) // FIXME - Hope this works well
			SDL_PauseAudioDevice(g_sdlAudioDevice, 1);
		return 0;
	case SDL_APP_DIDENTERBACKGROUND:
		g_sdl_eventCallback_EnterBackgroundLastTicks = BEL_ST_GetTicksMS();
		return 0;
	case SDL_APP_WILLENTERFOREGROUND:
		// FIXME!!! - Hope this works well
		g_be_audioMainThread_lastCallTicks += (BEL_ST_GetTicksMS() - g_sdl_eventCallback_EnterBackgroundLastTicks);
		return 0;
	case SDL_APP_DIDENTERFOREGROUND:
		// HACK - These may be done from a different thread,
		// but should be relatively simple anyway
		BEL_ST_ForceHostDisplayUpdate();
#ifdef REFKEEN_ENABLE_LAUNCHER
		BE_ST_Launcher_MarkGfxCache();
#endif
		if (g_sdlAudioSubsystemUp) // FIXME - Hope this works well
			SDL_PauseAudioDevice(g_sdlAudioDevice, 0);
		return 0;
	default:
		return 1; // Just send to SDL_PollEvent as usual
	}
}

#endif // REFKEEN_CONFIG_EVENTS_CALLBACK
