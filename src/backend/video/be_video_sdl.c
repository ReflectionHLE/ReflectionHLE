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
#include "be_video_textures.h"

// TODO: Make these static later
SDL_Window *g_sdlWindow;
SDL_Renderer *g_sdlRenderer;

BE_ST_Texture *BEL_ST_CreateARGBTexture(int w, int h, bool isTarget, bool isLinear)
{
	// TODO: Consider using SDL_TEXTUREACCESS_STATIC later
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, isLinear ? "linear" : "nearest");
	return SDL_CreateTexture(
	         g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
	         isTarget ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STREAMING,
	         w, h);
}

int BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst)
{
	int ret = SDL_RenderCopy(g_sdlRenderer, texture, NULL, dst);
	if (ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_RenderCopy failed,\n%s\n", SDL_GetError());
	return ret;
}

void *BEL_ST_LockTexture(BE_ST_Texture *texture)
{
	void *pixels;
	int pitch;
	if (SDL_LockTexture(texture, NULL, &pixels, &pitch))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_LockTexture failed,\n%s\n", SDL_GetError());
		pixels = NULL;
	}
	return pixels;
}

int BEL_ST_SetRenderTarget(BE_ST_Texture *texture)
{
	int ret = SDL_SetRenderTarget(g_sdlRenderer, texture);
	if (ret)
	{
		if (texture)
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderTarget failed to set render target,\n%s\n", SDL_GetError());
		else
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_SetRenderTarget failed to reset render target,\n%s\n", SDL_GetError());
	}
	return ret;
}
