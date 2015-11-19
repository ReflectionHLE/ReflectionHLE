#include <stdio.h>
#include <string.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_gamever.h"
#include "be_st.h"
#include "be_st_sdl_private.h"

#define BE_ST_MAXJOYSTICKS 8
#define BE_ST_EMU_JOYSTICK_RANGEMAX 5000 // id_in.c MaxJoyValue
#define BE_ST_DEFAULT_FARPTRSEGOFFSET 0x14

#if (defined REFKEEN_VER_CATARM) || (defined REFKEEN_VER_CATAPOC)
#define BE_ST_ENABLE_FARPTR_CFG 1
#endif

/*** Last BE_ST_PollEvents time ***/
static uint32_t g_sdlLastPollEventsTime;

static void (*g_sdlKeyboardInterruptFuncPtr)(uint8_t) = 0;

// Defined internally, but also used in launcher code. extern is required for C++.
#ifdef __cplusplus
extern
#endif
const int g_sdlJoystickAxisBinaryThreshold = 16384, g_sdlJoystickAxisDeadZone = 3200, g_sdlJoystickAxisMax = 32767, g_sdlJoystickAxisMaxMinusDeadZone = 32767-3200;

static SDL_Joystick *g_sdlJoysticks[BE_ST_MAXJOYSTICKS];
SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS]; // Also used in launcher
SDL_JoystickID g_sdlJoysticksInstanceIds[BE_ST_MAXJOYSTICKS];

/*** These represent button states, although a call to BEL_ST_AltControlScheme_CleanUp zeros these out ***/
static bool g_sdlControllersButtonsStates[SDL_CONTROLLER_BUTTON_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
static bool g_sdlControllersAxesStates[SDL_CONTROLLER_AXIS_MAX][2];

/*** These are similar states for a few mouse buttons, required as relative mouse mode is toggled on or off in the middle ***/
static bool g_sdlMouseButtonsStates[3];

static bool g_sdlRelativeMouseMotion = false;

/*** Emulated mouse and joysticks states (mouse motion state is split for technical reasons) ***/
static int g_sdlEmuMouseButtonsState;
static int16_t g_sdlEmuMouseMotionAccumulatedState[2];
static int16_t g_sdlEmuMouseMotionAbsoluteState[2];
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
// SPECIAL - If Back button (or similar) is pressed while an on-screen keyboard is shown, remember this!
static int g_sdlOnScreenKeyboardManualExitScanCode;

#define NUM_OF_CONTROLLER_MAPS_IN_STACK 8

static bool g_sdlControllerSchemeNeedsCleanUp;

static struct {
	const BE_ST_ControllerMapping *stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];
	const BE_ST_ControllerMapping **currPtr;
	const BE_ST_ControllerMapping **endPtr;
} g_sdlControllerMappingPtrsStack;

// Current mapping, doesn't have to be *(g_sdlControllerMappingPtrsStack.currPtr) as game code can change this (e.g., helper keys)
const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr;

static BE_ST_ControllerMapping g_sdlControllerMappingDefault;
// HACK - These "mapping" is used for identification of on-screen keyboards (using pointers comparisons)
BE_ST_ControllerMapping g_beStControllerMappingTextInput;
BE_ST_ControllerMapping g_beStControllerMappingDebugKeys;

// The index is taken off the enum above so ENSURE THESE ARE CONSISTENT!
//
// HACK: If this is updated, also check g_sdlCfgEntries!!!
static const char *g_sdlControlSchemeKeyMapCfgKeyPrefixes[] = {
#ifdef REFKEEN_VER_KDREAMS
	"altcontrolscheme_jump=",
	"altcontrolscheme_throw=",
	"altcontrolscheme_stats=",
#else
	"altcontrolscheme_fire=",
	"altcontrolscheme_strafe=",
	"altcontrolscheme_drink=",
	"altcontrolscheme_bolt=", // Zapper in the Adventures Series
	"altcontrolscheme_nuke=", // Xterminator in the Adventures Series
	"altcontrolscheme_fastturn=",
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	"altcontrolscheme_scrolls=",
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	"altcontrolscheme_funckeys=",
#endif
	"altcontrolscheme_debugkeys=",
	0,
};


extern SDL_Window *g_sdlWindow;

static uint8_t g_sdlLastKeyScanCodeBeforeAnyReset; // May be reset by BE_ST_BiosScanCode

void BE_ST_InitGfx(void);
void BE_ST_InitAudio(void);
void BE_ST_InitTiming(void);
void BE_ST_ShutdownAudio(void);
void BE_ST_ShutdownGfx(void);
static void BEL_ST_ConditionallyAddJoystick(int device_index);
static void BEL_ST_ParseConfig(void);
static void BEL_ST_SaveConfig(void);
void BEL_ST_SetRelativeMouseMotion(bool enable);

#ifdef REFKEEN_VER_KDREAMS
#include "../rsrc/reflection-kdreams-icon-32x32.h"
#elif defined REFKEEN_VER_CAT3D
#include "../rsrc/reflection-cat3d-icon-32x32.h"
#elif defined REFKEEN_VER_CATABYSS
#include "../rsrc/reflection-catabyss-icon-32x32.h"
#elif defined REFKEEN_VER_CATARM
#include "../rsrc/reflection-catarm-icon-32x32.h"
#elif defined REFKEEN_VER_CATAPOC
#include "../rsrc/reflection-catapoc-icon-32x32.h"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif


