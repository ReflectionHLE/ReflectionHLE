#include <stdio.h>
#include <string.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_sdl.h"

#define BE_SDL_MAXJOYSTICKS 8
#define BE_SDL_EMU_JOYSTICK_RANGEMAX 5000 // id_in.c MaxJoyValue
#define BE_SDL_DEFAULT_FARPTRSEGOFFSET 0x14

#if (defined REFKEEN_VER_CATARM) || (defined REFKEEN_VER_CATAPOC)
#define BE_SDL_ENABLE_FARPTR_CFG 1
#endif

static void (*g_sdlKeyboardInterruptFuncPtr)(uint8_t) = 0;

static SDL_Joystick *g_sdlJoysticks[BE_SDL_MAXJOYSTICKS];
static SDL_GameController *g_sdlControllers[BE_SDL_MAXJOYSTICKS];

typedef enum { CONTROLSCHEMEMAP_TYPE_KEYVAL = 0, CONTROLSCHEMEMAP_TYPE_HELPER } SchemeMapTypeEnumT;
enum { CONTROLSCHEMEMAP_HELPER_FUNCKEYS, CONTROLSCHEMEMAP_HELPER_SCROLLS };

typedef struct {
	SchemeMapTypeEnumT type;
	int val;
} BESDLControllerMapEntry;

typedef enum { CONTROLSCHEME_DEFAULT, CONTROLSCHEME_FACE, CONTROLSCHEME_PAGE, CONTROLSCHEME_MENU, CONTROLSCHEME_GAME, CONTROLSCHEME_INPUTWAIT, CONTROLSCHEME_TEXTINPUT } SchemeTypeEnumT;

typedef struct {
	BESDLControllerMapEntry buttonsMap[SDL_CONTROLLER_BUTTON_MAX];
	BESDLControllerMapEntry axesMap[SDL_CONTROLLER_AXIS_MAX][2];
	SchemeTypeEnumT schemeType;
	// Misc. buffer that may be useful in case of calling pop function
	char miscBuffer[9];
} BESDLControllerMap;

/*** These represent button states (pressed/released), although a call to BEL_SDL_AltControlScheme_CleanUp zeros these out ***/
static bool g_sdlControllersButtonsStates[BE_SDL_MAXJOYSTICKS][SDL_CONTROLLER_BUTTON_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
static bool g_sdlControllersAxesStates[BE_SDL_MAXJOYSTICKS][SDL_CONTROLLER_AXIS_MAX][2];

/*** Same as above, but represent actual states of buttons and axes (even after cleaning up) ***/
static bool g_sdlControllersActualButtonsStates[BE_SDL_MAXJOYSTICKS][SDL_CONTROLLER_BUTTON_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
static bool g_sdlControllersActualAxesStates[BE_SDL_MAXJOYSTICKS][SDL_CONTROLLER_AXIS_MAX][2];

#define NUM_OF_CONTROLLER_MAPS_IN_STACK 8

static bool g_sdlControllerSchemeNeedsCleanUp;

static struct {
	BESDLControllerMap stack[SDL_CONTROLLER_BUTTON_MAX];
	BESDLControllerMap *currPtr;
	BESDLControllerMap *endPtr;
} g_sdlControllertoScanCodeMaps;

// Used e.g., when a few choices should be temporarily shown during gameplay,
// but otherwise the "current mapping" is a different one for the game itself
static BESDLControllerMap g_sdlControllerLowPriorityMap;

// Either g_sdlControllertoScanCodeMaps.currPtr, or &g_sdlControllertoScanCodeMaps
static BESDLControllerMap *g_sdlControllerActualCurrPtr;

static const BESDLControllerMap g_sdlControllerToScanCodeMap_default = {
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ENTER, // SDL_CONTROLLER_BUTTON_A
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_B
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_X
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_Y
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_UP, // SDL_CONTROLLER_DPAD_UP
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_DPAD_DOWN
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LEFT, // SDL_CONTROLLER_DPAD_LEFT
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_DPAD_RIGHT

	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_LEFTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_LEFTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_RIGHTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_RIGHTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_DEFAULT
};

// This one is NOT const because it is (partially) configurable
// (CONTROLSCHEMEMAP_TYPE_KEYVAL should be 0...)
static BESDLControllerMap g_sdlControllerToScanCodeMap_inGameTemplate = {
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_A
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_B
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_X
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_Y
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_DPAD_UP
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_DPAD_DOWN
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_DPAD_LEFT
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_DPAD_RIGHT

	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_LEFTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_LEFTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_RIGHTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_RIGHTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_DEFAULT
};




enum {
	CONTROLSCHEME_CONFIG_BUTMAP_BEFOREFIRST = -1, /* The actual first entry is to be numbered 0 */
#ifdef REFKEEN_VER_KDREAMS
	CONTROLSCHEME_CONFIG_BUTMAP_JUMP,
	CONTROLSCHEME_CONFIG_BUTMAP_THROW,
	CONTROLSCHEME_CONFIG_BUTMAP_STATS,
#else
	CONTROLSCHEME_CONFIG_BUTMAP_FIRE,
	CONTROLSCHEME_CONFIG_BUTMAP_STRAFE,
	CONTROLSCHEME_CONFIG_BUTMAP_DRINK,
	CONTROLSCHEME_CONFIG_BUTMAP_BOLT, // Zapper in the Adventures Series
	CONTROLSCHEME_CONFIG_BUTMAP_NUKE, // Xterminator in the Adventures Series
	CONTROLSCHEME_CONFIG_BUTMAP_FASTTURN,
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	CONTROLSCHEME_CONFIG_BUTMAP_SCROLLS,
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	CONTROLSCHEME_CONFIG_BUTMAP_FUNCKEYS,
#endif
	CONTROLSCHEME_CONFIG_BUTMAP_AFTERLAST,
};

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

static SDL_GameControllerAxis g_sdlXAxisForMenuMouse, g_sdlYAxisForMenuMouse;


extern SDL_Window *g_sdlWindow;

uint8_t g_sdlLastKeyScanCode;

void BE_SDL_InitGfx(void);
void BE_SDL_InitAudio(void);
void BE_SDL_ShutdownAudio(void);
void BE_SDL_ShutdownGfx(void);
static void BEL_SDL_ParseConfig(void);
static BESDLControllerMapEntry * BEL_SDL_GetKeyMapPtrFromCfgVal(BESDLControllerMap *controllerMapPtr, int mappingCfgVal);

void BE_SDL_InitAll(void)
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

	BEL_SDL_ParseConfig();
	BE_SDL_InitGfx();
	BE_SDL_InitAudio();
	if (g_refKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else
	{
		SDL_ShowCursor(false);
	}
	// This technically requires SDL 2.0.2, which has been available for a year now; Should be called BEFORE init so SDL_CONTROLLERDEVICEADDED events actually arrive
	SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
	// Preparing a controller scheme (with no special UI) in case the relevant feature is enabled
	memcpy(&g_sdlControllertoScanCodeMaps.stack[0], &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr = &g_sdlControllertoScanCodeMaps.stack[0];
	g_sdlControllertoScanCodeMaps.endPtr = &g_sdlControllertoScanCodeMaps.stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];
	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	// Fill in-game scheme with configurable key mappings (but not the first two, which can further be configured - for keyboard usage)
#ifdef REFKEEN_VER_KDREAMS
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_STATS])->val = EMULATEDKEYSCANCODE_SPACE;
#else
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_DRINK])->val = EMULATEDKEYSCANCODE_SPACE;
#ifdef REFKEEN_VER_CAT3D
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_BOLT])->val = EMULATEDKEYSCANCODE_B;
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FASTTURN])->val = EMULATEDKEYSCANCODE_RSHIFT;
#else
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_BOLT])->val = EMULATEDKEYSCANCODE_Z;
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FASTTURN])->val = EMULATEDKEYSCANCODE_TAB;
#endif
	BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_NUKE])->val = EMULATEDKEYSCANCODE_ENTER;
#endif

	BESDLControllerMapEntry *mapEntry;
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	mapEntry = BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_SCROLLS]);
	mapEntry->type = CONTROLSCHEMEMAP_TYPE_HELPER;
	mapEntry->val = CONTROLSCHEMEMAP_HELPER_SCROLLS;
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	mapEntry = BEL_SDL_GetKeyMapPtrFromCfgVal(&g_sdlControllerToScanCodeMap_inGameTemplate, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FUNCKEYS]);
	mapEntry->type = CONTROLSCHEMEMAP_TYPE_HELPER;
	mapEntry->val = CONTROLSCHEMEMAP_HELPER_FUNCKEYS;
