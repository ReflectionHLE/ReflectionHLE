#include "SDL.h"
#include "id_heads.h"
#include "be_cross.h"

static void (*g_sdlKeyboardInterruptFuncPtr)(id0_byte_t) = 0;

void BE_SDL_InitAll()
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER) < 0)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "SDL backend initialization failed,\n%s\n", SDL_GetError());
		exit(0);
	}
	BE_SDL_InitAudio();
}

void BE_SDL_ShutdownAll()
{
	BE_SDL_ShutdownAudio();
	SDL_Quit();
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

    {false,  EMULATEDKEYSCANCODE_LCTRL, 0}, // SDL_SCANCODE_LCTRL
    {false,  EMULATEDKEYSCANCODE_LSHIFT, 0}, // SDL_SCANCODE_LSHIFT
    {false,  EMULATEDKEYSCANCODE_LALT, 0}, // SDL_SCANCODE_LALT

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

static void BEL_SDL_HandleEmuKeyboardEvent(bool isPressed, emulatedDOSKeyEvent keyEvent)
{
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
		case SDL_QUIT:
			SDL_Quit();
			exit(0);
		default: ;
		}
	}
}
