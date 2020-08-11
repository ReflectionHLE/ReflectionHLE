/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_gamever.h" // Enable VSync by default for EGA/VGA, not CGA
#include "be_st.h"
#include "be_st_launcher.h"
#include "be_st_sdl_private.h"
#include "be_st_ega_lookup_tables.h"
#include "be_title_and_version.h"

// Some of these are also used in launcher
SDL_Window *g_sdlWindow;
SDL_Renderer *g_sdlRenderer;
SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
SDL_Rect g_sdlAspectCorrectionRect, g_sdlAspectCorrectionBorderedRect;

int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;

static bool g_sdlIsSoftwareRendered;
static bool g_sdlDoRefreshGfxOutput;
bool g_sdlForceGfxControlUiRefresh;

void BE_ST_MarkGfxForUpdate(void)
{
	g_sdlDoRefreshGfxOutput = true;
}

#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

#define GFX_TEX_WIDTH 320
#define GFX_TEX_HEIGHT 200
#define VGA_TXT_TEX_WIDTH 720
#define VGA_TXT_TEX_HEIGHT 400
//#define EGACGA_TXT_TEX_WIDTH 640
//#define EGACGA_TXT_TEX_HEIGHT 200

// Overscan border dimensions (for each side of the screen)
#define ENGINE_VGA_16COLORGFX_OVERSCAN_LEFT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_16COLORGFX_OVERSCAN_RIGHT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_256COLORGFX_OVERSCAN_LEFT_AFTER_DOUBLING 8 // Doubling from 4 for us
#define ENGINE_VGA_256COLORGFX_OVERSCAN_RIGHT_AFTER_DOUBLING 8 // Doubling from 4 for us
#define ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING 7    // 200-line doubling
#define ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING 7 // 200-line doubling
#define ENGINE_VGA_TXT_OVERSCAN_LEFT 9
#define ENGINE_VGA_TXT_OVERSCAN_RIGHT 9
#define ENGINE_VGA_TXT_OVERSCAN_TOP 7
#define ENGINE_VGA_TXT_OVERSCAN_BOTTOM 7

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16

int g_sdlDebugFingerRectSideLen;

extern const uint8_t g_vga_8x16TextFont[256*8*16];
// We can use a union because the memory contents are refreshed on screen mode change
// (well, not on change between modes 0xD and 0xE, both sharing planar A000:0000)
static union {
	uint64_t egaGfx[0x10000]; // Contents of A000:0000, de-planed (1 byte per pixel)
	uint8_t vgaGfx[0x40000]; // Same but for 256-colors. Similarly linear.
	uint8_t text[TXT_COLS_NUM*TXT_ROWS_NUM*2]; // Textual contents of B800:0000
} g_sdlVidMem;

// Used for simple caching of EGA/VGA graphics (due to page flipping and more)
// and similarly CGA graphics (modified only at one place)
static union {
	uint8_t egaGfx[2*GFX_TEX_WIDTH*GFX_TEX_HEIGHT]; // Support 640x200 mode for Catacomb Abyss
	uint8_t cgaGfx[GFX_TEX_WIDTH*GFX_TEX_HEIGHT];
} g_sdlHostScrMem, g_sdlHostScrMemCache;

static uint16_t g_sdlScreenStartAddress = 0;
static int g_sdlScreenMode = 3;
static int g_sdlTexWidth, g_sdlTexHeight;
static uint8_t g_sdlPelPanning = 0;
static int g_sdlPixLineWidth = 8*40; // Originally stored a byte, while measuring this in bytes instead of pixels
static int16_t g_sdlSplitScreenLine = -1;
static int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
static bool g_sdlTxtCursorEnabled = true;
static int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

/* Tracked fingers definitions (multi-touch input) */

#define MAX_NUM_OF_TRACKED_FINGERS 10

typedef struct {
	SDL_TouchID touchId;
	SDL_FingerID fingerId;
	int lastX, lastY;
	// Used in multiple mappings, but not necessarily all of them
	bool isDefaultBinaryStateToggle;
	// This one depends on current mapping in use
	union {
		struct {
			int x, y;
		} key;
		int padButtonScanCode;
		int touchMappingIndex;
	} miscData;
} BESDLTrackedFinger;

static BESDLTrackedFinger g_sdlTrackedFingers[MAX_NUM_OF_TRACKED_FINGERS];
static int g_nOfTrackedFingers = 0;

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

// These are given as (x, y) offset pairs within the non-scaled,
// face buttons image, assuming longest texts possible (3 chars long)
static const int g_sdlControllerFaceButtonsTextLocs[] = {19, 42, 36, 25, 2, 25, 19, 8};
static const int g_sdlControllerDpadTextLocs[] = {15, 34, 28, 21, 2, 21, 15, 8};

static SDL_Rect g_sdlControllerFaceButtonsRect, g_sdlControllerDpadRect, g_sdlControllerTextInputRect, g_sdlControllerDebugKeysRect;
static SDL_Texture *g_sdlFaceButtonsTexture, *g_sdlDpadTexture, *g_sdlTextInputTexture, *g_sdlDebugKeysTexture;
static bool g_sdlFaceButtonsAreShown, g_sdlDpadIsShown, g_sdlTextInputUIIsShown, g_sdlDebugKeysUIIsShown, g_sdlTouchControlsAreShown;

static int g_sdlFaceButtonsScanCodes[4], g_sdlDpadScanCodes[4];

static SDL_Rect g_sdlOnScreenTouchControlsRects[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];
static SDL_Texture *g_sdlOnScreenTouchControlsTextures[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];
static int g_sdlNumOfOnScreenTouchControls = 0;
static SDL_Rect g_sdlInputTouchControlsRects[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];

typedef struct {
	const char **xpmImage;
	int xpmWidth, xpmHeight;
	SDL_Texture *texture;
} BESDLCachedOnScreenTouchControl;

static BESDLCachedOnScreenTouchControl g_sdlCachedOnScreenTouchControls[ALTCONTROLLER_MAX_NUM_OF_TOUCH_CONTROLS];
static int g_nOfCachedTouchControlsTextures = 0;

// With alternative game controllers scheme, all UI is hidden if no controller is connected.
// Similar handling is done for touch input. Furthermore, some UI is shared.
bool g_sdlShowControllerUI;
bool g_sdlShowTouchUI;

// Shared among all kinds of keyboard UI
static int g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY;
static bool g_sdlKeyboardUISelectedKeyIsMarked;
// Text input specific
static bool g_sdlTextInputIsKeyPressed, g_sdlTextInputIsShifted;
// Debug keys specific
static bool g_sdlDebugKeysPressed[ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT][ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH];

void BEL_ST_RecreateSDLWindowAndRenderer(int x, int y, int windowWidth, int windowHeight, int fullWidth, int fullHeight, Uint32 windowFlags, int driverIndex, Uint32 rendererFlags);
static void BEL_ST_CalcWindowDimsFromCfg(int *outWidth, int *outHeight);

void BE_ST_InitGfx(void)
{
	if (g_refKeenCfg.sdlRendererDriver >= 0)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(g_refKeenCfg.sdlRendererDriver, &info);
		g_sdlIsSoftwareRendered = (info.flags & SDL_RENDERER_SOFTWARE);
	}
	else
	{
		g_sdlIsSoftwareRendered = false;
	}

	uint32_t windowFlagsToSet = 0;
	int windowWidthToSet, windowHeightToSet;
	BEL_ST_CalcWindowDimsFromCfg(&windowWidthToSet, &windowHeightToSet);
	if (g_refKeenCfg.isFullscreen)
		windowFlagsToSet = (g_refKeenCfg.fullWidth && g_refKeenCfg.fullHeight) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;

	if (!g_sdlIsSoftwareRendered || g_refKeenCfg.forceFullSoftScaling)
		windowFlagsToSet |= SDL_WINDOW_RESIZABLE;
	// Vanilla Keen Dreams and Keen 4-6 have no VSync in the CGA builds
#ifdef BE_ST_ADAPT_VSYNC_DEFAULT_TO_KDREAMS
	uint32_t rendererFlagsToSet = SDL_RENDERER_ACCELERATED | (((g_refKeenCfg.vSync == VSYNC_ON) || ((g_refKeenCfg.vSync == VSYNC_AUTO) && (refkeen_current_gamever != BE_GAMEVER_KDREAMSC105))) ? SDL_RENDERER_PRESENTVSYNC : 0);
#else
	uint32_t rendererFlagsToSet = SDL_RENDERER_ACCELERATED | ((g_refKeenCfg.vSync != VSYNC_OFF) ? SDL_RENDERER_PRESENTVSYNC : 0);
#endif
	BEL_ST_RecreateSDLWindowAndRenderer(
		SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum),
		windowWidthToSet, windowHeightToSet, g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight, windowFlagsToSet, g_refKeenCfg.sdlRendererDriver, rendererFlagsToSet
	);

	BE_ST_SetScreenMode(3); // Includes SDL_Texture handling and output rects preparation

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	g_sdlDoAbsMouseMotion = g_refKeenCfg.absMouseMotion;
#endif
}

