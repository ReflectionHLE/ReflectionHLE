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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "backend/audio/be_audio_main_thread.h"
#include "backend/video/be_video_textures.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h"
#include "be_st.h"
#include "be_st_launcher.h"
#include "be_st_sdl_private.h"

#define BE_ST_EMU_MOUSE_XRANGE 640
#define BE_ST_EMU_MOUSE_YRANGE 200

// Using example of values from here:
// http://www.intel-assembler.it/portale/5/program-joystick-port-210h/program-joystick-port-210h.asp
#define BE_ST_EMU_JOYSTICK_RANGEMIN 8
#define BE_ST_EMU_JOYSTICK_RANGECENTER 330
#define BE_ST_EMU_JOYSTICK_RANGEMAX 980
// This one is for init with no joysticks, for Keen Dreams v1.00
// (It requires a large value, while 0 will lead to division by zero)
#define BE_ST_EMU_JOYSTICK_OVERRANGEMAX 16384

#define BE_ST_DEFAULT_FARPTRSEGOFFSET 0x14

/*** Last BE_ST_PollEvents time ***/
static uint32_t g_sdlLastPollEventsTime;

static void (*g_sdlKeyboardInterruptFuncPtr)(uint8_t) = 0;
static void (*g_sdlAppQuitCallback)(void) = 0;

// Defined internally, but also used in launcher code. extern is required for C++.
#ifdef __cplusplus
extern
#endif
const int g_sdlJoystickAxisBinaryThreshold = 16384, g_sdlJoystickAxisDeadZone = 3200, g_sdlJoystickAxisMax = 32767, g_sdlJoystickAxisMaxMinusDeadZone = 32767-3200;

static SDL_Joystick *g_sdlJoysticks[BE_ST_MAXJOYSTICKS];
SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS]; // Also used in launcher
SDL_JoystickID g_sdlJoysticksInstanceIds[BE_ST_MAXJOYSTICKS];

/*** These represent button states, although a call to BEL_ST_AltControlScheme_CleanUp zeros these out ***/
static bool g_sdlControllersButtonsStates[BE_ST_CTRL_BUT_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
static bool g_sdlControllersAxesStates[BE_ST_CTRL_AXIS_MAX][2];

/*** These are similar states for a few mouse buttons, required as relative mouse mode is toggled on or off in the middle ***/
bool g_sdlMouseButtonsStates[3];

/*** Another internal state, used for default mapping action ***/
bool g_sdlDefaultMappingBinaryState;

/*** Emulated mouse and joysticks states (mouse motion state is split for technical reasons) ***/
int g_sdlEmuMouseButtonsState;
int16_t g_sdlEmuMouseMotionAccumulatedState[2];
int16_t g_sdlEmuMouseCursorPos[2];
static int16_t g_sdlEmuMouseMotionFromJoystick[2];
int16_t g_sdlVirtualMouseCursorState[2]; // Used e.g., for touch input handling
static int g_sdlEmuJoyButtonsState;
static int16_t g_sdlEmuJoyMotionState[4];

/*** Key repeat emulation ***/
static int g_sdlEmuKeyboardLastPressedScanCode = 0; // 0 on release
static bool g_sdlEmuKeyboardLastPressedIsSpecial;
static uint32_t g_sdlEmuKeyboardLastScanCodePressTime;
static uint32_t g_sdlEmuKeyboardLastScanCodePressTimeDelay;

static bool g_sdlEmuKeyboardStateByScanCode[BE_ST_SC_MAX];

/*** Similar repeat emulation for arrow movement in on-screen keyboard (key press repeats are emulated separately)   ***/
/*** HACK: With debug keys, once a key is selected, key repeat is in effect! (but that's the case with actual keys.) ***/
static int g_sdlOnScreenKeyboardLastPressedDirButton;
static uint32_t g_sdlOnScreenKeyboardLastDirButtonPressTime;
static uint32_t g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay;

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

// The index is taken off the enum above so ENSURE THESE ARE CONSISTENT!
//
// HACK: If this is updated, also check g_sdlCfgEntries!!!
static const char *g_sdlControlSchemeKeyMapCfgKeyPrefixes[] = {
	"altcontrolscheme_up=",
	"altcontrolscheme_down=",
	"altcontrolscheme_left=",
	"altcontrolscheme_right=",
#ifdef REFKEEN_HAS_VER_KDREAMS
	"altcontrolscheme_jump=",
	"altcontrolscheme_throw=",
	"altcontrolscheme_stats=",
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	"altcontrolscheme_fire=",
	"altcontrolscheme_strafe=",
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	"altcontrolscheme_drink=",
	"altcontrolscheme_bolt=", // Zapper in the Adventures Series
	"altcontrolscheme_nuke=", // Xterminator in the Adventures Series
	"altcontrolscheme_fastturn=",
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	"altcontrolscheme_scrolls=",
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	"altcontrolscheme_use=",
	"altcontrolscheme_run=",
	"altcontrolscheme_weapon1=",
	"altcontrolscheme_weapon2=",
	"altcontrolscheme_weapon3=",
	"altcontrolscheme_weapon4=",
	"altcontrolscheme_weapon5=",
	"altcontrolscheme_weapon6=",
	"altcontrolscheme_map=",
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	"altcontrolscheme_funckeys=",
#endif
	"altcontrolscheme_debugkeys=",
	0,
};


extern int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;
extern bool g_sdlForceGfxControlUiRefresh;
extern bool g_sdlShowControllerUI;
extern bool g_sdlShowTouchUI;

extern bool g_sdlAudioSubsystemUp;

static uint8_t g_sdlLastKeyScanCodeBeforeAnyReset; // May be reset by BE_ST_BiosScanCode

void BE_ST_InitGfx(void);
void BE_ST_InitTiming(void);
void BE_ST_ShutdownAudio(void);
void BE_ST_ShutdownGfx(void);
static void BEL_ST_ConditionallyAddJoystick(int device_index);
static void BEL_ST_ParseConfig(void);
static void BEL_ST_SaveConfig(void);


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

static void BEL_ST_AltControlScheme_CleanUp(void);

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

// Enumerated by SDL_GameControllerButton, for most
static const char *g_sdlControlSchemeKeyMapCfgVals[] = {
	"a", "b", "x", "y", 0, 0, 0, "lstick", "rstick", "lshoulder", "rshoulder", "dpadup", "dpaddown", "dpadleft", "dpadright", 0, "paddle1", "paddle2", "paddle3", "paddle4",
	"ltrigger", "rtrigger", // Actually axes but these are added as extras
	"" // for any entry which is not set
};

RefKeenConfig g_refKeenCfg;

#ifdef REFKEEN_VER_KDREAMS
#define REFKEEN_CONFIG_FILENAME "reflection-kdreams.cfg"
#elif defined REFKEEN_VER_CAT3D
#define REFKEEN_CONFIG_FILENAME "reflection-cat3d.cfg"
#elif defined REFKEEN_VER_CATABYSS
#define REFKEEN_CONFIG_FILENAME "reflection-catabyss.cfg"
#elif defined REFKEEN_VER_CATARM
#define REFKEEN_CONFIG_FILENAME "reflection-catarm.cfg"
#elif defined REFKEEN_VER_CATAPOC
#define REFKEEN_CONFIG_FILENAME "reflection-catapoc.cfg"
#elif defined REFKEEN_HAS_VER_CATACOMB_ALL
#define REFKEEN_CONFIG_FILENAME "reflection-catacomb.cfg"
#elif defined REFKEEN_HAS_VER_WOLF3D_ALL // TODO: Optionally separate cfg per version?
#define REFKEEN_CONFIG_FILENAME "reflection-wolf3d.cfg"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
static void BEL_ST_ParseSetting_FullScreen(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.isFullscreen = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.isFullscreen = false;
	}
}
#endif

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
static void BEL_ST_ParseSetting_FullRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.fullWidth, &g_refKeenCfg.fullHeight);
}
#endif

