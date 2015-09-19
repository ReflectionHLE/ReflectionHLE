#include <stdio.h>
#include <string.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_st.h"

#define BE_ST_MAXJOYSTICKS 8
#define BE_ST_EMU_JOYSTICK_RANGEMAX 5000 // id_in.c MaxJoyValue
#define BE_ST_DEFAULT_FARPTRSEGOFFSET 0x14

#if (defined REFKEEN_VER_CATARM) || (defined REFKEEN_VER_CATAPOC)
#define BE_ST_ENABLE_FARPTR_CFG 1
#endif

static void (*g_sdlKeyboardInterruptFuncPtr)(uint8_t) = 0;

// Defined internally, but also used in launcher code. extern is required for C++.
#ifdef __cplusplus
extern
#endif
const int g_sdlJoystickAxisBinaryThreshold = 8192, g_sdlJoystickAxisDeadZone = 3200, g_sdlJoystickAxisMax = 32767, g_sdlJoystickAxisMaxMinusDeadZone = 32767-3200;

static SDL_Joystick *g_sdlJoysticks[BE_ST_MAXJOYSTICKS];
SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS]; // Also used in launcher

// Also used in launcher
/*** These represent button states (pressed/released), although a call to BEL_ST_AltControlScheme_CleanUp zeros these out ***/
bool g_sdlControllersButtonsStates[BE_ST_MAXJOYSTICKS][SDL_CONTROLLER_BUTTON_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
bool g_sdlControllersAxesStates[BE_ST_MAXJOYSTICKS][SDL_CONTROLLER_AXIS_MAX][2];

/*** Same as above, but represent actual states of buttons and axes (even after cleaning up) ***/
static bool g_sdlControllersActualButtonsStates[BE_ST_MAXJOYSTICKS][SDL_CONTROLLER_BUTTON_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
static bool g_sdlControllersActualAxesStates[BE_ST_MAXJOYSTICKS][SDL_CONTROLLER_AXIS_MAX][2];

#define NUM_OF_CONTROLLER_MAPS_IN_STACK 8

static bool g_sdlControllerSchemeNeedsCleanUp;

static struct {
	const BE_ST_ControllerMapping *stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];
	const BE_ST_ControllerMapping **currPtr;
	const BE_ST_ControllerMapping **endPtr;
} g_sdlControllerMappingPtrsStack;

// Current mapping, doesn't have to be *(g_sdlControllerMappingPtrsStack.currPtr) as game code can change this (e.g., helper keys)
static const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr;

static BE_ST_ControllerMapping g_sdlControllerMappingDefault;
// HACK - This "mapping" is used for identification of text-input (using pointers comparison)
static BE_ST_ControllerMapping g_sdlControllerMappingTextInput;

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
	0,
};


extern SDL_Window *g_sdlWindow;

uint8_t g_sdlLastKeyScanCode;

