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

#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h" // Enable VSync by default for EGA/VGA, not CGA
#include "be_st.h"
#include "be_st_sdl_private.h"
#include "be_video.h"
#include "be_video_emu.h"
#include "be_video_textures.h"
#include "be_video_ui.h"

bool g_sdlShowControllerUI;
bool g_sdlShowTouchUI;

/* Tracked fingers definitions (multi-touch input) */

#define MAX_NUM_OF_TRACKED_FINGERS 10

BESDLTrackedFinger g_sdlTrackedFingers[MAX_NUM_OF_TRACKED_FINGERS];
int g_nOfTrackedFingers = 0;

/*** Game controller UI resource definitions ***/

#include "../rsrc/pad_font_mono.xpm"
#include "../rsrc/pad_thumb_buttons.xpm"
#include "../rsrc/pad_dpad.xpm"

#define ALTCONTROLLER_FONT_XPM_ROW_OFFSET 3
#define ALTCONTROLLER_PAD_XPM_ROW_OFFSET 8

#define ALTCONTROLLER_DPAD_PIX_DIM 48
#define ALTCONTROLLER_FACEBUTTONS_PIX_DIM 56
// The max. of the above
#define ALTCONTROLLER_PAD_MAX_PIX_DIM ((ALTCONTROLLER_DPAD_PIX_DIM > ALTCONTROLLER_FACEBUTTONS_PIX_DIM) ? ALTCONTROLLER_DPAD_PIX_DIM : ALTCONTROLLER_FACEBUTTONS_PIX_DIM)

#define ALTCONTROLLER_CHAR_PIX_WIDTH 6
#define ALTCONTROLLER_CHAR_PIX_HEIGHT 8
#define ALTCONTROLLER_CHAR_TOTAL_PIX_WIDTH 570

#define ALTCONTROLLER_EDGE_PIX_DIST 2

#define ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH 22
#define ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT 12


// Measured in keys
#define ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH 14
#define ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT 4

#define ALTCONTROLLER_TEXTINPUT_PIX_WIDTH (ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
#define ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT (ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT)

// Again measured in keys
#define ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH 17
#define ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT 5

#define ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH (ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
#define ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT (ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT)


#define ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS 20

// Shared among all kinds of keyboard UI
static int g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY;
static bool g_sdlKeyboardUISelectedKeyIsMarked;
// Text input specific
static bool g_sdlTextInputIsKeyPressed, g_sdlTextInputIsShifted;
// Debug keys specific
static bool g_sdlDebugKeysPressed[ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT][ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH];


// These are given as (x, y) offset pairs within the non-scaled,
// face buttons image, assuming longest texts possible (3 chars long)
static const int g_sdlControllerFaceButtonsTextLocs[] = {19, 42, 36, 25, 2, 25, 19, 8};
static const int g_sdlControllerDpadTextLocs[] = {15, 34, 28, 21, 2, 21, 15, 8};

BE_ST_Rect g_sdlControllerFaceButtonsRect, g_sdlControllerDpadRect, g_sdlControllerTextInputRect, g_sdlControllerDebugKeysRect;
BE_ST_Texture *g_sdlFaceButtonsTexture, *g_sdlDpadTexture, *g_sdlTextInputTexture, *g_sdlDebugKeysTexture;
bool g_sdlFaceButtonsAreShown, g_sdlDpadIsShown, g_sdlTextInputUIIsShown, g_sdlDebugKeysUIIsShown, g_sdlTouchControlsAreShown;

static int g_sdlFaceButtonsScanCodes[4], g_sdlDpadScanCodes[4];

BE_ST_Rect g_sdlOnScreenTouchControlsRects[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];
BE_ST_Texture *g_sdlOnScreenTouchControlsTextures[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];
int g_sdlNumOfOnScreenTouchControls = 0;
static BE_ST_Rect g_sdlInputTouchControlsRects[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];

typedef struct {
	const char **xpmImage;
	int xpmWidth, xpmHeight;
	BE_ST_Texture *texture;
} BESDLCachedOnScreenTouchControl;

static BESDLCachedOnScreenTouchControl g_sdlCachedOnScreenTouchControls[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];
static int g_nOfCachedTouchControlsTextures = 0;


// Scancode names for controller face buttons and d-pad UI, based on DOS
// scancodes (doesn't include "special" keys for which 0xE0 is sent) and more
static const char *g_sdlDOSScanCodePadStrs[] = {
	NULL, "Esc",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\x11",
	"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Ent",
	"Ctl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
	"\x1E", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", NULL,
	NULL, "Alt", "[_]", "Cap",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num", "Scl",
	"Hom", "\x18", "PgU", NULL, "\x1B", NULL, "\x1A", NULL, "End", "\x19", "PgD",
	"Ins", "Del",
};

// Same, but for text input keyboard in non-shifted state
static const char *g_sdlDOSScanCodeTextInputNonShiftedStrs[] = {
	NULL, "Esc",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\x11",
	NULL, "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Ent",
	NULL, "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
	"\x1E", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", NULL,
	NULL, NULL, "[_]", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"\x1B\x1B", "\x18", NULL, NULL, "\x1B", NULL, "\x1A", "+", "\x1A\x1A", "\x19", NULL,
	NULL, "Del",
};

// Same but shifted
static const char *g_sdlDOSScanCodeTextInputShiftedStrs[] = {
	NULL, "Esc",
	"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "\x11",
	NULL, "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "Ent",
	NULL, "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "~",
	"\x1E", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", NULL,
	NULL, NULL, "[_]", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"\x1B\x1B", "\x18", NULL, NULL, "\x1B", NULL, "\x1A", "+", "\x1A\x1A", "\x19", NULL,
	NULL, "Del",
};

// Same, but for debug keys
static const char *g_sdlDOSScanCodeDebugKeysStrs[] = {
	NULL, "Esc",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\x11",
	"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Ent",
	"Ctl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
	"L\x1E", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "R\x1E",
	"*  ", // SPECIAL - Scancode is also used by Print Screen key
	"Alt", "[_]", "Cap",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num", "Scl",
	"7  ", "8 \x18", "9  ", "-  ", "4 \x1B", "5  ", "6 \x1A", "+  ", "1  ", "2 \x19", "3  ",
	"0  ", ".  ",
	NULL, NULL, NULL,
	"F11", "F12",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x59-0x5F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x60-0x6F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x70-0x7F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x80-0x8F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x90-0x9F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xA0-0xAF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xB0-0xBF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xC0-0xCF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xD0-0xDF
	NULL, // 0xE0
	"| |", // Pause key, 0xE1 (worth to add empty space above just for this?)
};

// Text input: One of the shifted and non-shifted variants
static const char **g_sdlDOSScanCodeTextInputStrs_Ptr;

// Text input keyboard layout definition (probably better we don't use "special" keys e.g., with scancode 0xE0 sent, even though there shouldn't be a difference)

static const BE_ST_ScanCode_T g_sdlDOSScanCodeTextInputLayout[ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT][ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH] = {
	{BE_ST_SC_GRAVE, BE_ST_SC_1, BE_ST_SC_2, BE_ST_SC_3, BE_ST_SC_4, BE_ST_SC_5, BE_ST_SC_6, BE_ST_SC_7, BE_ST_SC_8, BE_ST_SC_9, BE_ST_SC_0, BE_ST_SC_MINUS, BE_ST_SC_EQUALS, BE_ST_SC_BSPACE},

	{BE_ST_SC_KP_4/*LEFT*/, BE_ST_SC_Q, BE_ST_SC_W, BE_ST_SC_E, BE_ST_SC_R, BE_ST_SC_T, BE_ST_SC_Y, BE_ST_SC_U, BE_ST_SC_I, BE_ST_SC_O, BE_ST_SC_P, BE_ST_SC_LBRACKET, BE_ST_SC_RBRACKET, BE_ST_SC_HOME},

	{BE_ST_SC_KP_6/*RIGHT*/, BE_ST_SC_A, BE_ST_SC_S, BE_ST_SC_D, BE_ST_SC_F, BE_ST_SC_G, BE_ST_SC_H, BE_ST_SC_J, BE_ST_SC_K, BE_ST_SC_L, BE_ST_SC_SEMICOLON, BE_ST_SC_QUOTE, BE_ST_SC_BACKSLASH, BE_ST_SC_END},

	{BE_ST_SC_LSHIFT, BE_ST_SC_Z, BE_ST_SC_X, BE_ST_SC_C, BE_ST_SC_V, BE_ST_SC_B, BE_ST_SC_N, BE_ST_SC_M, BE_ST_SC_COMMA, BE_ST_SC_PERIOD, BE_ST_SC_SLASH, BE_ST_SC_SPACE, BE_ST_SC_KP_PERIOD/*DEL*/, BE_ST_SC_ENTER},
};