static void BEL_ST_ParseSetting_WindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.winWidth, &g_refKeenCfg.winHeight);
}

#ifdef REFKEEN_ENABLE_LAUNCHER
/*
static void BEL_ST_ParseSetting_LauncherWindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.launcherWinWidth, &g_refKeenCfg.launcherWinHeight);
}
*/
static void BEL_ST_ParseSetting_LauncherWindowType(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "default"))
	{
		g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_DEFAULT;
	}
	else if (!strcmp(buffer, "full"))
	{
		g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_FULL;
	}
	else if (!strcmp(buffer, "software"))
	{
		g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_SOFTWARE;
	}
}

static void BEL_ST_ParseSetting_LauncherExeArgs(const char *keyprefix, const char *buffer)
{
	BE_Cross_safeandfastcstringcopy(g_refKeenCfg.launcherExeArgs, g_refKeenCfg.launcherExeArgs+sizeof(g_refKeenCfg.launcherExeArgs), buffer);
}
#endif

static void BEL_ST_ParseSetting_LastSelectedGameExe(const char *keyprefix, const char *buffer)
{
	BE_Cross_safeandfastcstringcopy(g_refKeenCfg.lastSelectedGameExe, g_refKeenCfg.lastSelectedGameExe+sizeof(g_refKeenCfg.lastSelectedGameExe), buffer);
}

static void BEL_ST_ParseSetting_LastSelectedGameVer(const char *keyprefix, const char *buffer)
{
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
		if (!strcmp(buffer, refkeen_gamever_strs[i]))
		{
			g_refKeenCfg.lastSelectedGameVer = i;
			break;
		}
}

static void BEL_ST_ParseSetting_DisplayNum(const char *keyprefix, const char *buffer)
{
	if (sscanf(buffer, "%d", &g_refKeenCfg.displayNum) == 1)
		if ((g_refKeenCfg.displayNum < 0) || (g_refKeenCfg.displayNum >= SDL_GetNumVideoDisplays()))
			g_refKeenCfg.displayNum = 0;
}

