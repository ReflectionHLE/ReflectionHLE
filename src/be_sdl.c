#include <stdio.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_sdl.h"

#define BE_SDL_MAXJOYSTICKS 8
#define BE_SDL_EMU_JOYSTICK_RANGEMAX 5000 // id_in.c MaxJoyValue
#define BE_SDL_DEFAULT_FARPTRSEGOFFSET 0x14

static void (*g_sdlKeyboardInterruptFuncPtr)(uint8_t) = 0;

static SDL_Joystick *g_sdlJoysticks[BE_SDL_MAXJOYSTICKS];
static SDL_GameController *g_sdlControllers[BE_SDL_MAXJOYSTICKS];

typedef enum { CONTROLSCHEME_DEFAULT = 0, CONTROLSCHEME_FACE, CONTROLSCHEME_PAGE, CONTROLSCHEME_MENU, CONTROLSCHEME_GAME, CONTROLSCHEME_TEXTINPUT } SchemeTypeEnumT;

typedef struct {
	int buttonsMap[SDL_CONTROLLER_BUTTON_MAX];
	int axesMap[SDL_CONTROLLER_AXIS_MAX][2];
	SchemeTypeEnumT schemeType;
	// Misc. buffer that may be useful in case of calling pop function
	char miscBuffer[5];
} BESDLControllerMap;

static bool g_sdlControllersButtonsStates[BE_SDL_MAXJOYSTICKS][SDL_CONTROLLER_BUTTON_MAX];
// We may optionally use analog axes as buttons (e.g., using stick as arrow keys, triggers as buttons)
static bool g_sdlControllersAxesStates[BE_SDL_MAXJOYSTICKS][SDL_CONTROLLER_AXIS_MAX][2];

#define NUM_OF_CONTROLLER_MAPS_IN_STACK 8

static bool g_sdlControllerSchemeNeedsCleanUp;

static struct {
	BESDLControllerMap stack[SDL_CONTROLLER_BUTTON_MAX];
	BESDLControllerMap *currPtr;
	BESDLControllerMap *endPtr;
} g_sdlControllertoScanCodeMaps;

static const BESDLControllerMap g_sdlControllerToScanCodeMap_default = {
	EMULATEDKEYSCANCODE_ENTER, // SDL_CONTROLLER_BUTTON_A
	EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_B
	0, // SDL_CONTROLLER_BUTTON_X
	0, // SDL_CONTROLLER_BUTTON_Y
	EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	0, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER, unused
	0, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, unused
	EMULATEDKEYSCANCODE_UP, // SDL_CONTROLLER_DPAD_UP
	EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_DPAD_DOWN
	EMULATEDKEYSCANCODE_LEFT, // SDL_CONTROLLER_DPAD_LEFT
	EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_DPAD_RIGHT

	0, 0, // SDL_CONTROLLER_AXIS_LEFTX
	0, 0, // SDL_CONTROLLER_AXIS_LEFTY
	0, 0, // SDL_CONTROLLER_AXIS_RIGHTX
	0, 0, // SDL_CONTROLLER_AXIS_RIGHTY
	0, 0, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	0, 0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_DEFAULT
};

extern SDL_Window *g_sdlWindow;

uint8_t g_sdlLastKeyScanCode;

void BE_SDL_InitGfx(void);
void BE_SDL_InitAudio(void);
void BE_SDL_ShutdownAudio(void);
void BE_SDL_ShutdownGfx(void);
static void BEL_SDL_ParseConfig(void);

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
	// Preparing a controller scheme (with no special UI) in case the relevant feature is enabled
	memcpy(&g_sdlControllertoScanCodeMaps.stack[0], &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr = &g_sdlControllertoScanCodeMaps.stack[0];
	g_sdlControllertoScanCodeMaps.endPtr = &g_sdlControllertoScanCodeMaps.stack[NUM_OF_CONTROLLER_MAPS_IN_STACK];

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
		if (g_refKeenCfg.useAltControlScheme)
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

static void BEL_SDL_ParseSetting_FullScreen(const char *buffer)
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

static void BEL_SDL_ParseSetting_FullRes(const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.fullWidth, &g_refKeenCfg.fullHeight);
}

static void BEL_SDL_ParseSetting_WindowRes(const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.winWidth, &g_refKeenCfg.winHeight);
}

static void BEL_SDL_ParseSetting_DisplayNum(const char *buffer)
{
	sscanf(buffer, "%d", &g_refKeenCfg.displayNum);
}