// Debug keys keyboard layout definition (again not using "special" keys, but the Pause key, for which 6 scancodes are sent on press only, is here)

static const BE_ST_ScanCode_T g_sdlDOSScanCodeDebugKeysLayout[ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT][ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH] = {
	{BE_ST_SC_ESC, BE_ST_SC_F1, BE_ST_SC_F2, BE_ST_SC_F3, BE_ST_SC_F4, BE_ST_SC_F5, BE_ST_SC_F6, BE_ST_SC_F7, BE_ST_SC_F8, BE_ST_SC_F9, BE_ST_SC_F10, BE_ST_SC_F11, BE_ST_SC_F12, BE_ST_SC_PAUSE, BE_ST_SC_NUMLOCK, BE_ST_SC_KP_MULTIPLY, BE_ST_SC_KP_MINUS},

	{BE_ST_SC_GRAVE, BE_ST_SC_1, BE_ST_SC_2, BE_ST_SC_3, BE_ST_SC_4, BE_ST_SC_5, BE_ST_SC_6, BE_ST_SC_7, BE_ST_SC_8, BE_ST_SC_9, BE_ST_SC_0, BE_ST_SC_MINUS, BE_ST_SC_EQUALS, BE_ST_SC_BSPACE, BE_ST_SC_KP_7, BE_ST_SC_KP_8, BE_ST_SC_KP_9},

	{BE_ST_SC_TAB, BE_ST_SC_Q, BE_ST_SC_W, BE_ST_SC_E, BE_ST_SC_R, BE_ST_SC_T, BE_ST_SC_Y, BE_ST_SC_U, BE_ST_SC_I, BE_ST_SC_O, BE_ST_SC_P, BE_ST_SC_LBRACKET, BE_ST_SC_RBRACKET, BE_ST_SC_BACKSLASH, BE_ST_SC_KP_4, BE_ST_SC_KP_5, BE_ST_SC_KP_6},

	{BE_ST_SC_CAPSLOCK, BE_ST_SC_A, BE_ST_SC_S, BE_ST_SC_D, BE_ST_SC_F, BE_ST_SC_G, BE_ST_SC_H, BE_ST_SC_J, BE_ST_SC_K, BE_ST_SC_L, BE_ST_SC_SEMICOLON, BE_ST_SC_QUOTE, BE_ST_SC_SPACE, BE_ST_SC_SCROLLLOCK, BE_ST_SC_KP_1, BE_ST_SC_KP_2, BE_ST_SC_KP_3},

	{BE_ST_SC_LSHIFT, BE_ST_SC_Z, BE_ST_SC_X, BE_ST_SC_C, BE_ST_SC_V, BE_ST_SC_B, BE_ST_SC_N, BE_ST_SC_M, BE_ST_SC_COMMA, BE_ST_SC_PERIOD, BE_ST_SC_SLASH, BE_ST_SC_RSHIFT, BE_ST_SC_LALT, BE_ST_SC_LCTRL, BE_ST_SC_KP_0, BE_ST_SC_KP_PERIOD, BE_ST_SC_KP_PLUS},
};


extern const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr;
extern bool g_sdlMouseButtonsStates[3];
extern int g_sdlEmuMouseButtonsState;


static bool g_sdlSomeOnScreenControlWasAccessibleWithMouse = false; // Used internally in BEL_ST_ConditionallyShowAltInputPointer

/*static*/ void BEL_ST_ConditionallyShowAltInputPointer(void)
{
	if (g_refKeenCfg.touchInputToggle == TOUCHINPUT_FORCED)
		return;

	bool someOnScreenControlIsAccessibleWithMouse = ((g_sdlShowControllerUI || g_sdlShowTouchUI) && (g_sdlFaceButtonsAreShown || g_sdlDpadIsShown || g_sdlTextInputUIIsShown || g_sdlDebugKeysUIIsShown/* || (g_sdlShowTouchUI && g_sdlTouchControlsAreShown)*/));

	if (someOnScreenControlIsAccessibleWithMouse != g_sdlSomeOnScreenControlWasAccessibleWithMouse)
	{
		// Better reset these when on-screen controls usable by the mouse are shown/hidden.
		// Since touchInputToggle differs from TOUCHINPUT_FORCED, this EXCLUDES the touch controls.
		g_sdlMouseButtonsStates[0] = g_sdlMouseButtonsStates[1] = g_sdlMouseButtonsStates[2] = 0;
		g_sdlEmuMouseButtonsState = 0;
	}

	g_sdlSomeOnScreenControlWasAccessibleWithMouse = someOnScreenControlIsAccessibleWithMouse;

	if (someOnScreenControlIsAccessibleWithMouse)
		BEL_ST_SetMouseMode(BE_ST_MOUSEMODE_ABS_WITH_CURSOR);
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	else if (g_sdlDoAbsMouseMotion && g_sdlControllerMappingActualCurr->absoluteFingerPositioning)
		BEL_ST_SetMouseMode(BE_ST_MOUSEMODE_ABS_WITHOUT_CURSOR);
#endif
	else if (
		(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN) ||
		(g_refKeenCfg.mouseGrab == MOUSEGRAB_COMMONLY) ||
		((g_refKeenCfg.mouseGrab == MOUSEGRAB_AUTO) && g_sdlControllerMappingActualCurr->grabMouse)
	)
		BEL_ST_SetMouseMode(BE_ST_MOUSEMODE_REL);
	else
		BEL_ST_SetMouseMode(BE_ST_MOUSEMODE_ABS_WITH_CURSOR);
}