static void BEL_ST_ParseSetting_RememberDisplayNum(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.rememberDisplayNum = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.rememberDisplayNum = false;
	}
}

static void BEL_ST_ParseSetting_SDLRendererDriver(const char *keyprefix, const char *buffer)
{
	SDL_RendererInfo info;
	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
	{
		SDL_GetRenderDriverInfo(i, &info);
		if (!strcmp(info.name, buffer))
		{
			g_refKeenCfg.sdlRendererDriver = i;
			break;
		}
	}
}

static void BEL_ST_ParseSetting_VSync(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "on"))
	{
		g_refKeenCfg.vSync = VSYNC_ON;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_refKeenCfg.vSync = VSYNC_OFF;
	}
	else if (!strcmp(buffer, "auto"))
	{
		g_refKeenCfg.vSync = VSYNC_AUTO;
	}
}

static void BEL_ST_ParseSetting_Bilinear(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.isBilinear = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.isBilinear = false;
	}
}

static void BEL_ST_ParseSetting_ScaleType(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "aspect"))
	{
		g_refKeenCfg.scaleType = SCALE_ASPECT;
	}
	else if (!strcmp(buffer, "fill"))
	{
		g_refKeenCfg.scaleType = SCALE_FILL;
	}
}

static void BEL_ST_ParseSetting_ScaleFactor(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.scaleFactor = atoi(buffer);
}

static void BEL_ST_ParseSetting_ForceFullSoftScaling(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.forceFullSoftScaling = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.forceFullSoftScaling = false;
	}
}

static void BEL_ST_ParseSetting_MouseGrab(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "commonly"))
	{
		g_refKeenCfg.mouseGrab = MOUSEGRAB_COMMONLY;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_refKeenCfg.mouseGrab = MOUSEGRAB_OFF;
	}
	else if (!strcmp(buffer, "auto"))
	{
		g_refKeenCfg.mouseGrab = MOUSEGRAB_AUTO;
	}
}

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
static void BEL_ST_ParseSetting_AbsMouseMotion(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.absMouseMotion = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.absMouseMotion = false;
	}
}
#endif

static void BEL_ST_ParseSetting_SndInterThreadBufferRatio(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.sndInterThreadBufferRatio = atoi(buffer);
	if (g_refKeenCfg.sndInterThreadBufferRatio <= 0)
		g_refKeenCfg.sndInterThreadBufferRatio = 2;
}

static void BEL_ST_ParseSetting_SndSampleRate(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.sndSampleRate = atoi(buffer);
	if (g_refKeenCfg.sndSampleRate <= 0)
		g_refKeenCfg.sndSampleRate = 49716; // TODO - Shared define again
}

static void BEL_ST_ParseSetting_SoundSubSystem(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.sndSubSystem = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.sndSubSystem = false;
	}
}

static void BEL_ST_ParseSetting_OPLEmulation(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.oplEmulation = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.oplEmulation = false;
	}
}

#ifndef REFKEEN_RESAMPLER_NONE
static void BEL_ST_ParseSetting_UseResampler(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.useResampler = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.useResampler = false;
	}
}
#endif

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
static void BEL_ST_ParseSetting_TouchInputToggle(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "auto"))
	{
		g_refKeenCfg.touchInputToggle = TOUCHINPUT_AUTO;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_refKeenCfg.touchInputToggle = TOUCHINPUT_OFF;
	}
	else if (!strcmp(buffer, "forced"))
	{
		g_refKeenCfg.touchInputToggle = TOUCHINPUT_FORCED;
	}
}

static void BEL_ST_ParseSetting_TouchInputDebugging(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.touchInputDebugging = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.touchInputDebugging = false;
	}
}
#endif // REFKEEN_CONFIG_ENABLE_TOUCHINPUT

static void BEL_ST_ParseSetting_AlternativeControlScheme(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.isEnabled = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.isEnabled = false;
	}
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap(const char *keyprefix, const char *buffer)
{
	int keyindex, valindex;
	for (keyindex = 0; keyindex < BE_ST_CTRL_CFG_BUTMAP_AFTERLAST; ++keyindex)
	{
		if (!strcmp(keyprefix, g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex]))
			break;
	}
	if (keyindex == BE_ST_CTRL_CFG_BUTMAP_AFTERLAST)
	{
		BE_ST_ExitWithErrorMsg("BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap: Invalid config key!\n");
	}
	for (valindex = 0; valindex < (int)BE_Cross_ArrayLen(g_sdlControlSchemeKeyMapCfgVals); ++valindex)
	{
		// Empty strings are used for SDL game controller buttons we wish to skip
		if (g_sdlControlSchemeKeyMapCfgVals[valindex] && !strcmp(buffer, g_sdlControlSchemeKeyMapCfgVals[valindex]))
		{
			g_refKeenCfg.altControlScheme.actionMappings[keyindex] = valindex;
			return;
		}
	}
	g_refKeenCfg.altControlScheme.actionMappings[keyindex] = BE_Cross_ArrayLen(g_sdlControlSchemeKeyMapCfgVals) - 1; // SPECIAL - A way to toggle this off
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeLeftStick(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.useLeftStick = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.useLeftStick = false;
	}
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeRightStick(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.useRightStick = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.useRightStick = false;
	}
}