static void BEL_SDL_ParseSetting_SDLRendererDriver(const char *buffer)
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

static void BEL_SDL_ParseSetting_VSync(const char *buffer)
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

static void BEL_SDL_ParseSetting_Bilinear(const char *buffer)
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

static void BEL_SDL_ParseSetting_ScaleType(const char *buffer)
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

static void BEL_SDL_ParseSetting_ScaleFactor(const char *buffer)
{
	g_refKeenCfg.scaleFactor = atoi(buffer);
}

static void BEL_SDL_ParseSetting_AutolockCursor(const char *buffer)
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

static void BEL_SDL_ParseSetting_SndSampleRate(const char *buffer)
{
	g_refKeenCfg.sndSampleRate = atoi(buffer);
}

static void BEL_SDL_ParseSetting_DisableSoundSubSystem(const char *buffer)
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

static void BEL_SDL_ParseSetting_AlternativeControlScheme(const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.useAltControlScheme = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.useAltControlScheme = false;
	}
}

// HACK (cfg file may be rewritten and we don't want to remove any setting)
static bool g_sdlIsFarPtrSegOffsetSettingRead = false;

static void BEL_SDL_ParseSetting_FarPtrSegOffset(const char *buffer)
{
	unsigned int segOffset;
	g_sdlIsFarPtrSegOffsetSettingRead = true;
	if (sscanf(buffer, "%X", &segOffset) == 1)
	{
		g_refKeenCfg.farPtrSegOffset = segOffset;
	}
}