void BE_ST_InitGfx(void);
void BE_ST_InitAudio(void);
void BE_ST_InitTiming(void);
void BE_ST_ShutdownAudio(void);
void BE_ST_ShutdownGfx(void);
static void BEL_ST_ConditionallyAddJoystick(int which);
static void BEL_ST_ParseConfig(void);
static void BEL_ST_SaveConfig(void);

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
	else
	{
		// Disable unnecessary high-rate polling of joystick events (mostly motion events), but we should still detect (dis)connections of devices
		SDL_EventState(SDL_JOYAXISMOTION, SDL_IGNORE);
		SDL_EventState(SDL_JOYBALLMOTION, SDL_IGNORE);
		SDL_EventState(SDL_JOYHATMOTION, SDL_IGNORE);
		SDL_EventState(SDL_JOYBUTTONDOWN, SDL_IGNORE);
		SDL_EventState(SDL_JOYBUTTONUP, SDL_IGNORE);

		SDL_EventState(SDL_CONTROLLERAXISMOTION, SDL_IGNORE);
		SDL_EventState(SDL_CONTROLLERBUTTONDOWN, SDL_IGNORE);
		SDL_EventState(SDL_CONTROLLERBUTTONUP, SDL_IGNORE);
		//SDL_GameControllerEventState(SDL_DISABLE);
		//SDL_JoystickEventState(SDL_DISABLE);
	}

	BEL_ST_ParseConfig();
	// This technically requires SDL 2.0.2, which has been available for a year now; Should be called BEFORE any SDL_CONTROLLERDEVICEADDED event should arrive (so e.g., before SDL_PollEvents).
	SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
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
	memset(g_sdlControllersActualButtonsStates, 0, sizeof(g_sdlControllersActualButtonsStates));
	memset(g_sdlControllersActualAxesStates, 0, sizeof(g_sdlControllersActualAxesStates));

	// BEFORE checking for more joysticks been attached/removed in BE_ST_PollEvents, add what's currently available
	int nOfJoysticks = SDL_NumJoysticks();
	if (nOfJoysticks > BE_ST_MAXJOYSTICKS)
		nOfJoysticks = BE_ST_MAXJOYSTICKS;
	for (int i = 0; i < nOfJoysticks; ++i)
		BEL_ST_ConditionallyAddJoystick(i);

	if (g_refKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else
	{
		SDL_ShowCursor(false);
	}

	BE_ST_PollEvents(); // e.g., to "reset" some states, and detect joysticks
	SDL_GetRelativeMouseState(NULL, NULL); // Reset
}

void BE_ST_ShutdownAll(void)
{
	if (g_refKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else
	{
		SDL_ShowCursor(true);
	}
	BE_ST_ShutdownAudio();
	BE_ST_ShutdownGfx();
	SDL_Quit();
}

void BE_ST_HandleExit(int status)
{
	SDL_Event event;
	bool keepRunning = true;
	// SPECIAL - Listening to joystick or controller events is considered acceptable here since we quit...
	SDL_EventState(SDL_JOYAXISMOTION, SDL_ENABLE);
	SDL_EventState(SDL_JOYBALLMOTION, SDL_ENABLE);
	SDL_EventState(SDL_JOYHATMOTION, SDL_ENABLE);
	SDL_EventState(SDL_JOYBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_JOYBUTTONUP, SDL_ENABLE);

	SDL_EventState(SDL_CONTROLLERAXISMOTION, SDL_ENABLE);
	SDL_EventState(SDL_CONTROLLERBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_CONTROLLERBUTTONUP, SDL_ENABLE);
	//SDL_GameControllerEventState(SDL_ENABLE);
	//SDL_JoystickEventState(SDL_ENABLE);

	while (keepRunning)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					void BE_ST_SetGfxOutputRects(void);
					BE_ST_SetGfxOutputRects();
				}
				break;
			case SDL_JOYHATMOTION:
				if (event.jhat.value == SDL_HAT_CENTERED)
					break; // Ignore
			case SDL_KEYDOWN:
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
		void BEL_ST_UpdateHostDisplay(void);
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
	"a", "b", "x", "y", 0, 0, 0, 0, 0, "lshoulder", "rshoulder", 0, 0, 0, 0,
	"ltrigger", "rtrigger", // Actually axes but these are added as extras
	"" // for any entry which is not set
};

RefKeenConfig g_refKeenCfg;

#ifdef REFKEEN_VER_KDREAMS
#define REFKEEN_CONFIG_FILEPATH "refkdreams.cfg"
#elif defined REFKEEN_VER_CAT3D
#define REFKEEN_CONFIG_FILEPATH "refcat3d.cfg"
#elif defined REFKEEN_VER_CATABYSS
#define REFKEEN_CONFIG_FILEPATH "refcatabyss.cfg"
#elif defined REFKEEN_VER_CATARM
#define REFKEEN_CONFIG_FILEPATH "refcatarm.cfg"
#elif defined REFKEEN_VER_CATAPOC
#define REFKEEN_CONFIG_FILEPATH "refcatapoc.cfg"
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