#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
static void BEL_ST_ParseSetting_AlternativeControlSchemeAnalogMotion(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.analogMotion = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.analogMotion = false;
	}
}
#endif

// HACK (cfg file may be rewritten and we don't want to remove any setting)
static bool g_sdlIsManualGameVerModeSettingRead = false;
static void BEL_ST_ParseSetting_ManualGameVerMode(const char *keyprefix, const char *buffer)
{
	g_sdlIsManualGameVerModeSettingRead = true;
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.manualGameVerMode = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.manualGameVerMode = false;
	}
}

#ifdef BE_CROSS_ENABLE_FARPTR_CFG
// Same HACK again
static bool g_sdlIsFarPtrSegOffsetSettingRead = false;

static void BEL_ST_ParseSetting_FarPtrSegOffset(const char *keyprefix, const char *buffer)
{
	unsigned int segOffset;
	g_sdlIsFarPtrSegOffsetSettingRead = true;
	if (sscanf(buffer, "%X", &segOffset) == 1)
	{
		g_refKeenCfg.farPtrSegOffset = segOffset;
	}
}
#endif

typedef struct {
	const char *cfgPrefix; // Includes '=' sign
	void (*handlerPtr)(const char *, const char *);
} BESDLCfgEntry;

static BESDLCfgEntry g_sdlCfgEntries[] = {
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	{"fullscreen=", &BEL_ST_ParseSetting_FullScreen},
#endif
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
	{"fullres=", &BEL_ST_ParseSetting_FullRes},
#endif
	{"windowres=", &BEL_ST_ParseSetting_WindowRes},
#ifdef REFKEEN_ENABLE_LAUNCHER
//	{"launcherwindowres=", &BEL_ST_ParseSetting_LauncherWindowRes},
	{"launcherwindowtype=", &BEL_ST_ParseSetting_LauncherWindowType},
	{"launcherexeargs=", &BEL_ST_ParseSetting_LauncherExeArgs},
#endif
	{"lastselectedgameexe=", &BEL_ST_ParseSetting_LastSelectedGameExe},
	{"lastselectedgamever=", &BEL_ST_ParseSetting_LastSelectedGameVer},
	{"displaynum=", &BEL_ST_ParseSetting_DisplayNum},
	{"rememberdisplaynum=", &BEL_ST_ParseSetting_RememberDisplayNum},
	{"sdlrenderer=", &BEL_ST_ParseSetting_SDLRendererDriver},
	{"vsync=", &BEL_ST_ParseSetting_VSync},
	{"bilinear=", &BEL_ST_ParseSetting_Bilinear},
	{"scaletype=", &BEL_ST_ParseSetting_ScaleType},
	{"scalefactor=", &BEL_ST_ParseSetting_ScaleFactor},
	{"forcefullsoftscaling=", &BEL_ST_ParseSetting_ForceFullSoftScaling},
	{"mousegrab=", &BEL_ST_ParseSetting_MouseGrab},
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	{"absmousemotion=", &BEL_ST_ParseSetting_AbsMouseMotion},
#endif
	{"sndinterthreadbufferratio=", &BEL_ST_ParseSetting_SndInterThreadBufferRatio},
	{"sndsamplerate=", &BEL_ST_ParseSetting_SndSampleRate},
	{"sndsubsystem=", &BEL_ST_ParseSetting_SoundSubSystem},
	{"oplemulation=", &BEL_ST_ParseSetting_OPLEmulation},
#ifndef REFKEEN_RESAMPLER_NONE
	{"useresampler=", &BEL_ST_ParseSetting_UseResampler},
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	{"touchinput=", &BEL_ST_ParseSetting_TouchInputToggle},
	{"touchinputdebugging=", &BEL_ST_ParseSetting_TouchInputDebugging},
#endif
	{"altcontrolscheme=", &BEL_ST_ParseSetting_AlternativeControlScheme},

	// HACK: Copy-paste... if this is updated, check g_sdlControlSchemeKeyMapCfgKeyPrefixes too!!!
	{"altcontrolscheme_up=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_down=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_left=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_right=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#ifdef REFKEEN_HAS_VER_KDREAMS
	{"altcontrolscheme_jump=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_throw=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_stats=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	{"altcontrolscheme_fire=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_strafe=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	{"altcontrolscheme_drink=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_bolt=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_nuke=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_fastturn=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	{"altcontrolscheme_scrolls=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	{"altcontrolscheme_use=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_run=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon1=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon2=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon3=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon4=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon5=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon6=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_map=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	{"altcontrolscheme_funckeys=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
	{"altcontrolscheme_debugkeys=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},

	{"altcontrolscheme_lstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeLeftStick},
	{"altcontrolscheme_rstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeRightStick},
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	{"altcontrolscheme_analogmotion=", &BEL_ST_ParseSetting_AlternativeControlSchemeAnalogMotion},
#endif

	{"manualgamevermode=", &BEL_ST_ParseSetting_ManualGameVerMode},
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	{"farptrsegoffset=", &BEL_ST_ParseSetting_FarPtrSegOffset},
#endif
};



static void BEL_ST_ParseConfig(void)
{
	// Defaults
	g_refKeenCfg.isFullscreen = false; // Always exists internally, regardless of REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	g_refKeenCfg.fullWidth = 0;
	g_refKeenCfg.fullHeight = 0;
	g_refKeenCfg.winWidth = 0;
	g_refKeenCfg.winHeight = 0;
#ifdef REFKEEN_ENABLE_LAUNCHER
/*
	g_refKeenCfg.launcherWinWidth = 0;
	g_refKeenCfg.launcherWinHeight = 0;
*/
	g_refKeenCfg.launcherExeArgs[0] = '\0';
	g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_DEFAULT;
#endif
	g_refKeenCfg.lastSelectedGameExe[0] = '\0';
	g_refKeenCfg.lastSelectedGameVer = BE_GAMEVER_LAST;
	g_refKeenCfg.displayNum = 0;
	g_refKeenCfg.rememberDisplayNum = true;
	g_refKeenCfg.sdlRendererDriver = -1;
	g_refKeenCfg.vSync = VSYNC_OFF;
	g_refKeenCfg.isBilinear = true;
	g_refKeenCfg.scaleType = SCALE_ASPECT;
	g_refKeenCfg.scaleFactor = 2;
	g_refKeenCfg.forceFullSoftScaling = false;
	g_refKeenCfg.mouseGrab = MOUSEGRAB_AUTO;
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	g_refKeenCfg.absMouseMotion = false;
#endif
	g_refKeenCfg.sndInterThreadBufferRatio = 2;
	g_refKeenCfg.sndSampleRate = 48000; // 49716 may lead to unexpected behaviors on Android
	g_refKeenCfg.sndSubSystem = true;
	g_refKeenCfg.oplEmulation = true;
#ifndef REFKEEN_RESAMPLER_NONE
	g_refKeenCfg.useResampler = true;
#endif
#if (defined REFKEEN_CONFIG_ENABLE_TOUCHINPUT) && (defined REFKEEN_CONFIG_AUTODETECT_TOUCHINPUT_BY_DEFAULT)
	g_refKeenCfg.touchInputToggle = TOUCHINPUT_AUTO;
#else
	g_refKeenCfg.touchInputToggle = TOUCHINPUT_OFF;
#endif
	g_refKeenCfg.touchInputDebugging = false;
	g_refKeenCfg.altControlScheme.isEnabled = true;

#ifdef REFKEEN_HAS_VER_KDREAMS // Reserve the d-pad for weapons/feeds in Wolf3D/S3DNA
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_UP] = BE_ST_CTRL_BUT_DPAD_UP;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DOWN] = BE_ST_CTRL_BUT_DPAD_DOWN;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_LEFT] = BE_ST_CTRL_BUT_DPAD_LEFT;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RIGHT] = BE_ST_CTRL_BUT_DPAD_RIGHT;
#else // FIXME: HACK (extra 2 are for triggers)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_UP] = BE_ST_CTRL_BUT_MAX+2;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DOWN] = BE_ST_CTRL_BUT_MAX+2;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_LEFT] = BE_ST_CTRL_BUT_MAX+2;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RIGHT] = BE_ST_CTRL_BUT_MAX+2;
#endif
#ifdef REFKEEN_HAS_VER_KDREAMS
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_JUMP] = BE_ST_CTRL_BUT_A;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_THROW] = BE_ST_CTRL_BUT_B;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STATS] = BE_ST_CTRL_BUT_X;
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE] = BE_ST_CTRL_BUT_LSHOULDER;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE] = BE_ST_CTRL_BUT_B;
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DRINK] = BE_ST_CTRL_BUT_A;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_BOLT] = BE_ST_CTRL_BUT_X;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_NUKE] = BE_ST_CTRL_BUT_Y;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FASTTURN] = BE_ST_CTRL_BUT_RSHOULDER;
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_SCROLLS] = BE_ST_CTRL_BUT_MAX+1; // HACK for getting right trigger (technically an axis)
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_USE] = BE_ST_CTRL_BUT_MAX+1; // HACK for getting right trigger (technically an axis)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RUN] = BE_ST_CTRL_BUT_RSHOULDER;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON1] = BE_ST_CTRL_BUT_DPAD_DOWN;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON2] = BE_ST_CTRL_BUT_DPAD_RIGHT;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON3] = BE_ST_CTRL_BUT_DPAD_LEFT;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON4] = BE_ST_CTRL_BUT_DPAD_UP;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON5] = BE_ST_CTRL_BUT_X;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON6] = BE_ST_CTRL_BUT_Y;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_MAP] = BE_ST_CTRL_BUT_A;
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS] = BE_ST_CTRL_BUT_MAX; // HACK for getting left trigger (technically an axis)
#endif
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS] = BE_ST_CTRL_BUT_LSTICK;

	g_refKeenCfg.altControlScheme.useLeftStick = true;
	g_refKeenCfg.altControlScheme.useRightStick = false;
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	g_refKeenCfg.altControlScheme.analogMotion = false;
#endif

	g_refKeenCfg.manualGameVerMode = false;
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	g_refKeenCfg.farPtrSegOffset = BE_ST_DEFAULT_FARPTRSEGOFFSET;
#endif
	// Try to load config
	FILE *fp = BE_Cross_open_additionalfile_for_reading(REFKEEN_CONFIG_FILENAME);
	if (!fp)
	{
		return;
	}
	char buffer[80];
	while (fgets(buffer, sizeof(buffer), fp))
	{
		size_t len = strlen(buffer);
		if (!len)
		{
			continue;
		}
		if (buffer[len-1] == '\n')
		{
			buffer[len-1] = '\0';
		}
		for (int i = 0; i < (int)BE_Cross_ArrayLen(g_sdlCfgEntries); ++i)
		{
			if (!strncmp(g_sdlCfgEntries[i].cfgPrefix, buffer, strlen(g_sdlCfgEntries[i].cfgPrefix)))
			{
				g_sdlCfgEntries[i].handlerPtr(g_sdlCfgEntries[i].cfgPrefix, buffer+strlen(g_sdlCfgEntries[i].cfgPrefix));
				break;
			}
		}
	}
	fclose(fp);
}