#endif
	// Simple menu mouse mapping (FIXME use enum)
	g_sdlXAxisForMenuMouse = (g_refKeenCfg.altControlScheme.menuMouseMapping == 1) ? SDL_CONTROLLER_AXIS_LEFTX : SDL_CONTROLLER_AXIS_RIGHTX;
	g_sdlYAxisForMenuMouse = (g_refKeenCfg.altControlScheme.menuMouseMapping == 1) ? SDL_CONTROLLER_AXIS_LEFTY : SDL_CONTROLLER_AXIS_RIGHTY;


	g_sdlControllerSchemeNeedsCleanUp = true;

	BE_SDL_PollEvents(); // e.g., to "reset" some states, and detect joysticks
	SDL_GetRelativeMouseState(NULL, NULL); // Reset
}

void BE_SDL_ShutdownAll(void)
{
	if (g_refKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else
	{
		SDL_ShowCursor(true);
	}
	BE_SDL_ShutdownAudio();
	BE_SDL_ShutdownGfx();
	SDL_Quit();
}

void BE_SDL_HandleExit(int status)
{
	SDL_Event event;
	bool keepRunning = true;
	while (keepRunning)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					void BE_SDL_SetGfxOutputRects(void);
					BE_SDL_SetGfxOutputRects();
				}
				break;
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_QUIT:
				keepRunning = false;
				break;
			default: ;
			}
		}
		// Events are not sent for SDL joysticks/game controllers
		if (g_refKeenCfg.altControlScheme.isEnabled)
		{
			for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
			{
				if (!(g_sdlControllers[i]))
				{
					continue;
				}
				for (int but = 0; but < SDL_CONTROLLER_BUTTON_MAX; ++but)
				{
					if (SDL_GameControllerGetButton(g_sdlControllers[i], (SDL_GameControllerButton)but))
					{
						keepRunning = false;
						break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
			{
				if (!(g_sdlJoysticks[i]))
				{
					continue;
				}
				int nButtons = SDL_JoystickNumButtons(g_sdlJoysticks[i]);
				for (int but = 0; but < nButtons; ++but)
				{
					if (SDL_JoystickGetButton(g_sdlJoysticks[i], but))
					{
						keepRunning = false;
						break;
					}
				}
			}
		}
		SDL_Delay(1);
		// TODO: Make this more efficient
		void BEL_SDL_UpdateHostDisplay(void);
		BEL_SDL_UpdateHostDisplay();
	}
	BE_SDL_ShutdownAll();
	exit(0);
}



// Enumerated by SDL_GameControllerButton, for most
static const char *g_sdlControlSchemeKeyMapCfgVals[] = {
	"a", "b", "x", "y", 0, 0, 0, 0, 0, "lshoulder", "rshoulder", 0, 0, 0, 0,
	"ltrigger", "rtrigger", // Actually axes but these are added as extras
};

// Values for in-game (digital) movement
static const char *g_sdlControlSchemeMovementMapCfgVals[] = {
	"dpad", "lstick", "rstick",
};

// Values for menu mouse control
static const char *g_sdlControlSchemeMenuMouseMapCfgVals[] = {
	"none", "lstick", "rstick",
};

RefKeenConfig g_refKeenCfg;

#ifdef REFKEEN_VER_KDREAMS
#define REFKEEN_DREAMS_CONFIG_FILEPATH "refkdreams.cfg"
#elif defined REFKEEN_VER_CAT3D
#define REFKEEN_DREAMS_CONFIG_FILEPATH "refcat3d.cfg"
#elif defined REFKEEN_VER_CATABYSS
#define REFKEEN_DREAMS_CONFIG_FILEPATH "refcatabyss.cfg"
#elif defined REFKEEN_VER_CATARM
#define REFKEEN_DREAMS_CONFIG_FILEPATH "refcatarm.cfg"
#elif defined REFKEEN_VER_CATAPOC
#define REFKEEN_DREAMS_CONFIG_FILEPATH "refcatapoc.cfg"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

static void BEL_SDL_ParseSetting_FullScreen(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_FullRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.fullWidth, &g_refKeenCfg.fullHeight);
}

static void BEL_SDL_ParseSetting_WindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.winWidth, &g_refKeenCfg.winHeight);
}

static void BEL_SDL_ParseSetting_DisplayNum(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%d", &g_refKeenCfg.displayNum);
}

static void BEL_SDL_ParseSetting_SDLRendererDriver(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_VSync(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_Bilinear(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_ScaleType(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_ScaleFactor(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.scaleFactor = atoi(buffer);
}

static void BEL_SDL_ParseSetting_AutolockCursor(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_SndSampleRate(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.sndSampleRate = atoi(buffer);
}

static void BEL_SDL_ParseSetting_DisableSoundSubSystem(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.disableSoundSubSystem = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.disableSoundSubSystem = false;
	}
}

static void BEL_SDL_ParseSetting_AlternativeControlScheme(const char *keyprefix, const char *buffer)
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

static void BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap(const char *keyprefix, const char *buffer)
{
	int keyindex, valindex;
	for (keyindex = 0; keyindex < CONTROLSCHEME_CONFIG_BUTMAP_AFTERLAST; ++keyindex)
	{
		if (!strcmp(keyprefix, g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex]))
			break;
	}
	if (keyindex == CONTROLSCHEME_CONFIG_BUTMAP_AFTERLAST)
	{
		BE_SDL_SetScreenMode(3);
		BE_SDL_Simplified_printf("Error in BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap: Invalid config key, this shouldn't happen!\n");
		BE_SDL_HandleExit(0);
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
}

static void BEL_SDL_ParseSetting_AlternativeControlSchemeMovementMap(const char *keyprefix, const char *buffer)
{
	for (int valindex = 0; valindex < (int)(sizeof(g_sdlControlSchemeMovementMapCfgVals)/sizeof(*g_sdlControlSchemeMovementMapCfgVals)); ++valindex)
	{
		if (!strcmp(buffer, g_sdlControlSchemeMovementMapCfgVals[valindex]))
		{
			g_refKeenCfg.altControlScheme.movementMapping = valindex;
			return;
		}
	}
}

static void BEL_SDL_ParseSetting_AlternativeControlSchemeMenuMouseMap(const char *keyprefix, const char *buffer)
{
	for (int valindex = 0; valindex < (int)(sizeof(g_sdlControlSchemeMenuMouseMapCfgVals)/sizeof(*g_sdlControlSchemeMenuMouseMapCfgVals)); ++valindex)
	{
		if (!strcmp(buffer, g_sdlControlSchemeMenuMouseMapCfgVals[valindex]))
		{
			g_refKeenCfg.altControlScheme.menuMouseMapping = valindex;
			return;
		}
	}
}

#ifdef BE_SDL_ENABLE_FARPTR_CFG
// HACK (cfg file may be rewritten and we don't want to remove any setting)
static bool g_sdlIsFarPtrSegOffsetSettingRead = false;

static void BEL_SDL_ParseSetting_FarPtrSegOffset(const char *keyprefix, const char *buffer)
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
	{"fullscreen=", &BEL_SDL_ParseSetting_FullScreen},
	{"fullres=", &BEL_SDL_ParseSetting_FullRes},
	{"windowres=", &BEL_SDL_ParseSetting_WindowRes},
	{"displaynum=", &BEL_SDL_ParseSetting_DisplayNum},
	{"sdlrenderer=", &BEL_SDL_ParseSetting_SDLRendererDriver},
	{"vsync=", &BEL_SDL_ParseSetting_VSync},
	{"bilinear=", &BEL_SDL_ParseSetting_Bilinear},
	{"scaletype=", &BEL_SDL_ParseSetting_ScaleType},
	{"scalefactor=", &BEL_SDL_ParseSetting_ScaleFactor},
	{"autolock=", &BEL_SDL_ParseSetting_AutolockCursor},
	{"sndsamplerate=", &BEL_SDL_ParseSetting_SndSampleRate},
	{"disablesndsubsystem=", &BEL_SDL_ParseSetting_DisableSoundSubSystem},
	{"altcontrolscheme=", &BEL_SDL_ParseSetting_AlternativeControlScheme},

	// HACK: Copy-paste... if this is updated, check g_sdlControlSchemeKeyMapCfgKeyPrefixes too!!!
#ifdef REFKEEN_VER_KDREAMS
	{"altcontrolscheme_jump=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_throw=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_stats=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
#else
	{"altcontrolscheme_fire=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_strafe=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_drink=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_bolt=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_nuke=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_fastturn=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	{"altcontrolscheme_scrolls=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	{"altcontrolscheme_funckeys=", &BEL_SDL_ParseSetting_AlternativeControlSchemeKeyMap},
#endif

	{"altcontrolscheme_movement=", &BEL_SDL_ParseSetting_AlternativeControlSchemeMovementMap},
	{"altcontrolscheme_menumouse=", &BEL_SDL_ParseSetting_AlternativeControlSchemeMenuMouseMap},

#ifdef BE_SDL_ENABLE_FARPTR_CFG
	{"farptrsegoffset=", &BEL_SDL_ParseSetting_FarPtrSegOffset},
#endif
};

static void BEL_SDL_ParseConfig(void)
{
	// Defaults
	g_refKeenCfg.isFullscreen = false;
	g_refKeenCfg.fullWidth = 0;
	g_refKeenCfg.fullHeight = 0;
	g_refKeenCfg.winWidth = 0;
	g_refKeenCfg.winHeight = 0;
	g_refKeenCfg.displayNum = 0;
	g_refKeenCfg.sdlRendererDriver = -1;
	g_refKeenCfg.vSync = VSYNC_AUTO;
	g_refKeenCfg.isBilinear = true;
	g_refKeenCfg.scaleType = SCALE_ASPECT;
	g_refKeenCfg.scaleFactor = 2;
	g_refKeenCfg.autolockCursor = false;
	g_refKeenCfg.sndSampleRate = 49716; // TODO should be a shared define
	g_refKeenCfg.disableSoundSubSystem = false;
	g_refKeenCfg.altControlScheme.isEnabled = false;

#ifdef REFKEEN_VER_KDREAMS
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_JUMP] = SDL_CONTROLLER_BUTTON_A;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_THROW] = SDL_CONTROLLER_BUTTON_B;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_STATS] = SDL_CONTROLLER_BUTTON_X;
#else
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FIRE] = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_STRAFE] = SDL_CONTROLLER_BUTTON_B;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_DRINK] = SDL_CONTROLLER_BUTTON_A;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_BOLT] = SDL_CONTROLLER_BUTTON_X;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_NUKE] = SDL_CONTROLLER_BUTTON_Y;
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FASTTURN] = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_SCROLLS] = SDL_CONTROLLER_BUTTON_MAX+1; // HACK for getting right trigger (technically an axis)
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FUNCKEYS] = SDL_CONTROLLER_BUTTON_MAX; // HACK for getting left trigger (technically an axis)
#endif
	g_refKeenCfg.altControlScheme.movementMapping = 0; // FIXME should be an enum constant
	g_refKeenCfg.altControlScheme.menuMouseMapping = 1; // FIXME should be an enum constant