SDL_Surface *g_be_sdl_windowIconSurface = NULL;

void BE_ST_InitCommon(void)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER) < 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL backend initialization failed,\n%s\n", SDL_GetError());
		exit(0);
	}

	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL game controller subsystem initialization (including joystick subsystem) failed, disabled,\n%s\n", SDL_GetError());
	}

	g_be_sdl_windowIconSurface = SDL_CreateRGBSurfaceFrom(RefKeen_Window_Icon, 32, 32, 8, 32, 0, 0, 0, 0);
	SDL_SetPaletteColors(g_be_sdl_windowIconSurface->format->palette, RefKeen_Window_Icon_Palette, '0', 9);
	SDL_SetColorKey(g_be_sdl_windowIconSurface, SDL_TRUE, SDL_MapRGB(g_be_sdl_windowIconSurface->format, 0xCC, 0xFF, 0xCC));

	// MUST be called BEFORE parsing config (of course!)
	BE_Cross_PrepareAppPaths();

	BEL_ST_ParseConfig();
	// This technically requires SDL 2.0.2, which has been available for a year now; Should be called BEFORE any SDL_CONTROLLERDEVICEADDED event should arrive (so e.g., before SDL_PollEvent).
	FILE *fp = BE_Cross_open_additionalfile_for_reading("gamecontrollerdb.txt");
	if (fp)
		SDL_GameControllerAddMappingsFromRW(SDL_RWFromFP(fp, SDL_TRUE), 1);
}

void BE_ST_PrepareForGameStartup(void)
{
	BE_ST_InitGfx();
	BE_ST_InitAudio();
	BE_ST_InitTiming();

	// Preparing a controller scheme (with no special UI) in case the relevant feature is enabled
	g_sdlControllerMappingPtrsStack.stack[0] = &g_sdlControllerMappingDefault;
	g_sdlControllerMappingPtrsStack.currPtr = &g_sdlControllerMappingPtrsStack.stack[0];
	g_sdlControllerMappingPtrsStack.endPtr = &g_sdlControllerMappingPtrsStack.stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];
	g_sdlControllerMappingActualCurr = g_sdlControllerMappingPtrsStack.stack[0];

	g_sdlControllerSchemeNeedsCleanUp = false;

	memset(g_sdlControllersButtonsStates, 0, sizeof(g_sdlControllersButtonsStates));
	memset(g_sdlControllersAxesStates, 0, sizeof(g_sdlControllersAxesStates));

	// BEFORE checking for more joysticks been attached/removed in BE_ST_PollEvents, add what's currently available
	int nOfJoysticks = SDL_NumJoysticks();
	if (nOfJoysticks > BE_ST_MAXJOYSTICKS)
		nOfJoysticks = BE_ST_MAXJOYSTICKS;
	for (int i = 0; i < nOfJoysticks; ++i)
		BEL_ST_ConditionallyAddJoystick(i);

	BEL_ST_SetRelativeMouseMotion(true);

	// Reset these first
	memset(g_sdlEmuKeyboardStateByScanCode, 0, sizeof(g_sdlEmuKeyboardStateByScanCode));

	g_sdlEmuMouseButtonsState = 0;
	memset(g_sdlEmuMouseMotionAbsoluteState, 0, sizeof(g_sdlEmuMouseMotionAbsoluteState));
	g_sdlEmuJoyButtonsState = 0;
	// A bit tricky, should be reported as centered *if* any joystick is connected (and *not* while using modern controller scheme)
	// Note: A single controller may support up to all 4
	g_sdlEmuJoyMotionState[0] = g_sdlEmuJoyMotionState[1] = g_sdlEmuJoyMotionState[2] = g_sdlEmuJoyMotionState[3] = 0;
	for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		if (g_sdlJoysticks[i])
		{
			g_sdlEmuJoyMotionState[0] = g_sdlEmuJoyMotionState[1] = g_sdlEmuJoyMotionState[2] = g_sdlEmuJoyMotionState[3] = BE_ST_EMU_JOYSTICK_RANGEMAX/2; // These ones aren't centered around 0...
			break;
		}
	// Then use this to reset/update some states, and detect joysticks
	BE_ST_PollEvents();
	// Events may gradually fill these, especially after messing with a mouse cursor, so reset AFTER poll
	g_sdlEmuMouseMotionAccumulatedState[0] = g_sdlEmuMouseMotionAccumulatedState[1] = 0;

}

void BE_ST_ShutdownAll(void)
{
	BEL_ST_SetRelativeMouseMotion(false);
	BE_ST_ShutdownAudio();
	BE_ST_ShutdownGfx();
	SDL_FreeSurface(g_be_sdl_windowIconSurface);
	SDL_Quit();
}

static void BEL_ST_AltControlScheme_CleanUp(void);

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
			case SDL_JOYHATMOTION:
				if (event.jhat.value == SDL_HAT_CENTERED)
					break; // Ignore
				// Fall-through
			case SDL_KEYDOWN:
				if (event.key.repeat)
					break; // Ignore
				// Fall-through
			case SDL_MOUSEBUTTONDOWN:
			case SDL_JOYBUTTONDOWN:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_QUIT:
				keepRunning = false;
				break;
			default: ;
			}
		}
		SDL_Delay(1);
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
	"a", "b", "x", "y", 0, 0, 0, "lstick", "rstick", "lshoulder", "rshoulder", 0, 0, 0, 0,
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
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

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