static void BEL_ST_CreatePadTextureIfNeeded(BE_ST_Texture **padTexturePtrPtr, int len)
{
	if (*padTexturePtrPtr)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(padTexturePtrPtr, len, len, false, false);
	if (!(*padTexturePtrPtr))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 pad texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode((SDL_Texture *)*padTexturePtrPtr, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_RedrawTextToBuffer(uint32_t *picPtr, int picWidth, const char *text)
{
	for (int currRow = 0, fontXpmIndex = ALTCONTROLLER_FONT_XPM_ROW_OFFSET; currRow < ALTCONTROLLER_CHAR_PIX_HEIGHT; ++currRow, picPtr += picWidth, ++fontXpmIndex)
	{
		const char *fontRowPtr = pad_font_mono_xpm[fontXpmIndex];
		uint32_t *currPtr = picPtr;
		for (const char *chPtr = text; *chPtr; ++chPtr)
		{
			const char *fontPixPtr = fontRowPtr + ALTCONTROLLER_CHAR_PIX_WIDTH * (*chPtr);
			for (int currCharCol = 0; currCharCol < ALTCONTROLLER_CHAR_PIX_WIDTH; ++currCharCol, ++currPtr, ++fontPixPtr)
			{
				if (*fontPixPtr == '+')
				{
					*currPtr = g_sdlCGAGfxBGRAScreenColors[0];
				}
			}
		}
	}
}

static void BEL_ST_PrepareToShowOnePad(const int *scanCodes, const char **padXpm, BE_ST_Texture **padTexturePtrPtr, bool *areButtonsShownPtr, const int *textLocs, int len)
{
	BEL_ST_CreatePadTextureIfNeeded(padTexturePtrPtr, len);

	uint32_t pixels[ALTCONTROLLER_PAD_MAX_PIX_DIM*ALTCONTROLLER_PAD_MAX_PIX_DIM];
	uint32_t *currPtr = pixels;
	for (int currRow = 0, xpmIndex = ALTCONTROLLER_PAD_XPM_ROW_OFFSET; currRow < len; ++currRow, ++xpmIndex)
	{
		const char *xpmRowPtr = padXpm[xpmIndex];
		for (int currCol = 0; currCol < len; ++currCol, ++currPtr, ++xpmRowPtr)
		{
			switch (*xpmRowPtr)
			{
			case ' ':
				*currPtr = 0x00000000; // HACK (BGRA, working with any order) because we don't have it defined elsewhere
				break;
			case '@':
				*currPtr = g_sdlEGABGRAScreenColors[8]; // Gray
				break;
			case '.':
				*currPtr = g_sdlEGABGRAScreenColors[7]; // Light gray
				break;
			// HACK - Compress 4 XPM colors into one
			case '+':
			case '#':
			case '$':
			case '%':
				*currPtr = g_sdlEGABGRAScreenColors[15]; // White
				break;
			}
		}
	}
	// Special case
	static const int arrowsScanCodes[] = {BE_ST_SC_DOWN, BE_ST_SC_RIGHT, BE_ST_SC_LEFT, BE_ST_SC_UP};
	if ((padXpm != pad_dpad_xpm) || memcmp(scanCodes, arrowsScanCodes, sizeof(arrowsScanCodes)))
		for (int counter = 0; counter < 4; ++scanCodes, ++counter)
		{
			if (!(*scanCodes))
				continue;

			const char *str = g_sdlDOSScanCodePadStrs[*scanCodes];
			BEL_ST_RedrawTextToBuffer(pixels + textLocs[2*counter] + textLocs[2*counter+1]*len + (3-strlen(str))*(ALTCONTROLLER_CHAR_PIX_WIDTH/2), len, str);
		}
	// Add some alpha channel
	currPtr = pixels;
	for (int pixCounter = 0; pixCounter < len*len; ++pixCounter, ++currPtr)
	{
		*currPtr &= 0xBFFFFFFF; // BGRA
	}
	BEL_ST_UpdateTexture(*padTexturePtrPtr, NULL, pixels, 4*len);
	*areButtonsShownPtr = true;

	g_sdlForceGfxControlUiRefresh = true;
}

/*static*/ void BEL_ST_PrepareToShowControllerUI(const BE_ST_ControllerMapping *mapping)
{
	const int faceButtonsScancodes[4] = {
		(mapping->buttons[BE_ST_CTRL_BUT_A].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_A].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_B].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_B].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_X].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_X].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_Y].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_Y].val : '\0'
	};
	const int dpadScancodes[4] = {
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_DOWN].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_DOWN].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_RIGHT].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_RIGHT].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_LEFT].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_LEFT].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_UP].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_UP].val : '\0'
	};
	const int emptyScancodesArray[4] = {'\0'};

	memcpy(g_sdlFaceButtonsScanCodes, faceButtonsScancodes, sizeof(faceButtonsScancodes));
	if (memcmp(&faceButtonsScancodes, &emptyScancodesArray, sizeof(emptyScancodesArray)))
		BEL_ST_PrepareToShowOnePad(faceButtonsScancodes, pad_thumb_buttons_xpm, &g_sdlFaceButtonsTexture, &g_sdlFaceButtonsAreShown, g_sdlControllerFaceButtonsTextLocs, ALTCONTROLLER_FACEBUTTONS_PIX_DIM);

	memcpy(g_sdlDpadScanCodes, dpadScancodes, sizeof(dpadScancodes));
	if (memcmp(&dpadScancodes, &emptyScancodesArray, sizeof(emptyScancodesArray)))
		BEL_ST_PrepareToShowOnePad(dpadScancodes, pad_dpad_xpm, &g_sdlDpadTexture, &g_sdlDpadIsShown, g_sdlControllerDpadTextLocs, ALTCONTROLLER_DPAD_PIX_DIM);

	g_nOfTrackedFingers = 0;

	BEL_ST_ConditionallyShowAltInputPointer();
}