#ifdef BE_SDL_ENABLE_FARPTR_CFG
	g_refKeenCfg.farPtrSegOffset = BE_SDL_DEFAULT_FARPTRSEGOFFSET;
#endif
	// Try to load config
	FILE *fp = fopen(REFKEEN_DREAMS_CONFIG_FILEPATH, "r");
	if (fp)
	{
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
	// Try to save current settings just in case (first time file is created or new fields added)
	fp = fopen(REFKEEN_DREAMS_CONFIG_FILEPATH, "w");
	if (!fp)
	{
		return;
	}
	fprintf(fp, "fullscreen=%s\n", g_refKeenCfg.isFullscreen ? "true" : "false");
	fprintf(fp, "fullres=%dx%d\n", g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight);
	fprintf(fp, "windowres=%dx%d\n", g_refKeenCfg.winWidth, g_refKeenCfg.winHeight);
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
	fprintf(fp, "disablesndsubsystem=%s\n", g_refKeenCfg.disableSoundSubSystem ? "true" : "false");
	fprintf(fp, "altcontrolscheme=%s\n", g_refKeenCfg.altControlScheme.isEnabled ? "true" : "false");
	// Go through an array of keys
	for (int keyindex = 0; keyindex < CONTROLSCHEME_CONFIG_BUTMAP_AFTERLAST; ++keyindex)
	{
		fprintf(fp, "%s%s\n", g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex], g_sdlControlSchemeKeyMapCfgVals[g_refKeenCfg.altControlScheme.actionMappings[keyindex]]);
	}
	fprintf(fp, "altcontrolscheme_movement=%s\n", g_sdlControlSchemeMovementMapCfgVals[g_refKeenCfg.altControlScheme.movementMapping]);
	fprintf(fp, "altcontrolscheme_menumouse=%s\n", g_sdlControlSchemeMenuMouseMapCfgVals[g_refKeenCfg.altControlScheme.menuMouseMapping]);