void BE_ST_ShutdownGfx(void)
{
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlFaceButtonsTexture);
	g_sdlFaceButtonsTexture = NULL;
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlDpadTexture);
	g_sdlDpadTexture = NULL;
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTextInputTexture);
	g_sdlTextInputTexture = NULL;
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlDebugKeysTexture);
	g_sdlDebugKeysTexture = NULL;
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTexture);
	g_sdlTexture = NULL;
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
	g_sdlTargetTexture = NULL;
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
}

void BEL_ST_RecreateSDLWindowAndRenderer(int x, int y, int windowWidth, int windowHeight, int fullWidth, int fullHeight, Uint32 windowFlags, int driverIndex, Uint32 rendererFlags)
{
	static int prev_x, prev_y, prev_driverIndex;
	static uint32_t prev_rendererFlags;

	if (g_sdlWindow)
	{
		// This is a little bit of a hack:
		// - x and y are compared to previous values, currently used to pick a display to use (in a multi-display setup).
		// - Since the actual flags of a window may differ from what we requested (due to toggling fullscreen or any other reason),
		// we support skipping window recreation only if fullscreen state did not change.
		// - Renderer flags are compared to the previously requested flags.
		// - Same is done with with renderer driver index. If -1 is used anywhere, this makes reuse of the same window more probable.
		//
		// However, if only the full screen resolution has changed, we update the window's display mode accordingly.
		if ((x == prev_x) && (y == prev_y) &&
		    ((windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) == (SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP)) &&
		    (driverIndex == prev_driverIndex) && (rendererFlags == prev_rendererFlags)
		)
			goto setupforfullscreen;

		SDL_DestroyRenderer(g_sdlRenderer);
		g_sdlRenderer = NULL;
		SDL_DestroyWindow(g_sdlWindow);
		g_sdlWindow = NULL;
	}

	// HACK - Create non-fullscreen window and then set as fullscreen, if required.
	// Reason is this lets us set non-fullscreen window size (for fullscreen toggling).
	g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, x, y, windowWidth, windowHeight, windowFlags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
	// A hack for Android x86 on VirtualBox - Try creating an OpenGL ES 1.1 context instead of 2.0
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateSDLWindowAndRenderer: Failed to create SDL2 window, forcing OpenGL (ES) version to 1.1 and retrying,\n%s\n", SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, x, y, windowWidth, windowHeight, windowFlags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateSDLWindowAndRenderer: Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}

	SDL_SetWindowIcon(g_sdlWindow, g_be_sdl_windowIconSurface);
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, driverIndex, rendererFlags);
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateSDLWindowAndRenderer: Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		exit(0);
	}

	prev_x = x;
	prev_y = y;
	prev_driverIndex = driverIndex;
	prev_rendererFlags = rendererFlags;

setupforfullscreen:

	// In case non-desktop fullscreen resolution is desired (even if window is currently *not* fullscreen);
	// But do so AFTER creating renderer! (Looks like SDL_CreateRenderer may re-create the window.)
	if (fullWidth && fullHeight)
	{
		SDL_DisplayMode mode;
		SDL_GetWindowDisplayMode(g_sdlWindow, &mode);
		mode.w = fullWidth;
		mode.h = fullHeight;
		SDL_SetWindowDisplayMode(g_sdlWindow, &mode);
	}
	SDL_SetWindowFullscreen(g_sdlWindow, windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

static void BEL_ST_RecreateTexture(void)
{
	if (g_sdlTexture)
	{
		BEL_ST_SDLDestroyTextureWrapper(&g_sdlTexture);
	}
	if (g_sdlTargetTexture)
	{
		BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
	}
	// Try using render target
	if ((g_refKeenCfg.scaleFactor > 1) && g_refKeenCfg.isBilinear)
	{
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTexture, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight, "nearest");
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
		// Try, if we fail then simply don't use this
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTargetTexture, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, g_sdlTexWidth*g_refKeenCfg.scaleFactor, g_sdlTexHeight*g_refKeenCfg.scaleFactor, "linear");
		if (g_sdlTargetTexture)
			BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BEL_ST_RecreateTexture: SDL2 target texture created successfully\n");
		else
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateTexture:  Failed to (re)create SDL2 target texture, continuing anyway\n%s\n", SDL_GetError());
	}
	else
	{
		// Use just a single texture
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTexture, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight, g_refKeenCfg.isBilinear ? "linear" : "nearest");
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
	}
}

// This code piece exists in order to handle SDL_RENDER* events

#define MAX_TEXTURES_POOL_SIZE 32

typedef struct {
	SDL_Texture **texturePtr;
	const char *scaleQuality;
} BESDLManagedTexture;

static BESDLManagedTexture g_sdlManagedTexturesPool[MAX_TEXTURES_POOL_SIZE];
static int g_sdlNumOfManagedTexturesInPool = 0;

void BEL_ST_SDLCreateTextureWrapper(SDL_Texture **pTexture, Uint32 format, int access, int w, int h, const char *scaleQuality)
{
	int i;
	for (i = 0; i < g_sdlNumOfManagedTexturesInPool; ++i)
		if (g_sdlManagedTexturesPool[i].texturePtr == pTexture)
		{
			g_sdlManagedTexturesPool[i].scaleQuality = scaleQuality;
			break;
		}

	if (i >= MAX_TEXTURES_POOL_SIZE)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_SDLCreateTextureWrapper: Managed textures pool overflow error!\n");
		//Destroy window and renderer?
		exit(0);
	}

	if (i == g_sdlNumOfManagedTexturesInPool)
	{
		BESDLManagedTexture *managedTexture = &g_sdlManagedTexturesPool[g_sdlNumOfManagedTexturesInPool++];
		managedTexture->texturePtr = pTexture;
		managedTexture->scaleQuality = scaleQuality;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality);
	*pTexture = SDL_CreateTexture(g_sdlRenderer, format, access, w, h);
}

void BEL_ST_SDLDestroyTextureWrapper(SDL_Texture **pTexture)
{
	if (*pTexture == NULL)
		return;

	for (int i = 0; i < g_sdlNumOfManagedTexturesInPool; ++i)
		if (g_sdlManagedTexturesPool[i].texturePtr == pTexture)
		{
			// Remove managed texture without moving the rest, except for maybe the last
			g_sdlManagedTexturesPool[i] = g_sdlManagedTexturesPool[--g_sdlNumOfManagedTexturesInPool];
			break;
		}

	SDL_DestroyTexture(*pTexture);
}

void BEL_ST_RecreateAllTextures(void)
{
	Uint32 format;
	int access, w, h;
	BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BEL_ST_RecreateAllTextures: Recreating textures\n");
	BESDLManagedTexture *managedTexture = g_sdlManagedTexturesPool;
	for (int i = 0; i < g_sdlNumOfManagedTexturesInPool; ++i, ++managedTexture)
	{
		SDL_Texture **pTexture = managedTexture->texturePtr;
		if (*pTexture == NULL)
			continue;

		SDL_QueryTexture(*pTexture, &format, &access, &w, &h);
		SDL_DestroyTexture(*pTexture);

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, managedTexture->scaleQuality);
		*pTexture = SDL_CreateTexture(g_sdlRenderer, format, access, w, h);
		if (*pTexture == NULL)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateAllTextures: Failed to recreate SDL2 texture %d out of %d,\n%s\n", i+1, g_sdlNumOfManagedTexturesInPool, SDL_GetError());
			exit(1);
		}
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BEL_ST_RecreateAllTextures: Recreated texture no %d out of %d\n", i+1, g_sdlNumOfManagedTexturesInPool);
	}

	BEL_ST_ForceHostDisplayUpdate();
	g_sdlDoRefreshGfxOutput = true; // BE_ST_MarkGfxForUpdate();
#ifdef REFKEEN_ENABLE_LAUNCHER
	BE_ST_Launcher_MarkGfxCache();
#endif
	// Also need to force refresh this way
	if (g_sdlScreenMode == 4) // CGA graphics
		g_sdlHostScrMemCache.cgaGfx[0] = g_sdlHostScrMem.cgaGfx[0]^0xFF;
	else
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF;
}

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


// Colors in BGRA format/order for CGA mode 4 (on certain platforms)

static const uint32_t g_sdlCGAGfxBGRAScreenColors[] = {
	0xff000000/*black*/,
	0xff00ffff/*light cyan*/,
	0xffff00ff/*light magenta*/,
	0xffffffff/*white*/
};

// Same but for EGA/VGA modes 0Dh-0Eh (and colored text modes on CGA/EGA/VGA)
// Note: Also used in launcher; extern is added for C++
#ifdef __cplusplus
extern
#endif
const uint32_t g_sdlEGABGRAScreenColors[] = {
	0xff000000/*black*/, 0xff0000aa/*blue*/, 0xff00aa00/*green*/, 0xff00aaaa/*cyan*/,
	0xffaa0000/*red*/, 0xffaa00aa/*magenta*/, 0xffaa5500/*brown*/, 0xffaaaaaa/*light gray*/,
	0xff555555/*gray*/, 0xff5555ff/*light blue*/, 0xff55ff55/*light green*/, 0xff55ffff/*light cyan*/,
	0xffff5555/*light red*/, 0xffff55ff/*light magenta*/, 0xffffff55/*yellow*/, 0xffffffff/*white*/
};