static void BEL_ST_RecreateTouchControlTexture(BESDLCachedOnScreenTouchControl *touchControl)
{
	if (touchControl->texture)
		BEL_ST_SDLDestroyTextureWrapper(&touchControl->texture);
	int texWidth = touchControl->xpmWidth, texHeight = touchControl->xpmHeight;
	BEL_ST_SDLCreateTextureWrapper(&touchControl->texture, texWidth, texHeight, false, false);
	BE_ST_Texture *texture = touchControl->texture;
	if (!texture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateTouchControlTexture: Failed to (re)create SDL2 touch control texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode((SDL_Texture *)texture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
	// Update texture
	uint32_t *pixels = (uint32_t *)malloc(4*texWidth*texHeight);
	if (pixels == NULL)
	{
		BE_ST_ExitWithErrorMsg("BEL_ST_RecreateTouchControlTexture: Out of memory for drawing to textures!");
	}
	uint32_t *currPtr = pixels;
	const char **xpmImage = touchControl->xpmImage;
	for (int currRow = 0; currRow < texHeight; ++currRow)
	{
		const char *xpmRowPtr = xpmImage[currRow];
		for (int currCol = 0; currCol < texWidth; ++currCol, ++currPtr, ++xpmRowPtr)
		{
			switch (*xpmRowPtr)
			{
			case ' ':
				*currPtr = 0x00000000; // HACK (BGRA, working with any order) because we don't have it defined elsewhere
				break;
			case '@':
				*currPtr = g_sdlEGABGRAScreenColors[8]; // Gray
				break;
			case '.':
				*currPtr = g_sdlEGABGRAScreenColors[7]; // Light gray
				break;
			// HACK - Compress 4 XPM colors into one
			case '+':
			case '#':
			case '$':
			case '%':
				*currPtr = g_sdlEGABGRAScreenColors[15]; // White
				break;
			}
			*currPtr &= 0xBFFFFFFF; // Add some alpha channel
		}
	}
	BEL_ST_UpdateTexture(texture, NULL, pixels, 4*texWidth);
	free(pixels);
}

void BE_ST_AltControlScheme_InitTouchControlsUI(BE_ST_OnscreenTouchControl *onScreenTouchControls)
{
	if (g_refKeenCfg.touchInputToggle == TOUCHINPUT_OFF)
		return;

	int i;
	BE_ST_OnscreenTouchControl *touchControl;
	for (i = 0, touchControl = onScreenTouchControls; touchControl->xpmImage; ++i, ++touchControl)
	{
		int j;
		BESDLCachedOnScreenTouchControl *cachedTouchControl;
		for (j = 0, cachedTouchControl = g_sdlCachedOnScreenTouchControls; j < g_nOfCachedTouchControlsTextures; ++j, ++cachedTouchControl)
			if ((touchControl->xpmImage == cachedTouchControl->xpmImage) &&
			    (touchControl->xpmWidth == cachedTouchControl->xpmWidth) &&
			    (touchControl->xpmHeight == cachedTouchControl->xpmHeight)
			)
			{
				touchControl->miscData = cachedTouchControl; // Re-use texture
				continue;
			}

		if (g_nOfCachedTouchControlsTextures == ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS)
			BE_ST_ExitWithErrorMsg("BEL_ST_AltControlScheme_InitTouchControlsUI: On-screen touch controls overflow!");

		cachedTouchControl->xpmImage = touchControl->xpmImage;
		cachedTouchControl->xpmWidth = touchControl->xpmWidth;
		cachedTouchControl->xpmHeight = touchControl->xpmHeight;

		BEL_ST_RecreateTouchControlTexture(cachedTouchControl);

		touchControl->miscData = cachedTouchControl;
		++g_nOfCachedTouchControlsTextures;
	}
}

void BEL_ST_PrepareToShowTouchControls(const BE_ST_ControllerMapping *mapping)
{
	int i;
	BE_ST_OnscreenTouchControl *touchControl;
	for (i = 0, touchControl = mapping->onScreenTouchControls; touchControl->xpmImage; ++i, ++touchControl)
	{
		if (i == ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS)
			BE_ST_ExitWithErrorMsg("BEL_ST_PrepareToShowTouchControls: On-screen touch controls overflow!");

		g_sdlOnScreenTouchControlsTextures[i] = ((BESDLCachedOnScreenTouchControl *)(touchControl->miscData))->texture;
	}
	g_sdlNumOfOnScreenTouchControls = i;

	// Also verify this
	BE_ST_TouchControlSingleMap *singleMap;
	for (i = 0, singleMap = mapping->touchMappings; singleMap->xpmImage; ++i, ++singleMap)
		;
	if (i > ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS)
		BE_ST_ExitWithErrorMsg("BEL_ST_PrepareToShowTouchControls: Touch controls overflow!");

	g_nOfTrackedFingers = 0;
	g_sdlTouchControlsAreShown = true;

	BEL_ST_SetTouchControlsRects();
}

/*static*/ void BEL_ST_RedrawKeyToBuffer(uint32_t *picPtr, int picWidth, const char *text, bool isMarked, bool isPressed)
{
#if 0
	// This can happen for space that should be skipped
	if (!text)
		text = "";
#endif

	/*** Draw frame ***/
	uint32_t frameColor = g_sdlEGABGRAScreenColors[isMarked ? 15/*White*/ : 8/*Gray*/];
	uint32_t *currPtr = picPtr;
	// Frame top
	for (int currCol = 0; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH; ++currCol, ++currPtr)
	{
		*currPtr = frameColor;
	}
	// Frame left + key + Frame right
	currPtr += picWidth-ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH;
	for (int currRow = 1; currRow < ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1; ++currRow)
	{
		// Frame left pixel
		*(currPtr++) = frameColor;
		// Line between frame sides
		for (int currCol = 1; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH-1; ++currCol, ++currPtr)
		{
			*currPtr = g_sdlEGABGRAScreenColors[isPressed ? 12/*Light red*/ : 7/*Light gray*/];
		}
		// Frame right pixel
		*currPtr = frameColor;

		currPtr += picWidth-ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH+1;
	}
	// Frame bottom
	for (int currCol = 0; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH; ++currCol, ++currPtr)
	{
		*currPtr = frameColor;
	}
	/*** Draw text ***/
	BEL_ST_RedrawTextToBuffer(picPtr + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH-ALTCONTROLLER_CHAR_PIX_WIDTH*strlen(text))/2 + picWidth*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-ALTCONTROLLER_CHAR_PIX_HEIGHT)/2, picWidth, text);
	// Add some alpha channel (shouldn't be a lot)
	currPtr = picPtr;
	for (int currRow = 0; currRow < ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT; ++currRow, currPtr += picWidth-ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
	{
		for (int currCol = 0; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH; ++currCol, ++currPtr)
		{
			*currPtr &= 0xDFFFFFFF; // BGRA
		}
	}
}

static void BEL_ST_CreateTextInputTextureIfNeeded(void)
{
	if (g_sdlTextInputTexture)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(&g_sdlTextInputTexture, ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT, false, false);
	if (!g_sdlTextInputTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 text input texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode((SDL_Texture *)g_sdlTextInputTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_CreateDebugKeysTextureIfNeeded(void)
{
	if (g_sdlDebugKeysTexture)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(&g_sdlDebugKeysTexture, ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT, false, false);
	if (!g_sdlDebugKeysTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 debug keys texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode((SDL_Texture *)g_sdlDebugKeysTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_RedrawWholeTextInputUI(void)
{
	uint32_t pixels[ALTCONTROLLER_TEXTINPUT_PIX_WIDTH*ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT];
	uint32_t *currPtr = pixels;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT; ++currKeyRow, currPtr += ALTCONTROLLER_TEXTINPUT_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1))
	{
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH; ++currKeyCol, currPtr += ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
		{
			BEL_ST_RedrawKeyToBuffer(currPtr, ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, g_sdlDOSScanCodeTextInputStrs_Ptr[(int)g_sdlDOSScanCodeTextInputLayout[currKeyRow][currKeyCol]], false, false);
		}
	}
	// Simpler to do so outside the loop
	BEL_ST_RedrawKeyToBuffer(pixels + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*g_sdlKeyboardUISelectedKeyX) + ALTCONTROLLER_TEXTINPUT_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT*g_sdlKeyboardUISelectedKeyY), ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, g_sdlDOSScanCodeTextInputStrs_Ptr[(int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]], true, g_sdlTextInputIsKeyPressed);

	BEL_ST_UpdateTexture(g_sdlTextInputTexture, NULL, pixels, 4*ALTCONTROLLER_TEXTINPUT_PIX_WIDTH);
}

static void BEL_ST_RedrawWholeDebugKeysUI(void)
{
	uint32_t pixels[ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH*ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT];
	uint32_t *currPtr = pixels;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT; ++currKeyRow, currPtr += ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1))
	{
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH; ++currKeyCol, currPtr += ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
		{
			BEL_ST_RedrawKeyToBuffer(currPtr, ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[currKeyRow][currKeyCol]], false, g_sdlDebugKeysPressed[currKeyRow][currKeyCol]);
		}
	}
	// Simpler to do so outside the loop
	BEL_ST_RedrawKeyToBuffer(pixels + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*g_sdlKeyboardUISelectedKeyX) + ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT*g_sdlKeyboardUISelectedKeyY), ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]], true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	BEL_ST_UpdateTexture(g_sdlDebugKeysTexture, NULL, pixels, 4*ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH);
}


/*static*/ void BEL_ST_PrepareToShowTextInputUI(void)
{
	BEL_ST_CreateTextInputTextureIfNeeded();

	g_sdlKeyboardUISelectedKeyX = 0;
	g_sdlKeyboardUISelectedKeyY = 0;
	g_sdlKeyboardUISelectedKeyIsMarked = true;
	g_sdlTextInputIsKeyPressed = false;
	g_sdlTextInputIsShifted = false;
	g_sdlDOSScanCodeTextInputStrs_Ptr = g_sdlDOSScanCodeTextInputNonShiftedStrs;

	BEL_ST_RedrawWholeTextInputUI();
	g_sdlTextInputUIIsShown = true;

	g_sdlForceGfxControlUiRefresh = true;

	g_nOfTrackedFingers = 0;

	BEL_ST_ConditionallyShowAltInputPointer();
}

/*static*/ void BEL_ST_PrepareToShowDebugKeysUI(void)
{
	BEL_ST_CreateDebugKeysTextureIfNeeded();

	g_sdlKeyboardUISelectedKeyX = 0;
	g_sdlKeyboardUISelectedKeyY = 0;
	g_sdlKeyboardUISelectedKeyIsMarked = true;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT; ++currKeyRow)
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH; ++currKeyCol)
			g_sdlDebugKeysPressed[currKeyRow][currKeyCol] = false;

	BEL_ST_RedrawWholeDebugKeysUI();
	g_sdlDebugKeysUIIsShown = true;

	g_sdlForceGfxControlUiRefresh = true;

	g_nOfTrackedFingers = 0;

	BEL_ST_ConditionallyShowAltInputPointer();
}

static void BEL_ST_ToggleTextInputUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlDOSScanCodeTextInputStrs_Ptr[(int)g_sdlDOSScanCodeTextInputLayout[y][x]], isMarked, isPressed);

	BE_ST_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	BEL_ST_UpdateTexture(g_sdlTextInputTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}

static void BEL_ST_ToggleDebugKeysUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[y][x]], isMarked, isPressed);

	BE_ST_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	BEL_ST_UpdateTexture(g_sdlDebugKeysTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}