#ifdef BE_SDL_ENABLE_FARPTR_CFG
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
    {false,  EMULATEDKEYSCANCODE_A}, // SDL_SCANCODE_A
    {false,  EMULATEDKEYSCANCODE_B}, // SDL_SCANCODE_B
    {false,  EMULATEDKEYSCANCODE_C}, // SDL_SCANCODE_C
    {false,  EMULATEDKEYSCANCODE_D}, // SDL_SCANCODE_D
    {false,  EMULATEDKEYSCANCODE_E}, // SDL_SCANCODE_E
    {false,  EMULATEDKEYSCANCODE_F}, // SDL_SCANCODE_F
    {false,  EMULATEDKEYSCANCODE_G}, // SDL_SCANCODE_G
    {false,  EMULATEDKEYSCANCODE_H}, // SDL_SCANCODE_H
    {false,  EMULATEDKEYSCANCODE_I}, // SDL_SCANCODE_I
    {false,  EMULATEDKEYSCANCODE_J}, // SDL_SCANCODE_J
    {false,  EMULATEDKEYSCANCODE_K}, // SDL_SCANCODE_K
    {false,  EMULATEDKEYSCANCODE_L}, // SDL_SCANCODE_L
    {false,  EMULATEDKEYSCANCODE_M}, // SDL_SCANCODE_M
    {false,  EMULATEDKEYSCANCODE_N}, // SDL_SCANCODE_N
    {false,  EMULATEDKEYSCANCODE_O}, // SDL_SCANCODE_O
    {false,  EMULATEDKEYSCANCODE_P}, // SDL_SCANCODE_P
    {false,  EMULATEDKEYSCANCODE_Q}, // SDL_SCANCODE_Q
    {false,  EMULATEDKEYSCANCODE_R}, // SDL_SCANCODE_R
    {false,  EMULATEDKEYSCANCODE_S}, // SDL_SCANCODE_S
    {false,  EMULATEDKEYSCANCODE_T}, // SDL_SCANCODE_T
    {false,  EMULATEDKEYSCANCODE_U}, // SDL_SCANCODE_U
    {false,  EMULATEDKEYSCANCODE_V}, // SDL_SCANCODE_V
    {false,  EMULATEDKEYSCANCODE_W}, // SDL_SCANCODE_W
    {false,  EMULATEDKEYSCANCODE_X}, // SDL_SCANCODE_X
    {false,  EMULATEDKEYSCANCODE_Y}, // SDL_SCANCODE_Y
    {false,  EMULATEDKEYSCANCODE_Z}, // SDL_SCANCODE_Z

    {false,  EMULATEDKEYSCANCODE_1}, // SDL_SCANCODE_1
    {false,  EMULATEDKEYSCANCODE_2}, // SDL_SCANCODE_2
    {false,  EMULATEDKEYSCANCODE_3}, // SDL_SCANCODE_3
    {false,  EMULATEDKEYSCANCODE_4}, // SDL_SCANCODE_4
    {false,  EMULATEDKEYSCANCODE_5}, // SDL_SCANCODE_5
    {false,  EMULATEDKEYSCANCODE_6}, // SDL_SCANCODE_6
    {false,  EMULATEDKEYSCANCODE_7}, // SDL_SCANCODE_7
    {false,  EMULATEDKEYSCANCODE_8}, // SDL_SCANCODE_8
    {false,  EMULATEDKEYSCANCODE_9}, // SDL_SCANCODE_9
    {false,  EMULATEDKEYSCANCODE_0}, // SDL_SCANCODE_0

    {false,  EMULATEDKEYSCANCODE_ENTER}, // SDL_SCANCODE_RETURN
    {false,  EMULATEDKEYSCANCODE_ESC}, // SDL_SCANCODE_ESCAPE
    {false,  EMULATEDKEYSCANCODE_BSPACE}, // SDL_SCANCODE_BACKSPACE
    {false,  EMULATEDKEYSCANCODE_TAB}, // SDL_SCANCODE_TAB
    {false,  EMULATEDKEYSCANCODE_SPACE}, // SDL_SCANCODE_SPACE

    {false,  EMULATEDKEYSCANCODE_MINUS}, // SDL_SCANCODE_MINUS
    {false,  EMULATEDKEYSCANCODE_EQUALS}, // SDL_SCANCODE_EQUALS
    {false,  EMULATEDKEYSCANCODE_LBRACKET}, // SDL_SCANCODE_LEFTBRACKET
    {false,  EMULATEDKEYSCANCODE_RBRACKET}, // SDL_SCANCODE_RIGHTBRACKET
    {false,  EMULATEDKEYSCANCODE_BACKSLASH}, // SDL_SCANCODE_BACKSLASH

    emptyDOSKeyEvent, // SDL_SCANCODE_NONUSHASH

    {false,  EMULATEDKEYSCANCODE_SEMICOLON}, // SDL_SCANCODE_SEMICOLON
    {false,  EMULATEDKEYSCANCODE_QUOTE}, // SDL_SCANCODE_APOSTROPHE
    {false,  EMULATEDKEYSCANCODE_GRAVE}, // SDL_SCANCODE_GRAVE
    {false,  EMULATEDKEYSCANCODE_COMMA}, // SDL_SCANCODE_COMMA
    {false,  EMULATEDKEYSCANCODE_PERIOD}, // SDL_SCANCODE_PERIOD
    {false,  EMULATEDKEYSCANCODE_SLASH}, // SDL_SCANCODE_SLASH

    {false,  EMULATEDKEYSCANCODE_CAPSLOCK}, // SDL_SCANCODE_CAPSLOCK

    {false,  EMULATEDKEYSCANCODE_F1}, // SDL_SCANCODE_F1
    {false,  EMULATEDKEYSCANCODE_F2}, // SDL_SCANCODE_F2
    {false,  EMULATEDKEYSCANCODE_F3}, // SDL_SCANCODE_F3
    {false,  EMULATEDKEYSCANCODE_F4}, // SDL_SCANCODE_F4
    {false,  EMULATEDKEYSCANCODE_F5}, // SDL_SCANCODE_F5
    {false,  EMULATEDKEYSCANCODE_F6}, // SDL_SCANCODE_F6
    {false,  EMULATEDKEYSCANCODE_F7}, // SDL_SCANCODE_F7
    {false,  EMULATEDKEYSCANCODE_F8}, // SDL_SCANCODE_F8
    {false,  EMULATEDKEYSCANCODE_F9}, // SDL_SCANCODE_F9
    {false,  EMULATEDKEYSCANCODE_F10}, // SDL_SCANCODE_F10
    {false,  EMULATEDKEYSCANCODE_F11}, // SDL_SCANCODE_F11
    {false,  EMULATEDKEYSCANCODE_F12}, // SDL_SCANCODE_F12

    {false,  EMULATEDKEYSCANCODE_PRINTSCREEN}, // SDL_SCANCODE_PRINTSCREEN
    {false,  EMULATEDKEYSCANCODE_SCROLLLOCK}, // SDL_SCANCODE_SCROLLLOCK
    {false,  EMULATEDKEYSCANCODE_PAUSE}, // SDL_SCANCODE_PAUSE
    {true,  EMULATEDKEYSCANCODE_INSERT}, // SDL_SCANCODE_INSERT
    {true,  EMULATEDKEYSCANCODE_HOME}, // SDL_SCANCODE_HOME
    {true,  EMULATEDKEYSCANCODE_PAGEUP}, // SDL_SCANCODE_PAGEUP
    {true,  EMULATEDKEYSCANCODE_DELETE}, // SDL_SCANCODE_DELETE
    {true,  EMULATEDKEYSCANCODE_END}, // SDL_SCANCODE_END
    {true,  EMULATEDKEYSCANCODE_PAGEDOWN}, // SDL_SCANCODE_PAGEDOWN
    {true,  EMULATEDKEYSCANCODE_RIGHT}, // SDL_SCANCODE_RIGHT
    {true,  EMULATEDKEYSCANCODE_LEFT}, // SDL_SCANCODE_LEFT
    {true,  EMULATEDKEYSCANCODE_DOWN}, // SDL_SCANCODE_DOWN
    {true,  EMULATEDKEYSCANCODE_UP}, // SDL_SCANCODE_UP

    {false,  EMULATEDKEYSCANCODE_NUMLOCK}, // SDL_SCANCODE_NUMLOCKCLEAR
    {true,  EMULATEDKEYSCANCODE_KP_DIVIDE}, // SDL_SCANCODE_KP_DIVIDE
    {false,  EMULATEDKEYSCANCODE_KP_MULTIPLY}, // SDL_SCANCODE_KP_MULTIPLY
    {false,  EMULATEDKEYSCANCODE_KP_MINUS}, // SDL_SCANCODE_KP_MINUS
    {false,  EMULATEDKEYSCANCODE_KP_PLUS}, // SDL_SCANCODE_KP_PLUS
    {true,  EMULATEDKEYSCANCODE_KP_ENTER}, // SDL_SCANCODE_KP_ENTER
    {false,  EMULATEDKEYSCANCODE_KP_1}, // SDL_SCANCODE_KP_1
    {false,  EMULATEDKEYSCANCODE_KP_2}, // SDL_SCANCODE_KP_2
    {false,  EMULATEDKEYSCANCODE_KP_3}, // SDL_SCANCODE_KP_3
    {false,  EMULATEDKEYSCANCODE_KP_4}, // SDL_SCANCODE_KP_4
    {false,  EMULATEDKEYSCANCODE_KP_5}, // SDL_SCANCODE_KP_5
    {false,  EMULATEDKEYSCANCODE_KP_6}, // SDL_SCANCODE_KP_6
    {false,  EMULATEDKEYSCANCODE_KP_7}, // SDL_SCANCODE_KP_7
    {false,  EMULATEDKEYSCANCODE_KP_8}, // SDL_SCANCODE_KP_8
    {false,  EMULATEDKEYSCANCODE_KP_9}, // SDL_SCANCODE_KP_9
    {false,  EMULATEDKEYSCANCODE_KP_0}, // SDL_SCANCODE_KP_0
    {false,  EMULATEDKEYSCANCODE_KP_PERIOD}, // SDL_SCANCODE_KP_PERIOD

    {false,  EMULATEDKEYSCANCODE_LESSTHAN}, // SDL_SCANCODE_NONUSBACKSLASH

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

    {false,  EMULATEDKEYSCANCODE_LCTRL}, // SDL_SCANCODE_LCTRL
    {false,  EMULATEDKEYSCANCODE_LSHIFT}, // SDL_SCANCODE_LSHIFT
    {false,  EMULATEDKEYSCANCODE_LALT}, // SDL_SCANCODE_LALT

    emptyDOSKeyEvent, // SDL_SCANCODE_LGUI

    {true,  EMULATEDKEYSCANCODE_RCTRL}, // SDL_SCANCODE_RCTRL
    {false,  EMULATEDKEYSCANCODE_RSHIFT}, // SDL_SCANCODE_RSHIFT
    {true,  EMULATEDKEYSCANCODE_RALT}, // SDL_SCANCODE_RALT

    // More SDL 2.0 scancodes follow, but are ignored
};
#else
#error "SDL <2.0 support is unimplemented!"
#endif


void BE_SDL_StartKeyboardService(void (*funcPtr)(uint8_t))
{
	g_sdlKeyboardInterruptFuncPtr = funcPtr;
}

void BE_SDL_StopKeyboardService(void)
{
	g_sdlKeyboardInterruptFuncPtr = 0;
}

