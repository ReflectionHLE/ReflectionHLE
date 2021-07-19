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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "backend/events/be_events_sdl.h"
#include "backend/input/be_input.h"
#include "backend/input/be_input_controller_mappings.h"
#include "backend/input/be_input_sdl.h"
#include "backend/video/be_video_ui.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_st_sdl_private.h"

extern bool g_sdlShowControllerUI;
extern bool g_sdlShowTouchUI;

void BE_ST_InitGfx(void);
void BE_ST_InitTiming(void);
void BE_ST_ShutdownAudio(void);
void BE_ST_ShutdownGfx(void);
void BEL_ST_ConditionallyAddJoystick(int device_index); // Implementation-specific
void BEL_ST_ParseConfig(void);
void BEL_ST_SaveConfig(void);


void BE_ST_InitCommon(void)
{
	// SDL_INIT_VIDEO implies SDL_INIT_EVENTS, and SDL_INIT_TIMER caused
	// problems (hangs) in Emscripten prototype builds.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL backend initialization failed,\n%s\n", SDL_GetError());
		exit(0);
	}

#ifdef REFKEEN_CONFIG_RESET_SDL_HINT_ACCELEROMETER_AS_JOYSTICK
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL game controller subsystem initialization (including joystick subsystem) failed, disabled,\n%s\n", SDL_GetError());
	}

	// MUST be called BEFORE parsing config (of course!)
	BE_Cross_PrepareAppPaths();

	BEL_ST_ParseConfig();
	// This technically requires SDL 2.0.2, which has been available for a year now; Should be called BEFORE any SDL_CONTROLLERDEVICEADDED event should arrive (so e.g., before SDL_PollEvent).
	FILE *fp = BE_Cross_open_additionalfile_for_reading("gamecontrollerdb.txt");
	if (fp)
		SDL_GameControllerAddMappingsFromRW(SDL_RWFromFP(fp, SDL_TRUE), 1);

	// HACK - If game is *not* started from launcher, TOUCHINPUT_AUTO has
	// the same initial behaviors as TOUCHINPUT_OFF here (i.e., not showing touch UI).
	// Otherwise, the launcher **guesses** the correct value to set based
	// on a recent event type (e.g., mouse event -> no touch UI is shown).
	g_sdlShowTouchUI = (g_refKeenCfg.touchInputToggle == TOUCHINPUT_FORCED);

#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK
	g_sdlEventsCallbackToMainSem = SDL_CreateSemaphore(0);
	if (!g_sdlEventsCallbackToMainSem)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Cannot create events callback to main semaphore,\n%s\n", SDL_GetError());
		exit(0);
	}
	g_sdlMainToEventsCallbackSem = SDL_CreateSemaphore(0);
	if (!g_sdlMainToEventsCallbackSem)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Cannot create main to events callback semaphore,\n%s\n", SDL_GetError());
		exit(0);
	}

	SDL_SetEventFilter(BEL_ST_EventsCallback, NULL);
#endif
}

void BE_ST_PrepareForGameStartupWithoutAudio(void)
{
	BE_ST_InitGfx();
	//BE_ST_InitAudio(); // Not yet, need to select game version (so have gfx ready for possible errors), and then check if we want digi audio output
	BE_ST_InitTiming();

	//g_sdlControllerSchemeNeedsCleanUp = false;

	memset(g_sdlControllersButtonsStates, 0, sizeof(g_sdlControllersButtonsStates));
	memset(g_sdlControllersAxesStates, 0, sizeof(g_sdlControllersAxesStates));

	g_sdlShowControllerUI = false;

	// BEFORE checking for more joysticks being attached/removed in BE_ST_PollEvents, add what's currently available
	int nOfJoysticks = SDL_NumJoysticks();
	if (nOfJoysticks > BE_ST_MAXJOYSTICKS)
		nOfJoysticks = BE_ST_MAXJOYSTICKS;
	for (int i = 0; i < nOfJoysticks; ++i)
		BEL_ST_ConditionallyAddJoystick(i);

	// Reset these first
	memset(g_sdlEmuKeyboardStateByScanCode, 0, sizeof(g_sdlEmuKeyboardStateByScanCode));

	g_sdlEmuMouseButtonsState = 0;
	memset(g_sdlEmuMouseMotionFromJoystick, 0, sizeof(g_sdlEmuMouseMotionFromJoystick));
	g_sdlEmuJoyButtonsState = 0;
	// A bit tricky, should be reported as centered *if* any joystick is connected (and *not* while using modern controller scheme)
	// Note 1: A single controller may support up to all 4 axes
	// Note 2: Assigning 0 here may lead to division by zero in Keen Dreams v1.00
	g_sdlEmuJoyMotionState[0] = g_sdlEmuJoyMotionState[1] = g_sdlEmuJoyMotionState[2] = g_sdlEmuJoyMotionState[3] = BE_ST_EMU_JOYSTICK_OVERRANGEMAX;
	for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		if (g_sdlJoysticks[i])
		{
			g_sdlEmuJoyMotionState[0] = g_sdlEmuJoyMotionState[1] = g_sdlEmuJoyMotionState[2] = g_sdlEmuJoyMotionState[3] = BE_ST_EMU_JOYSTICK_RANGECENTER;
			break;
		}
	// Then use this to reset/update some states, and detect joysticks
	BE_ST_PollEvents();
	// Events may gradually fill these, especially after messing with a mouse cursor, so reset AFTER poll
	g_sdlEmuMouseMotionAccumulatedState[0] = g_sdlEmuMouseMotionAccumulatedState[1] = 0;
	BE_ST_ResetEmuMouse();
}

void BE_ST_ShutdownAll(void)
{
	BEL_ST_SetMouseMode(BE_ST_MOUSEMODE_ABS_WITH_CURSOR);
	BE_ST_ShutdownAudio();
	BE_ST_ShutdownGfx();
	SDL_Quit();
}

void BE_ST_HandleExit(int status)
{
	SDL_Event event;
	bool keepRunning = true;

	BEL_ST_AltControlScheme_CleanUp(); // Used for removal of any remaining controller UI (and possibly some more)

	while (keepRunning)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
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
			case SDL_JOYHATMOTION:
				if (event.jhat.value != SDL_HAT_CENTERED) // Otherwise ignore
					keepRunning = false;
				break;
			case SDL_KEYDOWN:
				if (!event.key.repeat) // Otherwise ignore
					keepRunning = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_JOYBUTTONDOWN:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_QUIT:
				keepRunning = false;
				break;
			default: ;
			}
		}
#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK
		BEL_ST_CheckForExitFromEventsCallback();
#endif
		BEL_ST_SleepMS(1);
		// TODO: Make this more efficient
		BEL_ST_UpdateHostDisplay();
	}
	BE_ST_QuickExit();
}

void BE_ST_ExitWithErrorMsg(const char *msg)
{
	BE_ST_SetScreenMode(3);
	BE_ST_puts(msg);
	BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "%s\n", msg);
	BE_ST_HandleExit(1);
}

void BE_ST_QuickExit(void)
{
	BEL_ST_SaveConfig();
	BE_ST_ShutdownAll();
	exit(0);
}