static void BEL_ST_ToggleOffAllTextInputUIKeysTrackedbyFingers(void)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		BEL_ST_ToggleTextInputUIKey(g_sdlTrackedFingers[i].miscData.key.x, g_sdlTrackedFingers[i].miscData.key.y, false, false);

	g_nOfTrackedFingers = 0;
	g_sdlForceGfxControlUiRefresh = true;
}

static void BEL_ST_ToggleOffAllDebugKeysUIKeysTrackedbyFingers(void)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
	{
		int keyX = g_sdlTrackedFingers[i].miscData.key.x;
		int keyY = g_sdlTrackedFingers[i].miscData.key.y;
		BEL_ST_ToggleDebugKeysUIKey(keyX, keyY, false, g_sdlDebugKeysPressed[keyY][keyX]);
	}

	g_nOfTrackedFingers = 0;
	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_ToggleShiftStateInTextInputUI(void);

static void BEL_ST_ChangeKeyStateInTextInputUI(BE_ST_ScanCode_T scanCode, bool isPressed)
{
	if (scanCode == BE_ST_SC_LSHIFT)
	{
		if (isPressed)
			BEL_ST_ToggleShiftStateInTextInputUI();
		return;
	}

	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	dosKeyEvent.dosScanCode = scanCode;
	BEL_ST_HandleEmuKeyboardEvent(isPressed, false, dosKeyEvent);
}

static void BEL_ST_ChangeKeyStateInDebugKeysUI(BE_ST_ScanCode_T scanCode, bool isPressed)
{
	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	dosKeyEvent.dosScanCode = scanCode;
	BEL_ST_HandleEmuKeyboardEvent(isPressed, false, dosKeyEvent);
}


void BEL_ST_MoveUpInTextInputUI(void)
{
	BEL_ST_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlTextInputIsKeyPressed)
		{
			BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlTextInputIsKeyPressed = false;
		}
		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		--g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY < 0)
			g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT-1;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveDownInTextInputUI(void)
{
	BEL_ST_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlTextInputIsKeyPressed)
		{
			BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlTextInputIsKeyPressed = false;
		}
		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		++g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY >= ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT)
			g_sdlKeyboardUISelectedKeyY = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveLeftInTextInputUI(void)
{
	BEL_ST_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlTextInputIsKeyPressed)
		{
			BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlTextInputIsKeyPressed = false;
		}
		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		--g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX < 0)
			g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH-1;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}

	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveRightInTextInputUI(void)
{
	BEL_ST_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlTextInputIsKeyPressed)
		{
			BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlTextInputIsKeyPressed = false;
		}
		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		++g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX >= ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH)
			g_sdlKeyboardUISelectedKeyX = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveUpInDebugKeysUI(void)
{
	BEL_ST_ToggleOffAllDebugKeysUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

		--g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY < 0)
			g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT-1;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveDownInDebugKeysUI(void)
{
	BEL_ST_ToggleOffAllDebugKeysUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

		++g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY >= ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT)
			g_sdlKeyboardUISelectedKeyY = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveLeftInDebugKeysUI(void)
{
	BEL_ST_ToggleOffAllDebugKeysUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

		--g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX < 0)
			g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH-1;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveRightInDebugKeysUI(void)
{
	BEL_ST_ToggleOffAllDebugKeysUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

		++g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX >= ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH)
			g_sdlKeyboardUISelectedKeyX = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}


void BEL_ST_ToggleShiftStateInTextInputUI(void)
{
	g_sdlTextInputIsShifted = !g_sdlTextInputIsShifted;
	g_sdlDOSScanCodeTextInputStrs_Ptr = g_sdlTextInputIsShifted ? g_sdlDOSScanCodeTextInputShiftedStrs : g_sdlDOSScanCodeTextInputNonShiftedStrs;

	BEL_ST_RedrawWholeTextInputUI();

	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	dosKeyEvent.dosScanCode = BE_ST_SC_LSHIFT;
	BEL_ST_HandleEmuKeyboardEvent(g_sdlTextInputIsShifted, false, dosKeyEvent);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_ToggleKeyPressInTextInputUI(bool toggle)
{
	BEL_ST_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (!g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUISelectedKeyX = g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);
		g_sdlForceGfxControlUiRefresh = true;
		return;
	}

	if (g_sdlTextInputIsKeyPressed == toggle)
		return;
	g_sdlTextInputIsKeyPressed = toggle;

	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, toggle);

	g_sdlForceGfxControlUiRefresh = true;

	BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], toggle);
}

void BEL_ST_ToggleKeyPressInDebugKeysUI(void)
{
	BEL_ST_ToggleOffAllDebugKeysUIKeysTrackedbyFingers();

	bool *keyStatus = &g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX];

	if (!g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUISelectedKeyX = g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
		BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, *keyStatus);
		g_sdlForceGfxControlUiRefresh = true;
		return;
	}

	*keyStatus ^= true;

	BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, *keyStatus);

	g_sdlForceGfxControlUiRefresh = true;

	BEL_ST_ChangeKeyStateInDebugKeysUI(g_sdlDOSScanCodeDebugKeysLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], *keyStatus);
}

/*** Pointer stuff common to all kinds of controller / touch input UI ***/

static BESDLTrackedFinger *BEL_ST_ProcessAndGetPressedTrackedFinger(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
		{
			if (g_refKeenCfg.touchInputDebugging)
			{
				g_sdlTrackedFingers[i].lastX = x;
				g_sdlTrackedFingers[i].lastY = y;
				g_sdlForceGfxControlUiRefresh = true;
			}
			return NULL; // In case of some mistaken double-tap of same finger
		}

	if (g_nOfTrackedFingers == MAX_NUM_OF_TRACKED_FINGERS)
		return NULL;

	BESDLTrackedFinger *trackedFinger = &g_sdlTrackedFingers[g_nOfTrackedFingers++];
	trackedFinger->touchId = touchId;
	trackedFinger->fingerId = fingerId;
	trackedFinger->isDefaultBinaryStateToggle = false;
	if (g_refKeenCfg.touchInputDebugging)
	{
		trackedFinger->lastX = x;
		trackedFinger->lastY = y;
		g_sdlForceGfxControlUiRefresh = true;
	}
	return trackedFinger;
}

static BESDLTrackedFinger *BEL_ST_ProcessAndGetMovedTrackedFinger(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
			break;

	if (i == g_nOfTrackedFingers)
		return NULL;

	BESDLTrackedFinger *trackedFinger = &g_sdlTrackedFingers[i];
	if (g_refKeenCfg.touchInputDebugging)
	{
		trackedFinger->lastX = x;
		trackedFinger->lastY = y;
		g_sdlForceGfxControlUiRefresh = true;
	}
	return trackedFinger;
}

static BESDLTrackedFinger *BEL_ST_GetReleasedTrackedFinger(SDL_TouchID touchId, SDL_FingerID fingerId)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
			return &g_sdlTrackedFingers[i];

	return NULL;
}

static void BEL_ST_RemoveTrackedFinger(BESDLTrackedFinger *trackedFinger)
{
	*trackedFinger = g_sdlTrackedFingers[--g_nOfTrackedFingers]; // Remove finger entry without moving the rest, except for maybe the last
	if (g_refKeenCfg.touchInputDebugging)
		g_sdlForceGfxControlUiRefresh = true; // Remove debugging finger mark from screen
}

static void BEL_ST_UnmarkAndReleaseSelectedKeyInTextInputUI(void)
{
	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlTextInputIsKeyPressed)
		{
			BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlTextInputIsKeyPressed = false;
		}
		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
		g_sdlKeyboardUISelectedKeyIsMarked = false;
	}
}

static void BEL_ST_UnmarkSelectedKeyInDebugKeysUI(void)
{
	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		BEL_ST_ToggleDebugKeysUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);
		g_sdlKeyboardUISelectedKeyIsMarked = false;
	}
}