static void BEL_ST_SaveConfig(void)
{
	// Try to save current settings just in case (first time file is created or new fields added)
	FILE *fp = BE_Cross_open_additionalfile_for_overwriting(REFKEEN_CONFIG_FILENAME);
	if (!fp)
	{
		return;
	}
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	fprintf(fp, "fullscreen=%s\n", g_refKeenCfg.isFullscreen ? "true" : "false");
#endif
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
	fprintf(fp, "fullres=%dx%d\n", g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight);
#endif
	fprintf(fp, "windowres=%dx%d\n", g_refKeenCfg.winWidth, g_refKeenCfg.winHeight);
#ifdef REFKEEN_ENABLE_LAUNCHER
	//fprintf(fp, "launcherwindowres=%dx%d\n", g_refKeenCfg.launcherWinWidth, g_refKeenCfg.launcherWinHeight);
	fprintf(fp, "launcherwindowtype=%s\n", g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_DEFAULT ? "default" : (g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_FULL ? "full" : "software"));
	fprintf(fp, "launcherexeargs=%s\n", g_refKeenCfg.launcherExeArgs);
#endif
	fprintf(fp, "lastselectedgameexe=%s\n", g_refKeenCfg.lastSelectedGameExe);
	fprintf(fp, "lastselectedgamever=%s\n", (g_refKeenCfg.lastSelectedGameVer != BE_GAMEVER_LAST) ? refkeen_gamever_strs[g_refKeenCfg.lastSelectedGameVer] : "");

	if (g_refKeenCfg.rememberDisplayNum)
		g_refKeenCfg.displayNum = BEL_ST_GetWindowDisplayNum();

	fprintf(fp, "displaynum=%d\n", g_refKeenCfg.displayNum);
	fprintf(fp, "rememberdisplaynum=%s\n", g_refKeenCfg.rememberDisplayNum ? "true" : "false");
	if (g_refKeenCfg.sdlRendererDriver < 0)
	{
		fprintf(fp, "sdlrenderer=auto\n");
	}
	else
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(g_refKeenCfg.sdlRendererDriver, &info); // g_refKeenCfg.sdlRendererDriver should be a valid renderer driver index here
		fprintf(fp, "sdlrenderer=%s\n", info.name);
	}
	fprintf(fp, "vsync=%s\n", (g_refKeenCfg.vSync == VSYNC_AUTO) ? "auto" : ((g_refKeenCfg.vSync == VSYNC_ON) ? "on" : "off"));
	fprintf(fp, "bilinear=%s\n", g_refKeenCfg.isBilinear ? "true" : "false");
	fprintf(fp, "scaletype=%s\n", (g_refKeenCfg.scaleType == SCALE_ASPECT) ? "aspect" : "fill");
	fprintf(fp, "scalefactor=%d\n", g_refKeenCfg.scaleFactor);
	fprintf(fp, "forcefullsoftscaling=%s\n", g_refKeenCfg.forceFullSoftScaling ? "true" : "false");
	fprintf(fp, "mousegrab=%s\n", (g_refKeenCfg.mouseGrab == MOUSEGRAB_AUTO) ? "auto" : ((g_refKeenCfg.mouseGrab == MOUSEGRAB_COMMONLY) ? "commonly" : "off"));
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	fprintf(fp, "absmousemotion=%s\n", g_refKeenCfg.absMouseMotion ? "true" : "false");
#endif
	fprintf(fp, "sndinterthreadbufferratio=%d\n", g_refKeenCfg.sndInterThreadBufferRatio);
	fprintf(fp, "sndsamplerate=%d\n", g_refKeenCfg.sndSampleRate);
	fprintf(fp, "sndsubsystem=%s\n", g_refKeenCfg.sndSubSystem ? "true" : "false");
	fprintf(fp, "oplemulation=%s\n", g_refKeenCfg.oplEmulation ? "true" : "false");