#ifdef BE_ST_ENABLE_FARPTR_CFG
// HACK (cfg file may be rewritten and we don't want to remove any setting)
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
	{"displaynum=", &BEL_ST_ParseSetting_DisplayNum},
	{"sdlrenderer=", &BEL_ST_ParseSetting_SDLRendererDriver},
	{"vsync=", &BEL_ST_ParseSetting_VSync},
	{"bilinear=", &BEL_ST_ParseSetting_Bilinear},
	{"scaletype=", &BEL_ST_ParseSetting_ScaleType},
	{"scalefactor=", &BEL_ST_ParseSetting_ScaleFactor},
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

	{"altcontrolscheme_dpad=", &BEL_ST_ParseSetting_AlternativeControlSchemeDpad},
	{"altcontrolscheme_lstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeLeftStick},
	{"altcontrolscheme_rstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeRightStick},
#ifdef REFKEEN_VER_CATACOMB_ALL
	{"altcontrolscheme_analogmotion=", &BEL_ST_ParseSetting_AlternativeControlSchemeAnalogMotion},
#endif

#ifdef BE_ST_ENABLE_FARPTR_CFG
	{"farptrsegoffset=", &BEL_ST_ParseSetting_FarPtrSegOffset},
#endif
};


// Little hack
BE_FILE_T BEL_Cross_open_from_dir(const char *filename, bool isOverwriteRequest, const char *searchdir);


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
	g_refKeenCfg.displayNum = 0;
	g_refKeenCfg.sdlRendererDriver = -1;
	g_refKeenCfg.vSync = VSYNC_AUTO;
	g_refKeenCfg.isBilinear = true;
	g_refKeenCfg.scaleType = SCALE_ASPECT;
	g_refKeenCfg.scaleFactor = 2;
	g_refKeenCfg.autolockCursor = false;
	g_refKeenCfg.sndSampleRate = 49716; // TODO should be a shared define
	g_refKeenCfg.sndSubSystem = true;
	g_refKeenCfg.oplEmulation = true;
	g_refKeenCfg.altControlScheme.isEnabled = false;

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
	g_refKeenCfg.altControlScheme.useDpad = true;
	g_refKeenCfg.altControlScheme.useLeftStick = true;
	g_refKeenCfg.altControlScheme.useRightStick = false;
#ifdef REFKEEN_VER_CATACOMB_ALL
	g_refKeenCfg.altControlScheme.analogMotion = false;
#endif

#ifdef BE_ST_ENABLE_FARPTR_CFG
	g_refKeenCfg.farPtrSegOffset = BE_ST_DEFAULT_FARPTRSEGOFFSET;
#endif
	// Try to load config
	FILE *fp = BEL_Cross_open_from_dir(REFKEEN_CONFIG_FILEPATH, false, ".");
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
	FILE *fp = BEL_Cross_open_from_dir(REFKEEN_CONFIG_FILEPATH, true, ".");
	if (!fp)
	{
		return;
	}
	fprintf(fp, "fullscreen=%s\n", g_refKeenCfg.isFullscreen ? "true" : "false");
	fprintf(fp, "fullres=%dx%d\n", g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight);
	fprintf(fp, "windowres=%dx%d\n", g_refKeenCfg.winWidth, g_refKeenCfg.winHeight);
	fprintf(fp, "launcherwindowres=%dx%d\n", g_refKeenCfg.launcherWinWidth, g_refKeenCfg.launcherWinHeight);
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
#ifdef BE_ST_ENABLE_FARPTR_CFG
	if (g_sdlIsFarPtrSegOffsetSettingRead)
	{
		// This should be a relatively hidden setting
		fprintf(fp, "farptrsegoffset=%X\n", g_refKeenCfg.farPtrSegOffset);
	}
#endif
	fclose(fp);
}