void BEL_ST_CheckMovedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger || trackedFinger->isDefaultBinaryStateToggle)
		return;

	if ((x < g_sdlControllerTextInputRect.x) || (x >= g_sdlControllerTextInputRect.x+g_sdlControllerTextInputRect.w)
	    || (y < g_sdlControllerTextInputRect.y) || (y >= g_sdlControllerTextInputRect.y+g_sdlControllerTextInputRect.h))
		return;

	BEL_ST_UnmarkAndReleaseSelectedKeyInTextInputUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerTextInputRect.x)*ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerTextInputRect.w;
	int keyY = (y-g_sdlControllerTextInputRect.y)*ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerTextInputRect.h;

	if ((trackedFinger->miscData.key.x != keyX) || (trackedFinger->miscData.key.y != keyY))
	{
		BEL_ST_ToggleTextInputUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, false, false);
		BEL_ST_ToggleTextInputUIKey(keyX, keyY, true, false);
		trackedFinger->miscData.key.x = keyX;
		trackedFinger->miscData.key.y = keyY;
	}

	g_sdlForceGfxControlUiRefresh = true;
}

extern const int g_sdlJoystickAxisMax;
extern bool g_sdlDefaultMappingBinaryState;

bool BEL_ST_AltControlScheme_HandleEntry(const BE_ST_ControllerSingleMap *map, int value, bool *lastBinaryStatusPtr);

void BEL_ST_CheckPressedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerTextInputRect.x) || (x >= g_sdlControllerTextInputRect.x+g_sdlControllerTextInputRect.w)
	    || (y < g_sdlControllerTextInputRect.y) || (y >= g_sdlControllerTextInputRect.y+g_sdlControllerTextInputRect.h)
	)
	{
		trackedFinger->isDefaultBinaryStateToggle = true;
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax, &g_sdlDefaultMappingBinaryState);
		return;
	}

	BEL_ST_UnmarkAndReleaseSelectedKeyInTextInputUI();

	// Normalize coordinates to keys
	trackedFinger->miscData.key.x = (x-g_sdlControllerTextInputRect.x)*ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerTextInputRect.w;
	trackedFinger->miscData.key.y = (y-g_sdlControllerTextInputRect.y)*ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerTextInputRect.h;

	BEL_ST_ToggleTextInputUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, true, false);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_CheckReleasedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return;

	int prevKeyX = trackedFinger->miscData.key.x;
	int prevKeyY = trackedFinger->miscData.key.y;

	bool isDefaultBinaryStateToggle = trackedFinger->isDefaultBinaryStateToggle;
	BEL_ST_RemoveTrackedFinger(trackedFinger);

	if (isDefaultBinaryStateToggle)
	{
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, 0, &g_sdlDefaultMappingBinaryState);
		return;
	}

	if ((x >= g_sdlControllerTextInputRect.x) && (x < g_sdlControllerTextInputRect.x+g_sdlControllerTextInputRect.w)
	    && (y >= g_sdlControllerTextInputRect.y) && (y < g_sdlControllerTextInputRect.y+g_sdlControllerTextInputRect.h))
	{
		// Do key press and release (including any special handling possibly required for shift key)
		// FIXME: A delay may be required here in certain cases, but this works for now...
		BE_ST_ScanCode_T scanCode = g_sdlDOSScanCodeTextInputLayout[prevKeyY][prevKeyX];
		BEL_ST_ChangeKeyStateInTextInputUI(scanCode, true);
		BEL_ST_ChangeKeyStateInTextInputUI(scanCode, false);
	}

	BEL_ST_UnmarkAndReleaseSelectedKeyInTextInputUI();
	BEL_ST_ToggleTextInputUIKey(prevKeyX, prevKeyY, false, false);

	g_sdlForceGfxControlUiRefresh = true;
}


void BEL_ST_CheckMovedPointerInDebugKeysUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger || trackedFinger->isDefaultBinaryStateToggle)
		return;

	if ((x < g_sdlControllerDebugKeysRect.x) || (x >= g_sdlControllerDebugKeysRect.x+g_sdlControllerDebugKeysRect.w)
	    || (y < g_sdlControllerDebugKeysRect.y) || (y >= g_sdlControllerDebugKeysRect.y+g_sdlControllerDebugKeysRect.h))
		return;

	BEL_ST_UnmarkSelectedKeyInDebugKeysUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerDebugKeysRect.x)*ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH/g_sdlControllerDebugKeysRect.w;
	int keyY = (y-g_sdlControllerDebugKeysRect.y)*ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT/g_sdlControllerDebugKeysRect.h;

	if ((trackedFinger->miscData.key.x != keyX) || (trackedFinger->miscData.key.y != keyY))
	{
		BEL_ST_ToggleDebugKeysUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, false, g_sdlDebugKeysPressed[trackedFinger->miscData.key.y][trackedFinger->miscData.key.x]);
		BEL_ST_ToggleDebugKeysUIKey(keyX, keyY, true, g_sdlDebugKeysPressed[keyY][keyX]);
		trackedFinger->miscData.key.x = keyX;
		trackedFinger->miscData.key.y = keyY;
	}

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_CheckPressedPointerInDebugKeysUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerDebugKeysRect.x) || (x >= g_sdlControllerDebugKeysRect.x+g_sdlControllerDebugKeysRect.w)
	    || (y < g_sdlControllerDebugKeysRect.y) || (y >= g_sdlControllerDebugKeysRect.y+g_sdlControllerDebugKeysRect.h)
	)
	{
		trackedFinger->isDefaultBinaryStateToggle = true;
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax, &g_sdlDefaultMappingBinaryState);
		return;
	}

	BEL_ST_UnmarkSelectedKeyInDebugKeysUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerDebugKeysRect.x)*ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH/g_sdlControllerDebugKeysRect.w;
	int keyY = (y-g_sdlControllerDebugKeysRect.y)*ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT/g_sdlControllerDebugKeysRect.h;

	trackedFinger->miscData.key.x = keyX;
	trackedFinger->miscData.key.y = keyY;
	BEL_ST_ToggleDebugKeysUIKey(keyX, keyY, true, g_sdlDebugKeysPressed[keyY][keyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_CheckReleasedPointerInDebugKeysUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return;

	int prevKeyX = trackedFinger->miscData.key.x;
	int prevKeyY = trackedFinger->miscData.key.y;

	bool isDefaultBinaryStateToggle = trackedFinger->isDefaultBinaryStateToggle;
	BEL_ST_RemoveTrackedFinger(trackedFinger);

	if (isDefaultBinaryStateToggle)
	{
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, 0, &g_sdlDefaultMappingBinaryState);
		return;
	}

	bool *keyStatus = &g_sdlDebugKeysPressed[prevKeyY][prevKeyX];

	if ((x >= g_sdlControllerDebugKeysRect.x) && (x < g_sdlControllerDebugKeysRect.x+g_sdlControllerDebugKeysRect.w)
	    && (y >= g_sdlControllerDebugKeysRect.y) && (y < g_sdlControllerDebugKeysRect.y+g_sdlControllerDebugKeysRect.h))
	{
		*keyStatus ^= true;
		BEL_ST_ChangeKeyStateInDebugKeysUI(g_sdlDOSScanCodeDebugKeysLayout[prevKeyY][prevKeyX], *keyStatus);
	}

	BEL_ST_UnmarkSelectedKeyInDebugKeysUI();
	BEL_ST_ToggleDebugKeysUIKey(prevKeyX, prevKeyY, false, *keyStatus);

	g_sdlForceGfxControlUiRefresh = true;
}

