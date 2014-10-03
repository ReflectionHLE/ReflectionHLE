/* Copyright (C) 2014 NY00123
 *
 * This file is part of Chocolate Keen Dreams.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "SDL.h"
#include "id_heads.h"

#define BE_SDL_MAXJOYSTICKS 8
#define BE_SDL_EMU_JOYSTICK_RANGEMAX 5000 // id_in.c MaxJoyValue

static void (*g_sdlKeyboardInterruptFuncPtr)(id0_byte_t) = 0;

static SDL_Joystick *g_sdlJoysticks[BE_SDL_MAXJOYSTICKS];

extern SDL_Window *g_sdlWindow;

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
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL joystick subsystem initialization failed, disabled,\n%s\n", SDL_GetError());
	}
	BEL_SDL_ParseConfig();
	BE_SDL_InitGfx();
	BE_SDL_InitAudio();
	if (g_chocolateKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else
	{
		SDL_ShowCursor(false);
	}
	SDL_GetRelativeMouseState(NULL, NULL); // Reset
	BE_SDL_PollEvents(); // e.g., to "reset" some states, and detect joysticks
}

void BE_SDL_ShutdownAll(void)
{
	if (g_chocolateKeenCfg.autolockCursor || (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
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
	// TODO Check joystick events
	while (true)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					void BE_SDL_SetAspectCorrectionRect(void);
					BE_SDL_SetAspectCorrectionRect();
				}
				break;
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_QUIT:
				SDL_Quit();
				BE_SDL_ShutdownAll();
				exit(0);
				break;
			default: ;
			}
		}
		SDL_Delay(1);
		// TODO: Make this more efficient
		void BE_SDL_UpdateHostDisplay(void);
		BE_SDL_UpdateHostDisplay();
	}
}



ChocolateKeenConfig g_chocolateKeenCfg;

#define CHOCOLATE_KEEN_DREAMS_CONFIG_FILEPATH "chocolate-keen-dreams.cfg"

static void BEL_SDL_ParseSetting_FullScreen(const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_chocolateKeenCfg.isFullscreen = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_chocolateKeenCfg.isFullscreen = false;
	}
}

static void BEL_SDL_ParseSetting_FullRes(const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_chocolateKeenCfg.fullWidth, &g_chocolateKeenCfg.fullHeight);
}

static void BEL_SDL_ParseSetting_WindowRes(const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_chocolateKeenCfg.winWidth, &g_chocolateKeenCfg.winHeight);
}

static void BEL_SDL_ParseSetting_DisplayNum(const char *buffer)
{
	sscanf(buffer, "%d", &g_chocolateKeenCfg.displayNum);
}

static void BEL_SDL_ParseSetting_SDLRendererDriver(const char *buffer)
{
	SDL_RendererInfo info;
	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
	{
		SDL_GetRenderDriverInfo(i, &info);
		if (!strcmp(info.name, buffer))
		{
			g_chocolateKeenCfg.sdlRendererDriver = i;
			break;
		}
	}
}

static void BEL_SDL_ParseSetting_VSync(const char *buffer)
{
	if (!strcmp(buffer, "on"))
	{
		g_chocolateKeenCfg.vSync = VSYNC_ON;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_chocolateKeenCfg.vSync = VSYNC_OFF;
	}
	else if (!strcmp(buffer, "auto"))
	{
		g_chocolateKeenCfg.vSync = VSYNC_AUTO;
	}
}

static void BEL_SDL_ParseSetting_Bilinear(const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_chocolateKeenCfg.isBilinear = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_chocolateKeenCfg.isBilinear = false;
	}
}

static void BEL_SDL_ParseSetting_ScaleType(const char *buffer)
{
	if (!strcmp(buffer, "aspect"))
	{
		g_chocolateKeenCfg.scaleType = SCALE_ASPECT;
	}
	else if (!strcmp(buffer, "fill"))
	{
		g_chocolateKeenCfg.scaleType = SCALE_FILL;
	}
}

static void BEL_SDL_ParseSetting_ScaleFactor(const char *buffer)
{
	g_chocolateKeenCfg.scaleFactor = atoi(buffer);
}

static void BEL_SDL_ParseSetting_AutolockCursor(const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_chocolateKeenCfg.autolockCursor = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_chocolateKeenCfg.autolockCursor = false;
	}
}

static void BEL_SDL_ParseSettings_SndSampleRate(const char *buffer)
{
	g_chocolateKeenCfg.sndSampleRate = atoi(buffer);
}

typedef struct {
	const char *cfgPrefix; // Includes '=' sign
	void (*handlerPtr)(const char *);
} ChocoKeenCfgEntry;

static ChocoKeenCfgEntry g_sdlCfgEntries[] = {
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
	{"sndsamplerate=", &BEL_SDL_ParseSettings_SndSampleRate},
};

static void BEL_SDL_ParseConfig(void)
{
	// Defaults
	g_chocolateKeenCfg.isFullscreen = false;
	g_chocolateKeenCfg.fullWidth = 0;
	g_chocolateKeenCfg.fullHeight = 0;
	g_chocolateKeenCfg.winWidth = 0;
	g_chocolateKeenCfg.winHeight = 0;
	g_chocolateKeenCfg.displayNum = 0;
	g_chocolateKeenCfg.sdlRendererDriver = -1;
	g_chocolateKeenCfg.vSync = VSYNC_AUTO;
	g_chocolateKeenCfg.isBilinear = true;
	g_chocolateKeenCfg.scaleType = SCALE_ASPECT;
	g_chocolateKeenCfg.scaleFactor = 2;
	g_chocolateKeenCfg.autolockCursor = false;
	g_chocolateKeenCfg.sndSampleRate = 49716; // TODO should be a shared define
	// Try to load config
	FILE *fp = fopen(CHOCOLATE_KEEN_DREAMS_CONFIG_FILEPATH, "r");
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
			for (int i = 0; i < sizeof(g_sdlCfgEntries)/sizeof(*g_sdlCfgEntries); ++i)
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
	fp = fopen(CHOCOLATE_KEEN_DREAMS_CONFIG_FILEPATH, "w");
	if (!fp)
	{
		return;
	}
	fprintf(fp, "fullscreen=%s\n", g_chocolateKeenCfg.isFullscreen ? "true" : "false");
	fprintf(fp, "fullres=%dx%d\n", g_chocolateKeenCfg.fullWidth, g_chocolateKeenCfg.fullHeight);
	fprintf(fp, "windowres=%dx%d\n", g_chocolateKeenCfg.winWidth, g_chocolateKeenCfg.winHeight);
	fprintf(fp, "displaynum=%d\n", g_chocolateKeenCfg.displayNum);
	fprintf(fp, "sdlrenderer=%s\n", "auto");
	fprintf(fp, "vsync=%s\n", g_chocolateKeenCfg.vSync == VSYNC_AUTO ? "auto" : (g_chocolateKeenCfg.vSync == VSYNC_ON ? "on" : "off"));
	fprintf(fp, "bilinear=%s\n", g_chocolateKeenCfg.isBilinear ? "true" : "false");
	fprintf(fp, "scaletype=%s\n", (g_chocolateKeenCfg.scaleType == SCALE_ASPECT) ? "aspect" : "fill");
	fprintf(fp, "scalefactor=%d\n", g_chocolateKeenCfg.scaleFactor);
	fprintf(fp, "autolock=%s\n", g_chocolateKeenCfg.autolockCursor ? "true" : "false");
	fprintf(fp, "sndsamplerate=%d\n", g_chocolateKeenCfg.sndSampleRate);
	fclose(fp);
}




typedef enum EmulatedKeyScancode_T {
     EMULATEDKEYSCANCODE_ESC = 1,
     EMULATEDKEYSCANCODE_1,
     EMULATEDKEYSCANCODE_2,
     EMULATEDKEYSCANCODE_3,
     EMULATEDKEYSCANCODE_4,
     EMULATEDKEYSCANCODE_5,
     EMULATEDKEYSCANCODE_6,
     EMULATEDKEYSCANCODE_7,
     EMULATEDKEYSCANCODE_8,
     EMULATEDKEYSCANCODE_9,
     EMULATEDKEYSCANCODE_0, // 0Bh
     EMULATEDKEYSCANCODE_MINUS,
     EMULATEDKEYSCANCODE_EQUALS,
     EMULATEDKEYSCANCODE_BSPACE,
     EMULATEDKEYSCANCODE_TAB,
     EMULATEDKEYSCANCODE_Q, // 10h
     EMULATEDKEYSCANCODE_W,
     EMULATEDKEYSCANCODE_E,
     EMULATEDKEYSCANCODE_R,
     EMULATEDKEYSCANCODE_T,
     EMULATEDKEYSCANCODE_Y,
     EMULATEDKEYSCANCODE_U,
     EMULATEDKEYSCANCODE_I,
     EMULATEDKEYSCANCODE_O,
     EMULATEDKEYSCANCODE_P,
     EMULATEDKEYSCANCODE_LBRACKET,
     EMULATEDKEYSCANCODE_RBRACKET,
     EMULATEDKEYSCANCODE_ENTER,
     EMULATEDKEYSCANCODE_LCTRL,
     EMULATEDKEYSCANCODE_A, // 1Eh
     EMULATEDKEYSCANCODE_S,
     EMULATEDKEYSCANCODE_D,
     EMULATEDKEYSCANCODE_F,
     EMULATEDKEYSCANCODE_G,
     EMULATEDKEYSCANCODE_H,
     EMULATEDKEYSCANCODE_J,
     EMULATEDKEYSCANCODE_K,
     EMULATEDKEYSCANCODE_L,
     EMULATEDKEYSCANCODE_SEMICOLON,
     EMULATEDKEYSCANCODE_QUOTE,
     EMULATEDKEYSCANCODE_GRAVE,
     EMULATEDKEYSCANCODE_LSHIFT,
     EMULATEDKEYSCANCODE_BACKSLASH,
     EMULATEDKEYSCANCODE_Z, // 2Ch
     EMULATEDKEYSCANCODE_X,
     EMULATEDKEYSCANCODE_C,
     EMULATEDKEYSCANCODE_V,
     EMULATEDKEYSCANCODE_B,
     EMULATEDKEYSCANCODE_N,
     EMULATEDKEYSCANCODE_M,
     EMULATEDKEYSCANCODE_COMMA,
     EMULATEDKEYSCANCODE_PERIOD,
     EMULATEDKEYSCANCODE_SLASH,
     EMULATEDKEYSCANCODE_RSHIFT,
     EMULATEDKEYSCANCODE_PRINTSCREEN, // 37h but kind of special
     EMULATEDKEYSCANCODE_LALT,
     EMULATEDKEYSCANCODE_SPACE,
     EMULATEDKEYSCANCODE_CAPSLOCK,
     EMULATEDKEYSCANCODE_F1, // 3Bh
     EMULATEDKEYSCANCODE_F2,
     EMULATEDKEYSCANCODE_F3,
     EMULATEDKEYSCANCODE_F4,
     EMULATEDKEYSCANCODE_F5,
     EMULATEDKEYSCANCODE_F6,
     EMULATEDKEYSCANCODE_F7,
     EMULATEDKEYSCANCODE_F8,
     EMULATEDKEYSCANCODE_F9,
     EMULATEDKEYSCANCODE_F10,
     EMULATEDKEYSCANCODE_NUMLOCK, // 45h
     EMULATEDKEYSCANCODE_SCROLLLOCK,
     EMULATEDKEYSCANCODE_KP_7,
     EMULATEDKEYSCANCODE_KP_8,
     EMULATEDKEYSCANCODE_KP_9,
     EMULATEDKEYSCANCODE_KP_MINUS,
     EMULATEDKEYSCANCODE_KP_4,
     EMULATEDKEYSCANCODE_KP_5,
     EMULATEDKEYSCANCODE_KP_6,
     EMULATEDKEYSCANCODE_KP_PLUS,
     EMULATEDKEYSCANCODE_KP_1,
     EMULATEDKEYSCANCODE_KP_2,
     EMULATEDKEYSCANCODE_KP_3,
     EMULATEDKEYSCANCODE_KP_0,
     EMULATEDKEYSCANCODE_KP_PERIOD, // 53h
     // A couple of "special" keys (scancode E0h sent first)
     EMULATEDKEYSCANCODE_KP_DIVIDE = 0x35,
     EMULATEDKEYSCANCODE_KP_ENTER = 0x1C,
     // Back to a few "non-special" keys
     EMULATEDKEYSCANCODE_F11 = 0x57,
     EMULATEDKEYSCANCODE_F12 = 0x58,
     // And again special keys
     EMULATEDKEYSCANCODE_INSERT = 0x52,
     EMULATEDKEYSCANCODE_DELETE = 0x53,
     EMULATEDKEYSCANCODE_HOME = 0x47,
     EMULATEDKEYSCANCODE_END = 0x4F,
     EMULATEDKEYSCANCODE_PAGEUP = 0x49,
     EMULATEDKEYSCANCODE_PAGEDOWN = 0x51,
     EMULATEDKEYSCANCODE_LEFT = 0x4B,
     EMULATEDKEYSCANCODE_RIGHT = 0x4D,
     EMULATEDKEYSCANCODE_UP = 0x48,
     EMULATEDKEYSCANCODE_DOWN = 0x50,
     EMULATEDKEYSCANCODE_RALT = 0x38,
     EMULATEDKEYSCANCODE_RCTRL = 0x1D,
     // Two extra kes
     EMULATEDKEYSCANCODE_LESSTHAN = 0x56,
     EMULATEDKEYSCANCODE_KP_MULTIPLY = 0x37,
     // This one is different from all the rest (6 scancodes sent on press ONLY)
     EMULATEDKEYSCANCODE_PAUSE = 0xE1,
} EmulatedKeyScancode_T;

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

void BE_SDL_StartKeyboardService(void (*funcPtr)(id0_byte_t))
{
	g_sdlKeyboardInterruptFuncPtr = funcPtr;
}

void BE_SDL_StopKeyboardService(void)
{
	g_sdlKeyboardInterruptFuncPtr = 0;
}

void BE_SDL_GetMouseDelta(id0_int_t *x, id0_int_t *y)
{
	int ourx, oury;
	SDL_GetRelativeMouseState(&ourx, &oury);
	*x = ourx;
	*y = oury;
}

id0_word_t BE_SDL_GetMouseButtons(void)
{
	static id0_word_t results[] = {0, 1, 4, 5, 2, 3, 6, 7};
	return results[SDL_GetMouseState(NULL, NULL) & 7];
}

void BE_SDL_GetJoyAbs(id0_word_t joy, id0_word_t *xp, id0_word_t *yp)
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

id0_word_t BE_SDL_GetJoyButtons(id0_word_t joy)
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

static void BEL_SDL_HandleEmuKeyboardEvent(bool isPressed, emulatedDOSKeyEvent keyEvent)
{
	if (!g_sdlKeyboardInterruptFuncPtr) // e.g., on init
	{
		return;
	}
	if (keyEvent.dosScanCode == EMULATEDKEYSCANCODE_PAUSE)
	{
		if (isPressed)
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
		if (keyEvent.isSpecial)
		{
			g_sdlKeyboardInterruptFuncPtr(0xe0);
		}
		g_sdlKeyboardInterruptFuncPtr(keyEvent.dosScanCode | (isPressed ? 0 : 0x80));
	}
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
			if (event.jdevice.which < BE_SDL_MAXJOYSTICKS)
			{
				g_sdlJoysticks[event.jdevice.which] = SDL_JoystickOpen(event.jdevice.which);
			}
			break;
		case SDL_JOYDEVICEREMOVED:
		{
			for (int i = 0; i < BE_SDL_MAXJOYSTICKS; ++i)
			{
				if (g_sdlJoysticks[i] && SDL_JoystickInstanceID(g_sdlJoysticks[i]) == event.jdevice.which)
				{
					SDL_JoystickClose(g_sdlJoysticks[i]);
					g_sdlJoysticks[i] = NULL;
				}
			}
			break;
		}
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			case  SDL_WINDOWEVENT_RESIZED:
			{
				void BE_SDL_SetAspectCorrectionRect(void);
				BE_SDL_SetAspectCorrectionRect();
				BE_SDL_MarkGfxForPendingUpdate();
				BE_SDL_MarkGfxForUpdate();
				break;
			}
			case SDL_WINDOWEVENT_EXPOSED:
				BE_SDL_MarkGfxForPendingUpdate();
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
}