typedef struct {
	bool isSpecial; // Scancode of 0xE0 sent?
	uint8_t dosScanCode;
} emulatedDOSKeyEvent;

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
	int ourx, oury;
	SDL_GetRelativeMouseState(&ourx, &oury);

	if (g_refKeenCfg.altControlScheme.isEnabled)
	{
		SDL_GameControllerUpdate();

		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				// FIXME - Not very efficient
				for (int j = 0; j < BE_ST_CTRL_BUT_MAX; ++j)
				{
					if (!SDL_GameControllerGetButton(g_sdlControllers[i], (SDL_GameControllerButton)j))
						continue;

					const BE_ST_ControllerSingleMap *singleMap = &g_sdlControllerMappingActualCurr->buttons[j];
					if (singleMap->mapClass != BE_ST_CTRL_MAP_MOUSEMOTION)
						continue;

					int axisval = singleMap->secondaryVal;
					if (singleMap->val == 0) // Mouse x-axis
					{
						if (axisval*axisval > ourx*ourx)
							ourx = axisval;
					}
					else if (singleMap->val == 1) // Mouse y-axis
					{
						if (axisval*axisval > oury*oury)
							oury = axisval;
					}
				}

				for (int j = 0; j < BE_ST_CTRL_AXIS_MAX; ++j)
				{
					int normval = (int)SDL_GameControllerGetAxis(g_sdlControllers[i], (SDL_GameControllerAxis)j);
					const BE_ST_ControllerSingleMap *singleMap;
					if ((normval < -g_sdlJoystickAxisDeadZone) && (g_sdlControllerMappingActualCurr->axes[j][0].mapClass == BE_ST_CTRL_MAP_MOUSEMOTION))
						singleMap = &g_sdlControllerMappingActualCurr->axes[j][0];
					else if ((normval > g_sdlJoystickAxisDeadZone) && (g_sdlControllerMappingActualCurr->axes[j][1].mapClass == BE_ST_CTRL_MAP_MOUSEMOTION))
						singleMap = &g_sdlControllerMappingActualCurr->axes[j][1];
					else
						continue;

					if (normval < 0)
						normval = -normval;

					normval = (normval - g_sdlJoystickAxisDeadZone) * singleMap->secondaryVal / g_sdlJoystickAxisMaxMinusDeadZone;
					if (singleMap->val == 0) // Mouse x-axis
					{
						if (normval*normval > ourx*ourx)
							ourx = normval;
					}
					else if (singleMap->val == 1) // Mouse y-axis
					{
						if (normval*normval > oury*oury)
							oury = normval;
					}
				}
			}
		}
	}

	if (x)
	{
		*x = ourx;
	}
	if (y)
	{
		*y = oury;
	}
}

uint16_t BE_ST_GetMouseButtons(void)
{
	static uint16_t results[] = {0, 1, 4, 5, 2, 3, 6, 7};

	uint16_t schemeResult = 0;
	if (g_refKeenCfg.altControlScheme.isEnabled)
	{
		SDL_GameControllerUpdate();

		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				// FIXME - Not very efficient
				for (int j = 0; j < BE_ST_CTRL_BUT_MAX; ++j)
				{
					if (!SDL_GameControllerGetButton(g_sdlControllers[i], (SDL_GameControllerButton)j))
						continue;

					const BE_ST_ControllerSingleMap *singleMap = &g_sdlControllerMappingActualCurr->buttons[j];
					if (singleMap->mapClass != BE_ST_CTRL_MAP_MOUSEBUTTON)
						continue;

					schemeResult |= (1 << singleMap->val);
				}

				for (int j = 0; j < BE_ST_CTRL_AXIS_MAX; ++j)
				{
					int axisval = (int)SDL_GameControllerGetAxis(g_sdlControllers[i], (SDL_GameControllerAxis)j);
					const BE_ST_ControllerSingleMap *singleMap;
					if ((axisval < -g_sdlJoystickAxisBinaryThreshold) && (g_sdlControllerMappingActualCurr->axes[j][0].mapClass == BE_ST_CTRL_MAP_MOUSEBUTTON))
						singleMap = &g_sdlControllerMappingActualCurr->axes[j][0];
					else if ((axisval > g_sdlJoystickAxisBinaryThreshold) && (g_sdlControllerMappingActualCurr->axes[j][1].mapClass == BE_ST_CTRL_MAP_MOUSEBUTTON))
						singleMap = &g_sdlControllerMappingActualCurr->axes[j][1];
					else
						continue;

					schemeResult |= (1 << singleMap->val);
				}
			}
		}
	}

	return results[SDL_GetMouseState(NULL, NULL) & 7] | schemeResult;
}