// Returns matching scanCode if found (possibly 0 if not set), -1 if not
// pointed at button/dpad (also checking for transparent pixels, or 0 otherwise
static int BEL_ST_GetControllerUIScanCodeFromPointer(int x, int y)
{
	if ((x >= g_sdlControllerFaceButtonsRect.x) && (x < g_sdlControllerFaceButtonsRect.x+g_sdlControllerFaceButtonsRect.w)
	    && (y >= g_sdlControllerFaceButtonsRect.y) && (y < g_sdlControllerFaceButtonsRect.y+g_sdlControllerFaceButtonsRect.h))
	{
		// Normalize coordinates to pad
		x = (x-g_sdlControllerFaceButtonsRect.x)*ALTCONTROLLER_FACEBUTTONS_PIX_DIM/g_sdlControllerFaceButtonsRect.w;
		y = (y-g_sdlControllerFaceButtonsRect.y)*ALTCONTROLLER_FACEBUTTONS_PIX_DIM/g_sdlControllerFaceButtonsRect.h;
		switch (pad_thumb_buttons_xpm[y+ALTCONTROLLER_PAD_XPM_ROW_OFFSET][x])
		{
			case '%':
				return g_sdlFaceButtonsScanCodes[0];
			case '$':
				return g_sdlFaceButtonsScanCodes[1];
			case '#':
				return g_sdlFaceButtonsScanCodes[2];
			case '+':
				return g_sdlFaceButtonsScanCodes[3];
			case ' ':
				return -1; // Totally transparent
			default:
				return 0;
		}
	}
	else if ((x >= g_sdlControllerDpadRect.x) && (x < g_sdlControllerDpadRect.x+g_sdlControllerDpadRect.w)
	         && (y >= g_sdlControllerDpadRect.y) && (y < g_sdlControllerDpadRect.y+g_sdlControllerDpadRect.h))
	{
		// Normalize coordinates to pad
		x = (x-g_sdlControllerDpadRect.x)*ALTCONTROLLER_DPAD_PIX_DIM/g_sdlControllerDpadRect.w;
		y = (y-g_sdlControllerDpadRect.y)*ALTCONTROLLER_DPAD_PIX_DIM/g_sdlControllerDpadRect.h;
		switch (pad_dpad_xpm[y+ALTCONTROLLER_PAD_XPM_ROW_OFFSET][x])
		{
			case '%':
				return g_sdlDpadScanCodes[0];
			case '$':
				return g_sdlDpadScanCodes[1];
			case '#':
				return g_sdlDpadScanCodes[2];
			case '+':
				return g_sdlDpadScanCodes[3];
			case ' ':
				return -1; // Totally transparent
			default:
				return 0;
		}
	}
	return -1;
}

void BEL_ST_CheckPressedPointerInControllerUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	int scanCode = BEL_ST_GetControllerUIScanCodeFromPointer(x,y);
	trackedFinger->miscData.padButtonScanCode = scanCode;
	if (scanCode > 0)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = scanCode;
		BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
	}
	else if (scanCode < 0)
	{
		trackedFinger->isDefaultBinaryStateToggle = true;
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, g_sdlJoystickAxisMax, &g_sdlDefaultMappingBinaryState);
	}

}

void BEL_ST_CheckReleasedPointerInControllerUI(SDL_TouchID touchId, SDL_FingerID fingerId)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return;

	int scanCode = trackedFinger->miscData.padButtonScanCode;

	bool isDefaultBinaryStateToggle = trackedFinger->isDefaultBinaryStateToggle;
	BEL_ST_RemoveTrackedFinger(trackedFinger);

	if (isDefaultBinaryStateToggle)
	{
		BEL_ST_AltControlScheme_HandleEntry(&g_sdlControllerMappingActualCurr->defaultMapping, 0, &g_sdlDefaultMappingBinaryState);
		return;
	}

	if (scanCode)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = trackedFinger->miscData.padButtonScanCode;
		BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
	}
}

void BEL_ST_CheckMovedPointerInControllerUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger || trackedFinger->isDefaultBinaryStateToggle)
		return;

	int oldScanCode = trackedFinger->miscData.padButtonScanCode;
	int newScanCode = BEL_ST_GetControllerUIScanCodeFromPointer(x,y);

	if (newScanCode < 0)
		newScanCode = 0;

	trackedFinger->miscData.padButtonScanCode = newScanCode;

	if (oldScanCode != newScanCode)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		if (oldScanCode)
		{
			dosKeyEvent.dosScanCode = oldScanCode;
			BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
		}
		if (newScanCode)
		{
			dosKeyEvent.dosScanCode = newScanCode;
			BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
		}
	}
}

static int BEL_ST_GetPointedTouchControlIndex(int x, int y)
{
	if (!g_sdlControllerMappingActualCurr->touchMappings)
		return -1;

	BE_ST_TouchControlSingleMap *singleMap;
	int i;
	for (i = 0, singleMap = g_sdlControllerMappingActualCurr->touchMappings; singleMap->xpmImage; ++i, ++singleMap)
	{
		if ((x >= g_sdlInputTouchControlsRects[i].x) && (x < g_sdlInputTouchControlsRects[i].x+g_sdlInputTouchControlsRects[i].w)
		    && (y >= g_sdlInputTouchControlsRects[i].y) && (y < g_sdlInputTouchControlsRects[i].y+g_sdlInputTouchControlsRects[i].h))
		{
			const char **xpmImage = singleMap->xpmImage;
			int normalizedX = (x-g_sdlInputTouchControlsRects[i].x)*singleMap->xpmWidth/g_sdlInputTouchControlsRects[i].w;
			int normalizedY = (y-g_sdlInputTouchControlsRects[i].y)*singleMap->xpmHeight/g_sdlInputTouchControlsRects[i].h;
			if (xpmImage[normalizedY][normalizedX] != ' ') // Non-transparent pixel
				return i;
		}
	}
	return -1;
}

static void BEL_ST_HandleDefaultPointerActionInTouchControls(int touchControlIndex, bool isPressed)
{
	int i = 0;
	BE_ST_ControllerSingleMap *mapping;
	if (touchControlIndex >= 0)
		for (mapping = g_sdlControllerMappingActualCurr->touchMappings[touchControlIndex].mappings, i = 0;
		     (mapping->mapClass != BE_ST_CTRL_MAP_NONE) && (i < BE_ST_TOUCH_CTRL_MAX_MAPS);
		     ++mapping, ++i)
		{
			bool lastBinaryStatus = !isPressed;
			BEL_ST_AltControlScheme_HandleEntry(
				mapping,
				isPressed ? g_sdlJoystickAxisMax : 0,
				&lastBinaryStatus);
		}
	else
		BEL_ST_AltControlScheme_HandleEntry(
			&g_sdlControllerMappingActualCurr->defaultMapping,
			isPressed ? g_sdlJoystickAxisMax : 0,
			&g_sdlDefaultMappingBinaryState);
}

extern int g_sdlEmuMouseButtonsState;

void BEL_ST_CheckPressedPointerInTouchControls(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, bool forceAbsoluteFingerPositioning)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	int touchControlIndex = BEL_ST_GetPointedTouchControlIndex(x, y);
	trackedFinger->miscData.touchMappingIndex = touchControlIndex;

	if (forceAbsoluteFingerPositioning || ((touchControlIndex < 0) && g_sdlControllerMappingActualCurr->absoluteFingerPositioning))
	{
		trackedFinger->miscData.touchMappingIndex = -2; // Special mark
		// Mouse cursor control
		g_sdlEmuMouseButtonsState |= 1;
		BEL_ST_UpdateVirtualCursorPositionFromPointer(x, y);
	}
	else
		BEL_ST_HandleDefaultPointerActionInTouchControls(touchControlIndex, true);
}

void BEL_ST_CheckReleasedPointerInTouchControls(SDL_TouchID touchId, SDL_FingerID fingerId)
{
	BESDLTrackedFinger* trackedFinger = BEL_ST_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return;

	int prevTouchControlIndex = trackedFinger->miscData.touchMappingIndex;

	if (prevTouchControlIndex == -2)
		g_sdlEmuMouseButtonsState &= ~1; // Mouse cursor control
	else
		BEL_ST_HandleDefaultPointerActionInTouchControls(prevTouchControlIndex, false);

	BEL_ST_RemoveTrackedFinger(trackedFinger);
}