static void BEL_ST_ParseSetting_FullRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.fullWidth, &g_refKeenCfg.fullHeight);
}

static void BEL_ST_ParseSetting_WindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.winWidth, &g_refKeenCfg.winHeight);
}

static void BEL_ST_ParseSetting_LauncherWindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.launcherWinWidth, &g_refKeenCfg.launcherWinHeight);
}

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
	sscanf(buffer, "%d", &g_refKeenCfg.displayNum);
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

static void BEL_ST_ParseSetting_AutolockCursor(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.autolockCursor = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.autolockCursor = false;
	}
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
	for (valindex = 0; valindex < (int)(sizeof(g_sdlControlSchemeKeyMapCfgVals)/sizeof(*g_sdlControlSchemeKeyMapCfgVals)); ++valindex)
	{
		// Empty strings are used for SDL game controller buttons we wish to skip
		if (g_sdlControlSchemeKeyMapCfgVals[valindex] && !strcmp(buffer, g_sdlControlSchemeKeyMapCfgVals[valindex]))
		{
			g_refKeenCfg.altControlScheme.actionMappings[keyindex] = valindex;
			return;
		}
	}
	g_refKeenCfg.altControlScheme.actionMappings[keyindex] = (sizeof(g_sdlControlSchemeKeyMapCfgVals)/sizeof(*g_sdlControlSchemeKeyMapCfgVals)) - 1; // SPECIAL - A way to toggle this off
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeDpad(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.useDpad = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.useDpad = false;
	}
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

#ifdef REFKEEN_VER_CATACOMB_ALL
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

#ifdef BE_ST_ENABLE_FARPTR_CFG
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
	{"fullscreen=", &BEL_ST_ParseSetting_FullScreen},
	{"fullres=", &BEL_ST_ParseSetting_FullRes},
	{"windowres=", &BEL_ST_ParseSetting_WindowRes},
	{"launcherwindowres=", &BEL_ST_ParseSetting_LauncherWindowRes},
	{"launcherwindowtype=", &BEL_ST_ParseSetting_LauncherWindowType},
	{"launcherexeargs=", &BEL_ST_ParseSetting_LauncherExeArgs},
	{"lastselectedgamever=", &BEL_ST_ParseSetting_LastSelectedGameVer},
	{"displaynum=", &BEL_ST_ParseSetting_DisplayNum},
	{"sdlrenderer=", &BEL_ST_ParseSetting_SDLRendererDriver},
	{"vsync=", &BEL_ST_ParseSetting_VSync},
	{"bilinear=", &BEL_ST_ParseSetting_Bilinear},
	{"scaletype=", &BEL_ST_ParseSetting_ScaleType},
	{"scalefactor=", &BEL_ST_ParseSetting_ScaleFactor},
	{"forcefullsoftscaling=", &BEL_ST_ParseSetting_ForceFullSoftScaling},
	{"autolock=", &BEL_ST_ParseSetting_AutolockCursor},
	{"sndsamplerate=", &BEL_ST_ParseSetting_SndSampleRate},
	{"sndsubsystem=", &BEL_ST_ParseSetting_SoundSubSystem},
	{"oplemulation=", &BEL_ST_ParseSetting_OPLEmulation},
	{"altcontrolscheme=", &BEL_ST_ParseSetting_AlternativeControlScheme},

	// HACK: Copy-paste... if this is updated, check g_sdlControlSchemeKeyMapCfgKeyPrefixes too!!!
#ifdef REFKEEN_VER_KDREAMS
	{"altcontrolscheme_jump=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_throw=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_stats=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#else
	{"altcontrolscheme_fire=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_strafe=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_drink=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_bolt=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_nuke=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_fastturn=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	{"altcontrolscheme_scrolls=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	{"altcontrolscheme_funckeys=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
	{"altcontrolscheme_debugkeys=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},

	{"altcontrolscheme_dpad=", &BEL_ST_ParseSetting_AlternativeControlSchemeDpad},
	{"altcontrolscheme_lstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeLeftStick},
	{"altcontrolscheme_rstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeRightStick},
#ifdef REFKEEN_VER_CATACOMB_ALL
	{"altcontrolscheme_analogmotion=", &BEL_ST_ParseSetting_AlternativeControlSchemeAnalogMotion},
#endif

	{"manualgamevermode=", &BEL_ST_ParseSetting_ManualGameVerMode},
#ifdef BE_ST_ENABLE_FARPTR_CFG
	{"farptrsegoffset=", &BEL_ST_ParseSetting_FarPtrSegOffset},
#endif
};



static void BEL_ST_ParseConfig(void)
{
	// Defaults
	g_refKeenCfg.isFullscreen = false;
	g_refKeenCfg.fullWidth = 0;
	g_refKeenCfg.fullHeight = 0;
	g_refKeenCfg.winWidth = 0;
	g_refKeenCfg.winHeight = 0;
	g_refKeenCfg.launcherWinWidth = 0;
	g_refKeenCfg.launcherWinHeight = 0;
	g_refKeenCfg.launcherExeArgs[0] = '\0';
	g_refKeenCfg.lastSelectedGameVer = BE_GAMEVER_LAST;
	g_refKeenCfg.displayNum = 0;
	g_refKeenCfg.sdlRendererDriver = -1;
	g_refKeenCfg.vSync = VSYNC_AUTO;
	g_refKeenCfg.isBilinear = true;
	g_refKeenCfg.scaleType = SCALE_ASPECT;
	g_refKeenCfg.scaleFactor = 2;
	g_refKeenCfg.forceFullSoftScaling = false;
	g_refKeenCfg.autolockCursor = false;
	g_refKeenCfg.sndSampleRate = 49716; // TODO should be a shared define
	g_refKeenCfg.sndSubSystem = true;
	g_refKeenCfg.oplEmulation = true;
	g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_DEFAULT;
	g_refKeenCfg.altControlScheme.isEnabled = true;

#ifdef REFKEEN_VER_KDREAMS
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_JUMP] = SDL_CONTROLLER_BUTTON_A;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_THROW] = SDL_CONTROLLER_BUTTON_B;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STATS] = SDL_CONTROLLER_BUTTON_X;
#else
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE] = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE] = SDL_CONTROLLER_BUTTON_B;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DRINK] = SDL_CONTROLLER_BUTTON_A;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_BOLT] = SDL_CONTROLLER_BUTTON_X;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_NUKE] = SDL_CONTROLLER_BUTTON_Y;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FASTTURN] = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_SCROLLS] = SDL_CONTROLLER_BUTTON_MAX+1; // HACK for getting right trigger (technically an axis)
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS] = SDL_CONTROLLER_BUTTON_MAX; // HACK for getting left trigger (technically an axis)
#endif
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS] = SDL_CONTROLLER_BUTTON_LEFTSTICK;

	g_refKeenCfg.altControlScheme.useDpad = true;
	g_refKeenCfg.altControlScheme.useLeftStick = true;
	g_refKeenCfg.altControlScheme.useRightStick = false;