void BE_SDL_GetMouseDelta(int16_t *x, int16_t *y)
{
	int ourx, oury;
	SDL_GetRelativeMouseState(&ourx, &oury);

	// FIXME: Use enum for menuMouseMapping
	if (g_refKeenCfg.altControlScheme.isEnabled && g_refKeenCfg.altControlScheme.menuMouseMapping && (g_sdlControllerActualCurrPtr->schemeType == CONTROLSCHEME_MENU))
	{
		for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				// Remove deadzone and then scale
				int currnormx = SDL_GameControllerGetAxis(g_sdlControllers[i], g_sdlXAxisForMenuMouse);
				currnormx = (currnormx >= 8192) ? ((currnormx-8192) / 4096) : (currnormx <= -8192) ? ((currnormx+8192) / 4096) : 0;
				int currnormy = SDL_GameControllerGetAxis(g_sdlControllers[i], g_sdlYAxisForMenuMouse);
				currnormy = (currnormy >= 8192) ? ((currnormy-8192) / 4096) : (currnormy <= -8192) ? ((currnormy+8192) / 4096) : 0;
				// FIXME: We should really use an ABS function here...
				if (currnormx*currnormx > ourx*ourx)
				{
					ourx = currnormx;
				}
				if (currnormy*currnormy > oury*oury)
				{
					oury = currnormy;
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

uint16_t BE_SDL_GetMouseButtons(void)
{
	static uint16_t results[] = {0, 1, 4, 5, 2, 3, 6, 7};
	return results[SDL_GetMouseState(NULL, NULL) & 7];
}

void BE_SDL_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp)
{
	int emuAxisStart = (joy != 0) ? 2 : 0;
	int minX = BE_SDL_EMU_JOYSTICK_RANGEMAX, minY = BE_SDL_EMU_JOYSTICK_RANGEMAX, maxX = 0, maxY = 0;
	for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i, emuAxisStart ^= 2)
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
				int axisNormalizedVal = ((int)SDL_JoystickGetAxis(g_sdlJoysticks[i], axis)+32768)*BE_SDL_EMU_JOYSTICK_RANGEMAX/65535;
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
	*xp = minX < (BE_SDL_EMU_JOYSTICK_RANGEMAX-maxX) ? minX : maxX;
	*yp = minY < (BE_SDL_EMU_JOYSTICK_RANGEMAX-maxY) ? minY : maxY;
}

uint16_t BE_SDL_GetJoyButtons(uint16_t joy)
{
	int emuButMaskStart = (joy != 0) ? 4 : 1;
	int result = 0;
	for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i, emuButMaskStart ^= 5)
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

int16_t BE_SDL_KbHit(void)
{
	return g_sdlLastKeyScanCode;
}

int16_t BE_SDL_BiosScanCode(int16_t command)
{
	if (command == 1)
	{
		return g_sdlLastKeyScanCode;
	}

	while (!g_sdlLastKeyScanCode)
	{
		BE_SDL_ShortSleep();
	}
	int16_t result = g_sdlLastKeyScanCode;
	g_sdlLastKeyScanCode = 0;
	return result;
}


static void BEL_SDL_HandleEmuKeyboardEvent(bool isPressed, emulatedDOSKeyEvent keyEvent)
{
	if (keyEvent.dosScanCode == EMULATEDKEYSCANCODE_PAUSE)
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


#if 0
// TODO: Should be (at least partially) configurable
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATADVENTURES)
static const BESDLControllerMap g_sdlControllerToScanCodeMap_inGameTemplate = {
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_SPACE, // SDL_CONTROLLER_BUTTON_A, drink potion
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_LALT*/, // SDL_CONTROLLER_BUTTON_B, strafe (configurable KEYBOARD SCANCODE)
#ifdef REFKEEN_VER_CAT3D
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_B, // SDL_CONTROLLER_BUTTON_X, cast nuke in Cat3D
#elif defined REFKEEN_VER_CATADVENTURES
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_Z, // SDL_CONTROLLER_BUTTON_X, cast nuke in CatAdv
#endif
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ENTER, // SDL_CONTROLLER_BUTTON_Y, cast bolt
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_LCTRL*/, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER, fire (configurable KEYBOARD SCANCODE)
#ifdef REFKEEN_VER_CAT3D
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RSHIFT, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, turn fast and run in Cat3D
#elif defined REFKEEN_VER_CATADVENTURES
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_TAB, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, turn fast in CatAdv (and that's it)
#endif
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_UP*/, // SDL_CONTROLLER_DPAD_UP (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_DOWN*/, // SDL_CONTROLLER_DPAD_DOWN (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_LEFT*/, // SDL_CONTROLLER_DPAD_LEFT (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_RIGHT*/, // SDL_CONTROLLER_DPAD_RIGHT (configurable KEYBOARD SCANCODE)

	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LEFT, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_LEFTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_UP, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_LEFTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LEFT, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_RIGHTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_UP, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_RIGHTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_HELPER, CONTROLSCHEMEMAP_HELPER_FUNCKEYS, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_HELPER, CONTROLSCHEMEMAP_HELPER_SCROLL, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT
#else
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT
#endif

	CONTROLSCHEME_GAME
};
#elif defined REFKEEN_VER_KDREAMS
static const BESDLControllerMap g_sdlControllerToScanCodeMap_inGameTemplate = {
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_LCTRL*/, // SDL_CONTROLLER_BUTTON_A, jump (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_LALT*/, // SDL_CONTROLLER_BUTTON_B, throw flowerpower (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_SPACE, // SDL_CONTROLLER_BUTTON_X, show stats
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_Y
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_UP*/, // SDL_CONTROLLER_DPAD_UP (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_DOWN*/, // SDL_CONTROLLER_DPAD_DOWN (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_LEFT*/, // SDL_CONTROLLER_DPAD_LEFT (configurable KEYBOARD SCANCODE)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0/*EMULATEDKEYSCANCODE_RIGHT*/, // SDL_CONTROLLER_DPAD_RIGHT (configurable KEYBOARD SCANCODE)

	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LEFT, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_LEFTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_UP, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_LEFTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LEFT, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_RIGHTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_UP, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_RIGHTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_HELPER, CONTROLSCHEMEMAP_HELPER_FUNCKEYS, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	0, 0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_GAME
};
#endif
#endif

// Some arbitrary choices
static const BESDLControllerMap g_sdlControllerToScanCodeMap_inputWait = {
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ENTER, // SDL_CONTROLLER_BUTTON_A
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LALT, // SDL_CONTROLLER_BUTTON_B
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_SPACE, // SDL_CONTROLLER_BUTTON_X
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LCTRL, // SDL_CONTROLLER_BUTTON_Y
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LSHIFT, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_TAB, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_UP, // SDL_CONTROLLER_DPAD_UP
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_DPAD_DOWN
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_LEFT, // SDL_CONTROLLER_DPAD_LEFT
	CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_DPAD_RIGHT

	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_LEFTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_LEFTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_RIGHTX
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, // SDL_CONTROLLER_AXIS_RIGHTY
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_F1, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	CONTROLSCHEMEMAP_TYPE_KEYVAL, 0, CONTROLSCHEMEMAP_TYPE_KEYVAL, EMULATEDKEYSCANCODE_BSPACE, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_INPUTWAIT
};

static BESDLControllerMapEntry * BEL_SDL_GetKeyMapPtrFromCfgVal(BESDLControllerMap *controllerMapPtr, int mappingCfgVal)
{
	if (mappingCfgVal < SDL_CONTROLLER_BUTTON_MAX)
		return &(controllerMapPtr->buttonsMap[mappingCfgVal]);
	// Special handling for triggers
	return &(controllerMapPtr->axesMap[mappingCfgVal-SDL_CONTROLLER_BUTTON_MAX+SDL_CONTROLLER_AXIS_TRIGGERLEFT][1]);
}

void BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low(const char *scanCodes, BESDLControllerMap *controllerMapPtr, bool clearAll);

static bool BEL_SDL_AltControlScheme_HandleEntry(BESDLControllerMapEntry entry, bool isPressed)
{
	switch (entry.type)
	{
	case CONTROLSCHEMEMAP_TYPE_KEYVAL:
	{
		if (!entry.val)
			return false; // Nothing is defined here

		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = entry.val;
		BEL_SDL_HandleEmuKeyboardEvent(isPressed, dosKeyEvent);
		break;
	}
	case CONTROLSCHEMEMAP_TYPE_HELPER:
		if (!isPressed)
			return true; // Do nothing but confirm

		switch (entry.val)
		{
		case CONTROLSCHEMEMAP_HELPER_FUNCKEYS: // Misc. in-game shortcuts (to sub-panels/menus)
#ifdef REFKEEN_VER_KDREAMS
			BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low((const char []){EMULATEDKEYSCANCODE_F1, EMULATEDKEYSCANCODE_F2, EMULATEDKEYSCANCODE_F3, EMULATEDKEYSCANCODE_F5, EMULATEDKEYSCANCODE_F6, EMULATEDKEYSCANCODE_F7, EMULATEDKEYSCANCODE_ESC, 0}, &g_sdlControllerLowPriorityMap, true);
			g_sdlControllerActualCurrPtr = &g_sdlControllerLowPriorityMap;
#elif defined REFKEEN_VER_CATADVENTURES
			BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low((const char []){EMULATEDKEYSCANCODE_F1, EMULATEDKEYSCANCODE_F2, EMULATEDKEYSCANCODE_F3, EMULATEDKEYSCANCODE_F4, EMULATEDKEYSCANCODE_ESC, 0}, &g_sdlControllerLowPriorityMap, true);
			g_sdlControllerActualCurrPtr = &g_sdlControllerLowPriorityMap;
#endif
			break;
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
		case CONTROLSCHEMEMAP_HELPER_SCROLLS: // Scroll reading
			BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low((const char []){EMULATEDKEYSCANCODE_1, EMULATEDKEYSCANCODE_2, EMULATEDKEYSCANCODE_3, EMULATEDKEYSCANCODE_4, EMULATEDKEYSCANCODE_5, EMULATEDKEYSCANCODE_6, EMULATEDKEYSCANCODE_7, EMULATEDKEYSCANCODE_8, EMULATEDKEYSCANCODE_ESC, 0}, &g_sdlControllerLowPriorityMap, true);
			g_sdlControllerActualCurrPtr = &g_sdlControllerLowPriorityMap;
			break;
#endif
		}
		break;
	}
	return true;
}


static void BEL_SDL_AltControlScheme_CleanUp(void)
{
	if (!g_sdlControllerSchemeNeedsCleanUp)
		return;

	if (g_sdlControllerActualCurrPtr->schemeType == CONTROLSCHEME_TEXTINPUT)
	{
		int BEL_SDL_GetPressedKeyScanCodeFromTextInputUI(void);
		bool BEL_SDL_IsTextInputUIShifted(void);

		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = BEL_SDL_GetPressedKeyScanCodeFromTextInputUI();
		// Don't forget to "release" a key pressed in the text input UI
		if (dosKeyEvent.dosScanCode)
		{
			BEL_SDL_HandleEmuKeyboardEvent(false, dosKeyEvent);
		}
		// Shift key may further be held, don't forget this too!
		if ((dosKeyEvent.dosScanCode != EMULATEDKEYSCANCODE_LSHIFT) && BEL_SDL_IsTextInputUIShifted())
		{
			dosKeyEvent.dosScanCode = EMULATEDKEYSCANCODE_LSHIFT;
			BEL_SDL_HandleEmuKeyboardEvent(false, dosKeyEvent);
		}
	}
	// Otherwise simulate key releases based on the mapping
	else
	{
		for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
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
						BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->buttonsMap[but], false);
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
						BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->axesMap[axis][0], false);
					}
					// Repeat with positive
					if (g_sdlControllersAxesStates[i][axis][1])
					{
						// Do NOT force this - axis can be re-detected as "pressed" (in this direction) immediately otherwise!
						// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than a key press.
						//g_sdlControllersAxesStates[i][axis][1] = false;
						BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->axesMap[axis][1], false);
					}
				}
			}
		}
	}

	memset(g_sdlControllersButtonsStates, 0, sizeof(g_sdlControllersButtonsStates));
	memset(g_sdlControllersAxesStates, 0, sizeof(g_sdlControllersAxesStates));

	extern void BEL_SDL_HideAltInputUI(void);
	BEL_SDL_HideAltInputUI();

	g_sdlControllerSchemeNeedsCleanUp = false;
}


