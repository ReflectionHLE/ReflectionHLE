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

#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h" // Enable VSync by default for EGA/VGA, not CGA
#include "be_st.h"
#include "be_st_launcher.h"
#include "be_st_sdl_private.h"
//#include "be_st_egavga_lookup_tables.h"
#include "be_title_and_version.h"
#include "backend/video/be_video.h"
#include "backend/video/be_video_emu.h"
#include "backend/video/be_video_ui.h"

// Some of these are also used in launcher
SDL_Window *g_sdlWindow;
SDL_Renderer *g_sdlRenderer;
SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
SDL_Rect g_sdlAspectCorrectionRect, g_sdlAspectCorrectionBorderedRect;

int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;

static bool g_sdlIsSoftwareRendered;
bool g_sdlDoRefreshGfxOutput;
bool g_sdlForceGfxControlUiRefresh;

void BE_ST_MarkGfxForUpdate(void)
{
	g_sdlDoRefreshGfxOutput = true;
}

#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

int g_sdlDebugFingerRectSideLen;

extern const uint8_t g_vga_8x16TextFont[256*8*16];

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

	uint32_t rendererFlags = BEL_ST_GetSDLRendererFlagsToSet(false);
	BEL_ST_RecreateSDLWindowAndRenderer(
		SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum),
		windowWidthToSet, windowHeightToSet, g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight, windowFlagsToSet, g_refKeenCfg.sdlRendererDriver, rendererFlags
	);

	BE_ST_SetScreenMode(3); // Includes SDL_Texture handling and output rects preparation

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	g_sdlDoAbsMouseMotion = g_refKeenCfg.absMouseMotion;
#endif
}

void BE_ST_ShutdownGfx(void)
{
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlFaceButtonsTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlDpadTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTextInputTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlDebugKeysTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
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

uint32_t BEL_ST_GetSDLRendererFlagsToSet(bool isLauncherWindow)
{
	// Vanilla Keen Dreams and Keen 4-6 have no VSync in the CGA builds
	uint32_t flags = SDL_RENDERER_ACCELERATED;
#ifdef BE_ST_ADAPT_VSYNC_DEFAULT_TO_KDREAMS
	if ((g_refKeenCfg.vSync == VSYNC_ON) ||
	    ((g_refKeenCfg.vSync == VSYNC_AUTO) &&
	     (isLauncherWindow || (refkeen_current_gamever != BE_GAMEVER_KDREAMSC105))))
		flags |= SDL_RENDERER_PRESENTVSYNC;
#else
	if ((g_refKeenCfg.vSync == VSYNC_ON) || (g_refKeenCfg.vSync == VSYNC_AUTO))
		flags |= SDL_RENDERER_PRESENTVSYNC;
#endif
	return flags;
}

void BEL_ST_RecreateTexture(void)
{
	if (g_sdlTexture)
		BEL_ST_SDLDestroyTextureWrapper(&g_sdlTexture);
	if (g_sdlTargetTexture)
		BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
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
	*pTexture = NULL;
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


// Colors in BGRA format/order for CGA mode 4 (on certain platforms)
#ifdef __cplusplus
extern
#endif
const uint32_t g_sdlCGAGfxBGRAScreenColors[] = {
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
#ifdef __cplusplus
extern
#endif
const uint32_t g_sdlVGADefaultBGRAScreenColors[] = {
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

extern const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr;

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

	if (g_sdlIsSoftwareRendered && !g_refKeenCfg.forceFullSoftScaling)
	{
		// Special case - We may resize window based on mode, but only if
		// allowResize == true (to prevent any possible infinite resizes loop)
		if (allowResize && (!(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN)))
		{
			if ((g_refKeenCfg.scaleFactor*srcBorderedWidth != winWidth) || (g_refKeenCfg.scaleFactor*srcBorderedHeight != winHeight))
			{
				SDL_SetWindowSize(g_sdlWindow, g_refKeenCfg.scaleFactor*srcBorderedWidth, g_refKeenCfg.scaleFactor*srcBorderedHeight);
				// Not sure this will help, but still trying...
				SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
			}
		}

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
		// Turn off the border in such a case
		srcBorderLeft = srcBorderRight = srcBorderTop = srcBorderBottom = 0;
		srcBorderedWidth = srcWidth;
		srcBorderedHeight = srcHeight;

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
		uint64_t scaledSrcBorderedWidth = (4*srcHeight)*srcBorderedWidth, scaledSrcBorderedHeight = (3*srcWidth)*srcBorderedHeight;
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

	// Save modified window size
	if (!(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		g_refKeenCfg.winWidth = winWidth;
		g_refKeenCfg.winHeight = winHeight;
	}

	// Finish with internal (non-bordered) rect
	g_sdlAspectCorrectionRect.x = g_sdlAspectCorrectionBorderedRect.x + g_sdlAspectCorrectionBorderedRect.w*srcBorderLeft/srcBorderedWidth;
	g_sdlAspectCorrectionRect.y = g_sdlAspectCorrectionBorderedRect.y + g_sdlAspectCorrectionBorderedRect.h*srcBorderTop/srcBorderedHeight;
	g_sdlAspectCorrectionRect.w = g_sdlAspectCorrectionBorderedRect.w*srcWidth/srcBorderedWidth;
	g_sdlAspectCorrectionRect.h = g_sdlAspectCorrectionBorderedRect.h*srcHeight/srcBorderedHeight;

	int minWinDim = (winWidth >= winHeight) ? winHeight : winWidth;

	BEL_ST_SetCommonUIRects();
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
	BEL_ST_SetGfxOutputRects(false);
	BEL_ST_ConditionallyShowAltInputPointer();
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
	uint32_t currRefreshTicks = BEL_ST_GetTicksMS();
	if (currRefreshTicks - g_be_sdlLastRefreshTicks >= 100)
		g_sdlForceGfxControlUiRefresh = true;

	if (g_sdlScreenMode == 3) // Text mode
	{
		// For graphics modes we don't have to refresh if g_sdlDoRefreshGfxOutput is set to false,
		// but in textual mode we have blinking characters and cursor to take care of
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		bool areBlinkingCharsShown = (((uint64_t)(70086*BEL_ST_GetTicksMS()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = g_sdlTxtCursorEnabled && (((uint64_t)(70086*BEL_ST_GetTicksMS()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
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
	g_be_sdlLastRefreshTicks = BEL_ST_GetTicksMS();
}