#ifdef REFKEEN_VER_CATACOMB_ALL
	g_refKeenCfg.altControlScheme.analogMotion = false;
#endif

	g_refKeenCfg.manualGameVerMode = false;
#ifdef BE_ST_ENABLE_FARPTR_CFG
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
		for (int i = 0; i < (int)(sizeof(g_sdlCfgEntries)/sizeof(*g_sdlCfgEntries)); ++i)
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
	fprintf(fp, "fullscreen=%s\n", g_refKeenCfg.isFullscreen ? "true" : "false");
	fprintf(fp, "fullres=%dx%d\n", g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight);
	fprintf(fp, "windowres=%dx%d\n", g_refKeenCfg.winWidth, g_refKeenCfg.winHeight);
	fprintf(fp, "launcherwindowres=%dx%d\n", g_refKeenCfg.launcherWinWidth, g_refKeenCfg.launcherWinHeight);
	fprintf(fp, "launcherwindowtype=%s\n", g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_DEFAULT ? "default" : (g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_FULL ? "full" : "software"));
	fprintf(fp, "launcherexeargs=%s\n", g_refKeenCfg.launcherExeArgs);
	fprintf(fp, "lastselectedgamever=%s\n", (g_refKeenCfg.lastSelectedGameVer != BE_GAMEVER_LAST) ? refkeen_gamever_strs[g_refKeenCfg.lastSelectedGameVer] : "");
	fprintf(fp, "displaynum=%d\n", g_refKeenCfg.displayNum);
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
	fprintf(fp, "vsync=%s\n", g_refKeenCfg.vSync == VSYNC_AUTO ? "auto" : (g_refKeenCfg.vSync == VSYNC_ON ? "on" : "off"));
	fprintf(fp, "bilinear=%s\n", g_refKeenCfg.isBilinear ? "true" : "false");
	fprintf(fp, "scaletype=%s\n", (g_refKeenCfg.scaleType == SCALE_ASPECT) ? "aspect" : "fill");
	fprintf(fp, "scalefactor=%d\n", g_refKeenCfg.scaleFactor);
	fprintf(fp, "forcefullsoftscaling=%s\n", g_refKeenCfg.forceFullSoftScaling ? "true" : "false");
	fprintf(fp, "autolock=%s\n", g_refKeenCfg.autolockCursor ? "true" : "false");
	fprintf(fp, "sndsamplerate=%d\n", g_refKeenCfg.sndSampleRate);
	fprintf(fp, "sndsubsystem=%s\n", g_refKeenCfg.sndSubSystem ? "true" : "false");
	fprintf(fp, "oplemulation=%s\n", g_refKeenCfg.oplEmulation ? "true" : "false");
	fprintf(fp, "altcontrolscheme=%s\n", g_refKeenCfg.altControlScheme.isEnabled ? "true" : "false");
	// Go through an array of keys
	for (int keyindex = 0; keyindex < BE_ST_CTRL_CFG_BUTMAP_AFTERLAST; ++keyindex)
	{
		fprintf(fp, "%s%s\n", g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex], g_sdlControlSchemeKeyMapCfgVals[g_refKeenCfg.altControlScheme.actionMappings[keyindex]]);
	}
	fprintf(fp, "altcontrolscheme_dpad=%s\n", g_refKeenCfg.altControlScheme.useDpad ? "true" : "false");
	fprintf(fp, "altcontrolscheme_lstick=%s\n", g_refKeenCfg.altControlScheme.useLeftStick ? "true" : "false");
	fprintf(fp, "altcontrolscheme_rstick=%s\n", g_refKeenCfg.altControlScheme.useRightStick ? "true" : "false");
#ifdef REFKEEN_VER_CATACOMB_ALL
	fprintf(fp, "altcontrolscheme_analogmotion=%s\n", g_refKeenCfg.altControlScheme.analogMotion ? "true" : "false");