void BE_ST_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp)
{
	int emuAxisStart = (joy != 0) ? 2 : 0;
	int minX = BE_ST_EMU_JOYSTICK_RANGEMAX, minY = BE_ST_EMU_JOYSTICK_RANGEMAX, maxX = 0, maxY = 0;

	SDL_JoystickUpdate();

	for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i, emuAxisStart ^= 2)
	{
		if (!(g_sdlJoysticks[i]))
		{
			continue;
		}
		int nAxes = SDL_JoystickNumAxes(g_sdlJoysticks[i]);
		for (int axis = 0, emuAxis = emuAxisStart; axis < nAxes; ++axis)
		{
			// Ignore emulated axes 2&3 (check just 0&1)
			if (emuAxis < 2)
			{
				int axisNormalizedVal = ((int)SDL_JoystickGetAxis(g_sdlJoysticks[i], axis)+32768)*BE_ST_EMU_JOYSTICK_RANGEMAX/65535;
				if (emuAxis == 0)
				{
					minX = (minX < axisNormalizedVal) ? minX : axisNormalizedVal;
					maxX = (maxX > axisNormalizedVal) ? maxX : axisNormalizedVal;
				}
				else
				{
					minY = (minY < axisNormalizedVal) ? minY : axisNormalizedVal;
					maxY = (maxY > axisNormalizedVal) ? maxY : axisNormalizedVal;
				}
			}
			if (++emuAxis == 4)
			{
				emuAxis = 0;
			}
		}
	}
	// Pick horizontal axis furthest from the center, and similarly vertical
	// NOTE: If we do not have any joystick connected, center is NOT returned.
	// Reason is the way joystick detection is done under DOS.
	*xp = minX < (BE_ST_EMU_JOYSTICK_RANGEMAX-maxX) ? minX : maxX;
	*yp = minY < (BE_ST_EMU_JOYSTICK_RANGEMAX-maxY) ? minY : maxY;
}

uint16_t BE_ST_GetJoyButtons(uint16_t joy)
{
	int emuButMaskStart = (joy != 0) ? 4 : 1;
	int result = 0;

	SDL_JoystickUpdate();

	for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i, emuButMaskStart ^= 5)
	{
		if (!(g_sdlJoysticks[i]))
		{
			continue;
		}
		int nButtons = SDL_JoystickNumButtons(g_sdlJoysticks[i]);
		for (int but = 0, emuButMask = emuButMaskStart; but < nButtons; ++but)
		{
			// Ignore emulated buttons 2&3 (check just 0&1)
			if ((emuButMask < 4) && SDL_JoystickGetButton(g_sdlJoysticks[i], but))
			{
				result |= emuButMask;
			}
			emuButMask <<= 1;
			if (emuButMask == 16)
			{
				emuButMask = 1;
			}
		}
	}
	return result;
}

int16_t BE_ST_KbHit(void)
{
	return g_sdlLastKeyScanCode;
}

int16_t BE_ST_BiosScanCode(int16_t command)
{
	if (command == 1)
	{
		return g_sdlLastKeyScanCode;
	}

	while (!g_sdlLastKeyScanCode)
	{
		BE_ST_ShortSleep();
	}
	int16_t result = g_sdlLastKeyScanCode;
	g_sdlLastKeyScanCode = 0;
	return result;
}


static void BEL_ST_HandleEmuKeyboardEvent(bool isPressed, emulatedDOSKeyEvent keyEvent)
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
		}
	}
	else
	{
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
			g_sdlLastKeyScanCode = keyEvent.dosScanCode;
		}
	}
}

static void BEL_ST_AltControlScheme_CleanUp(void);
static void BEL_ST_AltControlScheme_ConditionallyShowButtonsUI(void);