void BE_SDL_AltControlScheme_Push(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	// Don't forget this! - Also the only case where g_sdlControllerSchemeNeedsCleanUp is *not* set to true afterwards
	// (so when e.g., BE_SDL_AltControlScheme_PrepareInGameControls is called, no cleanup of wrong mapping is done again)
	BEL_SDL_AltControlScheme_CleanUp();

	++g_sdlControllertoScanCodeMaps.currPtr;
	if (g_sdlControllertoScanCodeMaps.currPtr == g_sdlControllertoScanCodeMaps.endPtr)
	{
		BE_SDL_SetScreenMode(3);
		BE_SDL_Simplified_printf("Error in BE_SDL_AltControlScheme_Push: Out of stack bounds!\n");
		BE_SDL_HandleExit(0);
	}

	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;
}

static void BEL_SDL_AltControlScheme_QuicklyShowCurrentUI(void)
{
	switch (g_sdlControllertoScanCodeMaps.currPtr->schemeType)
	{
	case CONTROLSCHEME_FACE:
	{
		extern void BEL_SDL_PrepareToShowFaceButtonsAndDpad(const char *scanCodes);
		BEL_SDL_PrepareToShowFaceButtonsAndDpad(g_sdlControllertoScanCodeMaps.currPtr->miscBuffer);
		break;
	}
	case CONTROLSCHEME_TEXTINPUT:
	{
		extern void BEL_SDL_PrepareToShowTextInputUI(void);
		BEL_SDL_PrepareToShowTextInputUI();
		break;
	}
	default:
		;
	}
}

void BE_SDL_AltControlScheme_Pop(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

	if (g_sdlControllertoScanCodeMaps.currPtr == &g_sdlControllertoScanCodeMaps.stack[0])
	{
		BE_SDL_SetScreenMode(3);
		BE_SDL_Simplified_printf("Error in BE_SDL_AltControlScheme_Pop: Popped more than necessary!\n");
		BE_SDL_HandleExit(0);
	}
	--g_sdlControllertoScanCodeMaps.currPtr;

	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	BEL_SDL_AltControlScheme_QuicklyShowCurrentUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}



void BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low(const char *scanCodes, BESDLControllerMap *controllerMapPtr, bool clearAll)
{
	BEL_SDL_AltControlScheme_CleanUp();
	g_sdlControllerActualCurrPtr = controllerMapPtr;

	if (clearAll)
	{
		memset(controllerMapPtr, 0, sizeof(g_sdlControllerToScanCodeMap_default));
	}
	else
	{
		memcpy(controllerMapPtr, &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	}
	controllerMapPtr->schemeType = CONTROLSCHEME_FACE;

	int counter = 0, sdlButtonNum = SDL_CONTROLLER_BUTTON_A; // A->B->X->Y (same order as SDL button numbers), then D-pad Down->Right->Left->Up (NOT the same order)
	char *actualScanCodePtr = controllerMapPtr->miscBuffer;
	for (const char *chPtr = scanCodes; *chPtr; ++chPtr)
	{
		// FIXME - Support gamepads with less buttons?
		// SPECIAL KEYS - These can use different buttons instead
		if ((*chPtr == EMULATEDKEYSCANCODE_ESC) || (*chPtr == EMULATEDKEYSCANCODE_PAUSE))
		{
			continue;
		}
		if (++counter == 9)
		{
			// Better be consistent
			BE_SDL_SetScreenMode(3);
			BE_SDL_puts("Error in BE_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes:");
			BE_SDL_puts("Gotten too many scancodes as the input, input scancodes string:");
			BE_SDL_puts(scanCodes); // Do NOT use printf cause if we support some formatting this is not safe!
			BE_SDL_HandleExit(0);
		}
		*actualScanCodePtr++ = *chPtr;
		controllerMapPtr->buttonsMap[sdlButtonNum].val = *chPtr;
		switch (sdlButtonNum)
		{
		case SDL_CONTROLLER_BUTTON_A: sdlButtonNum = SDL_CONTROLLER_BUTTON_B; break;
		case SDL_CONTROLLER_BUTTON_B: sdlButtonNum = SDL_CONTROLLER_BUTTON_X; break;
		case SDL_CONTROLLER_BUTTON_X: sdlButtonNum = SDL_CONTROLLER_BUTTON_Y; break;
		case SDL_CONTROLLER_BUTTON_Y: sdlButtonNum = SDL_CONTROLLER_BUTTON_DPAD_DOWN; break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: sdlButtonNum = SDL_CONTROLLER_BUTTON_DPAD_RIGHT; break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: sdlButtonNum = SDL_CONTROLLER_BUTTON_DPAD_LEFT; break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: sdlButtonNum = SDL_CONTROLLER_BUTTON_DPAD_UP; break;
		//case SDL_CONTROLLER_BUTTON_DPAD_UP:
		}
	}
	*actualScanCodePtr = '\0';
	extern void BEL_SDL_PrepareToShowFaceButtonsAndDpad(const char *scanCodes);
	BEL_SDL_PrepareToShowFaceButtonsAndDpad(controllerMapPtr->miscBuffer); // This shouldn't care about pause and co.

	g_sdlControllerSchemeNeedsCleanUp = true;
}


void BE_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes(const char *scanCodes)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low(scanCodes, g_sdlControllertoScanCodeMaps.currPtr, false);
}

