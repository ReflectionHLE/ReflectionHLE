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

void BEL_ST_DestroyTexture(BE_ST_Texture *texture) { SDL_DestroyTexture(texture); }

int BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst)
{
	int ret;
	if (dst)
	{
		SDL_Rect sdl_rect = {dst->x, dst->y, dst->w, dst->h};
		ret = SDL_RenderCopy(g_sdlRenderer, texture, NULL, &sdl_rect);
	}
	else
		ret = SDL_RenderCopy(g_sdlRenderer, texture, NULL, NULL);

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

void BEL_ST_UnlockTexture(BE_ST_Texture *texture) { SDL_UnlockTexture((SDL_Texture *)texture); }

void BEL_ST_UpdateTexture(BE_ST_Texture *texture, const BE_ST_Rect *rect, const void *pixels, int pitch)
{
	int ret;
	if (rect)
	{
		SDL_Rect sdl_rect = {rect->x, rect->y, rect->w, rect->h};
		ret = SDL_UpdateTexture(texture, &sdl_rect, pixels, pitch);
	}
	else
		ret = SDL_UpdateTexture(texture, NULL, pixels, pitch);

	if (ret)
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "SDL_UpdateTexture failed,\n%s\n", SDL_GetError());
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