static bool BEL_ST_AltControlScheme_HandleEntry(const BE_ST_ControllerSingleMap *map, bool isPressed)
{
	switch (map->mapClass)
	{
	case BE_ST_CTRL_MAP_KEYSCANCODE:
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = map->val;
		BEL_ST_HandleEmuKeyboardEvent(isPressed, dosKeyEvent);
		return true;
	}
	case BE_ST_CTRL_MAP_OTHERMAPPING:
		if (!isPressed)
			return true; // Do nothing but confirm

		// This isn't exactly the same as PrepareControllerMapping
		BEL_ST_AltControlScheme_CleanUp();
		g_sdlControllerMappingActualCurr = map->otherMappingPtr;

		BEL_ST_AltControlScheme_ConditionallyShowButtonsUI();

		g_sdlControllerSchemeNeedsCleanUp = true;
		return true;
	}
	return false;
}


static void BEL_ST_ConditionallyAddJoystick(int which)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
	{
		g_sdlJoysticks[which] = SDL_JoystickOpen(which);
	}
	else if (SDL_IsGameController(which))
	{
		g_sdlControllers[which] = SDL_GameControllerOpen(which);
		memset(g_sdlControllersActualButtonsStates[which], 0, sizeof(g_sdlControllersActualButtonsStates[which]));
		memset(g_sdlControllersActualAxesStates[which], 0, sizeof(g_sdlControllersActualAxesStates[which]));

		extern bool g_sdlShowControllerUI;
		g_sdlShowControllerUI = true;
		extern bool g_sdlForceGfxControlUiRefresh;
		g_sdlForceGfxControlUiRefresh = true;
	}
}


/* WARNING: In theory there may be a CleanUp -> HandleEntry -> CleanUp cycle,
 * but it can never occur since isPressed is set to false
 */
static void BEL_ST_AltControlScheme_CleanUp(void)
{
	if (!g_sdlControllerSchemeNeedsCleanUp)
		return;

	if (g_sdlControllerMappingActualCurr == &g_sdlControllerMappingTextInput)
	{
		int BEL_ST_GetPressedKeyScanCodeFromTextInputUI(void);
		bool BEL_ST_IsTextInputUIShifted(void);

		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = BEL_ST_GetPressedKeyScanCodeFromTextInputUI();
		// Don't forget to "release" a key pressed in the text input UI
		if (dosKeyEvent.dosScanCode)
		{
			BEL_ST_HandleEmuKeyboardEvent(false, dosKeyEvent);
		}
		// Shift key may further be held, don't forget this too!
		if ((dosKeyEvent.dosScanCode != BE_ST_SC_LSHIFT) && BEL_ST_IsTextInputUIShifted())
		{
			dosKeyEvent.dosScanCode = BE_ST_SC_LSHIFT;
			BEL_ST_HandleEmuKeyboardEvent(false, dosKeyEvent);
		}
	}
	else // Otherwise simulate key releases based on the mapping
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				for (int but = 0; but < SDL_CONTROLLER_BUTTON_MAX; ++but)
				{
					if (g_sdlControllersButtonsStates[i][but])
					{
						// Do NOT force this - button can be re-detected as "pressed" immediately otherwise!
						// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than a key press.
						//g_sdlControllersButtonsStates[i][but] = false;
						BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->buttons[but], false);
					}
				}
				// Repeat with analog axes
				for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; ++axis)
				{
					// Is pressed in the negative direction?
					if (g_sdlControllersAxesStates[i][axis][0])
					{
						// Do NOT force this - axis can be re-detected as "pressed" (in this direction) immediately otherwise!
						// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than a key press.
						//g_sdlControllersAxesStates[i][axis][0] = false;
						BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][0], false);
					}
					// Repeat with positive
					if (g_sdlControllersAxesStates[i][axis][1])
					{
						// Do NOT force this - axis can be re-detected as "pressed" (in this direction) immediately otherwise!
						// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than a key press.
						//g_sdlControllersAxesStates[i][axis][1] = false;
						BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][1], false);
					}
				}
			}
		}
	}

	memset(g_sdlControllersButtonsStates, 0, sizeof(g_sdlControllersButtonsStates));
	memset(g_sdlControllersAxesStates, 0, sizeof(g_sdlControllersAxesStates));

	extern void BEL_ST_HideAltInputUI(void);
	BEL_ST_HideAltInputUI();

	g_sdlControllerSchemeNeedsCleanUp = false;
}