void BE_SDL_AltControlScheme_PreparePageScrollingControls(int prevPageScan, int nextPageScan)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_CleanUp();
	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_PAGE;
	g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_LEFTSHOULDER].val = prevPageScan;
	g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].val = nextPageScan;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareMenuControls(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_CleanUp();
	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_MENU;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareInGameControls(int primaryScanCode, int secondaryScanCode, int upScanCode, int downScanCode, int leftScanCode, int rightScanCode)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_CleanUp();
	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_inGameTemplate, sizeof(g_sdlControllerToScanCodeMap_inGameTemplate));

#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATADVENTURES)
	BEL_SDL_GetKeyMapPtrFromCfgVal(g_sdlControllertoScanCodeMaps.currPtr, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_FIRE])->val = primaryScanCode;
	BEL_SDL_GetKeyMapPtrFromCfgVal(g_sdlControllertoScanCodeMaps.currPtr, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_STRAFE])->val = secondaryScanCode;
#elif (defined REFKEEN_VER_KDREAMS)
	BEL_SDL_GetKeyMapPtrFromCfgVal(g_sdlControllertoScanCodeMaps.currPtr, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_JUMP])->val = primaryScanCode;
	BEL_SDL_GetKeyMapPtrFromCfgVal(g_sdlControllertoScanCodeMaps.currPtr, g_refKeenCfg.altControlScheme.actionMappings[CONTROLSCHEME_CONFIG_BUTMAP_THROW])->val = secondaryScanCode;
#endif
	switch (g_refKeenCfg.altControlScheme.movementMapping)
	{ // HACK (named constants are better)
	case 0:
		g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_DPAD_UP].val = upScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_DPAD_DOWN].val = downScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_DPAD_LEFT].val = leftScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT].val = rightScanCode;
		break;
	case 1:
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_LEFTY][0].val = upScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_LEFTY][1].val = downScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_LEFTX][0].val = leftScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_LEFTX][1].val = rightScanCode;
		break;
	case 2:
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_RIGHTY][0].val = upScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_RIGHTY][1].val = downScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_RIGHTX][0].val = leftScanCode;
		g_sdlControllertoScanCodeMaps.currPtr->axesMap[SDL_CONTROLLER_AXIS_RIGHTX][1].val = rightScanCode;
		break;
	}
	//
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_GAME;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareInputWaitControls(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_CleanUp();
	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_inputWait, sizeof(g_sdlControllerToScanCodeMap_inputWait));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_INPUTWAIT;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareTextInput(void)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
		return;

	BEL_SDL_AltControlScheme_CleanUp();
	g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_TEXTINPUT;

	extern void BEL_SDL_PrepareToShowTextInputUI(void);
	BEL_SDL_PrepareToShowTextInputUI();

	g_sdlControllerSchemeNeedsCleanUp = true;
}