#ifndef REFKEEN_RESAMPLER_NONE
	fprintf(fp, "useresampler=%s\n", g_refKeenCfg.useResampler ? "true" : "false");
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	fprintf(fp, "touchinput=%s\n", (g_refKeenCfg.touchInputToggle == TOUCHINPUT_AUTO) ? "auto" : ((g_refKeenCfg.touchInputToggle == TOUCHINPUT_FORCED) ? "forced" : "off"));
	fprintf(fp, "touchinputdebugging=%s\n", g_refKeenCfg.touchInputDebugging ? "true" : "false");
#endif
	fprintf(fp, "altcontrolscheme=%s\n", g_refKeenCfg.altControlScheme.isEnabled ? "true" : "false");
	// Go through an array of keys
	for (int keyindex = 0; keyindex < BE_ST_CTRL_CFG_BUTMAP_AFTERLAST; ++keyindex)
	{
		fprintf(fp, "%s%s\n", g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex], g_sdlControlSchemeKeyMapCfgVals[g_refKeenCfg.altControlScheme.actionMappings[keyindex]]);
	}
	fprintf(fp, "altcontrolscheme_lstick=%s\n", g_refKeenCfg.altControlScheme.useLeftStick ? "true" : "false");
	fprintf(fp, "altcontrolscheme_rstick=%s\n", g_refKeenCfg.altControlScheme.useRightStick ? "true" : "false");
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	fprintf(fp, "altcontrolscheme_analogmotion=%s\n", g_refKeenCfg.altControlScheme.analogMotion ? "true" : "false");
#endif
	if (g_sdlIsManualGameVerModeSettingRead)
	{
		// This should be a relatively hidden setting
		fprintf(fp, "manualgamevermode=%s\n", g_refKeenCfg.manualGameVerMode ? "true" : "false");
	}
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	if (g_sdlIsFarPtrSegOffsetSettingRead)
	{
		// Another hidden setting
		fprintf(fp, "farptrsegoffset=%X\n", g_refKeenCfg.farPtrSegOffset);
	}