typedef struct {
	const char *cfgPrefix; // Includes '=' sign
	void (*handlerPtr)(const char *);
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
	{"farptrsegoffset=", &BEL_SDL_ParseSetting_FarPtrSegOffset},
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
	g_refKeenCfg.useAltControlScheme = false;
	g_refKeenCfg.farPtrSegOffset = BE_SDL_DEFAULT_FARPTRSEGOFFSET;
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
					g_sdlCfgEntries[i].handlerPtr(buffer+strlen(g_sdlCfgEntries[i].cfgPrefix));
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
	fprintf(fp, "altcontrolscheme=%s\n", g_refKeenCfg.useAltControlScheme ? "true" : "false");
	if (g_sdlIsFarPtrSegOffsetSettingRead)
	{
		// This should be a relatively hidden setting
		fprintf(fp, "farptrsegoffset=%X\n", g_refKeenCfg.farPtrSegOffset);
	}
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

	if (g_refKeenCfg.useAltControlScheme && (g_sdlControllertoScanCodeMaps.currPtr->schemeType == CONTROLSCHEME_MENU))
	{
		for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				int currnormx = SDL_GameControllerGetAxis(g_sdlControllers[i], SDL_CONTROLLER_AXIS_LEFTX)/4096, currnormy = SDL_GameControllerGetAxis(g_sdlControllers[i], SDL_CONTROLLER_AXIS_LEFTY)/4096;
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



// TODO: Should be (at least partially) configurable
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATADVENTURES)
static const BESDLControllerMap g_sdlControllerToScanCodeMap_inGame = {
	EMULATEDKEYSCANCODE_SPACE, // SDL_CONTROLLER_BUTTON_A, drink potion
	EMULATEDKEYSCANCODE_LALT, // SDL_CONTROLLER_BUTTON_B, strafe
#ifdef REFKEEN_VER_CAT3D
	EMULATEDKEYSCANCODE_B, // SDL_CONTROLLER_BUTTON_X, cast nuke in Cat3D
#elif defined REFKEEN_VER_CATADVENTURES
	EMULATEDKEYSCANCODE_Z, // SDL_CONTROLLER_BUTTON_X, cast nuke in CatAdv
#endif
	EMULATEDKEYSCANCODE_ENTER, // SDL_CONTROLLER_BUTTON_Y, cast bolt
	EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	EMULATEDKEYSCANCODE_LCTRL, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER, fire
#ifdef REFKEEN_VER_CAT3D
	EMULATEDKEYSCANCODE_RSHIFT, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, turn fast and run in Cat3D
#elif defined REFKEEN_VER_CATADVENTURES
	EMULATEDKEYSCANCODE_TAB, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, turn fast in CatAdv (and that's it)
#endif
	EMULATEDKEYSCANCODE_UP, // SDL_CONTROLLER_DPAD_UP
	EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_DPAD_DOWN
	EMULATEDKEYSCANCODE_LEFT, // SDL_CONTROLLER_DPAD_LEFT
	EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_DPAD_RIGHT

	EMULATEDKEYSCANCODE_LEFT, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_LEFTX
	EMULATEDKEYSCANCODE_UP, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_LEFTY
	EMULATEDKEYSCANCODE_LEFT, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_RIGHTX
	EMULATEDKEYSCANCODE_UP, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_RIGHTY
	0, 0, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	0, 0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_GAME
};
#elif defined REFKEEN_VER_KDREAMS
static const BESDLControllerMap g_sdlControllerToScanCodeMap_inGame = {
	EMULATEDKEYSCANCODE_LCTRL, // SDL_CONTROLLER_BUTTON_A, jump
	EMULATEDKEYSCANCODE_LALT, // SDL_CONTROLLER_BUTTON_B, throw flowerpower
	EMULATEDKEYSCANCODE_SPACE, // SDL_CONTROLLER_BUTTON_X, show stats
	0, // SDL_CONTROLLER_BUTTON_Y
	EMULATEDKEYSCANCODE_ESC, // SDL_CONTROLLER_BUTTON_BACK
	0, // SDL_CONTROLLER_BUTTON_GUIDE, unused
	EMULATEDKEYSCANCODE_PAUSE, // SDL_CONTROLLER_BUTTON_START, special case (pause key)
	0, // SDL_CONTROLLER_BUTTON_LEFTSTICK, unused
	0, // SDL_CONTROLLER_BUTTON_RIGHTSTICK, unused
	0, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER, unused
	0, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, unused
	EMULATEDKEYSCANCODE_UP, // SDL_CONTROLLER_DPAD_UP
	EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_DPAD_DOWN
	EMULATEDKEYSCANCODE_LEFT, // SDL_CONTROLLER_DPAD_LEFT
	EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_DPAD_RIGHT

	EMULATEDKEYSCANCODE_LEFT, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_LEFTX
	EMULATEDKEYSCANCODE_UP, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_LEFTY
	EMULATEDKEYSCANCODE_LEFT, EMULATEDKEYSCANCODE_RIGHT, // SDL_CONTROLLER_AXIS_RIGHTX
	EMULATEDKEYSCANCODE_UP, EMULATEDKEYSCANCODE_DOWN, // SDL_CONTROLLER_AXIS_RIGHTY
	0, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
	0, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

	CONTROLSCHEME_GAME
};
#endif

void BEL_SDL_AltControlScheme_CleanUp(void)
{
	if (!g_sdlControllerSchemeNeedsCleanUp)
		return;

	if (g_sdlControllertoScanCodeMaps.currPtr->schemeType == CONTROLSCHEME_TEXTINPUT)
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
						// Do NOT force this - button can be re-detected as "presse" immediately otherwise!
						// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than a key press.
						//g_sdlControllersButtonsStates[i][but] = false;
						if (g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[but])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[but];
							BEL_SDL_HandleEmuKeyboardEvent(false, dosKeyEvent);
						}
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
						if (g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][0])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][0];
							BEL_SDL_HandleEmuKeyboardEvent(false, dosKeyEvent);
						}
					}
					// Repeat with positive
					if (g_sdlControllersAxesStates[i][axis][1])
					{
						// Do NOT force this - axis can be re-detected as "pressed" (in this direction) immediately otherwise!
						// FIXME: Unfortunately this means a mistaken key release event can be sent, but hopefully it's less of an issue than a key press.
						//g_sdlControllersAxesStates[i][axis][1] = false;
						if (g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][1])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][1];
							BEL_SDL_HandleEmuKeyboardEvent(false, dosKeyEvent);
						}
					}
				}
			}
		}
	}

	extern void BEL_SDL_HideAltInputUI(void);
	BEL_SDL_HideAltInputUI();
	g_sdlControllerSchemeNeedsCleanUp = false;
}


void BE_SDL_AltControlScheme_Push(void)
{
	if (!g_refKeenCfg.useAltControlScheme)
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
}