#endif
	if (g_sdlIsManualGameVerModeSettingRead)
	{
		// This should be a relatively hidden setting
		fprintf(fp, "manualgamevermode=%s\n", g_refKeenCfg.manualGameVerMode ? "true" : "false");
	}
#ifdef BE_ST_ENABLE_FARPTR_CFG
	if (g_sdlIsFarPtrSegOffsetSettingRead)
	{
		// Another hidden setting
		fprintf(fp, "farptrsegoffset=%X\n", g_refKeenCfg.farPtrSegOffset);
	}
#endif
	fclose(fp);
}


void BEL_ST_SetRelativeMouseMotion(bool enable)
{
	if (g_sdlRelativeMouseMotion == enable)
		return;
	g_sdlRelativeMouseMotion = enable;

	if (g_refKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		SDL_SetRelativeMouseMode(enable ? SDL_TRUE : SDL_FALSE);
	}
	else
	{
		SDL_ShowCursor(!enable);
	}
	// Reset these
	g_sdlMouseButtonsStates[0] = g_sdlMouseButtonsStates[1] = g_sdlMouseButtonsStates[2] = 0;
	// Also that (HACK)
	g_sdlEmuMouseButtonsState = 0;
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
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
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

void BE_ST_GetMouseDelta(int16_t *x, int16_t *y)
{
	BE_ST_PollEvents();

	if (x)
		*x = g_sdlEmuMouseMotionAccumulatedState[0] + g_sdlEmuMouseMotionAbsoluteState[0];
	if (y)
		*y = g_sdlEmuMouseMotionAccumulatedState[1] + g_sdlEmuMouseMotionAbsoluteState[1];

	g_sdlEmuMouseMotionAccumulatedState[0] = g_sdlEmuMouseMotionAccumulatedState[1] = 0;
}

uint16_t BE_ST_GetMouseButtons(void)
{
	BE_ST_PollEvents();

	return g_sdlEmuMouseButtonsState;
}

void BE_ST_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp)
{
	BE_ST_PollEvents();

	if (xp)
		*xp = g_sdlEmuJoyMotionState[2*joy];
	if (yp)
		*yp = g_sdlEmuJoyMotionState[2*joy+1];
}

uint16_t BE_ST_GetJoyButtons(uint16_t joy)
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

static void BEL_ST_AltControlScheme_ConditionallyShowControllerUI(void);

// May be similar to PrepareControllerMapping, but a bit different:
// Used in order to replace controller mapping with another one internally
// (e.g., showing helper function keys during gameplay, or hiding such keys)
void BEL_ST_ReplaceControllerMapping(const BE_ST_ControllerMapping *mapping)
{
	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = mapping;

	BEL_ST_AltControlScheme_ConditionallyShowControllerUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

static bool BEL_ST_AltControlScheme_HandleEntry(const BE_ST_ControllerSingleMap *map, int value, bool *lastBinaryStatusPtr)
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
		g_sdlEmuMouseMotionAbsoluteState[map->val] = (value <= g_sdlJoystickAxisDeadZone) ? 0 : (value - g_sdlJoystickAxisDeadZone) * map->secondaryVal / g_sdlJoystickAxisMaxMinusDeadZone;
		return true;
	case BE_ST_CTRL_MAP_OTHERMAPPING:
		if (!prevBinaryStatus && (*lastBinaryStatusPtr))
			BEL_ST_ReplaceControllerMapping(map->otherMappingPtr);
		return true; // Confirm either way
	}
	return false;
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

				extern bool g_sdlShowControllerUI;
				g_sdlShowControllerUI = true;
				extern bool g_sdlForceGfxControlUiRefresh;
				g_sdlForceGfxControlUiRefresh = true;
				void BEL_ST_ConditionallyShowAltInputPointer(void);
				BEL_ST_ConditionallyShowAltInputPointer();

				break;
			}
	}
}


/* WARNING: In theory there may be a Clear -> HandleEntry -> Clear cycle,
 * but it can never occur since isPressed is set to false
 */
static void BEL_ST_AltControlScheme_ClearBinaryStates(void)
{
	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	dosKeyEvent.dosScanCode = g_sdlOnScreenKeyboardManualExitScanCode;

	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		void BEL_ST_ReleasePressedKeysInTextInputUI(void);
		BEL_ST_ReleasePressedKeysInTextInputUI();
		// SPECIAL
		if (g_sdlOnScreenKeyboardManualExitScanCode)
		{
			BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
			g_sdlOnScreenKeyboardManualExitScanCode = 0;
		}
	}
	else if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		void BEL_ST_ReleasePressedKeysInDebugKeysUI(void);
		BEL_ST_ReleasePressedKeysInDebugKeysUI();
		// SPECIAL
		if (g_sdlOnScreenKeyboardManualExitScanCode)
		{
			BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
			g_sdlOnScreenKeyboardManualExitScanCode = 0;
		}
	}
	else // Otherwise simulate key releases based on the mapping
	{
		// But also don't forget this (e.g., if mouse is used)
		void BEL_ST_ReleasePressedKeysInControllerUI(void);
		BEL_ST_ReleasePressedKeysInControllerUI();

		// Simulate binary key/button/other action "releases" and clear button states.
		// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than an unexpected key press.
		for (int but = 0; but < SDL_CONTROLLER_BUTTON_MAX; ++but)
		{
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->buttons[but], 0, &g_sdlControllersButtonsStates[but]);
		}
		// Repeat with analog axes
		for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; ++axis)
		{
			// Is pressed in the negative direction?
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][0], 0, &g_sdlControllersAxesStates[axis][0]);
			// Repeat with positive
			BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][1], 0, &g_sdlControllersAxesStates[axis][1]);
		}
	}

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


