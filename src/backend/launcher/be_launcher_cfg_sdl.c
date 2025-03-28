/* Copyright (C) 2015-2025 NY00123
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
#include "be_features.h"
#include "be_launcher_cfg.h"
#include "be_launcher_cfg_sdl.h"
#include "SDL.h"

void BEL_Launcher_ReadDisplayNum(BESettingMenuItemPair *p)
{
	int nOfDisplays = SDL_GetNumVideoDisplays();
	if (nOfDisplays >= (int)BE_Cross_ArrayLen(g_be_videoSettingsChoices_displayNums))
		nOfDisplays = BE_Cross_ArrayLen(g_be_videoSettingsChoices_displayNums) - 1;
	// Trim list of display numbers in the launcher
	g_be_videoSettingsChoices_displayNums[nOfDisplays] = NULL;
	// Set choice
	p->menuItem->choice = BE_Cross_TypedClamp(int, *p->setting, 0, nOfDisplays - 1);
}

void BEL_Launcher_ReadRendererDriver(BESettingMenuItemPair *p)
{
	// Prepare list of renderer driversi and the launcher and then set choice
	int nOfSDLRendererDrivers = SDL_GetNumRenderDrivers();
	if (nOfSDLRendererDrivers > BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS)
		nOfSDLRendererDrivers = BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS;
	p->menuItem->choices[0] = "auto";
	p->menuItem->choices[nOfSDLRendererDrivers+1] = NULL;
	for (int i = 0; i < nOfSDLRendererDrivers; ++i)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		snprintf(g_be_videoSettingsChoices_sdlRendererDriversStrs[i], sizeof(g_be_videoSettingsChoices_sdlRendererDriversStrs[i]), "%s", info.name);
		p->menuItem->choices[i+1] = g_be_videoSettingsChoices_sdlRendererDriversStrs[i];
	}

	p->menuItem->choice = BE_Cross_TypedClamp(int, *p->setting + 1, 0, nOfSDLRendererDrivers);
}

void BEL_Launcher_WriteRendererDriver(BESettingMenuItemPair *p)
{
	*p->setting = p->menuItem->choice - 1;
}
