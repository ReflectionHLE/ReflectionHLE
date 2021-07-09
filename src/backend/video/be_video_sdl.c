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
#include "be_title_and_version.h"
#include "be_video_textures.h"

#include "SDL.h"

// TODO: Make these static later
SDL_Window *g_sdlWindow;
SDL_Renderer *g_sdlRenderer;
extern SDL_Surface *g_be_sdl_windowIconSurface;

void BEL_ST_RecreateWindowAndRenderer(
	int displayNo,
	int windowWidth, int windowHeight,
	int fullWidth, int fullHeight,
	Uint32 windowFlags, int driverIndex, Uint32 rendererFlags)
{
	static int prev_x, prev_y, prev_driverIndex;
	static uint32_t prev_rendererFlags;

	int x = SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNo);
	int y = SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNo);

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
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: Failed to create SDL2 window, forcing OpenGL (ES) version to 1.1 and retrying,\n%s\n", SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, x, y, windowWidth, windowHeight, windowFlags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}

	SDL_SetWindowIcon(g_sdlWindow, g_be_sdl_windowIconSurface);
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, driverIndex, rendererFlags);
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateWindowAndRenderer: Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
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

BE_ST_Texture *BEL_ST_CreateARGBTexture(int w, int h, bool isTarget, bool isLinear)
{
	// TODO: Consider using SDL_TEXTUREACCESS_STATIC later
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, isLinear ? "linear" : "nearest");
	return SDL_CreateTexture(
	         g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
	         isTarget ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STREAMING,
	         w, h);
}

void BEL_ST_DestroyTexture(BE_ST_Texture *texture)
{
	SDL_DestroyTexture((SDL_Texture *)texture);
}

int BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst)
{
	int ret;
	if (dst)
	{
		SDL_Rect sdl_rect = {dst->x, dst->y, dst->w, dst->h};
		ret = SDL_RenderCopy(g_sdlRenderer, (SDL_Texture *)texture, NULL, &sdl_rect);
	}
	else
		ret = SDL_RenderCopy(g_sdlRenderer, (SDL_Texture *)texture, NULL, NULL);

	if (ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderCopy failed,\n%s\n", SDL_GetError());
	return ret;
}

void *BEL_ST_LockTexture(BE_ST_Texture *texture)
{
	void *pixels;
	int pitch;
	if (SDL_LockTexture((SDL_Texture *)texture, NULL, &pixels, &pitch))
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
	int ret;
	if (rect)
	{
		SDL_Rect sdl_rect = {rect->x, rect->y, rect->w, rect->h};
		ret = SDL_UpdateTexture((SDL_Texture *)texture, &sdl_rect, pixels, pitch);
	}
	else
		ret = SDL_UpdateTexture((SDL_Texture *)texture, NULL, pixels, pitch);

	if (ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_UpdateTexture failed,\n%s\n", SDL_GetError());
}

int BEL_ST_SetRenderTarget(BE_ST_Texture *texture)
{
	int ret = SDL_SetRenderTarget(g_sdlRenderer, (SDL_Texture *)texture);
	if (ret)
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
	if (SDL_RenderClear(g_sdlRenderer))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderClear failed,\n%s\n", SDL_GetError());
}

void BEL_ST_SetDrawColor(uint32_t color)
{
	if (SDL_SetRenderDrawColor(
	      g_sdlRenderer, (color>>16)&255, (color>>8)&255, color&255, color>>24))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderDrawColor failed,\n%s\n", SDL_GetError());
}

void BEL_ST_RenderFill(const BE_ST_Rect *rect)
{
	SDL_Rect sdl_rect = {rect->x, rect->y, rect->w, rect->h};
	if (SDL_RenderFillRect(g_sdlRenderer, &sdl_rect))
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderFillRect failed,\n%s\n", SDL_GetError());
}

void BEL_ST_UpdateWindow(void)
{
	SDL_RenderPresent(g_sdlRenderer);
}
