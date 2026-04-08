/* Copyright (C) 2014-2026 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "be_cross.h"
#include "be_st_cfg.h"
#include "be_title_and_version.h"
#include "be_video_textures.h"

#include <SDL3/SDL.h>

#ifdef REFKEEN_UNIFIED_BUILD
#include "../rsrc/reflectionhle-icon-32x32.h"
#elif defined REFKEEN_VER_KDREAMS
#include "../rsrc/reflection-kdreams-icon-32x32.h"
#elif defined REFKEEN_VER_CAT3D
#include "../rsrc/reflection-cat3d-icon-32x32.h"
#elif defined REFKEEN_VER_CATABYSS
#include "../rsrc/reflection-catabyss-icon-32x32.h"
#elif defined REFKEEN_VER_CATARM
#include "../rsrc/reflection-catarm-icon-32x32.h"
#elif defined REFKEEN_VER_CATAPOC
#include "../rsrc/reflection-catapoc-icon-32x32.h"
#elif defined REFKEEN_HAS_VER_CATACOMB_ALL
#include "../rsrc/reflection-cat3d-icon-32x32.h"
#elif defined REFKEEN_HAS_VER_WOLF3D_ALL
#include "../rsrc/reflection-wolf3d-icon-32x32.h"
#elif defined REFKEEN_HAS_VER_BMENACE_ALL
#include "../rsrc/reflection-bmenace-icon-32x32.h"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

static SDL_Window *g_sdlWindow;
static SDL_Renderer *g_sdlRenderer;
static SDL_Surface *g_be_sdl_windowIconSurface;

static void BEL_ST_DestroyWindowAndRenderer_WithoutTheIcon(void)
{
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
}

static SDL_DisplayID BEL_ST_GetWindowDisplayId(void);

void BEL_ST_RecreateWindowAndRenderer(
	int windowWidth, int windowHeight,
	int fullWidth, int fullHeight,
	bool fullScreen, bool resizable, bool vsync, const char *driver)
{
	static const char *prev_driver;

	uint32_t windowFlags = resizable ? SDL_WINDOW_RESIZABLE : 0;
	if (fullScreen)
		windowFlags |= SDL_WINDOW_FULLSCREEN;

	if (!g_be_sdl_windowIconSurface)
	{
		g_be_sdl_windowIconSurface =
		  SDL_CreateSurfaceFrom(32, 32,
		                        SDL_GetPixelFormatForMasks(8,0,0,0,0),
		                        RefKeen_Window_Icon, 32);

		SDL_SetPaletteColors(SDL_GetSurfacePalette(g_be_sdl_windowIconSurface), RefKeen_Window_Icon_Palette, '0', 9);
		SDL_SetSurfaceColorKey(g_be_sdl_windowIconSurface, true, SDL_MapSurfaceRGB(g_be_sdl_windowIconSurface, 0xCC, 0xFF, 0xCC));
	}

	if (g_sdlWindow)
	{
		// Reuse launcher's window if we can
		if (driver == prev_driver)
			goto finish;

		BEL_ST_DestroyWindowAndRenderer_WithoutTheIcon();
	}

	// HACK - Create non-fullscreen window and then set as fullscreen, if required.
	// Reason is this lets us set non-fullscreen window size (for fullscreen toggling).
	g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, windowWidth, windowHeight, windowFlags & ~SDL_WINDOW_FULLSCREEN);
	// A hack for Android x86 on VirtualBox - Try creating an OpenGL ES 1.1 context instead of 2.0
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: Failed to create SDL2 window, forcing OpenGL (ES) version to 1.1 and retrying,\n%s\n", SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, windowWidth, windowHeight, windowFlags & ~SDL_WINDOW_FULLSCREEN);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}

	SDL_SetWindowIcon(g_sdlWindow, g_be_sdl_windowIconSurface);
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, driver);
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		exit(0);
	}

	prev_driver = driver;

finish:
	SDL_SetRenderVSync(g_sdlRenderer, vsync ? 1 : 0);

	// In case non-desktop fullscreen resolution is desired (even if window is currently *not* fullscreen);
	// But do so AFTER creating renderer! (Looks like SDL_CreateRenderer may re-create the window.)
	if (fullWidth && fullHeight)
	{
		SDL_DisplayID display = BEL_ST_GetWindowDisplayId();
		if (display)
		{
			const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(display);
			if (!mode)
				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: SDL_GetDesktopDisplayMode failed,\n%s\n", SDL_GetError());
			else
			{
				SDL_DisplayMode newMode = *mode;
				newMode.w = fullWidth;
				newMode.h = fullHeight;
				if (!SDL_SetWindowFullscreenMode(g_sdlWindow, &newMode))
					BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: SDL_SetWindowFullscreenMode failed,\n%s\n", SDL_GetError());
			}
		}
	}
	SDL_SetWindowFullscreen(g_sdlWindow, windowFlags & SDL_WINDOW_FULLSCREEN);
}

void BEL_ST_DestroyWindowAndRenderer(void)
{
	BEL_ST_DestroyWindowAndRenderer_WithoutTheIcon();
	SDL_DestroySurface(g_be_sdl_windowIconSurface);
	g_be_sdl_windowIconSurface = NULL;
}

BE_ST_Texture *BEL_ST_CreateARGBTexture(int w, int h, bool isTarget, bool isLinear)
{
	// TODO: Consider using SDL_TEXTUREACCESS_STATIC later
	SDL_Texture *ret = SDL_CreateTexture(
	      g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
	      isTarget ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STREAMING,
	      w, h);
	if (!ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_CreateTexture failed,\n%s\n", SDL_GetError());
	if (!isLinear)
		if (!SDL_SetTextureScaleMode(ret, SDL_SCALEMODE_NEAREST))
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL_SetTextureScaleMode failed,\n%s\n", SDL_GetError());
	return ret;
}

void BEL_ST_DestroyTexture(BE_ST_Texture *texture)
{
	SDL_DestroyTexture((SDL_Texture *)texture);
}

bool BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst)
{
	bool ret;
	if (dst)
	{
		SDL_FRect sdl_frect = {(float)dst->x, (float)dst->y,
		                       (float)dst->w, (float)dst->h};
		ret = SDL_RenderTexture(g_sdlRenderer, (SDL_Texture *)texture, NULL, &sdl_frect);
	}
	else
		ret = SDL_RenderTexture(g_sdlRenderer, (SDL_Texture *)texture, NULL, NULL);

	if (!ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderTexture failed,\n%s\n", SDL_GetError());
	return ret;
}

void *BEL_ST_LockTexture(BE_ST_Texture *texture)
{
	void *pixels;
	int pitch;
	if (!SDL_LockTexture((SDL_Texture *)texture, NULL, &pixels, &pitch))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_LockTexture failed,\n%s\n", SDL_GetError());
		pixels = NULL;
	}
	return pixels;
}

void BEL_ST_UnlockTexture(BE_ST_Texture *texture)
{
	SDL_UnlockTexture((SDL_Texture *)texture);
}

void BEL_ST_UpdateTexture(BE_ST_Texture *texture, const BE_ST_Rect *rect, const void *pixels, int pitch)
{
	bool ret;
	if (rect)
	{
		SDL_Rect sdl_rect = {rect->x, rect->y, rect->w, rect->h};
		ret = SDL_UpdateTexture((SDL_Texture *)texture, &sdl_rect, pixels, pitch);
	}
	else
		ret = SDL_UpdateTexture((SDL_Texture *)texture, NULL, pixels, pitch);

	if (!ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_UpdateTexture failed,\n%s\n", SDL_GetError());
}

void BEL_ST_SetTextureBlendMode(BE_ST_Texture *texture, bool blend)
{
	if (!SDL_SetTextureBlendMode((SDL_Texture *)texture, blend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetTextureBlendMode failed to set blend mode, blend == %d,\n%s\n", (int)blend, SDL_GetError());
}

bool BEL_ST_SetRenderTarget(BE_ST_Texture *texture)
{
	bool ret = SDL_SetRenderTarget(g_sdlRenderer, (SDL_Texture *)texture);
	if (!ret)
	{
		if (texture)
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderTarget failed to set render target,\n%s\n", SDL_GetError());
		else
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderTarget failed to reset render target,\n%s\n", SDL_GetError());
	}
	return ret;
}

void BEL_ST_RenderClear(void)
{
	if (!SDL_RenderClear(g_sdlRenderer))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderClear failed,\n%s\n", SDL_GetError());
}

void BEL_ST_SetDrawColor(uint32_t color)
{
	if (!SDL_SetRenderDrawColor(
	      g_sdlRenderer, (color>>16)&255, (color>>8)&255, color&255, color>>24))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderDrawColor failed,\n%s\n", SDL_GetError());
}

void BEL_ST_SetDrawBlendMode(bool blend)
{
	if (SDL_SetRenderDrawBlendMode(g_sdlRenderer, blend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderDrawBlendMode failed to set blend mode, blend == %d,\n%s\n", (int)blend, SDL_GetError());
}

void BEL_ST_RenderFill(const BE_ST_Rect *rect)
{
	SDL_FRect sdl_frect = {(float)rect->x, (float)rect->y,
	                       (float)rect->w, (float)rect->h};
	if (!SDL_RenderFillRect(g_sdlRenderer, &sdl_frect))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderFillRect failed,\n%s\n", SDL_GetError());
}

void BEL_ST_SetWindowFullScreenToggle(bool fullScreen)
{
	SDL_SetWindowFullscreen(g_sdlWindow, fullScreen);
}

bool BE_ST_HostGfx_GetFullScreenToggle(void)
{
	return ((SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN);
}

void BEL_ST_UpdateWindow(void)
{
	SDL_RenderPresent(g_sdlRenderer);
}

void BEL_ST_SetWindowSize(int w, int h)
{
	SDL_SetWindowSize(g_sdlWindow, w, h);
}

void BEL_ST_GetWindowSize(int *w, int *h)
{
	SDL_GetWindowSize(g_sdlWindow, w, h);
}

void BEL_ST_GetDesktopDisplayDims(int *w, int *h)
{
	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	if (!display)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_GetDesktopDisplayDims: SDL_GetPrimaryDisplay failed,\n%s\n", SDL_GetError());
	else
	{
		const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(display);
		if (!mode)
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_GetDesktopDisplayDims: SDL_GetDesktopDisplayMode failed,\n%s\n", SDL_GetError());
		else
		{
			*w = mode->w;
			*h = mode->h;
			return;
		}
	}
	*w = 640;
	*h = 480;
}

static SDL_DisplayID BEL_ST_GetWindowDisplayId(void)
{
	SDL_DisplayID ret = SDL_GetDisplayForWindow(g_sdlWindow);
	if (!ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_GetDisplayForWindow failed,\n%s\n", SDL_GetError());
	return ret;
}

bool BEL_ST_IsConfiguredForSWRendering(void)
{
	if (g_refKeenCfg.sdlRendererDriver >= 0)
	{
		const char *name = SDL_GetRenderDriver(g_refKeenCfg.sdlRendererDriver);
		if (name)
			return !strcmp(name, SDL_SOFTWARE_RENDERER);
	}
	return false;
}

void BEL_ST_SetRelativeMouseMode(bool relative)
{
	SDL_SetWindowRelativeMouseMode(g_sdlWindow, relative);
}