void BE_SDL_PollEvents(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			BEL_SDL_HandleEmuKeyboardEvent(event.type == SDL_KEYDOWN, sdlKeyMappings[event.key.keysym.scancode]);
			break;
		//case SDL_MOUSEBUTTONUP:
		//	BEL_SDL_toggleCursorConditionally(true);
		//	break;

		/* Don't use SDL_CONTROLLERDEVICEADDED with alternative controller schemes, and for the sake of consistency avoid SDL_CONTROLLERDEVICEREMOVED as well.
		 * Reason is that on init, there is a problem handling controller mappings loaded from the database using SDL_CONTROLLERDEVICEADDED
		 * (if loaded before init, the mappings seem to be deleted, otherwise SDL_CONTROLLERDEVICEADDED are just not spawned for these).
		 */
		case SDL_JOYDEVICEADDED:
			if (event.jdevice.which < BE_SDL_MAXJOYSTICKS)
			{
				if (!g_refKeenCfg.altControlScheme.isEnabled)
				{
					g_sdlJoysticks[event.jdevice.which] = SDL_JoystickOpen(event.jdevice.which);
				}
				else if (SDL_IsGameController(event.jdevice.which))
				{
					g_sdlControllers[event.jdevice.which] = SDL_GameControllerOpen(event.jdevice.which);
					memset(g_sdlControllersActualButtonsStates[event.jdevice.which], 0, sizeof(g_sdlControllersActualButtonsStates[event.jdevice.which]));
					memset(g_sdlControllersActualAxesStates[event.jdevice.which], 0, sizeof(g_sdlControllersActualAxesStates[event.jdevice.which]));

					extern bool g_sdlShowControllerUI;
					g_sdlShowControllerUI = true;
					extern bool g_sdlForceGfxControlUiRefresh;
					g_sdlForceGfxControlUiRefresh = true;
				}
			}
			break;
		case SDL_JOYDEVICEREMOVED:
			if (!g_refKeenCfg.altControlScheme.isEnabled)
			{
				for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
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
				for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
				{
					if (g_sdlControllers[i])
					{
						if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i])) == event.cdevice.which)
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
				void BE_SDL_SetGfxOutputRects(void);
				BE_SDL_SetGfxOutputRects();
				//BE_SDL_MarkGfxForPendingUpdate();
				BE_SDL_MarkGfxForUpdate();
				break;
			}
			case SDL_WINDOWEVENT_EXPOSED:
				//BE_SDL_MarkGfxForPendingUpdate();
				BE_SDL_MarkGfxForUpdate();
				break;
			break;
		case SDL_QUIT:
			BE_SDL_ShutdownAll();
			exit(0);
			break;
		default: ;
		}
	}
	// If SDL_GameController is used, we don't poll for its events.
	// Furthermore, we emulate keyboard/mouse events from DOS.
	if (g_refKeenCfg.altControlScheme.isEnabled)
	{
		for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
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
							if (g_sdlControllerActualCurrPtr->schemeType == CONTROLSCHEME_TEXTINPUT)
							{
								emulatedDOSKeyEvent dosKeyEvent;
								dosKeyEvent.isSpecial = false;
								dosKeyEvent.dosScanCode = 0;

								extern int BEL_SDL_MoveUpInTextInputUI(void);
								extern int BEL_SDL_MoveDownInTextInputUI(void);
								extern int BEL_SDL_MoveLeftInTextInputUI(void);
								extern int BEL_SDL_MoveRightInTextInputUI(void);
								extern int BEL_SDL_ToggleShiftStateInTextInputUI(bool *pToggle);
								extern int BEL_SDL_ToggleKeyPressInTextInputUI(bool *pToggle);
								switch (but)
								{
								case SDL_CONTROLLER_BUTTON_DPAD_UP:
									if (isPressed)
										dosKeyEvent.dosScanCode = BEL_SDL_MoveUpInTextInputUI();
									isPressed = false; // Ensure a recently pressed onscreen keyboard is released
									break;
								case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
									if (isPressed)
										dosKeyEvent.dosScanCode = BEL_SDL_MoveDownInTextInputUI();
									isPressed = false; // Ensure a recently pressed onscreen keyboard is released
									break;
								case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
									if (isPressed)
										dosKeyEvent.dosScanCode = BEL_SDL_MoveLeftInTextInputUI();
									isPressed = false; // Ensure a recently pressed onscreen keyboard is released
									break;
								case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
									if (isPressed)
										dosKeyEvent.dosScanCode = BEL_SDL_MoveRightInTextInputUI();
									isPressed = false; // Ensure a recently pressed onscreen keyboard is released
									break;
								// A few other special cases
								case SDL_CONTROLLER_BUTTON_START:
									dosKeyEvent.dosScanCode = EMULATEDKEYSCANCODE_PAUSE;
									break;
								case SDL_CONTROLLER_BUTTON_B:
								case SDL_CONTROLLER_BUTTON_BACK:
									dosKeyEvent.dosScanCode = EMULATEDKEYSCANCODE_ESC;
									break;
								case SDL_CONTROLLER_BUTTON_X:
									// Change shift state (or at least try to).
									// NOTE: This can modify isPressed.
									dosKeyEvent.dosScanCode = BEL_SDL_ToggleShiftStateInTextInputUI(&isPressed);
									break;
								default:
								{
									// Select key from UI.
									// NOTE: This can modify isPressed e.g., for shift key.
									dosKeyEvent.dosScanCode = BEL_SDL_ToggleKeyPressInTextInputUI(&isPressed);
								}
								}

								if (dosKeyEvent.dosScanCode)
								{
									BEL_SDL_HandleEmuKeyboardEvent(isPressed, dosKeyEvent);
								}
							}
							// Try the usual otherwise (similar handling done with analog axes, triggers included)
							else if (!BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->buttonsMap[but], isPressed))
							{
								// If there's no mapping *and* we have the following set...
								if (g_sdlControllerActualCurrPtr == &g_sdlControllerLowPriorityMap)
								{
									// Hides face buttons and/or d-pad shown in-game, then returns to usual gameplay (or theoretically other) mapping
									BEL_SDL_AltControlScheme_CleanUp();
									g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;
									BEL_SDL_AltControlScheme_QuicklyShowCurrentUI();
									g_sdlControllerSchemeNeedsCleanUp = true;
								}
							}
						}
					}
				}
				// Repeat with analog axes (ignored with text input scheme in use)
				if (g_sdlControllerActualCurrPtr->schemeType == CONTROLSCHEME_TEXTINPUT)
					continue;

				for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; ++axis)
				{
					int axisVal = SDL_GameControllerGetAxis(g_sdlControllers[i], (SDL_GameControllerAxis)axis);
					// Is pressed in the negative direction?
					bool isNegPressed = (axisVal <= -8192);

					if (isNegPressed != g_sdlControllersActualAxesStates[i][axis][0])
					{
						g_sdlControllersActualAxesStates[i][axis][0] = isNegPressed;
						if (isNegPressed != g_sdlControllersAxesStates[i][axis][0])
						{
							g_sdlControllersAxesStates[i][axis][0] = isNegPressed;
							if (!BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->axesMap[axis][0], isNegPressed))
							{
								// If there's no mapping *and* we have the following set...
								if (g_sdlControllerActualCurrPtr == &g_sdlControllerLowPriorityMap)
								{
									// Hides face buttons and/or d-pad shown in-game, then returns to usual gameplay (or theoretically other) mapping
									BEL_SDL_AltControlScheme_CleanUp();
									g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;
									BEL_SDL_AltControlScheme_QuicklyShowCurrentUI();
									g_sdlControllerSchemeNeedsCleanUp = true;
								}
							}
						}
					}
					// Repeat with positive
					bool isPosPressed = (axisVal >= 8192);
					if (isPosPressed != g_sdlControllersActualAxesStates[i][axis][1])
					{
						g_sdlControllersActualAxesStates[i][axis][1] = isPosPressed;
						if (isPosPressed != g_sdlControllersAxesStates[i][axis][1])
						{
							g_sdlControllersAxesStates[i][axis][1] = isPosPressed;
							if (!BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->axesMap[axis][1], isPosPressed))
							{
								// If there's no mapping *and* we have the following set...
								if (g_sdlControllerActualCurrPtr == &g_sdlControllerLowPriorityMap)
								{
									// Hides face buttons and/or d-pad shown in-game, then returns to usual gameplay (or theoretically other) mapping
									BEL_SDL_AltControlScheme_CleanUp();
									g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;
									BEL_SDL_AltControlScheme_QuicklyShowCurrentUI();
									g_sdlControllerSchemeNeedsCleanUp = true;
								}
							}
						}
					}
					// Repeat with positive
#if 0

					if (isPosPressed != g_sdlControllersAxesStates[i][axis][1])
					{
						g_sdlControllersAxesStates[i][axis][1] = isPosPressed;
						// If there's no mapping, this is a trigger press *and* some additional condition applies...
						if (!BEL_SDL_AltControlScheme_HandleEntry(g_sdlControllerActualCurrPtr->axesMap[axis][1], isPosPressed) && (g_sdlControllerActualCurrPtr == &g_sdlControllerLowPriorityMap) && ((axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) || (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)))
						{
							// Hides face buttons and/or d-pad shown in-game, then returns to usual gameplay (or theoretically other) mapping
							BEL_SDL_AltControlScheme_CleanUp();
							g_sdlControllerActualCurrPtr = g_sdlControllertoScanCodeMaps.currPtr;
							BEL_SDL_AltControlScheme_QuicklyShowCurrentUI();
							g_sdlControllerSchemeNeedsCleanUp = true;
						}

#if 0
						// SPECIAL CASES
						if (g_sdlControllerActualCurrPtr->schemeType == CONTROLSCHEME_GAME)
						{
							if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
							{
								// Misc. in-game shortcuts (to sub-panels/menus)
#ifdef REFKEEN_VER_KDREAMS
								BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low((const char []){EMULATEDKEYSCANCODE_F1, EMULATEDKEYSCANCODE_F2, EMULATEDKEYSCANCODE_F3, EMULATEDKEYSCANCODE_F5, EMULATEDKEYSCANCODE_F6, EMULATEDKEYSCANCODE_F7, EMULATEDKEYSCANCODE_ESC, 0}, &g_sdlControllerLowPriorityMap, true);
								g_sdlControllerActualCurrPtr = &g_sdlControllerLowPriorityMap;
#elif defined REFKEEN_VER_CATADVENTURES
								BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low((const char []){EMULATEDKEYSCANCODE_F1, EMULATEDKEYSCANCODE_F2, EMULATEDKEYSCANCODE_F3, EMULATEDKEYSCANCODE_F4, EMULATEDKEYSCANCODE_ESC, 0}, &g_sdlControllerLowPriorityMap, true);
								g_sdlControllerActualCurrPtr = &g_sdlControllerLowPriorityMap;
#endif
							}
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
							if (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
							{
								// Scroll reading
								BEL_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes_Low((const char []){EMULATEDKEYSCANCODE_1, EMULATEDKEYSCANCODE_2, EMULATEDKEYSCANCODE_3, EMULATEDKEYSCANCODE_4, EMULATEDKEYSCANCODE_5, EMULATEDKEYSCANCODE_6, EMULATEDKEYSCANCODE_7, EMULATEDKEYSCANCODE_8, EMULATEDKEYSCANCODE_ESC, 0}, &g_sdlControllerLowPriorityMap, true);
								g_sdlControllerActualCurrPtr = &g_sdlControllerLowPriorityMap;
							}
#endif
						}
						if (g_sdlControllerActualCurrPtr->axesMap[axis][1])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllerActualCurrPtr->axesMap[axis][1];
							BEL_SDL_HandleEmuKeyboardEvent(isPosPressed, dosKeyEvent);
						}
#endif
					}
#endif
				}
			}
		}
	}
	// HACK - If audio subsystem is disabled we still want to at least
	// make the sound callback run (so e.g., no loop gets stuck waiting
	// for sound playback to complete)
	extern bool g_sdlAudioSubsystemUp;
	if (! g_sdlAudioSubsystemUp)
	{
		BE_SDL_PrepareForManualAudioSDServiceCall();
	}
}

#ifdef BE_SDL_ENABLE_FARPTR_CFG
uint16_t BE_SDL_Compat_GetFarPtrRelocationSegOffset(void)
{
	return g_refKeenCfg.farPtrSegOffset;
}
#endif