void BE_SDL_AltControlScheme_Pop(void)
{
	if (!g_refKeenCfg.useAltControlScheme)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

	if (g_sdlControllertoScanCodeMaps.currPtr == &g_sdlControllertoScanCodeMaps.stack[0])
	{
		BE_SDL_SetScreenMode(3);
		BE_SDL_Simplified_printf("Error in BE_SDL_AltControlScheme_Pop: Popped more than necessary!\n");
		BE_SDL_HandleExit(0);
	}
	--g_sdlControllertoScanCodeMaps.currPtr;

	// FIXME: Any better way to handle these?
	switch (g_sdlControllertoScanCodeMaps.currPtr->schemeType)
	{
	case CONTROLSCHEME_FACE:
	{
		extern void BEL_SDL_PrepareToShowFaceButtons(const char *scanCodes);
		BEL_SDL_PrepareToShowFaceButtons(g_sdlControllertoScanCodeMaps.currPtr->miscBuffer);
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

	g_sdlControllerSchemeNeedsCleanUp = true;
}


bool BE_SDL_AltControlScheme_IsEnabled(void)
{
	return g_refKeenCfg.useAltControlScheme;
}

void BE_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes(const char *scanCodes)
{
	if (!g_refKeenCfg.useAltControlScheme)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_FACE;

	int counter = 0, sdlButtonNum = SDL_CONTROLLER_BUTTON_A; // A->B->X->Y
	char *actualScanCodePtr = g_sdlControllertoScanCodeMaps.currPtr->miscBuffer;
	for (const char *chPtr = scanCodes; *chPtr; ++chPtr)
	{
		// FIXME - Support gamepads with less buttons?
		// SPECIAL KEYS - These can use different buttons instead
		if ((*chPtr == EMULATEDKEYSCANCODE_ESC) || (*chPtr == EMULATEDKEYSCANCODE_PAUSE))
		{
			continue;
		}
		if (++counter == 5)
		{
			BE_SDL_SetScreenMode(3);
			BE_SDL_puts("Error in BE_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes:");
			BE_SDL_puts("Gotten too many scancodes as the input, input scancodes string:");
			BE_SDL_puts(scanCodes); // Do NOT use printf cause if we support some formatting this is not safe!
			BE_SDL_HandleExit(0);
		}
		*actualScanCodePtr++ = *chPtr;
		g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[sdlButtonNum++] = *chPtr;
	}
	*actualScanCodePtr = '\0';
	extern void BEL_SDL_PrepareToShowFaceButtons(const char *scanCodes);
	BEL_SDL_PrepareToShowFaceButtons(g_sdlControllertoScanCodeMaps.currPtr->miscBuffer); // This shouldn't care about pause and co.

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PreparePageScrollingControls(int prevPageScan, int nextPageScan)
{
	if (!g_refKeenCfg.useAltControlScheme)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_PAGE;
	g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = prevPageScan;
	g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = nextPageScan;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareMenuControls(void)
{
	if (!g_refKeenCfg.useAltControlScheme)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_default, sizeof(g_sdlControllerToScanCodeMap_default));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_MENU;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareInGameControls(void)
{
	if (!g_refKeenCfg.useAltControlScheme)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

	memcpy(g_sdlControllertoScanCodeMaps.currPtr,  &g_sdlControllerToScanCodeMap_inGame, sizeof(g_sdlControllerToScanCodeMap_inGame));
	g_sdlControllertoScanCodeMaps.currPtr->schemeType = CONTROLSCHEME_GAME;

	g_sdlControllerSchemeNeedsCleanUp = true;
}

void BE_SDL_AltControlScheme_PrepareTextInput(void)
{
	if (!g_refKeenCfg.useAltControlScheme)
		return;

	BEL_SDL_AltControlScheme_CleanUp();

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
		case SDL_JOYDEVICEADDED:
			if (g_refKeenCfg.useAltControlScheme)
				break;
			if (event.jdevice.which < BE_SDL_MAXJOYSTICKS)
			{
				g_sdlJoysticks[event.jdevice.which] = SDL_JoystickOpen(event.jdevice.which);
			}
			break;
		case SDL_JOYDEVICEREMOVED:
		{
			if (g_refKeenCfg.useAltControlScheme)
				break;
			for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
			{
				if (g_sdlJoysticks[i] && (SDL_JoystickInstanceID(g_sdlJoysticks[i]) == event.jdevice.which))
				{
					SDL_JoystickClose(g_sdlJoysticks[i]);
					g_sdlJoysticks[i] = NULL;
				}
			}
			break;
		}
		case SDL_CONTROLLERDEVICEADDED:
			if (!g_refKeenCfg.useAltControlScheme)
				break;
			if (event.cdevice.which < BE_SDL_MAXJOYSTICKS)
			{
				g_sdlControllers[event.cdevice.which] = SDL_GameControllerOpen(event.cdevice.which);
				memset(g_sdlControllersButtonsStates[event.cdevice.which], 0, sizeof(g_sdlControllersButtonsStates[event.cdevice.which]));
				memset(g_sdlControllersAxesStates[event.cdevice.which], 0, sizeof(g_sdlControllersAxesStates[event.cdevice.which]));
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			if (!g_refKeenCfg.useAltControlScheme)
				break;
			for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
			{
				if (g_sdlControllers[i] && (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i])) == event.cdevice.which))
				{
					SDL_GameControllerClose(g_sdlControllers[i]);
					g_sdlControllers[i] = NULL;
				}
			}
			break;
		}
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
			SDL_Quit();
			BE_SDL_ShutdownAll();
			exit(0);
			break;
		default: ;
		}
	}
	// If SDL_GameController is used, we don't poll for its events.
	// Furthermore, we emulate keyboard/mouse events from DOS.
	if (g_refKeenCfg.useAltControlScheme)
	{
		for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
		{
			if (g_sdlControllers[i])
			{
				for (int but = 0; but < SDL_CONTROLLER_BUTTON_MAX; ++but)
				{
					bool isPressed = SDL_GameControllerGetButton(g_sdlControllers[i], (SDL_GameControllerButton)but);
					if (isPressed != g_sdlControllersButtonsStates[i][but])
					{
						g_sdlControllersButtonsStates[i][but] = isPressed;
						// Special handling for text input
						if (g_sdlControllertoScanCodeMaps.currPtr->schemeType == CONTROLSCHEME_TEXTINPUT)
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
							// The rest of the cases
							default:
							{
								// Select key from UI, with a few special cases: Start (Pause), Back (Escape) or X (Shift state change) buttons.
								// NOTE: This can modify isPressed e.g., for shift key
								dosKeyEvent.dosScanCode = (but == SDL_CONTROLLER_BUTTON_START) ? EMULATEDKEYSCANCODE_PAUSE : ((but == SDL_CONTROLLER_BUTTON_BACK) ? EMULATEDKEYSCANCODE_ESC : ((but == SDL_CONTROLLER_BUTTON_X) ? BEL_SDL_ToggleShiftStateInTextInputUI(&isPressed) : BEL_SDL_ToggleKeyPressInTextInputUI(&isPressed)));
							}
							}

							if (dosKeyEvent.dosScanCode)
							{
								BEL_SDL_HandleEmuKeyboardEvent(isPressed, dosKeyEvent);
							}
						}
						// The usual otherwise (similar handling done with analog axes, triggers included)
						else if (g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[but])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllertoScanCodeMaps.currPtr->buttonsMap[but];
							BEL_SDL_HandleEmuKeyboardEvent(isPressed, dosKeyEvent);
						}
					}
				}
				// Repeat with analog axes (ignored with text input scheme in use)
				if (g_sdlControllertoScanCodeMaps.currPtr->schemeType == CONTROLSCHEME_TEXTINPUT)
					continue;

				for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; ++axis)
				{
					int axisVal = SDL_GameControllerGetAxis(g_sdlControllers[i], (SDL_GameControllerAxis)axis);
					// Is pressed in the negative direction?
					bool isNegPressed = (axisVal <= -8192);
					if (isNegPressed != g_sdlControllersAxesStates[i][axis][0])
					{
						g_sdlControllersAxesStates[i][axis][0] = isNegPressed;
						if (g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][0])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][0];
							BEL_SDL_HandleEmuKeyboardEvent(isNegPressed, dosKeyEvent);
						}
					}
					// Repeat with positive
					bool isPosPressed = (axisVal >= 8192);
					if (isPosPressed != g_sdlControllersAxesStates[i][axis][1])
					{
						g_sdlControllersAxesStates[i][axis][1] = isPosPressed;
						if (g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][1])
						{
							// FIXME: Can possibly make this more efficient
							emulatedDOSKeyEvent dosKeyEvent;
							dosKeyEvent.isSpecial = false;
							dosKeyEvent.dosScanCode = g_sdlControllertoScanCodeMaps.currPtr->axesMap[axis][1];
							BEL_SDL_HandleEmuKeyboardEvent(isPosPressed, dosKeyEvent);
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
	if (! g_sdlAudioSubsystemUp)
	{
		BE_SDL_PrepareForManualAudioSDServiceCall();
	}
}


uint16_t BE_SDL_Compat_GetFarPtrRelocationSegOffset(void)
{
	return g_refKeenCfg.farPtrSegOffset;
}