static void BEL_ST_AltControlScheme_ConditionallyShowControllerUI(void)
{
	if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
	{
		extern void BEL_ST_PrepareToShowTextInputUI(void);
		BEL_ST_PrepareToShowTextInputUI();
		g_sdlOnScreenKeyboardLastPressedDirButton = SDL_CONTROLLER_BUTTON_INVALID;
		g_sdlOnScreenKeyboardManualExitScanCode = 0;
	}
	else if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
	{
		extern void BEL_ST_PrepareToShowDebugKeysUI(void);
		BEL_ST_PrepareToShowDebugKeysUI();
		g_sdlOnScreenKeyboardLastPressedDirButton = SDL_CONTROLLER_BUTTON_INVALID;
		g_sdlOnScreenKeyboardManualExitScanCode = 0;
	}
	else if (g_sdlControllerMappingActualCurr->showUi)
	{
		extern void BEL_ST_PrepareToShowControllerUI(const BE_ST_ControllerMapping *mapping);
		BEL_ST_PrepareToShowControllerUI(g_sdlControllerMappingActualCurr);
	}
}


void BE_ST_AltControlScheme_Push(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_ST_AltControlScheme_CleanUp();

	++g_sdlControllerMappingPtrsStack.currPtr;
	if (g_sdlControllerMappingPtrsStack.currPtr == g_sdlControllerMappingPtrsStack.endPtr)
	{
		BE_ST_ExitWithErrorMsg("BE_ST_AltControlScheme_Push: Out of stack bounds!\n");
	}
}

void BE_ST_AltControlScheme_Pop(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_ST_AltControlScheme_CleanUp();

	if (g_sdlControllerMappingPtrsStack.currPtr == &g_sdlControllerMappingPtrsStack.stack[0])
	{
		BE_ST_ExitWithErrorMsg("BE_ST_AltControlScheme_Pop: Popped more than necessary!\n");
	}
	--g_sdlControllerMappingPtrsStack.currPtr;

	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr;

	BEL_ST_AltControlScheme_ConditionallyShowControllerUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_PrepareControllerMapping(const BE_ST_ControllerMapping *mapping)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr = mapping;

	BEL_ST_AltControlScheme_ConditionallyShowControllerUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}


/*** A couple of special handlers call from BE_ST_PollEvents ***/
static void BEL_ST_AltControlScheme_HandleTextInputEvent(int but, bool isPressed)
{
	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	dosKeyEvent.dosScanCode = 0;

	extern int BEL_ST_MoveUpInTextInputUI(void);
	extern int BEL_ST_MoveDownInTextInputUI(void);
	extern int BEL_ST_MoveLeftInTextInputUI(void);
	extern int BEL_ST_MoveRightInTextInputUI(void);
	extern int BEL_ST_ToggleShiftStateInTextInputUI(bool *pToggle);
	extern int BEL_ST_ToggleKeyPressInTextInputUI(bool *pToggle);
	switch (but)
	{
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		if (isPressed)
			dosKeyEvent.dosScanCode = BEL_ST_MoveUpInTextInputUI();
		isPressed = false; // Ensure a recently pressed onscreen keyboard is released
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		if (isPressed)
			dosKeyEvent.dosScanCode = BEL_ST_MoveDownInTextInputUI();
		isPressed = false; // Ensure a recently pressed onscreen keyboard is released
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		if (isPressed)
			dosKeyEvent.dosScanCode = BEL_ST_MoveLeftInTextInputUI();
		isPressed = false; // Ensure a recently pressed onscreen keyboard is released
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		if (isPressed)
			dosKeyEvent.dosScanCode = BEL_ST_MoveRightInTextInputUI();
		isPressed = false; // Ensure a recently pressed onscreen keyboard is released
		break;
	// A few other special cases
	case SDL_CONTROLLER_BUTTON_START:
		dosKeyEvent.dosScanCode = BE_ST_SC_PAUSE;
		break;
	case SDL_CONTROLLER_BUTTON_B:
	case SDL_CONTROLLER_BUTTON_BACK:
		dosKeyEvent.dosScanCode = BE_ST_SC_ESC;
		g_sdlOnScreenKeyboardManualExitScanCode = dosKeyEvent.dosScanCode; // Don't forget to "release" key e.g., if changing controller scheme!!
		break;
	case SDL_CONTROLLER_BUTTON_X:
		// Change shift state (or at least try to).
		// NOTE: This can modify isPressed.
		dosKeyEvent.dosScanCode = BEL_ST_ToggleShiftStateInTextInputUI(&isPressed);
		break;
	default:
	{
		// Select key from UI.
		// NOTE: This can modify isPressed e.g., for shift key.
		dosKeyEvent.dosScanCode = BEL_ST_ToggleKeyPressInTextInputUI(&isPressed);
	}
	}

	if (dosKeyEvent.dosScanCode)
	{
		BEL_ST_HandleEmuKeyboardEvent(isPressed, false, dosKeyEvent);
	}
}