void BEL_ST_CheckMovedPointerInTouchControls(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	int prevTouchControlIndex = trackedFinger->miscData.touchMappingIndex;
	if (prevTouchControlIndex == -2)
	{
		BEL_ST_UpdateVirtualCursorPositionFromPointer(x, y);  // Mouse cursor control
		return;
	}

	int touchControlIndex = BEL_ST_GetPointedTouchControlIndex(x, y);
	if (touchControlIndex != prevTouchControlIndex)
	{
		if (prevTouchControlIndex >= 0)
			BEL_ST_HandleDefaultPointerActionInTouchControls(prevTouchControlIndex, false);

		if (touchControlIndex >= 0)
			BEL_ST_HandleDefaultPointerActionInTouchControls(touchControlIndex, true);

		trackedFinger->miscData.touchMappingIndex = touchControlIndex;
	}
}

void BEL_ST_ReleasePressedKeysInTextInputUI(void)
{
	if (g_sdlTextInputIsKeyPressed)
	{
		BEL_ST_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
		//BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
		g_sdlTextInputIsKeyPressed = false;
	}

	// Shift key may further be held, don't forget this too!
	if (g_sdlTextInputIsShifted)
		BEL_ST_ToggleShiftStateInTextInputUI();

	g_nOfTrackedFingers = 0; // Remove all tracked fingers
	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_ReleasePressedKeysInDebugKeysUI(void)
{
	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT; ++currKeyRow)
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH; ++currKeyCol)
			if (g_sdlDebugKeysPressed[currKeyRow][currKeyCol])
			{
				BEL_ST_ChangeKeyStateInDebugKeysUI(g_sdlDOSScanCodeDebugKeysLayout[currKeyRow][currKeyCol], false);
				//BEL_ST_ToggleDebugKeysUIKey(currKeyCol, currKeyRow, false, false);
				g_sdlDebugKeysPressed[currKeyRow][currKeyCol] = false;
			}

	g_nOfTrackedFingers = 0; // Remove all tracked fingers
	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_ReleasePressedKeysInControllerUI(void)
{
	while (g_nOfTrackedFingers > 0)
		BEL_ST_CheckReleasedPointerInControllerUI(g_sdlTrackedFingers[0].touchId, g_sdlTrackedFingers[0].fingerId);
}

void BEL_ST_ReleasePressedButtonsInTouchControls(void)
{
	while (g_nOfTrackedFingers > 0)
		BEL_ST_CheckReleasedPointerInTouchControls(g_sdlTrackedFingers[0].touchId, g_sdlTrackedFingers[0].fingerId);
}

/*static*/ void BEL_ST_HideAltInputUI(void)
{
	g_sdlFaceButtonsAreShown = false;
	g_sdlDpadIsShown = false;
	g_sdlTextInputUIIsShown = false;
	g_sdlDebugKeysUIIsShown = false;
	g_sdlTouchControlsAreShown = false;

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_SetCommonUIRects(void)
{
	// 1. Use same dimensions independently of scaling.
	// 2. The dimensions of the controller UI are picked relatively
	// to the host window's internal contents (without borders), not
	// directly related to the client window size.
	// 3. Also taking the whole window into account
	// (this doesn't depend on "screen mode", borders and more).
	// 4. Finally, try to be consistent with the positioning and
	// sizes of touch controls (even though it's not necessary).
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);

	int minWinDim = (winWidth >= winHeight) ? winHeight : winWidth;
	g_sdlControllerFaceButtonsRect.w = g_sdlControllerFaceButtonsRect.h = 56*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
	g_sdlControllerFaceButtonsRect.x = winWidth-(56+8)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
	g_sdlControllerFaceButtonsRect.y = winHeight-(56+8)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
	// Repeat for D-pad (same dimensions as the face buttons, other side)
	g_sdlControllerDpadRect.w = g_sdlControllerDpadRect.h = 48*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
	g_sdlControllerDpadRect.x = minWinDim*8/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
	g_sdlControllerDpadRect.y = winHeight-(48+8)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
	// Also this - text-input keyboard (somewhat different because the keyboard is rectangular, but not square-shaped)
	g_sdlControllerTextInputRect.w = winWidth;
	g_sdlControllerTextInputRect.h = winHeight*3/8;
	if (g_sdlControllerTextInputRect.w * ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT > g_sdlControllerTextInputRect.h * ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH)
		g_sdlControllerTextInputRect.w = g_sdlControllerTextInputRect.h * ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH / ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT;
	else
		g_sdlControllerTextInputRect.h = g_sdlControllerTextInputRect.w * ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT / ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH;
	g_sdlControllerTextInputRect.x = (winWidth-g_sdlControllerTextInputRect.w)/2;
	g_sdlControllerTextInputRect.y = winHeight-g_sdlControllerTextInputRect.h;
	// Same with debug keys
	g_sdlControllerDebugKeysRect.w = winWidth;
	g_sdlControllerDebugKeysRect.h = winHeight*3/8;
	if (g_sdlControllerDebugKeysRect.w * ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT > g_sdlControllerDebugKeysRect.h * ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH)
		g_sdlControllerDebugKeysRect.w = g_sdlControllerDebugKeysRect.h * ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH / ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT;
	else
		g_sdlControllerDebugKeysRect.h = g_sdlControllerDebugKeysRect.w * ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT / ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH;
	g_sdlControllerDebugKeysRect.x = (winWidth-g_sdlControllerDebugKeysRect.w)/2;
	g_sdlControllerDebugKeysRect.y = winHeight-g_sdlControllerDebugKeysRect.h;
}

void BEL_ST_SetTouchControlsRects(void)
{
	if (((g_sdlControllerMappingActualCurr == NULL) || g_sdlControllerMappingActualCurr->touchMappings == NULL))
		return;

	BE_ST_Rect *currRect;
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
	int minWinDim = (winWidth >= winHeight) ? winHeight : winWidth;
	{
		BE_ST_OnscreenTouchControl *touchControl;
		for (currRect = g_sdlOnScreenTouchControlsRects, touchControl = g_sdlControllerMappingActualCurr->onScreenTouchControls; touchControl->xpmImage; ++currRect, ++touchControl)
		{
			if (touchControl->xpmPosX >= BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM/2)
			{
				currRect->x = winWidth-(BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-touchControl->xpmPosX)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			else
			{
				currRect->x = touchControl->xpmPosX*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			if (touchControl->xpmPosY >= BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM/2)
			{
				currRect->y = winHeight-(BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-touchControl->xpmPosY)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			else
			{
				currRect->y = touchControl->xpmPosY*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			currRect->w = touchControl->xpmWidth*winWidth/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			currRect->h = touchControl->xpmHeight*winHeight/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			if (currRect->w > currRect->h)
				currRect->w = currRect->h;
			else
				currRect->h = currRect->w;
		}
	}
	// FIXME - Code duplication
	{
		BE_ST_TouchControlSingleMap *currMapping;
		for (currRect = g_sdlInputTouchControlsRects, currMapping = g_sdlControllerMappingActualCurr->touchMappings; currMapping->xpmImage; ++currRect, ++currMapping)
		{
			if (currMapping->xpmPosX >= BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM/2)
			{
				currRect->x = winWidth-(BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-currMapping->xpmPosX)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			else
			{
				currRect->x = currMapping->xpmPosX*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			if (currMapping->xpmPosY >= BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM/2)
			{
				currRect->y = winHeight-(BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-currMapping->xpmPosY)*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			else
			{
				currRect->y = currMapping->xpmPosY*minWinDim/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			}
			currRect->w = currMapping->xpmWidth*winWidth/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			currRect->h = currMapping->xpmHeight*winHeight/BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM;
			if (currRect->w > currRect->h)
				currRect->w = currRect->h;
			else
				currRect->h = currRect->w;
		}
	}
}
