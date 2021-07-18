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

#include "backend/audio/be_audio_main_thread.h"
#include "backend/audio/be_audio_private.h"
#include "backend/input/be_input.h"
#include "backend/input/be_input_controller_mappings.h"
#include "backend/input/be_input_sdl.h"
#include "backend/input/be_input_ui.h"
#include "backend/video/be_video_textures.h"
#include "backend/video/be_video_ui.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h"
#include "be_st.h"
#include "be_st_launcher.h"
#include "be_st_sdl_private.h"

static void (*g_sdlAppQuitCallback)(void) = 0;

extern int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;
extern bool g_sdlForceGfxControlUiRefresh;
extern bool g_sdlShowControllerUI;
extern bool g_sdlShowTouchUI;

extern bool g_sdlAudioSubsystemUp;

void BE_ST_InitGfx(void);
void BE_ST_InitTiming(void);
void BE_ST_ShutdownAudio(void);
void BE_ST_ShutdownGfx(void);
void BEL_ST_ConditionallyAddJoystick(int device_index); // Implementation-specific
void BEL_ST_ParseConfig(void);
void BEL_ST_SaveConfig(void);


#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK
SDL_sem *g_sdlEventsCallbackToMainSem, *g_sdlMainToEventsCallbackSem;

static int BEL_ST_EventsCallback(void *userdata, SDL_Event *event);
#endif

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

#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK
static void BEL_ST_CheckForExitFromEventsCallback(void)
{
	if (SDL_SemTryWait(g_sdlEventsCallbackToMainSem) == 0)
	{
		BEL_ST_SaveConfig(); // From BE_ST_QuickExit
		SDL_SemPost(g_sdlMainToEventsCallbackSem);
		SDL_SemWait(g_sdlEventsCallbackToMainSem); // Wait here "forever"
	}
}
#else
#define BEL_ST_CheckForExitFromEventsCallback()
#endif

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
		BEL_ST_CheckForExitFromEventsCallback();
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


#define emptyDOSKeyEvent {false, 0}