static void BEL_ST_AltControlScheme_HandleDebugKeysEvent(int but, bool isPressed)
{
	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	dosKeyEvent.dosScanCode = 0;

	extern void BEL_ST_MoveUpInDebugKeysUI(void);
	extern void BEL_ST_MoveDownInDebugKeysUI(void);
	extern void BEL_ST_MoveLeftInDebugKeysUI(void);
	extern void BEL_ST_MoveRightInDebugKeysUI(void);
	extern int BEL_ST_ToggleKeyPressInDebugKeysUI(bool *pToggle);
	switch (but)
	{
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		if (isPressed)
			BEL_ST_MoveUpInDebugKeysUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		if (isPressed)
			BEL_ST_MoveDownInDebugKeysUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		if (isPressed)
			BEL_ST_MoveLeftInDebugKeysUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_MoveRightInDebugKeysUI();
		break;
	// A few other special cases
	case SDL_CONTROLLER_BUTTON_START:
		dosKeyEvent.dosScanCode = BE_ST_SC_PAUSE;
		break;
	case SDL_CONTROLLER_BUTTON_B:
	case SDL_CONTROLLER_BUTTON_BACK:
		dosKeyEvent.dosScanCode = BE_ST_SC_ESC;
		g_sdlOnScreenKeyboardManualExitScanCode = dosKeyEvent.dosScanCode; // Don't forget to "release" key e.g., if changing controller scheme!!
		break;
	default:
	{
		// Select or deselect key from UI, IF actual button is pressed.
		// NOTE: This returns in isPressed the status of the key.
		if (isPressed)
			dosKeyEvent.dosScanCode = BEL_ST_ToggleKeyPressInDebugKeysUI(&isPressed);
	}
	}

	if (dosKeyEvent.dosScanCode)
	{
		BEL_ST_HandleEmuKeyboardEvent(isPressed, false, dosKeyEvent);
	}
}