#endif
	fclose(fp);
}


static BESDLMouseModeEnum g_sdlMouseMode = BE_ST_MOUSEMODE_ABS_WITH_CURSOR;

void BEL_ST_SetMouseMode(BESDLMouseModeEnum mode)
{
	if (g_sdlMouseMode == mode)
		return;

	g_sdlMouseMode = mode;
	if (mode == BE_ST_MOUSEMODE_ABS_WITHOUT_CURSOR)
		SDL_ShowCursor(SDL_FALSE);
	else if (mode == BE_ST_MOUSEMODE_ABS_WITH_CURSOR)
		SDL_ShowCursor(SDL_TRUE);

	SDL_SetRelativeMouseMode((mode == BE_ST_MOUSEMODE_REL) ? SDL_TRUE : SDL_FALSE);
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


/*static*/ void BEL_ST_HandleEmuKeyboardEvent(bool isPressed, bool isRepeated, emulatedDOSKeyEvent keyEvent)
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

static void BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls(void);

// May be similar to PrepareControllerMapping, but a bit different:
// Used in order to replace controller mapping with another one internally
// (e.g., showing helper function keys during gameplay, or hiding such keys)
void BEL_ST_ReplaceControllerMapping(const BE_ST_ControllerMapping *mapping)
{
	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = mapping;

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
			BEL_ST_ReplaceControllerMapping(map->otherMappingPtr);
		return true; // Confirm either way
	}
	return false;
}

static void BEL_ST_AltControlScheme_ClearBinaryStates(void);

static void BEL_ST_DoHideTouchUI(void)
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

static void BEL_ST_CheckForHidingTouchUI(void)
{
	if ((g_refKeenCfg.touchInputToggle != TOUCHINPUT_AUTO) || !g_sdlShowTouchUI)
		return;
	BEL_ST_DoHideTouchUI();
}

static void BEL_ST_ConditionallyAddJoystick(int device_index)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			if (!g_sdlJoysticks[i])
			{
				g_sdlJoysticks[i] = SDL_JoystickOpen(device_index);
				g_sdlJoysticksInstanceIds[i] = SDL_JoystickInstanceID(g_sdlJoysticks[i]);
				BEL_ST_CheckForHidingTouchUI();
				break;
			}
	}
	else if (SDL_IsGameController(device_index))
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			if (!g_sdlControllers[i])
			{
				g_sdlControllers[i] = SDL_GameControllerOpen(device_index);
				g_sdlJoysticksInstanceIds[i] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i]));

				g_sdlShowControllerUI = true;
				g_sdlForceGfxControlUiRefresh = true;
				BEL_ST_ConditionallyShowAltInputPointer();
				BEL_ST_CheckForHidingTouchUI();
				break;
			}
	}
}