#if SDL_VERSION_ATLEAST(2,0,0)
const emulatedDOSKeyEvent sdlKeyMappings[SDL_NUM_SCANCODES] = {
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    {false,  BE_ST_SC_A}, // SDL_SCANCODE_A
    {false,  BE_ST_SC_B}, // SDL_SCANCODE_B
    {false,  BE_ST_SC_C}, // SDL_SCANCODE_C
    {false,  BE_ST_SC_D}, // SDL_SCANCODE_D
    {false,  BE_ST_SC_E}, // SDL_SCANCODE_E
    {false,  BE_ST_SC_F}, // SDL_SCANCODE_F
    {false,  BE_ST_SC_G}, // SDL_SCANCODE_G
    {false,  BE_ST_SC_H}, // SDL_SCANCODE_H
    {false,  BE_ST_SC_I}, // SDL_SCANCODE_I
    {false,  BE_ST_SC_J}, // SDL_SCANCODE_J
    {false,  BE_ST_SC_K}, // SDL_SCANCODE_K
    {false,  BE_ST_SC_L}, // SDL_SCANCODE_L
    {false,  BE_ST_SC_M}, // SDL_SCANCODE_M
    {false,  BE_ST_SC_N}, // SDL_SCANCODE_N
    {false,  BE_ST_SC_O}, // SDL_SCANCODE_O
    {false,  BE_ST_SC_P}, // SDL_SCANCODE_P
    {false,  BE_ST_SC_Q}, // SDL_SCANCODE_Q
    {false,  BE_ST_SC_R}, // SDL_SCANCODE_R
    {false,  BE_ST_SC_S}, // SDL_SCANCODE_S
    {false,  BE_ST_SC_T}, // SDL_SCANCODE_T
    {false,  BE_ST_SC_U}, // SDL_SCANCODE_U
    {false,  BE_ST_SC_V}, // SDL_SCANCODE_V
    {false,  BE_ST_SC_W}, // SDL_SCANCODE_W
    {false,  BE_ST_SC_X}, // SDL_SCANCODE_X
    {false,  BE_ST_SC_Y}, // SDL_SCANCODE_Y
    {false,  BE_ST_SC_Z}, // SDL_SCANCODE_Z

    {false,  BE_ST_SC_1}, // SDL_SCANCODE_1
    {false,  BE_ST_SC_2}, // SDL_SCANCODE_2
    {false,  BE_ST_SC_3}, // SDL_SCANCODE_3
    {false,  BE_ST_SC_4}, // SDL_SCANCODE_4
    {false,  BE_ST_SC_5}, // SDL_SCANCODE_5
    {false,  BE_ST_SC_6}, // SDL_SCANCODE_6
    {false,  BE_ST_SC_7}, // SDL_SCANCODE_7
    {false,  BE_ST_SC_8}, // SDL_SCANCODE_8
    {false,  BE_ST_SC_9}, // SDL_SCANCODE_9
    {false,  BE_ST_SC_0}, // SDL_SCANCODE_0

    {false,  BE_ST_SC_ENTER}, // SDL_SCANCODE_RETURN
    {false,  BE_ST_SC_ESC}, // SDL_SCANCODE_ESCAPE
    {false,  BE_ST_SC_BSPACE}, // SDL_SCANCODE_BACKSPACE
    {false,  BE_ST_SC_TAB}, // SDL_SCANCODE_TAB
    {false,  BE_ST_SC_SPACE}, // SDL_SCANCODE_SPACE

    {false,  BE_ST_SC_MINUS}, // SDL_SCANCODE_MINUS
    {false,  BE_ST_SC_EQUALS}, // SDL_SCANCODE_EQUALS
    {false,  BE_ST_SC_LBRACKET}, // SDL_SCANCODE_LEFTBRACKET
    {false,  BE_ST_SC_RBRACKET}, // SDL_SCANCODE_RIGHTBRACKET
    {false,  BE_ST_SC_BACKSLASH}, // SDL_SCANCODE_BACKSLASH

    emptyDOSKeyEvent, // SDL_SCANCODE_NONUSHASH

    {false,  BE_ST_SC_SEMICOLON}, // SDL_SCANCODE_SEMICOLON
    {false,  BE_ST_SC_QUOTE}, // SDL_SCANCODE_APOSTROPHE
    {false,  BE_ST_SC_GRAVE}, // SDL_SCANCODE_GRAVE
    {false,  BE_ST_SC_COMMA}, // SDL_SCANCODE_COMMA
    {false,  BE_ST_SC_PERIOD}, // SDL_SCANCODE_PERIOD
    {false,  BE_ST_SC_SLASH}, // SDL_SCANCODE_SLASH

    {false,  BE_ST_SC_CAPSLOCK}, // SDL_SCANCODE_CAPSLOCK

    {false,  BE_ST_SC_F1}, // SDL_SCANCODE_F1
    {false,  BE_ST_SC_F2}, // SDL_SCANCODE_F2
    {false,  BE_ST_SC_F3}, // SDL_SCANCODE_F3
    {false,  BE_ST_SC_F4}, // SDL_SCANCODE_F4
    {false,  BE_ST_SC_F5}, // SDL_SCANCODE_F5
    {false,  BE_ST_SC_F6}, // SDL_SCANCODE_F6
    {false,  BE_ST_SC_F7}, // SDL_SCANCODE_F7
    {false,  BE_ST_SC_F8}, // SDL_SCANCODE_F8
    {false,  BE_ST_SC_F9}, // SDL_SCANCODE_F9
    {false,  BE_ST_SC_F10}, // SDL_SCANCODE_F10
    {false,  BE_ST_SC_F11}, // SDL_SCANCODE_F11
    {false,  BE_ST_SC_F12}, // SDL_SCANCODE_F12

    {false,  BE_ST_SC_PRINTSCREEN}, // SDL_SCANCODE_PRINTSCREEN
    {false,  BE_ST_SC_SCROLLLOCK}, // SDL_SCANCODE_SCROLLLOCK
    {false,  BE_ST_SC_PAUSE}, // SDL_SCANCODE_PAUSE
    {true,  BE_ST_SC_INSERT}, // SDL_SCANCODE_INSERT
    {true,  BE_ST_SC_HOME}, // SDL_SCANCODE_HOME
    {true,  BE_ST_SC_PAGEUP}, // SDL_SCANCODE_PAGEUP
    {true,  BE_ST_SC_DELETE}, // SDL_SCANCODE_DELETE
    {true,  BE_ST_SC_END}, // SDL_SCANCODE_END
    {true,  BE_ST_SC_PAGEDOWN}, // SDL_SCANCODE_PAGEDOWN
    {true,  BE_ST_SC_RIGHT}, // SDL_SCANCODE_RIGHT
    {true,  BE_ST_SC_LEFT}, // SDL_SCANCODE_LEFT
    {true,  BE_ST_SC_DOWN}, // SDL_SCANCODE_DOWN
    {true,  BE_ST_SC_UP}, // SDL_SCANCODE_UP

    {false,  BE_ST_SC_NUMLOCK}, // SDL_SCANCODE_NUMLOCKCLEAR
    {true,  BE_ST_SC_KP_DIVIDE}, // SDL_SCANCODE_KP_DIVIDE
    {false,  BE_ST_SC_KP_MULTIPLY}, // SDL_SCANCODE_KP_MULTIPLY
    {false,  BE_ST_SC_KP_MINUS}, // SDL_SCANCODE_KP_MINUS
    {false,  BE_ST_SC_KP_PLUS}, // SDL_SCANCODE_KP_PLUS
    {true,  BE_ST_SC_KP_ENTER}, // SDL_SCANCODE_KP_ENTER
    {false,  BE_ST_SC_KP_1}, // SDL_SCANCODE_KP_1
    {false,  BE_ST_SC_KP_2}, // SDL_SCANCODE_KP_2
    {false,  BE_ST_SC_KP_3}, // SDL_SCANCODE_KP_3
    {false,  BE_ST_SC_KP_4}, // SDL_SCANCODE_KP_4
    {false,  BE_ST_SC_KP_5}, // SDL_SCANCODE_KP_5
    {false,  BE_ST_SC_KP_6}, // SDL_SCANCODE_KP_6
    {false,  BE_ST_SC_KP_7}, // SDL_SCANCODE_KP_7
    {false,  BE_ST_SC_KP_8}, // SDL_SCANCODE_KP_8
    {false,  BE_ST_SC_KP_9}, // SDL_SCANCODE_KP_9
    {false,  BE_ST_SC_KP_0}, // SDL_SCANCODE_KP_0
    {false,  BE_ST_SC_KP_PERIOD}, // SDL_SCANCODE_KP_PERIOD

    {false,  BE_ST_SC_LESSTHAN}, // SDL_SCANCODE_NONUSBACKSLASH

    // SDL 2.0 scancodes 101 to 223 (either actually defined or not) follow...
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    {false,  BE_ST_SC_ENTER}, // SDL_SCANCODE_SELECT (for Android, d-pad press)
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,

    {false,  BE_ST_SC_LCTRL}, // SDL_SCANCODE_LCTRL
    {false,  BE_ST_SC_LSHIFT}, // SDL_SCANCODE_LSHIFT
    {false,  BE_ST_SC_LALT}, // SDL_SCANCODE_LALT

    emptyDOSKeyEvent, // SDL_SCANCODE_LGUI

    {true,  BE_ST_SC_RCTRL}, // SDL_SCANCODE_RCTRL
    {false,  BE_ST_SC_RSHIFT}, // SDL_SCANCODE_RSHIFT
    {true,  BE_ST_SC_RALT}, // SDL_SCANCODE_RALT

    // SDL 2.0 scancodes 231 to 269...

    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,

    {false,  BE_ST_SC_ESC}, // SDL_SCANCODE_AC_BACK (for Android)

    // More SDL 2.0 scancodes follow, but are ignored
};
#else
#error "SDL <2.0 support is unimplemented!"
#endif


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
			BEL_ST_HandleEmuKeyboardEvent(event.type == SDL_KEYDOWN, false, sdlKeyMappings[event.key.keysym.scancode]);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if ((event.button.button >= 1) && (event.button.button <= 3))
			{
				if (g_sdlMouseButtonsStates[event.button.button-1])
					break; // Ignore (used in case pointer is shown/hidden)
				g_sdlMouseButtonsStates[event.button.button-1] = true;
			}

			BEL_ST_CheckForHidingTouchUI();

			if (BEL_ST_CheckCommonPointerPressCases(0, 0, event.button.x, event.button.y))
				break;

			if (event.button.button == SDL_BUTTON_LEFT)
				g_sdlEmuMouseButtonsState |= 1;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				g_sdlEmuMouseButtonsState |= 2;
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				g_sdlEmuMouseButtonsState |= 4;
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if ((event.button.button >= 1) && (event.button.button <= 3))
			{
				if (!g_sdlMouseButtonsStates[event.button.button-1])
					break; // Ignore (used in case pointer is shown/hidden)
				g_sdlMouseButtonsStates[event.button.button-1] = false;
			}

			if (BEL_ST_CheckCommonPointerReleaseCases(0, 0, event.button.x, event.button.y))
				break;

			if (event.button.button == SDL_BUTTON_LEFT)
				g_sdlEmuMouseButtonsState &= ~1;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				g_sdlEmuMouseButtonsState &= ~2;
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				g_sdlEmuMouseButtonsState &= ~4;
			break;
		case SDL_MOUSEMOTION:
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if (BEL_ST_CheckCommonPointerMoveCases(0, 0, event.motion.x, event.motion.y))
				break;

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
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
#endif
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
			if (!g_refKeenCfg.altControlScheme.isEnabled)
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
			if ((axis < 0) || (axis >= BE_ST_CTRL_AXIS_MAX))
				break;
			// Note: We handle BOTH sides, in case axisVal == 0,
			// or alternatively, the sign of axisVal changes, so
			// "release/clear" events can be properly sent.
			BEL_ST_AltControlScheme_HandleEntry(
				&g_sdlControllerMappingActualCurr->axes[axis][0],
				BE_Cross_TypedMax(int, -axisVal, 0),
				&g_sdlControllersAxesStates[axis][0]);
			if (!BEL_ST_AltControlScheme_HandleEntry(
				&g_sdlControllerMappingActualCurr->axes[axis][1],
				BE_Cross_TypedMax(int, axisVal, 0)
				, &g_sdlControllersAxesStates[axis][1]))
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
				if (isPressed == g_sdlControllersButtonsStates[but])
					break;
				g_sdlControllersButtonsStates[but] = isPressed;

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
			else if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->buttons[but], g_sdlJoystickAxisMax*isPressed, &g_sdlControllersButtonsStates[but]))
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

	BEL_ST_CheckForExitFromEventsCallback();

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

// Use this to catch a few special events here when required
static uint32_t g_sdl_eventCallback_EnterBackgroundLastTicks;

static int BEL_ST_EventsCallback(void *userdata, SDL_Event *event)
{
	extern SDL_AudioDeviceID g_sdlAudioDevice;
	extern uint32_t g_sdlManualAudioCallbackCallLastTicks;

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
		g_sdlManualAudioCallbackCallLastTicks += (BEL_ST_GetTicksMS() - g_sdl_eventCallback_EnterBackgroundLastTicks);
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
#endif