void BE_ST_PollEvents(void)
{
	SDL_Event event;
	g_sdlLastPollEventsTime = SDL_GetTicks();
	while (SDL_PollEvent(&event))
	{
		void BEL_ST_CheckPressedPointerInTextInputUI(int x, int y);
		void BEL_ST_CheckMovedPointerInTextInputUI(int x, int y);
		int BEL_ST_CheckReleasedPointerInTextInputUI(int x, int y);
		void BEL_ST_CheckPressedPointerInDebugKeysUI(int x, int y);
		void BEL_ST_CheckMovedPointerInDebugKeysUI(int x, int y);
		int BEL_ST_CheckReleasedPointerInDebugKeysUI(int x, int y, bool *pToggle);
		void BEL_ST_CheckPressedPointerInControllerUI(int x, int y);
		void BEL_ST_CheckMovedPointerInControllerUI(int x, int y);
		void BEL_ST_CheckReleasedPointerInControllerUI(void);

		extern bool g_sdlShowControllerUI;

		switch (event.type)
		{
		case SDL_KEYDOWN:
			if (event.key.repeat)
				break; // Ignore (we emulate key repeat on our own)
			// Fall-through
		case SDL_KEYUP:
			BEL_ST_HandleEmuKeyboardEvent(event.type == SDL_KEYDOWN, false, sdlKeyMappings[event.key.keysym.scancode]);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if ((event.button.button >= 1) && (event.button.button <= 3))
			{
				if (g_sdlMouseButtonsStates[event.button.button-1])
					break; // Ignore (used in case pointer is shown/hidden)
				g_sdlMouseButtonsStates[event.button.button-1] = true;
			}

			if (g_refKeenCfg.altControlScheme.isEnabled && g_sdlShowControllerUI)
			{
				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
				{
					BEL_ST_CheckPressedPointerInTextInputUI(event.button.x, event.button.y);
					break;
				}
				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
				{
					BEL_ST_CheckPressedPointerInDebugKeysUI(event.button.x, event.button.y);
					break;
				}
				if (g_sdlControllerMappingActualCurr->showUi)
				{
					BEL_ST_CheckPressedPointerInControllerUI(event.button.x, event.button.y);
					break;
				}
			}

			if (event.button.button == SDL_BUTTON_LEFT)
				g_sdlEmuMouseButtonsState |= 1;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				g_sdlEmuMouseButtonsState |= 2;
			break;
		case SDL_MOUSEBUTTONUP:
			if ((event.button.button >= 1) && (event.button.button <= 3))
			{
				if (!g_sdlMouseButtonsStates[event.button.button-1])
					break; // Ignore (used in case pointer is shown/hidden)
				g_sdlMouseButtonsStates[event.button.button-1] = false;
			}

			if (g_refKeenCfg.altControlScheme.isEnabled && g_sdlShowControllerUI)
			{
				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
				{
					emulatedDOSKeyEvent dosKeyEvent;
					dosKeyEvent.isSpecial = false;
					dosKeyEvent.dosScanCode = BEL_ST_CheckReleasedPointerInTextInputUI(event.button.x, event.button.y);
					if (dosKeyEvent.dosScanCode)
					{
						BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
						// FIXME: A delay may be required here in certain cases, but this works for now...
						BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
					}
					break;
				}
				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
				{
					emulatedDOSKeyEvent dosKeyEvent;
					dosKeyEvent.isSpecial = false;
					bool isPressed;
					dosKeyEvent.dosScanCode = BEL_ST_CheckReleasedPointerInDebugKeysUI(event.button.x, event.button.y, &isPressed);
					if (dosKeyEvent.dosScanCode)
						BEL_ST_HandleEmuKeyboardEvent(isPressed, false, dosKeyEvent);
					break;
				}
				if (g_sdlControllerMappingActualCurr->showUi)
				{
					BEL_ST_CheckReleasedPointerInControllerUI();
					break;
				}
			}

			if (event.button.button == SDL_BUTTON_LEFT)
				g_sdlEmuMouseButtonsState &= ~1;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				g_sdlEmuMouseButtonsState &= ~2;
			break;
		case SDL_MOUSEMOTION:
			if (g_refKeenCfg.altControlScheme.isEnabled && g_sdlShowControllerUI)
			{
				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
				{
					BEL_ST_CheckMovedPointerInTextInputUI(event.button.x, event.button.y);
					break;
				}
				if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingDebugKeys)
				{
					BEL_ST_CheckMovedPointerInDebugKeysUI(event.button.x, event.button.y);
					break;
				}
				if (g_sdlControllerMappingActualCurr->showUi)
				{
					BEL_ST_CheckMovedPointerInControllerUI(event.button.x, event.button.y);
					break;
				}
			}

			g_sdlEmuMouseMotionAccumulatedState[0] += event.motion.xrel;
			g_sdlEmuMouseMotionAccumulatedState[1] += event.motion.yrel;
			break;

		case SDL_JOYAXISMOTION:
			for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			{
				if (g_sdlJoysticks[i] && (g_sdlJoysticksInstanceIds[i] == event.jaxis.which))
				{
					g_sdlEmuJoyMotionState[(event.jaxis.axis + 2*i) % 4] = (event.jaxis.value+32768)*BE_ST_EMU_JOYSTICK_RANGEMAX/65535;
					break;
				}
			}
			break;
		case SDL_JOYBUTTONDOWN:
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
					extern bool g_sdlShowControllerUI;
					g_sdlShowControllerUI = false;
					extern bool g_sdlForceGfxControlUiRefresh;
					g_sdlForceGfxControlUiRefresh = true;
					void BEL_ST_ConditionallyShowAltInputPointer(void);
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
			// Note: We handle BOTH sides in case axisVal == 0, so "release/clear" events can be properly sent
			if (axisVal <= 0)
				BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][0], -axisVal, &g_sdlControllersAxesStates[axis][0]);
			if (axisVal >= 0)
				if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][1], axisVal, &g_sdlControllersAxesStates[axis][1]))
				{
					// Special case for triggers, treated like digital buttons
					if (((axis == BE_ST_CTRL_AXIS_LTRIGGER) || (axis == BE_ST_CTRL_AXIS_RTRIGGER))
					    && g_sdlControllerMappingActualCurr->prevMapping && (axisVal >= g_sdlJoystickAxisBinaryThreshold))
					{
						BEL_ST_ReplaceControllerMapping(g_sdlControllerMappingActualCurr->prevMapping);
					}
				}

			break;
		}

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			bool isPressed = (event.type == SDL_CONTROLLERBUTTONDOWN);
			int but = event.cbutton.button;

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
					case SDL_CONTROLLER_BUTTON_DPAD_UP:
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
						g_sdlOnScreenKeyboardLastPressedDirButton = but;
						g_sdlOnScreenKeyboardLastDirButtonPressTime = g_sdlLastPollEventsTime;
						g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
						break;
					default:
						g_sdlOnScreenKeyboardLastPressedDirButton = SDL_CONTROLLER_BUTTON_INVALID;
					}
				}
				else
				{
					if (but == g_sdlOnScreenKeyboardLastPressedDirButton)
						g_sdlOnScreenKeyboardLastPressedDirButton = SDL_CONTROLLER_BUTTON_INVALID;
				}
			}
			// Try the usual otherwise (similar, but not identical, handling done with analog axes, triggers included)
			else if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->buttons[but], g_sdlJoystickAxisMax*isPressed, &g_sdlControllersButtonsStates[but]))
			{
				if (g_sdlControllerMappingActualCurr->prevMapping && isPressed)
				{
					BEL_ST_ReplaceControllerMapping(g_sdlControllerMappingActualCurr->prevMapping);
				}
			}
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
		case SDL_QUIT:
			BE_ST_QuickExit();
			break;
		default: ;
		}
	}

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
		if ((g_sdlOnScreenKeyboardLastPressedDirButton != SDL_CONTROLLER_BUTTON_INVALID) && (g_sdlLastPollEventsTime - g_sdlOnScreenKeyboardLastDirButtonPressTime >= g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay))
		{
			if (g_sdlControllerMappingActualCurr == &g_beStControllerMappingTextInput)
				BEL_ST_AltControlScheme_HandleTextInputEvent(g_sdlOnScreenKeyboardLastPressedDirButton, true);
			else
				BEL_ST_AltControlScheme_HandleDebugKeysEvent(g_sdlOnScreenKeyboardLastPressedDirButton, true);

			g_sdlOnScreenKeyboardLastDirButtonPressTime += g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay;
			g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay = BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;
		
		}
	}

	// HACK - If audio subsystem is disabled we still want to at least
	// make the sound callback run (so e.g., no loop gets stuck waiting
	// for sound playback to complete)
	extern bool g_sdlAudioSubsystemUp;
	if (! g_sdlAudioSubsystemUp)
	{
		BE_ST_PrepareForManualAudioSDServiceCall();
	}
}

#ifdef BE_ST_ENABLE_FARPTR_CFG
uint16_t BE_ST_Compat_GetFarPtrRelocationSegOffset(void)
{
	return g_refKeenCfg.farPtrSegOffset;
}
#endif