static void BEL_ST_AltControlScheme_ConditionallyShowButtonsUI(void)
{
	if (g_sdlControllerMappingActualCurr->showUi)
	{
		extern void BEL_ST_PrepareToShowControllerUI(const BE_ST_ControllerMapping *mapping);
		BEL_ST_PrepareToShowControllerUI(g_sdlControllerMappingActualCurr);
	}
	else if (g_sdlControllerMappingActualCurr == &g_sdlControllerMappingTextInput)
	{
		extern void BEL_ST_PrepareToShowTextInputUI(void);
		BEL_ST_PrepareToShowTextInputUI();
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

	BEL_ST_AltControlScheme_ConditionallyShowButtonsUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_PrepareControllerMapping(const BE_ST_ControllerMapping *mapping)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr = mapping;

	BEL_ST_AltControlScheme_ConditionallyShowButtonsUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_ST_AltControlScheme_PrepareTextInput(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_ST_AltControlScheme_CleanUp();
	g_sdlControllerMappingActualCurr = *g_sdlControllerMappingPtrsStack.currPtr = &g_sdlControllerMappingTextInput;

	extern void BEL_ST_PrepareToShowTextInputUI(void);
	BEL_ST_PrepareToShowTextInputUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}


void BE_ST_PollEvents(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			BEL_ST_HandleEmuKeyboardEvent(event.type == SDL_KEYDOWN, sdlKeyMappings[event.key.keysym.scancode]);
			break;
		//case SDL_MOUSEBUTTONUP:
		//	BEL_ST_toggleCursorConditionally(true);
		//	break;

		/* Don't use SDL_CONTROLLERDEVICEADDED with alternative controller schemes, and for the sake of consistency avoid SDL_CONTROLLERDEVICEREMOVED as well.
		 * Reason is that on init, there is a problem handling controller mappings loaded from the database using SDL_CONTROLLERDEVICEADDED
		 * (if loaded before init, the mappings seem to be deleted, otherwise SDL_CONTROLLERDEVICEADDED are just not spawned for these).
		 */
		case SDL_JOYDEVICEADDED:
			if (event.jdevice.which < BE_ST_MAXJOYSTICKS)
				BEL_ST_ConditionallyAddJoystick(event.jdevice.which);
			break;
		case SDL_JOYDEVICEREMOVED:
			if (!g_refKeenCfg.altControlScheme.isEnabled)
			{
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
				{
					if (g_sdlJoysticks[i] && (SDL_JoystickInstanceID(g_sdlJoysticks[i]) == event.jdevice.which))
					{
						SDL_JoystickClose(g_sdlJoysticks[i]);
						g_sdlJoysticks[i] = NULL;
					}
				}
			}
			else
			{
				bool isAnyControllerStillConnected = false;
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
				{
					if (g_sdlControllers[i])
					{
						if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i])) == event.jdevice.which)
						{
							SDL_GameControllerClose(g_sdlControllers[i]);
							g_sdlControllers[i] = NULL;
						}
						else
						{
							isAnyControllerStillConnected = true;
						}
					}
				}
				if (!isAnyControllerStillConnected)
				{
					extern bool g_sdlShowControllerUI;
					g_sdlShowControllerUI = false;
					extern bool g_sdlForceGfxControlUiRefresh;
					g_sdlForceGfxControlUiRefresh = true;
				}
			}
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			case  SDL_WINDOWEVENT_RESIZED:
			{
				void BE_ST_SetGfxOutputRects(void);
				BE_ST_SetGfxOutputRects();
				//BE_ST_MarkGfxForPendingUpdate();
				BE_ST_MarkGfxForUpdate();
				break;
			}
			case SDL_WINDOWEVENT_EXPOSED:
				//BE_ST_MarkGfxForPendingUpdate();
				BE_ST_MarkGfxForUpdate();
				break;
			break;
		case SDL_QUIT:
			BE_ST_QuickExit();
			break;
		default: ;
		}
	}
	// If SDL_GameController is used, we don't poll for its events.
	// Furthermore, we emulate keyboard/mouse events from DOS.
	if (g_refKeenCfg.altControlScheme.isEnabled)
	{
		SDL_GameControllerUpdate();

		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				for (int but = 0; but < SDL_CONTROLLER_BUTTON_MAX; ++but)
				{
					bool isPressed = SDL_GameControllerGetButton(g_sdlControllers[i], (SDL_GameControllerButton)but);
					if (isPressed != g_sdlControllersActualButtonsStates[i][but])
					{
						g_sdlControllersActualButtonsStates[i][but] = isPressed;
						if (isPressed != g_sdlControllersButtonsStates[i][but])
						{
							g_sdlControllersButtonsStates[i][but] = isPressed;
							// Special handling for text input
							if (g_sdlControllerMappingActualCurr == &g_sdlControllerMappingTextInput)
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
									BEL_ST_HandleEmuKeyboardEvent(isPressed, dosKeyEvent);
								}
							}
							// Try the usual otherwise (similar, but not identical, handling done with analog axes, triggers included)
							else if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->buttons[but], isPressed))
							{
								if (g_sdlControllerMappingActualCurr->prevMapping && isPressed)
								{
									// This isn't exactly the same as PrepareControllerMapping
									BEL_ST_AltControlScheme_CleanUp();
									g_sdlControllerMappingActualCurr = g_sdlControllerMappingActualCurr->prevMapping;

									BEL_ST_AltControlScheme_ConditionallyShowButtonsUI();

									g_sdlControllerSchemeNeedsCleanUp = true;
									goto finish;
								}
							}
						}
					}
				}
				// Repeat with analog axes (ignored with text input scheme in use)
				if (g_sdlControllerMappingActualCurr == &g_sdlControllerMappingTextInput)
					continue;

				for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; ++axis)
				{
					int axisVal = (int)SDL_GameControllerGetAxis(g_sdlControllers[i], (SDL_GameControllerAxis)axis);
					// Is pressed in the negative direction?
					bool isNegPressed = (axisVal <= -g_sdlJoystickAxisBinaryThreshold);

					if (isNegPressed != g_sdlControllersActualAxesStates[i][axis][0])
					{
						g_sdlControllersActualAxesStates[i][axis][0] = isNegPressed;
						if (isNegPressed != g_sdlControllersAxesStates[i][axis][0])
						{
							g_sdlControllersAxesStates[i][axis][0] = isNegPressed;
							BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][0], isNegPressed);
						}
					}
					// Repeat with positive
					bool isPosPressed = (axisVal >= g_sdlJoystickAxisBinaryThreshold);
					if (isPosPressed != g_sdlControllersActualAxesStates[i][axis][1])
					{
						g_sdlControllersActualAxesStates[i][axis][1] = isPosPressed;
						if (isPosPressed != g_sdlControllersAxesStates[i][axis][1])
						{
							g_sdlControllersAxesStates[i][axis][1] = isPosPressed;
							if (!BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->axes[axis][1], isPosPressed))
							{
								// We treat triggers the same way as digital buttons here
								if (((axis == BE_ST_CTRL_AXIS_LTRIGGER) || (axis == BE_ST_CTRL_AXIS_RTRIGGER))
								    && g_sdlControllerMappingActualCurr->prevMapping && isPosPressed)
								{
									// This isn't exactly the same as PrepareControllerMapping
									BEL_ST_AltControlScheme_CleanUp();
									g_sdlControllerMappingActualCurr = g_sdlControllerMappingActualCurr->prevMapping;

									BEL_ST_AltControlScheme_ConditionallyShowButtonsUI();

									g_sdlControllerSchemeNeedsCleanUp = true;
									goto finish;
								}
							}
						}
					}
				}
			}
		}
	}

	// HACK - If audio subsystem is disabled we still want to at least
	// make the sound callback run (so e.g., no loop gets stuck waiting
	// for sound playback to complete)
	extern bool g_sdlAudioSubsystemUp;
finish:
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