void BEL_ST_ReleasePressedKeysInTextInputUI(void);
void BEL_ST_ReleasePressedKeysInDebugKeysUI(void);

/* WARNING: In theory there may be a Clear -> HandleEntry -> Clear cycle,
 * but it can never occur since isPressed is set to false
 */
static void BEL_ST_AltControlScheme_ClearBinaryStates(void)
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
		for (int but = 0; but < BE_ST_CTRL_BUT_MAX; ++but)
		{
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->buttons[but], 0, &g_sdlControllersButtonsStates[but]);
		}
		// Repeat with analog axes
		for (int axis = 0; axis < BE_ST_CTRL_AXIS_MAX; ++axis)
		{
			// Is pressed in the negative direction?
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][0], 0, &g_sdlControllersAxesStates[axis][0]);
			// Repeat with positive
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][1], 0, &g_sdlControllersAxesStates[axis][1]);
		}
	}

	// Check this for ALL possible mappings
	if (g_sdlDefaultMappingBinaryState)
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, 0, &g_sdlDefaultMappingBinaryState);

	memset(g_sdlControllersButtonsStates, 0, sizeof(g_sdlControllersButtonsStates));
	memset(g_sdlControllersAxesStates, 0, sizeof(g_sdlControllersAxesStates));
}

static void BEL_ST_AltControlScheme_CleanUp(void)
{
	if (!g_sdlControllerSchemeNeedsCleanUp)
		return;

	BEL_ST_AltControlScheme_ClearBinaryStates();

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


void BE_ST_AltControlScheme_Push(void)
{
	//if (!g_refKeenCfg.altControlScheme.isEnabled && (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF))
	//	return;

	BEL_ST_AltControlScheme_CleanUp();

	++g_sdlControllerMappingPtrsStack.currPtr;
	if (g_sdlControllerMappingPtrsStack.currPtr == g_sdlControllerMappingPtrsStack.endPtr)
	{
		BE_ST_ExitWithErrorMsg("BE_ST_AltControlScheme_Push: Out of stack bounds!\n");
	}
}

void BE_ST_AltControlScheme_Pop(void)
{
	//if (!g_refKeenCfg.altControlScheme.isEnabled && (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF))
	//	return;

	BEL_ST_AltControlScheme_CleanUp();

	if (g_sdlControllerMappingPtrsStack.currPtr == &g_sdlControllerMappingPtrsStack.stack[0])
	{
		BE_ST_ExitWithErrorMsg("BE_ST_AltControlScheme_Pop: Popped more than necessary!\n");
	}
	--g_sdlControllerMappingPtrsStack.currPtr;

	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr;

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

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_PrepareControllerMapping(const BE_ST_ControllerMapping *mapping)
{
	//if (!g_refKeenCfg.altControlScheme.isEnabled && (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF))
	//	return;

	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr = mapping;

	BEL_ST_AltControlScheme_ConditionallyShowOnScreenControls();

	g_sdlControllerSchemeNeedsCleanUp = true;
}


void BE_ST_AltControlScheme_UpdateVirtualMouseCursor(int x, int y)
{
	g_sdlVirtualMouseCursorState[0] = x;
	g_sdlVirtualMouseCursorState[1] = y;
}


/*** A couple of special handlers call from BE_ST_PollEvents ***/
static void BEL_ST_AltControlScheme_HandleTextInputEvent(int but, bool isPressed)
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


static void BEL_ST_AltControlScheme_HandleDebugKeysEvent(int but, bool isPressed)
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

/* Shared with mouse and touch inputs, where mouse's touchId is 0
 * (documentation of SDL_GetTouchDevice says 0 is returned for an invalid input index)
 */

void BEL_ST_CheckPressedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckMovedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckPressedPointerInDebugKeysUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckMovedPointerInDebugKeysUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInDebugKeysUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckPressedPointerInControllerUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckMovedPointerInControllerUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInControllerUI(SDL_TouchID touchId, SDL_FingerID fingerId);
void BEL_ST_CheckPressedPointerInTouchControls(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, bool forceAbsoluteFingerPositioning);
void BEL_ST_CheckMovedPointerInTouchControls(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y);
void BEL_ST_CheckReleasedPointerInTouchControls(SDL_TouchID touchId, SDL_FingerID fingerId);

static bool BEL_ST_CheckCommonPointerPressCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	if (!g_sdlShowControllerUI && !g_sdlShowTouchUI)
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

static bool BEL_ST_CheckCommonPointerReleaseCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	if (!g_sdlShowControllerUI && !g_sdlShowTouchUI)
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

static bool BEL_ST_CheckCommonPointerMoveCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	if (!g_sdlShowControllerUI && !g_sdlShowTouchUI)
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

#ifdef BE_CROSS_ENABLE_FARPTR_CFG
uint16_t BE_ST_Compat_GetFarPtrRelocationSegOffset(void)
{
	return g_refKeenCfg.farPtrSegOffset;
}
#endif