// Same but for VGA mode 013h
static const uint32_t g_sdlVGADefaultBGRAScreenColors[] = {
	0xff000000, 0xff0000aa, 0xff00aa00, 0xff00aaaa,
	0xffaa0000, 0xffaa00aa, 0xffaa5500, 0xffaaaaaa,
	0xff555555, 0xff5555ff, 0xff55ff55, 0xff55ffff,
	0xffff5555, 0xffff55ff, 0xffffff55, 0xffffffff,
	0xff000000, 0xff141414, 0xff202020, 0xff2c2c2c,
	0xff383838, 0xff454545, 0xff515151, 0xff616161,
	0xff717171, 0xff828282, 0xff929292, 0xffa2a2a2,
	0xffb6b6b6, 0xffcbcbcb, 0xffe3e3e3, 0xffffffff,
	0xff0000ff, 0xff4100ff, 0xff7d00ff, 0xffbe00ff,
	0xffff00ff, 0xffff00be, 0xffff007d, 0xffff0041,
	0xffff0000, 0xffff4100, 0xffff7d00, 0xffffbe00,
	0xffffff00, 0xffbeff00, 0xff7dff00, 0xff41ff00,
	0xff00ff00, 0xff00ff41, 0xff00ff7d, 0xff00ffbe,
	0xff00ffff, 0xff00beff, 0xff007dff, 0xff0041ff,
	0xff7d7dff, 0xff9e7dff, 0xffbe7dff, 0xffdf7dff,
	0xffff7dff, 0xffff7ddf, 0xffff7dbe, 0xffff7d9e,
	0xffff7d7d, 0xffff9e7d, 0xffffbe7d, 0xffffdf7d,
	0xffffff7d, 0xffdfff7d, 0xffbeff7d, 0xff9eff7d,
	0xff7dff7d, 0xff7dff9e, 0xff7dffbe, 0xff7dffdf,
	0xff7dffff, 0xff7ddfff, 0xff7dbeff, 0xff7d9eff,
	0xffb6b6ff, 0xffc7b6ff, 0xffdbb6ff, 0xffebb6ff,
	0xffffb6ff, 0xffffb6eb, 0xffffb6db, 0xffffb6c7,
	0xffffb6b6, 0xffffc7b6, 0xffffdbb6, 0xffffebb6,
	0xffffffb6, 0xffebffb6, 0xffdbffb6, 0xffc7ffb6,
	0xffb6ffb6, 0xffb6ffc7, 0xffb6ffdb, 0xffb6ffeb,
	0xffb6ffff, 0xffb6ebff, 0xffb6dbff, 0xffb6c7ff,
	0xff000071, 0xff1c0071, 0xff380071, 0xff550071,
	0xff710071, 0xff710055, 0xff710038, 0xff71001c,
	0xff710000, 0xff711c00, 0xff713800, 0xff715500,
	0xff717100, 0xff557100, 0xff387100, 0xff1c7100,
	0xff007100, 0xff00711c, 0xff007138, 0xff007155,
	0xff007171, 0xff005571, 0xff003871, 0xff001c71,
	0xff383871, 0xff453871, 0xff553871, 0xff613871,
	0xff713871, 0xff713861, 0xff713855, 0xff713845,
	0xff713838, 0xff714538, 0xff715538, 0xff716138,
	0xff717138, 0xff617138, 0xff557138, 0xff457138,
	0xff387138, 0xff387145, 0xff387155, 0xff387161,
	0xff387171, 0xff386171, 0xff385571, 0xff384571,
	0xff515171, 0xff595171, 0xff615171, 0xff695171,
	0xff715171, 0xff715169, 0xff715161, 0xff715159,
	0xff715151, 0xff715951, 0xff716151, 0xff716951,
	0xff717151, 0xff697151, 0xff617151, 0xff597151,
	0xff517151, 0xff517159, 0xff517161, 0xff517169,
	0xff517171, 0xff516971, 0xff516171, 0xff515971,
	0xff000041, 0xff100041, 0xff200041, 0xff300041,
	0xff410041, 0xff410030, 0xff410020, 0xff410010,
	0xff410000, 0xff411000, 0xff412000, 0xff413000,
	0xff414100, 0xff304100, 0xff204100, 0xff104100,
	0xff004100, 0xff004110, 0xff004120, 0xff004130,
	0xff004141, 0xff003041, 0xff002041, 0xff001041,
	0xff202041, 0xff282041, 0xff302041, 0xff382041,
	0xff412041, 0xff412038, 0xff412030, 0xff412028,
	0xff412020, 0xff412820, 0xff413020, 0xff413820,
	0xff414120, 0xff384120, 0xff304120, 0xff284120,
	0xff204120, 0xff204128, 0xff204130, 0xff204138,
	0xff204141, 0xff203841, 0xff203041, 0xff202841,
	0xff2c2c41, 0xff302c41, 0xff342c41, 0xff3c2c41,
	0xff412c41, 0xff412c3c, 0xff412c34, 0xff412c30,
	0xff412c2c, 0xff41302c, 0xff41342c, 0xff413c2c,
	0xff41412c, 0xff3c412c, 0xff34412c, 0xff30412c,
	0xff2c412c, 0xff2c4130, 0xff2c4134, 0xff2c413c,
	0xff2c4141, 0xff2c3c41, 0xff2c3441, 0xff2c3041,
	0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff000000, 0xff000000, 0xff000000, 0xff000000,
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


static void BEL_ST_CreatePadTextureIfNeeded(SDL_Texture **padTexturePtrPtr, int len)
{
	if (*padTexturePtrPtr)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(padTexturePtrPtr, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, len, len, "nearest");
	if (!(*padTexturePtrPtr))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 pad texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(*padTexturePtrPtr, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
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

static void BEL_ST_PrepareToShowOnePad(const int *scanCodes, const char **padXpm, SDL_Texture **padTexturePtrPtr, bool *areButtonsShownPtr, const int *textLocs, int len)
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
	SDL_UpdateTexture(*padTexturePtrPtr, NULL, pixels, 4*len);
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
	{
		BEL_ST_SDLDestroyTextureWrapper(&touchControl->texture);
		touchControl->texture = NULL;
	}
	int texWidth = touchControl->xpmWidth, texHeight = touchControl->xpmHeight;
	BEL_ST_SDLCreateTextureWrapper(&touchControl->texture, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, texWidth, texHeight, "nearest");
	SDL_Texture *texture = touchControl->texture;
	if (!texture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateTouchControlTexture: Failed to (re)create SDL2 touch control texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
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
	SDL_UpdateTexture(texture, NULL, pixels, 4*texWidth);
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

static void BEL_ST_SetTouchControlsRects(void);

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
	BEL_ST_SDLCreateTextureWrapper(&g_sdlTextInputTexture, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT, "nearest");
	if (!g_sdlTextInputTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 text input texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(g_sdlTextInputTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_CreateDebugKeysTextureIfNeeded(void)
{
	if (g_sdlDebugKeysTexture)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(&g_sdlDebugKeysTexture, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT, "nearest");
	if (!g_sdlDebugKeysTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 debug keys texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(g_sdlDebugKeysTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
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

	SDL_UpdateTexture(g_sdlTextInputTexture, NULL, pixels, 4*ALTCONTROLLER_TEXTINPUT_PIX_WIDTH);
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

	SDL_UpdateTexture(g_sdlDebugKeysTexture, NULL, pixels, 4*ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH);
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

	SDL_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	SDL_UpdateTexture(g_sdlTextInputTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}

static void BEL_ST_ToggleDebugKeysUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[y][x]], isMarked, isPressed);

	SDL_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	SDL_UpdateTexture(g_sdlDebugKeysTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
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
	bool lastBinaryStatus = !isPressed;
	BEL_ST_AltControlScheme_HandleEntry(
		(touchControlIndex >= 0) ? &g_sdlControllerMappingActualCurr->touchMappings[touchControlIndex].mapping : &g_sdlControllerMappingActualCurr->defaultMapping,
		isPressed ? g_sdlJoystickAxisMax : 0,
		(touchControlIndex >= 0) ? &lastBinaryStatus : &g_sdlDefaultMappingBinaryState
	);
}

void BEL_ST_UpdateVirtualCursorPositionFromPointer(int x, int y)
{
	extern int16_t g_sdlEmuMouseMotionAccumulatedState[2];
	extern int16_t g_sdlVirtualMouseCursorState[2];

	if (x < g_sdlAspectCorrectionRect.x)
		x = 0;
	else if (x >= g_sdlAspectCorrectionRect.x + g_sdlAspectCorrectionRect.w)
		x = GFX_TEX_WIDTH-1;
	else
		x = (x-g_sdlAspectCorrectionRect.x)*GFX_TEX_WIDTH/g_sdlAspectCorrectionRect.w;

	if (y < g_sdlAspectCorrectionRect.y)
		y = 0;
	else if (y >= g_sdlAspectCorrectionRect.y + g_sdlAspectCorrectionRect.h)
		y = GFX_TEX_HEIGHT-1;
	else
		y = (y-g_sdlAspectCorrectionRect.y)*GFX_TEX_HEIGHT/g_sdlAspectCorrectionRect.h;

	g_sdlEmuMouseMotionAccumulatedState[0] += x-g_sdlVirtualMouseCursorState[0];
	g_sdlEmuMouseMotionAccumulatedState[1] += y-g_sdlVirtualMouseCursorState[1];
	g_sdlVirtualMouseCursorState[0] = x;
	g_sdlVirtualMouseCursorState[1] = y;
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


static void BEL_ST_CalcWindowDimsFromCfg(int *outWidth, int *outHeight)
{
	int windowWidthToSet = g_refKeenCfg.winWidth;
	int windowHeightToSet = g_refKeenCfg.winHeight;
	if (!windowWidthToSet || !windowHeightToSet)
	{
		if (g_sdlIsSoftwareRendered)
		{
			windowWidthToSet = 640;
			windowHeightToSet = 480;
		}
		else
		{
			SDL_DisplayMode mode;
			SDL_GetDesktopDisplayMode(g_refKeenCfg.displayNum, &mode);
			// In the 200-lines modes on the VGA, where line doubling is in effect,
			// and after adding the overscan borders, the aspect ratio for the whole output
			// (after aspect correction i.e., multiplying height by 1.2) is 280:207.
			if (207*mode.w < 280*mode.h) // Thinner than 280:207
			{
				mode.h = mode.w*207/280;
			}
			else  // As wide as 280:207 at the least
			{
				mode.w = mode.h*280/207;
			}
			// Just for the sake of it, using the golden ratio...
			windowWidthToSet = mode.w*500/809;
			windowHeightToSet = mode.h*500/809;
		}
	}

	*outWidth = windowWidthToSet;
	*outHeight = windowHeightToSet;
}


static void BEL_ST_SetTouchControlsRects(void)
{
	if (((g_sdlControllerMappingActualCurr == NULL) || g_sdlControllerMappingActualCurr->touchMappings == NULL))
		return;

	SDL_Rect *currRect;
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

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
bool g_sdlDoAbsMouseMotion;
int g_sdlHostVirtualMouseCursorState[2];
static int g_sdlHostVirtualMouseCursorSideLen;
#endif

void BEL_ST_SetGfxOutputRects(bool allowResize)
{
	int srcWidth = g_sdlTexWidth;
	int srcHeight = g_sdlTexHeight;
	if (g_sdlScreenMode != 3)
	{
		// On the VGA, line doubling is in effect for graphical 200 lines modes
		if (g_sdlScreenMode != 0xe) // Is source 320x200 (not 640x200)?
			srcWidth *= 2;
		srcHeight *= 2;
	}
	int srcBorderLeft, srcBorderRight, srcBorderTop, srcBorderBottom;
	if (g_sdlScreenMode == 3)
	{
		srcBorderLeft = ENGINE_VGA_TXT_OVERSCAN_LEFT;
		srcBorderRight = ENGINE_VGA_TXT_OVERSCAN_RIGHT;
		srcBorderTop = ENGINE_VGA_TXT_OVERSCAN_TOP;
		srcBorderBottom = ENGINE_VGA_TXT_OVERSCAN_BOTTOM;
	}
	else
	{
		if (g_sdlScreenMode == 0x13)
		{
			srcBorderLeft = ENGINE_VGA_256COLORGFX_OVERSCAN_LEFT_AFTER_DOUBLING;
			srcBorderRight = ENGINE_VGA_256COLORGFX_OVERSCAN_RIGHT_AFTER_DOUBLING;
		}
		else
		{
			srcBorderLeft = ENGINE_VGA_16COLORGFX_OVERSCAN_LEFT_AFTER_DOUBLING;
			srcBorderRight = ENGINE_VGA_16COLORGFX_OVERSCAN_RIGHT_AFTER_DOUBLING;
		}
		srcBorderTop = ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING;
		srcBorderBottom = ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING;
	}
	int srcBorderedWidth = srcBorderLeft+srcWidth+srcBorderRight;
	int srcBorderedHeight = srcBorderTop+srcHeight+srcBorderBottom;
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);

	g_sdlLastReportedWindowWidth = winWidth;
	g_sdlLastReportedWindowHeight = winHeight;

	// Special case - We may resize window based on mode, but only if allowResize == true (to prevent any possible infinite resizes loop)
	if (allowResize && g_sdlIsSoftwareRendered && !g_refKeenCfg.forceFullSoftScaling && (!(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN)))
	{
		if ((g_refKeenCfg.scaleFactor*srcBorderedWidth != winWidth) || (g_refKeenCfg.scaleFactor*srcBorderedHeight != winHeight))
		{
			SDL_SetWindowSize(g_sdlWindow, g_refKeenCfg.scaleFactor*srcBorderedWidth, g_refKeenCfg.scaleFactor*srcBorderedHeight);
			// Not sure this will help, but still trying...
			SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
		}
	}

	// Save modified window size
	if (!(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		g_refKeenCfg.winWidth = winWidth;
		g_refKeenCfg.winHeight = winHeight;
	}

	if (g_sdlIsSoftwareRendered && !g_refKeenCfg.forceFullSoftScaling)
	{
		if (g_refKeenCfg.scaleFactor*srcBorderedWidth >= winWidth)
		{
			g_sdlAspectCorrectionBorderedRect.w = winWidth;
			g_sdlAspectCorrectionBorderedRect.x = 0;
		}
		else
		{
			g_sdlAspectCorrectionBorderedRect.w = g_refKeenCfg.scaleFactor*srcBorderedWidth;
			g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
		}

		if (g_refKeenCfg.scaleFactor*srcBorderedHeight >= winHeight)
		{
			g_sdlAspectCorrectionBorderedRect.h = winHeight;
			g_sdlAspectCorrectionBorderedRect.y = 0;
		}
		else
		{
			g_sdlAspectCorrectionBorderedRect.h = g_refKeenCfg.scaleFactor*srcBorderedHeight;
			g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
		}
	}
	else if (g_refKeenCfg.scaleType == SCALE_FILL)
	{
		g_sdlAspectCorrectionBorderedRect.w = winWidth;
		g_sdlAspectCorrectionBorderedRect.h = winHeight;
		g_sdlAspectCorrectionBorderedRect.x = 0;
		g_sdlAspectCorrectionBorderedRect.y = 0;
	}
	else
	{
		// Aspect correct - Includes overscan borders, should multiply
		// the height by 1.2 first (or the width by 5 and height 6),
		// if in 320x200 gfx mode, cause 1.2 == (320*3)/(4*200).
		// More generally, (srcwidth*3)/(4*srcheight).
		// So multiply width by 4*srcheight and height by 3*srcwidth;
		int scaledSrcBorderedWidth = (4*srcHeight)*srcBorderedWidth, scaledSrcBorderedHeight = (3*srcWidth)*srcBorderedHeight;
		if (scaledSrcBorderedHeight*winWidth < scaledSrcBorderedWidth*winHeight) // Thinner than scaledSrcBorderedWidth:scaledSrcBorderedHeight
		{
			g_sdlAspectCorrectionBorderedRect.w = winWidth;
			g_sdlAspectCorrectionBorderedRect.h = winWidth*scaledSrcBorderedHeight/scaledSrcBorderedWidth;
			g_sdlAspectCorrectionBorderedRect.x = 0;
			g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
		}
		else // As wide as scaledSrcBorderedWidth:scaledSrcBorderedHeight at the least
		{
			g_sdlAspectCorrectionBorderedRect.w = winHeight*scaledSrcBorderedWidth/scaledSrcBorderedHeight;
			g_sdlAspectCorrectionBorderedRect.h = winHeight;
			g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
			g_sdlAspectCorrectionBorderedRect.y = 0;
		}
	}
	// Finish with internal (non-bordered) rect
	g_sdlAspectCorrectionRect.x = g_sdlAspectCorrectionBorderedRect.x + g_sdlAspectCorrectionBorderedRect.w*srcBorderLeft/srcBorderedWidth;
	g_sdlAspectCorrectionRect.y = g_sdlAspectCorrectionBorderedRect.y + g_sdlAspectCorrectionBorderedRect.h*srcBorderTop/srcBorderedHeight;
	g_sdlAspectCorrectionRect.w = g_sdlAspectCorrectionBorderedRect.w*srcWidth/srcBorderedWidth;
	g_sdlAspectCorrectionRect.h = g_sdlAspectCorrectionBorderedRect.h*srcHeight/srcBorderedHeight;
	// As a little addition have this too - but:
	// 1. Use same dimensions independently of scaling.
	// 2. The dimensions of the controller UI are picked relatively to the host window's internal contents (without borders), not directly related to the client window size.
	// 3. Also taking the whole window into account (this doesn't depend on "screen mode", borders and more).
	// 4. Finally, try to be consistent with the positioning and sizes of touch controls (even though it's not necessary).
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

	g_sdlDebugFingerRectSideLen = minWinDim/4;
	BEL_ST_SetTouchControlsRects();

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	g_sdlHostVirtualMouseCursorState[0] = winWidth/2;
	g_sdlHostVirtualMouseCursorState[1] = winHeight/2;
	g_sdlHostVirtualMouseCursorSideLen = minWinDim/64;
#endif
}

void BEL_ST_ForceHostDisplayUpdate(void)
{
	g_sdlForceGfxControlUiRefresh = true; // HACK that technically does exactly what we want (even if controls are not drawn)
}

uint8_t *BE_ST_GetTextModeMemoryPtr(void)
{
	return g_sdlVidMem.text;
}

bool BE_ST_HostGfx_CanToggleAspectRatio(void)
{
	return (!g_sdlIsSoftwareRendered || g_refKeenCfg.forceFullSoftScaling);
}

bool BE_ST_HostGfx_GetAspectRatioToggle(void)
{
	return (g_refKeenCfg.scaleType == SCALE_FILL);
}

void BE_ST_HostGfx_SetAspectRatioToggle(bool aspectToggle)
{
	g_refKeenCfg.scaleType = aspectToggle ? SCALE_FILL : SCALE_ASPECT;
	BEL_ST_SetGfxOutputRects(false);
	BEL_ST_ForceHostDisplayUpdate();
}

bool BE_ST_HostGfx_CanToggleFullScreen(void)
{
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	return true;
#else
	return false;
#endif
}

bool BE_ST_HostGfx_GetFullScreenToggle(void)
{
	return ((SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN);
}

void BE_ST_HostGfx_SetFullScreenToggle(bool fullScreenToggle)
{
	if (fullScreenToggle)
		SDL_SetWindowFullscreen(g_sdlWindow, (g_refKeenCfg.fullWidth && g_refKeenCfg.fullHeight) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP);
	else
		SDL_SetWindowFullscreen(g_sdlWindow, 0);

	g_refKeenCfg.isFullscreen = BE_ST_HostGfx_GetFullScreenToggle();
}

void BE_ST_HostGfx_ToggleFullScreen(void)
{
	BE_ST_HostGfx_SetFullScreenToggle(!BE_ST_HostGfx_GetFullScreenToggle());
}

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
void BE_ST_HostGfx_SetAbsMouseCursorToggle(bool cursorToggle)
{
	g_sdlDoAbsMouseMotion = cursorToggle;
}
#endif

static uint32_t g_sdlEGACurrBGRAPalette[256], g_sdlEGACurrBGRAPaletteCache[256];
static uint32_t g_sdlEGALastBGRABorderColor;
static uint8_t g_overscanBorderColorIndex, g_overscanBorderColorIndexCache;

/* Gets a value represeting 6 EGA signals determining a color number and
 * returns it in a "Blue Green Red Intensity" 4-bit format.
 * Usually, the 6 signals represented by the given input mean:
 * "Blue Green Red Secondary-Blue Secondary-Green Secondary-Red". However, for
 * the historical reason of compatibility with CGA monitors, on the 200-lines
 * modes used by Keen the Secondary-Green signal is treated as an Intensity
 * one and the two other intensity signals are ignored.
 */
static int BEL_ST_ConvertEGASignalToEGAEntry(int color)
{
	return (color & 7) | ((color & 16) >> 1);
}


void BE_ST_SetScreenStartAddress(uint16_t crtc)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlScreenStartAddress != crtc);
	g_sdlScreenStartAddress = crtc;
}

void BE_ST_SetBorderColor(uint8_t color)
{
	if (g_sdlScreenMode == 0x13)
		g_overscanBorderColorIndex = color;
	else
		g_overscanBorderColorIndex = BEL_ST_ConvertEGASignalToEGAEntry(color);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGASetPaletteAndBorder(const uint8_t *palette)
{
	for (int entry = 0; entry < 16; ++entry)
		g_sdlEGACurrBGRAPalette[entry] = g_sdlEGABGRAScreenColors[BEL_ST_ConvertEGASignalToEGAEntry(palette[entry])];
	g_overscanBorderColorIndex = BEL_ST_ConvertEGASignalToEGAEntry(palette[16]);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGASetPalette(const uint8_t *palette)
{
	for (int entry = 0; entry < 256; ++entry, palette += 3)
		g_sdlEGACurrBGRAPalette[entry] =
			0xFC000000 | (palette[0] << 18) | (palette[1] << 10) | (palette[2] << 2);
	g_overscanBorderColorIndexCache = g_overscanBorderColorIndex^0xFF; // Force refresh
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGASetPelPanning(uint8_t panning)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlPelPanning != panning);
	g_sdlPelPanning = panning;
}

void BE_ST_EGASetLineWidth(uint8_t widthInBytes)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlPixLineWidth != 8*widthInBytes);
	g_sdlPixLineWidth = 8*widthInBytes;
}

void BE_ST_EGASetSplitScreen(int16_t linenum)
{
	// VGA only for now (200-lines graphics modes)
	if (g_sdlTexHeight == GFX_TEX_HEIGHT)
	{
		// Because 200-lines modes are really double-scanned to 400,
		// a linenum of x was originally replaced with 2x-1 in id_vw.c.
		// In practice it should've probably been 2x+1, and this is how
		// we "correct" it here (one less black line in Catacomb Abyss
		// before gameplay begins in a map, above the status bar).
		g_sdlSplitScreenLine = linenum/2;
	}
	else
		g_sdlSplitScreenLine = linenum;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXByteInPlane(uint16_t destOff, uint8_t srcVal, uint16_t planeNum)
{
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_repeat[1 << planeNum]) | (g_be_st_lookup_linear_to_egaplane[srcVal] << planeNum);
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_LinearToWrapped_MemCopy
static void BEL_ST_LinearToEGAPlane_MemCopy(uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num, uint16_t planeNum)
{
	uint64_t planeInvRepeatedMask = ~g_be_st_lookup_repeat[1 << planeNum];
	uint64_t *planeDstPtr = &g_sdlVidMem.egaGfx[planeDstOff];
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		for (int i = 0; i < num; ++i, ++planeDstPtr, ++linearSrc)
			*planeDstPtr = ((*planeDstPtr) & planeInvRepeatedMask) | (g_be_st_lookup_linear_to_egaplane[*linearSrc] << planeNum);
	}
	else
	{
		for (int i = 0; i < bytesToEnd; ++i, ++planeDstPtr, ++linearSrc)
			*planeDstPtr = ((*planeDstPtr) & planeInvRepeatedMask) | (g_be_st_lookup_linear_to_egaplane[*linearSrc] << planeNum);
		planeDstPtr = g_sdlVidMem.egaGfx;
		for (int i = 0; i < num-bytesToEnd; ++i, ++planeDstPtr, ++linearSrc)
			*planeDstPtr = ((*planeDstPtr) & planeInvRepeatedMask) | (g_be_st_lookup_linear_to_egaplane[*linearSrc] << planeNum);
	}
	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_LinearToVGAPlane_MemCopy(uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num, uint16_t planeNum)
{
	uint8_t *planeDstPtr = &g_sdlVidMem.vgaGfx[4*planeDstOff + planeNum];
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		for (int i = 0; i < num; ++i, planeDstPtr += 4, ++linearSrc)
			*planeDstPtr = *linearSrc;
	}
	else
	{
		for (int i = 0; i < bytesToEnd; ++i, planeDstPtr += 4, ++linearSrc)
			*planeDstPtr = *linearSrc;
		planeDstPtr = &g_sdlVidMem.vgaGfx[planeNum];
		for (int i = 0; i < num-bytesToEnd; ++i, planeDstPtr += 4, ++linearSrc)
			*planeDstPtr = *linearSrc;
	}
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToLinear_MemCopy
static void BEL_ST_EGAPlaneToLinear_MemCopy(uint8_t *linearDst, uint16_t planeSrcOff, uint16_t num, uint16_t planenum)
{
	const uint64_t *planeSrcPtr = &g_sdlVidMem.egaGfx[planeSrcOff];
	uint16_t bytesToEnd = 0x10000-planeSrcOff;
	if (num <= bytesToEnd)
	{
		for (int i = 0; i < num; ++i, ++linearDst, ++planeSrcPtr)
			*linearDst = BEL_ST_Lookup_EGAPlaneToLinear((*planeSrcPtr)>>planenum);
	}
	else
	{
		for (int i = 0; i < bytesToEnd; ++i, ++linearDst, ++planeSrcPtr)
			*linearDst = BEL_ST_Lookup_EGAPlaneToLinear((*planeSrcPtr)>>planenum);
		planeSrcPtr = g_sdlVidMem.egaGfx;
		for (int i = 0; i < num-bytesToEnd; ++i, ++linearDst, ++planeSrcPtr)
			*linearDst = BEL_ST_Lookup_EGAPlaneToLinear((*planeSrcPtr)>>planenum);
	}
	//No need to since we just read screen data
	//g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToWrapped_MemCopy
static void BEL_ST_EGAVGAPlaneToAllPlanes_MemCopy(
	uint16_t planeDstOff, uint16_t planeSrcOff, uint16_t num, int pixelsPerAddr)
{
	uint8_t *vidMemPtr = g_sdlVidMem.vgaGfx;
	uint16_t srcBytesToEnd = 0x10000-planeSrcOff;
	uint16_t dstBytesToEnd = 0x10000-planeDstOff;
	if (num <= srcBytesToEnd)
	{
		// Source is linear: Same as BE_Cross_LinearToWrapped_MemCopy here
		if (num <= dstBytesToEnd)
			memcpy(
				vidMemPtr + pixelsPerAddr*planeDstOff,
				vidMemPtr + pixelsPerAddr*planeSrcOff,
				pixelsPerAddr*num);
		else
		{
			memcpy(
				vidMemPtr + pixelsPerAddr*planeDstOff,
				vidMemPtr + pixelsPerAddr*planeSrcOff,
				pixelsPerAddr*dstBytesToEnd);
			memcpy(
				vidMemPtr,
				vidMemPtr + pixelsPerAddr*(planeSrcOff+dstBytesToEnd),
				pixelsPerAddr*(num-dstBytesToEnd));
		}
	}
	// Otherwise, check if at least the destination is linear
	else if (num <= dstBytesToEnd)
	{
		// Destination is linear: Same as
		// BE_Cross_WrappedToLinear_MemCopy, non-linear source
		memcpy(
			vidMemPtr + pixelsPerAddr*planeDstOff,
			vidMemPtr + pixelsPerAddr*planeSrcOff,
			pixelsPerAddr*srcBytesToEnd);
		memcpy(
			vidMemPtr + pixelsPerAddr*(planeDstOff+srcBytesToEnd),
			vidMemPtr,
			pixelsPerAddr*(num-srcBytesToEnd));
	}
	// BOTH buffers have wrapping. We don't check separately if
	// srcBytesToEnd==dstBytesToEnd (in such a case planeDstOff==planeSrcOff...)
	else if (srcBytesToEnd <= dstBytesToEnd)
	{
		memcpy(
			vidMemPtr + pixelsPerAddr*planeDstOff,
			vidMemPtr + pixelsPerAddr*planeSrcOff,
			pixelsPerAddr*srcBytesToEnd);
		memcpy(
			vidMemPtr + pixelsPerAddr*(planeDstOff+srcBytesToEnd),
			vidMemPtr,
			pixelsPerAddr*(dstBytesToEnd-srcBytesToEnd));
		memcpy(
			vidMemPtr,
			vidMemPtr + pixelsPerAddr*(dstBytesToEnd-srcBytesToEnd),
			pixelsPerAddr*(num-dstBytesToEnd));
	}
	else // srcBytesToEnd > dstBytesToEnd
	{
		memcpy(
			vidMemPtr + pixelsPerAddr*planeDstOff,
			vidMemPtr + pixelsPerAddr*planeSrcOff,
			pixelsPerAddr*dstBytesToEnd);
		memcpy(
			vidMemPtr,
			vidMemPtr + pixelsPerAddr*(planeSrcOff+dstBytesToEnd),
			pixelsPerAddr*(srcBytesToEnd-dstBytesToEnd));
		memcpy(
			vidMemPtr + pixelsPerAddr*(srcBytesToEnd-dstBytesToEnd),
			vidMemPtr,
			pixelsPerAddr*(num-srcBytesToEnd));
	}
	g_sdlDoRefreshGfxOutput = true;
}

// A similar analogue of memset
static void BEL_ST_EGAPlane_MemSet(uint64_t *planeDstPtr, uint16_t planeDstOff, uint8_t value, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		memset(planeDstPtr+planeDstOff, value, 8*num);
	}
	else
	{
		memset(planeDstPtr+planeDstOff, value, 8*bytesToEnd);
		memset(planeDstPtr, value, 8*(num-bytesToEnd));
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXBufferInPlane(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeNum)
{
	BEL_ST_LinearToEGAPlane_MemCopy(destOff, srcPtr, num, planeNum);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXBufferInPlane(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeNum)
{
	BEL_ST_LinearToVGAPlane_MemCopy(destOff, srcPtr, num, planeNum);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff)
{
	g_sdlVidMem.egaGfx[destOff] = g_sdlVidMem.egaGfx[srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr but with a specific plane (added for Catacomb Abyss vanilla bug reproduction/workaround)
void BE_ST_EGAUpdateGFXByteInPlaneScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t planeNum)
{
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_repeat[1<<planeNum]) | (g_sdlVidMem.egaGfx[srcOff] & g_be_st_lookup_repeat[1<<planeNum]);
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr but picking specific bits out of each byte
void BE_ST_EGAUpdateGFXBitsInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_bitsmask[bitsMask]) | (g_sdlVidMem.egaGfx[srcOff] & g_be_st_lookup_bitsmask[bitsMask]);
	g_sdlDoRefreshGfxOutput = true;
}


void BE_ST_EGAUpdateGFXBufferInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_ST_EGAVGAPlaneToAllPlanes_MemCopy(destOff, srcOff, num, 8);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_ST_EGAVGAPlaneToAllPlanes_MemCopy(destOff, srcOff, num, 4);
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t BE_ST_EGAFetchGFXByteFromPlane(uint16_t destOff, uint16_t planenum)
{
	return BEL_ST_Lookup_EGAPlaneToLinear(g_sdlVidMem.egaGfx[destOff]>>planenum);
}

void BE_ST_EGAFetchGFXBufferFromPlane(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum)
{
	BEL_ST_EGAPlaneToLinear_MemCopy(destPtr, srcOff, num, planenum);
}

void BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(uint16_t destOff, uint8_t color, uint8_t bitsMask)
{
	color &= 0xF; // We may get a larger value in The Catacombs Armageddon (sky color)
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_bitsmask[bitsMask]) | (g_be_st_lookup_repeat[color] & g_be_st_lookup_bitsmask[bitsMask]);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXBufferFrom4bitsPixel(uint16_t destOff, uint8_t color, uint16_t count)
{
	color &= 0xF; // We may get a larger value in The Catacombs Armageddon (sky color)
	BEL_ST_EGAPlane_MemSet(g_sdlVidMem.egaGfx, destOff, color, count);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAXorGFXByteByPlaneMask(uint16_t destOff, uint8_t srcVal, uint16_t planeMask)
{
	g_sdlVidMem.egaGfx[destOff] ^= ((g_be_st_lookup_repeat[planeMask] & g_be_st_lookup_bitsmask[srcVal]));
	g_sdlDoRefreshGfxOutput = true;
}



void BE_ST_CGAUpdateGFXBufferFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth)
{
	const uint8_t *endSegPtr = segPtr + 0x10000;
	uint8_t *cgaHostPtr = g_sdlHostScrMem.cgaGfx;
	// Remember this is a wrapped copy, offInSegPtr points inside a 65536-bytes long segment beginning at segPtr;
	// But we still want to skip some bytes, assuming byteLineWidth sources bytes per line (picking 80 out of these)
	int lineBytesLeft = byteLineWidth - GFX_TEX_WIDTH/4;
	for (int line = 0, byteInLine; line < GFX_TEX_HEIGHT; ++line)
	{
		for (byteInLine = 0; byteInLine < GFX_TEX_WIDTH/4; ++byteInLine, ++offInSegPtr, offInSegPtr = (offInSegPtr == endSegPtr) ? segPtr : offInSegPtr)
		{
			*cgaHostPtr++ = ((*offInSegPtr) & 0xC0) >> 6;
			*cgaHostPtr++ = ((*offInSegPtr) & 0x30) >> 4;
			*cgaHostPtr++ = ((*offInSegPtr) & 0x0C) >> 2;
			*cgaHostPtr++ = ((*offInSegPtr) & 0x03);
		}
		offInSegPtr += lineBytesLeft;
		if (offInSegPtr >= endSegPtr)
		{
			offInSegPtr = segPtr + (uint16_t)(offInSegPtr-segPtr);
		}
	}
	g_sdlDoRefreshGfxOutput |= memcmp(g_sdlHostScrMemCache.cgaGfx, g_sdlHostScrMem.cgaGfx, sizeof(g_sdlHostScrMem.cgaGfx));
	memcpy(g_sdlHostScrMemCache.cgaGfx, g_sdlHostScrMem.cgaGfx, sizeof(g_sdlHostScrMem.cgaGfx));
}


void BE_ST_SetScreenMode(int mode)
{
	g_sdlDoRefreshGfxOutput = true;
	g_overscanBorderColorIndex = 0;
	g_overscanBorderColorIndexCache = g_overscanBorderColorIndex^0xFF; // Force refresh
	g_sdlEGALastBGRABorderColor = g_sdlEGABGRAScreenColors[0];
	switch (mode)
	{
	case 3:
	{
		g_sdlTexWidth = VGA_TXT_TEX_WIDTH;
		g_sdlTexHeight = VGA_TXT_TEX_HEIGHT;
		g_sdlTxtCursorPosX = g_sdlTxtCursorPosY = 0;
		// REFKEEN - Re-use BE_ST_clrscr with default text and background
		// colors, but ensure we preserve the previously set colors
		// for functions like BE_ST_cprintf (required for DEMOCAT)
		int origTxtColor = g_sdlTxtColor, origTxtBackground = g_sdlTxtBackground;
		g_sdlTxtColor = 7;
		g_sdlTxtBackground = 0;
		g_sdlTxtCursorEnabled = true;
		BE_ST_clrscr();
		g_sdlTxtColor = origTxtColor;
		g_sdlTxtBackground = origTxtBackground;
		break;
	}
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlHostScrMem.cgaGfx, 0, sizeof(g_sdlHostScrMem.cgaGfx));
		g_sdlHostScrMemCache.cgaGfx[0] = g_sdlHostScrMem.cgaGfx[0]^0xFF; // Force refresh
		break;
	case 0xD:
	case 0x13:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		if (mode == 0x13)
			memcpy(g_sdlEGACurrBGRAPalette, g_sdlVGADefaultBGRAScreenColors, sizeof(g_sdlVGADefaultBGRAScreenColors));
		else
			memcpy(g_sdlEGACurrBGRAPalette, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlPelPanning = 0;
		g_sdlPixLineWidth = 8*40;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if ((mode == 0xD) && (g_sdlScreenMode != 0xE))
		{
			memset(g_sdlVidMem.egaGfx, 0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx, 0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	case 0xE:
		g_sdlTexWidth = 2*GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPalette, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlPelPanning = 0;
		g_sdlPixLineWidth = 8*80;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if (g_sdlScreenMode != 0xD)
		{
			memset(g_sdlVidMem.egaGfx,  0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx,  0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	}
	g_sdlScreenMode = mode;
	BEL_ST_SetGfxOutputRects(true);
	BEL_ST_RecreateTexture();
}

void BE_ST_textcolor(int color)
{
	g_sdlTxtColor = color;
}

void BE_ST_textbackground(int color)
{
	g_sdlTxtBackground = color;
}

void BE_ST_clrscr(void)
{
	uint8_t *currMemByte = g_sdlVidMem.text;
	for (int i = 0; i < TXT_COLS_NUM*TXT_ROWS_NUM; ++i)
	{
		*(currMemByte++) = ' ';
		*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_MoveTextCursorTo(int x, int y)
{
	g_sdlTxtCursorPosX = x;
	g_sdlTxtCursorPosY = y;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_ToggleTextCursor(bool isEnabled)
{
	g_sdlTxtCursorEnabled = isEnabled;
}

static uint8_t *BEL_ST_printchar(uint8_t *currMemByte, char ch, bool iscolored, bool requirecrchar)
{
	if (ch == '\t')
	{
		int nextCursorPosX = (g_sdlTxtCursorPosX & ~7) + 8;
		for (; g_sdlTxtCursorPosX != nextCursorPosX; ++g_sdlTxtCursorPosX)
		{
			*(currMemByte++) = ' ';
			*currMemByte = iscolored ? (g_sdlTxtColor | (g_sdlTxtBackground << 4)) : *currMemByte;
			++currMemByte;
		}
	}
	else if (ch == '\r')
	{
		if (!requirecrchar)
			return currMemByte; // Do nothing

		g_sdlTxtCursorPosX = 0; // Carriage return
		currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
	}
	else if (ch == '\n')
	{
		if (!requirecrchar)
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
		}
		++g_sdlTxtCursorPosY;
		currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	}
	else
	{
		*(currMemByte++) = ch;
		*currMemByte = iscolored ? (g_sdlTxtColor | (g_sdlTxtBackground << 4)) : *currMemByte;
		++currMemByte;
		++g_sdlTxtCursorPosX;
	}

	// Go to next line
	if (g_sdlTxtCursorPosX == TXT_COLS_NUM)
	{
		g_sdlTxtCursorPosX = 0; // Carriage return
		++g_sdlTxtCursorPosY; // Line feed
		currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
	}
	// Shift screen by one line
	if (g_sdlTxtCursorPosY == TXT_ROWS_NUM)
	{
		--g_sdlTxtCursorPosY;
		// Scroll one line down
		uint8_t lastAttr = g_sdlVidMem.text[sizeof(g_sdlVidMem.text)-1];
		memmove(g_sdlVidMem.text, g_sdlVidMem.text+2*TXT_COLS_NUM, sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM);
		currMemByte = g_sdlVidMem.text+sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM;
		// New empty line
		for (int i = 0; i < TXT_COLS_NUM; ++i)
		{
			*(currMemByte++) = ' ';
			*(currMemByte++) = lastAttr;
		}
		currMemByte -= 2*TXT_COLS_NUM; // Go back to beginning of line
	}

	return currMemByte;
}

void BE_ST_puts(const char *str)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		currMemByte = BEL_ST_printchar(currMemByte, *str, true, false);
	}
	BEL_ST_printchar(currMemByte, '\n', true, false);

	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_vprintf_impl(const char *format, va_list args, bool iscolored, bool requirecrchar);

void BE_ST_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	BEL_ST_vprintf_impl(format, args, false, false);
	va_end(args);
}

void BE_ST_cprintf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	BEL_ST_vprintf_impl(format, args, true, true);
	va_end(args);
}

void BE_ST_vprintf(const char *format, va_list args)
{
	BEL_ST_vprintf_impl(format, args, false, false);
}

// There's no colored version of vprintf in the original codebase

static void BEL_ST_vprintf_impl(const char *format, va_list args, bool iscolored, bool requirecrchar)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	while (*format)
	{
		if (*format == '%')
		{
			switch (*(++format))
			{
			case '%':
				currMemByte = BEL_ST_printchar(currMemByte, '%', iscolored, requirecrchar);
				break;
			case 's':
			{
				for (const char *str = va_arg(args, char *); *str; ++str)
				{
					currMemByte = BEL_ST_printchar(currMemByte, *str, iscolored, requirecrchar);
				}
				break;
			}
			default:
			{
				// Do NOT constify this cause of hack...
				char errorMsg[] = "BEL_ST_vprintf_impl: Unsupported format specifier flag: X\n";
				errorMsg[strlen(errorMsg)-2] = *format; // Hack
				BE_ST_ExitWithErrorMsg(errorMsg);
			}
			}
		}
		else
		{
			currMemByte = BEL_ST_printchar(currMemByte, *format, iscolored, requirecrchar);
		}
		++format;
	}
	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_FinishHostDisplayUpdate(void)
{
	g_sdlForceGfxControlUiRefresh = false;

	if (g_refKeenCfg.touchInputDebugging)
	{
		SDL_SetRenderDrawColor(g_sdlRenderer, 0xFF, 0x00, 0x00, 0xBF); // Includes some alpha
		SDL_SetRenderDrawBlendMode(g_sdlRenderer, SDL_BLENDMODE_BLEND);
		BESDLTrackedFinger *trackedFinger = g_sdlTrackedFingers;
		for (int i = 0; i < g_nOfTrackedFingers; ++i, ++trackedFinger)
		{
			SDL_Rect rect = {trackedFinger->lastX-g_sdlDebugFingerRectSideLen/2, trackedFinger->lastY-g_sdlDebugFingerRectSideLen/2, g_sdlDebugFingerRectSideLen, g_sdlDebugFingerRectSideLen};
			SDL_RenderFillRect(g_sdlRenderer, &rect);
		}
		SDL_SetRenderDrawBlendMode(g_sdlRenderer, SDL_BLENDMODE_NONE);
	}

	if (g_sdlShowTouchUI && g_sdlTouchControlsAreShown)
	{
		for (int i = 0; i < g_sdlNumOfOnScreenTouchControls; ++i)
			SDL_RenderCopy(g_sdlRenderer, g_sdlOnScreenTouchControlsTextures[i], NULL, &g_sdlOnScreenTouchControlsRects[i]);
	}
	if (g_sdlShowControllerUI || g_sdlShowTouchUI)
	{
		if (g_sdlFaceButtonsAreShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlFaceButtonsTexture, NULL, &g_sdlControllerFaceButtonsRect);
		}
		if (g_sdlDpadIsShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlDpadTexture, NULL, &g_sdlControllerDpadRect);
		}
		if (g_sdlTextInputUIIsShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlTextInputTexture, NULL, &g_sdlControllerTextInputRect);
		}
		if (g_sdlDebugKeysUIIsShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlDebugKeysTexture, NULL, &g_sdlControllerDebugKeysRect);
		}
	}

	SDL_RenderPresent(g_sdlRenderer);
}

static uint32_t g_be_sdlLastRefreshTicks = 0;

void BEL_ST_UpdateHostDisplay(void)
{
	// Refresh graphics from time to time in case a part of the window is overridden by anything,
	// like the Steam Overlay.
	uint32_t currRefreshTicks = SDL_GetTicks();
	if (currRefreshTicks - g_be_sdlLastRefreshTicks >= 100)
		g_sdlForceGfxControlUiRefresh = true;

	if (g_sdlScreenMode == 3) // Text mode
	{
		// For graphics modes we don't have to refresh if g_sdlDoRefreshGfxOutput is set to false,
		// but in textual mode we have blinking characters and cursor to take care of
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		bool areBlinkingCharsShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = g_sdlTxtCursorEnabled && (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
		if (!g_sdlDoRefreshGfxOutput && (wereBlinkingCharsShown == areBlinkingCharsShown) && (wasBlinkingCursorShown == isBlinkingCursorShown))
		{
			if (g_sdlForceGfxControlUiRefresh)
				goto dorefresh;
			return;
		}
		/****** Do update ******/
		wereBlinkingCharsShown = areBlinkingCharsShown;
		wasBlinkingCursorShown = isBlinkingCursorShown;
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *screenPixelPtr = (uint32_t *)pixels;
		uint8_t currChar;
		const uint8_t *currCharFontPtr;
		uint32_t *currScrPixelPtr, currBackgroundColor, currCharColor;
		int txtByteCounter = 0;
		int currCharPixX, currCharPixY;
		for (int currCharY = 0, currCharX; currCharY < TXT_ROWS_NUM; ++currCharY)
		{
			// Draw striped lines
			for (currCharX = 0; currCharX < TXT_COLS_NUM; ++currCharX)
			{
				currChar = g_sdlVidMem.text[txtByteCounter];
				// Luckily, the width*height product is always divisible by 8...
				// Note that the actual width is always 8,
				// even in VGA mode. We convert to 9 while drawing.
				currCharFontPtr = g_vga_8x16TextFont + currChar*16*8;
				++txtByteCounter;
				currBackgroundColor = g_sdlEGABGRAScreenColors[(g_sdlVidMem.text[txtByteCounter] >> 4) & 7];
				// Should the character blink?
				if (!(g_sdlVidMem.text[txtByteCounter] & 0x80) || areBlinkingCharsShown)
					currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[txtByteCounter] & 15];
				else
					currCharColor = currBackgroundColor;
				++txtByteCounter;
				currScrPixelPtr = screenPixelPtr;
				for (currCharPixY = 0; currCharPixY < VGA_TXT_CHAR_PIX_HEIGHT; ++currCharPixY)
				{
					/* NOTE: The char width is actually 8
					in both of the EGA and VGA fonts. On the
					VGA case, the 9th pixel is determined
					according to the 8th and char number. */
					for (currCharPixX = 0; currCharPixX < 8; ++currCharPixX, ++currCharFontPtr, ++currScrPixelPtr)
					{
						*currScrPixelPtr = (*currCharFontPtr) ? currCharColor : currBackgroundColor;
					}
					// Add an extra 9th column on VGA
					*currScrPixelPtr = ((currChar < 192) || (currChar > 223)) ? currBackgroundColor : *(currScrPixelPtr-1);
					currScrPixelPtr += (g_sdlTexWidth-VGA_TXT_CHAR_PIX_WIDTH+1);
				}
				screenPixelPtr += VGA_TXT_CHAR_PIX_WIDTH;
			}
			// Go to the character right below current one
			screenPixelPtr += g_sdlTexWidth*(VGA_TXT_CHAR_PIX_HEIGHT-1);
		}
		// Finish with outputting the cursor if required
		currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[1+((TXT_COLS_NUM*g_sdlTxtCursorPosY+g_sdlTxtCursorPosX)<<1)] & 15];
		if (isBlinkingCursorShown)
		{
			screenPixelPtr = (uint32_t *)pixels+g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosY*VGA_TXT_CHAR_PIX_HEIGHT*g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosX*VGA_TXT_CHAR_PIX_WIDTH;
			// Out of 3 last scanlines of char, draw to the first 2.
			screenPixelPtr += (VGA_TXT_CHAR_PIX_HEIGHT-3)*g_sdlTexWidth;
			for (currCharPixY = 0; currCharPixY < 2; currCharPixY++)
			{
				for (currCharPixX = 0; currCharPixX < VGA_TXT_CHAR_PIX_WIDTH; currCharPixX++, screenPixelPtr++)
					*screenPixelPtr = currCharColor;
				screenPixelPtr += g_sdlTexWidth - VGA_TXT_CHAR_PIX_WIDTH;
			}
		}
	}
	else if (g_sdlScreenMode == 4) // CGA graphics
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			if (g_sdlForceGfxControlUiRefresh)
				goto dorefresh;
			return;
		}
		// That's easy now since there isn't a lot that can be done...
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		uint8_t *currPalPixPtr = g_sdlHostScrMem.cgaGfx;
		for (int pixnum = 0; pixnum < GFX_TEX_WIDTH*GFX_TEX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlCGAGfxBGRAScreenColors[*currPalPixPtr];
		}
	}
	else // EGA/VGA graphics mode 0xD or 0xE, or VGA mode 0x13
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			if (g_sdlForceGfxControlUiRefresh)
				goto dorefresh;
			return;
		}
		uint32_t warpAroundOffset = (g_sdlScreenMode == 0x13) ? 0x40000 : 0x80000;
		uint32_t currLineFirstPixelNum =
			((g_sdlScreenMode == 0x13) ?
			 (4*g_sdlScreenStartAddress + g_sdlPelPanning) :
			 (8*g_sdlScreenStartAddress + g_sdlPelPanning)) % warpAroundOffset;
		uint8_t *currPalPixPtr;
		for (int line = 0; line < GFX_TEX_HEIGHT; ++line)
		{
			currPalPixPtr = g_sdlHostScrMem.egaGfx + line*g_sdlTexWidth;
			// REFKEEN - WARNING: Not checking if GFX_TEX_HEIGHT <= g_sdlPixLineWidth (but this isn't reproduced as of writing this)
			int pixelsToEgaMemEnd = warpAroundOffset-currLineFirstPixelNum;
			if (g_sdlTexWidth <= pixelsToEgaMemEnd)
			{
				memcpy(currPalPixPtr, (uint8_t *)g_sdlVidMem.egaGfx + currLineFirstPixelNum, g_sdlTexWidth);
			}
			else
			{
				memcpy(currPalPixPtr, (uint8_t *)g_sdlVidMem.egaGfx + currLineFirstPixelNum, pixelsToEgaMemEnd);
				currPalPixPtr += pixelsToEgaMemEnd;
				memcpy(currPalPixPtr, g_sdlVidMem.egaGfx, g_sdlTexWidth - pixelsToEgaMemEnd);
			}

			if (g_sdlSplitScreenLine == line)
			{
				currLineFirstPixelNum = g_sdlPelPanning; // NEXT line begins split screen, NOT g_sdlSplitScreenLine
			}
			else
			{
				currLineFirstPixelNum += g_sdlPixLineWidth;
				currLineFirstPixelNum %= warpAroundOffset;
			}
		}

		bool doUpdate = memcmp(g_sdlHostScrMemCache.egaGfx, g_sdlHostScrMem.egaGfx, sizeof(g_sdlHostScrMem.egaGfx));
		memcpy(g_sdlHostScrMemCache.egaGfx, g_sdlHostScrMem.egaGfx, sizeof(g_sdlHostScrMem.egaGfx));
		if (g_overscanBorderColorIndex != g_overscanBorderColorIndexCache)
		{
			g_overscanBorderColorIndexCache = g_overscanBorderColorIndex;
			g_sdlEGALastBGRABorderColor =
				(g_sdlScreenMode == 0x13) ?
				g_sdlEGACurrBGRAPalette[g_overscanBorderColorIndex] :
				g_sdlEGABGRAScreenColors[g_overscanBorderColorIndex];
			doUpdate = true;
		}

		if (!doUpdate)
		{
			int paletteEntry;
			for (paletteEntry = 0; paletteEntry < 256; ++paletteEntry)
			{
				if (g_sdlEGACurrBGRAPalette[paletteEntry] != g_sdlEGACurrBGRAPaletteCache[paletteEntry])
				{
					g_sdlEGACurrBGRAPaletteCache[paletteEntry] = g_sdlEGACurrBGRAPalette[paletteEntry];
					doUpdate = true;
				}
			}
			if (!doUpdate)
			{
				g_sdlDoRefreshGfxOutput = false;
				if (g_sdlForceGfxControlUiRefresh)
					goto dorefresh;
				return;
			}
		}
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		currPalPixPtr = g_sdlHostScrMem.egaGfx;
		for (int pixnum = 0; pixnum < g_sdlTexWidth*GFX_TEX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
			*currPixPtr = g_sdlEGACurrBGRAPalette[*currPalPixPtr];
	}

	g_sdlDoRefreshGfxOutput = false;
	SDL_UnlockTexture(g_sdlTexture);

dorefresh:

	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_sdlRenderer);
	SDL_SetRenderDrawColor(
		g_sdlRenderer, (g_sdlEGALastBGRABorderColor>>16)&0xFF,
		(g_sdlEGALastBGRABorderColor>>8)&0xFF,
		g_sdlEGALastBGRABorderColor&0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(g_sdlRenderer, &g_sdlAspectCorrectionBorderedRect);
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	if (g_sdlDoAbsMouseMotion && g_sdlControllerMappingActualCurr->absoluteFingerPositioning)
	{
		SDL_SetRenderDrawColor(g_sdlRenderer, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
		SDL_Rect rect = {g_sdlHostVirtualMouseCursorState[0]-g_sdlHostVirtualMouseCursorSideLen/2, g_sdlHostVirtualMouseCursorState[1]-g_sdlHostVirtualMouseCursorSideLen/2, g_sdlHostVirtualMouseCursorSideLen, g_sdlHostVirtualMouseCursorSideLen};
		SDL_RenderFillRect(g_sdlRenderer, &rect);
	}
#endif

	if (g_sdlTargetTexture)
	{
		if (SDL_SetRenderTarget(g_sdlRenderer, g_sdlTargetTexture) != 0)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_UpdateHostDisplay: Failed to set target texture as render target (disabling),\n%s\n", SDL_GetError());
			BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
			g_sdlTargetTexture = NULL;
			goto refreshwithnorendertarget;
		}
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, NULL);
		if (SDL_SetRenderTarget(g_sdlRenderer, NULL) != 0)
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_UpdateHostDisplay: Failed to set default render target!\n%s\n", SDL_GetError());
		SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionRect);
	}
	else
	{
refreshwithnorendertarget:
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionRect);
	}

	BEL_ST_FinishHostDisplayUpdate();
	g_be_sdlLastRefreshTicks = SDL_GetTicks();
}
