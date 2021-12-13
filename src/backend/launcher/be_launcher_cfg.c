/* Copyright (C) 2015-2021 NY00123
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
#include "be_launcher.h"
#include "be_launcher_cfg.h"
#include "../backend/audio/be_audio_private.h"

#ifdef REFKEEN_ENABLE_LAUNCHER

// Implementation-defined handlers
void BEL_Launcher_ReadDisplayNum(BESettingMenuItemPair *p);
void BEL_Launcher_ReadRendererDriver(BESettingMenuItemPair *p);
void BEL_Launcher_WriteRendererDriver(BESettingMenuItemPair *p);

static void BEL_Launcher_ReadScaleFactor(BESettingMenuItemPair *p)
{
	p->menuItem->choice = *p->setting - 1;
}

static void BEL_Launcher_WriteScaleFactor(BESettingMenuItemPair *p)
{
	*p->setting = p->menuItem->choice + 1;
}

static void BEL_Launcher_ReadSoundVolume(BESettingMenuItemPair *p)
{
	p->menuItem->choice = *p->setting - BE_AUDIO_VOL_MIN;
}

static void BEL_Launcher_WriteSoundVolume(BESettingMenuItemPair *p)
{
	*p->setting = p->menuItem->choice + BE_AUDIO_VOL_MIN;
}

static void BEL_Launcher_ReadSampleRate(BESettingMenuItemPair *p)
{
	unsigned i, opl_rate_index = 0;
	for (i = 0; i < BE_Cross_ArrayLen(g_be_soundsSettingsChoices_sndSampleRateVals); ++i)
	{
		if (g_be_soundsSettingsChoices_sndSampleRateVals[i] >= g_refKeenCfg.sndSampleRate)
		{
			p->menuItem->choice = i;
			return;
		}
		if (g_be_soundsSettingsChoices_sndSampleRateVals[i] == OPL_SAMPLE_RATE)
			opl_rate_index = i;
	}
	p->menuItem->choice = opl_rate_index;
}

static void BEL_Launcher_WriteSampleRate(BESettingMenuItemPair *p)
{
	*p->setting = g_be_soundsSettingsChoices_sndSampleRateVals[p->menuItem->choice];
}

static void BEL_Launcher_ReadNoVert(BESettingMenuItemPair *p)
{
	p->menuItem->choice = !(*p->setting);
}

static void BEL_Launcher_WriteNoVert(BESettingMenuItemPair *p)
{
	*p->setting = !(p->menuItem->choice);
}

extern BEMenuItem
	g_beVideoSettingsMenuItem_Fullscreen,
	g_beVideoSettingsMenuItem_DisplayNum,
	g_beVideoSettingsMenuItem_RememberDisplayNum,
	g_beVideoSettingsMenuItem_SDLRenderer,
	g_beVideoSettingsMenuItem_Bilinear,
	g_beVideoSettingsMenuItem_ScaleType,
	g_beVideoSettingsMenuItem_ScaleFactor,
	g_beVideoSettingsMenuItem_VSync,
	g_beVideoSettingsMenuItem_ForceFullSoftScaling,
	g_beVideoSettingsMenuItem_LauncherWindowType,
	g_beSoundSettingsMenuItem_SndSampleRate,
	g_beSoundSettingsMenuItem_SndSubSystem,
	g_beSoundSettingsMenuItem_OPLEmulation,
	g_beSoundSettingsMenuItem_SB,
	g_beSoundSettingsMenuItem_UseResampler,
	g_beInputSettingsMenuItem_ControllerScheme,
	g_beInputSettingsMenuItem_MouseGrab,
	g_beInputSettingsMenuItem_AbsMouseMotion,
	g_beInputSettingsMenuItem_VertAnalogMotion,
	g_beInputSettingsMenuItem_TouchControls,
	g_beInputSettingsMenuItem_TouchInputDebugging,
	g_beControllerSettingsMenuItem_LeftStick,
	g_beControllerSettingsMenuItem_RightStick,
	g_beControllerSettingsMenuItem_AnalogMotion,
	g_beControllerSettingsMenuItem_Action_KDreams_Up,
	g_beControllerSettingsMenuItem_Action_KDreams_Down,
	g_beControllerSettingsMenuItem_Action_KDreams_Left,
	g_beControllerSettingsMenuItem_Action_KDreams_Right,
	g_beControllerSettingsMenuItem_Action_KDreams_Jump,
	g_beControllerSettingsMenuItem_Action_KDreams_Throw,
	g_beControllerSettingsMenuItem_Action_KDreams_Stats,
	g_beControllerSettingsMenuItem_Action_KDreams_FuncKeys,
	g_beControllerSettingsMenuItem_Action_KDreams_DebugKeys,
	g_beControllerSettingsMenuItem_Action_Cat3D_Up,
	g_beControllerSettingsMenuItem_Action_Cat3D_Down,
	g_beControllerSettingsMenuItem_Action_Cat3D_Left,
	g_beControllerSettingsMenuItem_Action_Cat3D_Right,
	g_beControllerSettingsMenuItem_Action_Cat3D_Fire,
	g_beControllerSettingsMenuItem_Action_Cat3D_Strafe,
	g_beControllerSettingsMenuItem_Action_Cat3D_Drink,
	g_beControllerSettingsMenuItem_Action_Cat3D_Bolt,
	g_beControllerSettingsMenuItem_Action_Cat3D_Nuke,
	g_beControllerSettingsMenuItem_Action_Cat3D_FastTurn,
	g_beControllerSettingsMenuItem_Action_Cat3D_Scrolls,
	g_beControllerSettingsMenuItem_Action_Cat3D_FuncKeys,
	g_beControllerSettingsMenuItem_Action_Cat3D_DebugKeys,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Up,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Down,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Left,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Right,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Fire,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Strafe,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Use,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Run,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon1,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon2,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon3,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon4,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon5,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon6,
	g_beControllerSettingsMenuItem_Action_Wolf3D_Map,
	g_beControllerSettingsMenuItem_Action_Wolf3D_FuncKeys,
	g_beControllerSettingsMenuItem_Action_Wolf3D_DebugKeys,
	g_beDeviceVolumesMenuItem_PCSpkVol,
	g_beDeviceVolumesMenuItem_OPLVol,
	g_beDeviceVolumesMenuItem_DigiVol,
	g_beMiscSettingsMenuItem_LowFPS;

static BESettingMenuItemPair g_be_setting_pairs[] = {
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	{&g_refKeenCfg.isFullscreen, &g_beVideoSettingsMenuItem_Fullscreen},
#endif
	{&g_refKeenCfg.displayNum, &g_beVideoSettingsMenuItem_DisplayNum, BEL_Launcher_ReadDisplayNum},
	{&g_refKeenCfg.rememberDisplayNum, &g_beVideoSettingsMenuItem_RememberDisplayNum},

	{&g_refKeenCfg.sdlRendererDriver, &g_beVideoSettingsMenuItem_SDLRenderer,
		BEL_Launcher_ReadRendererDriver, BEL_Launcher_WriteRendererDriver},
	{&g_refKeenCfg.isBilinear, &g_beVideoSettingsMenuItem_Bilinear},
	{&g_refKeenCfg.scaleType, &g_beVideoSettingsMenuItem_ScaleType},
	{&g_refKeenCfg.scaleFactor, &g_beVideoSettingsMenuItem_ScaleFactor,
		BEL_Launcher_ReadScaleFactor, BEL_Launcher_WriteScaleFactor},
	{&g_refKeenCfg.vSync, &g_beVideoSettingsMenuItem_VSync},
	{&g_refKeenCfg.forceFullSoftScaling, &g_beVideoSettingsMenuItem_ForceFullSoftScaling},
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
	{&g_refKeenCfg.launcherWinType, &g_beVideoSettingsMenuItem_LauncherWindowType},
#endif

	{&g_refKeenCfg.sndSampleRate, &g_beSoundSettingsMenuItem_SndSampleRate,
		BEL_Launcher_ReadSampleRate, BEL_Launcher_WriteSampleRate},
	{&g_refKeenCfg.sndSubSystem, &g_beSoundSettingsMenuItem_SndSubSystem},
	{&g_refKeenCfg.oplEmulation, &g_beSoundSettingsMenuItem_OPLEmulation},
#ifdef BE_ST_ENABLE_SETTING_SB
	{&g_refKeenCfg.sb, &g_beSoundSettingsMenuItem_SB},
#endif
#ifndef REFKEEN_RESAMPLER_NONE
	{&g_refKeenCfg.useResampler, &g_beSoundSettingsMenuItem_UseResampler},
#endif

	{&g_refKeenCfg.altControlScheme.isEnabled, &g_beInputSettingsMenuItem_ControllerScheme},
	{&g_refKeenCfg.mouseGrab, &g_beInputSettingsMenuItem_MouseGrab},
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	{&g_refKeenCfg.absMouseMotion, &g_beInputSettingsMenuItem_AbsMouseMotion},
#endif
#ifdef BE_ST_ENABLE_SETTING_NOVERT
	{&g_refKeenCfg.novert, &g_beInputSettingsMenuItem_VertAnalogMotion,
		BEL_Launcher_ReadNoVert, BEL_Launcher_WriteNoVert},
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	{&g_refKeenCfg.touchInputToggle, &g_beInputSettingsMenuItem_TouchControls},
	{&g_refKeenCfg.touchInputDebugging, &g_beInputSettingsMenuItem_TouchInputDebugging},
#endif

	{&g_refKeenCfg.altControlScheme.useLeftStick, &g_beControllerSettingsMenuItem_LeftStick},
	{&g_refKeenCfg.altControlScheme.useRightStick, &g_beControllerSettingsMenuItem_RightStick},
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	{&g_refKeenCfg.altControlScheme.analogMotion, &g_beControllerSettingsMenuItem_AnalogMotion},
#endif

#ifdef REFKEEN_HAS_VER_KDREAMS
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_UP], &g_beControllerSettingsMenuItem_Action_KDreams_Up},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_DOWN], &g_beControllerSettingsMenuItem_Action_KDreams_Down},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_LEFT], &g_beControllerSettingsMenuItem_Action_KDreams_Left},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_RIGHT], &g_beControllerSettingsMenuItem_Action_KDreams_Right},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_JUMP], &g_beControllerSettingsMenuItem_Action_KDreams_Jump},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_THROW], &g_beControllerSettingsMenuItem_Action_KDreams_Throw},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_STATS], &g_beControllerSettingsMenuItem_Action_KDreams_Stats},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_FUNCKEYS], &g_beControllerSettingsMenuItem_Action_KDreams_FuncKeys},
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_DEBUGKEYS], &g_beControllerSettingsMenuItem_Action_KDreams_DebugKeys},
#endif

#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_UP], &g_beControllerSettingsMenuItem_Action_Cat3D_Up},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_DOWN], &g_beControllerSettingsMenuItem_Action_Cat3D_Down},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_LEFT], &g_beControllerSettingsMenuItem_Action_Cat3D_Left},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_RIGHT], &g_beControllerSettingsMenuItem_Action_Cat3D_Right},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FIRE], &g_beControllerSettingsMenuItem_Action_Cat3D_Fire},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_STRAFE], &g_beControllerSettingsMenuItem_Action_Cat3D_Strafe},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_DRINK], &g_beControllerSettingsMenuItem_Action_Cat3D_Drink},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_BOLT], &g_beControllerSettingsMenuItem_Action_Cat3D_Bolt},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_NUKE], &g_beControllerSettingsMenuItem_Action_Cat3D_Nuke},
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FASTTURN], &g_beControllerSettingsMenuItem_Action_Cat3D_FastTurn},
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_SCROLLS], &g_beControllerSettingsMenuItem_Action_Cat3D_Scrolls},
#endif
#ifdef REFKEEN_HAS_VER_CATADVENTURES
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FUNCKEYS], &g_beControllerSettingsMenuItem_Action_Cat3D_FuncKeys},
#endif
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_DEBUGKEYS], &g_beControllerSettingsMenuItem_Action_Cat3D_DebugKeys},
#endif

#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_UP], &g_beControllerSettingsMenuItem_Action_Wolf3D_Up},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_DOWN], &g_beControllerSettingsMenuItem_Action_Wolf3D_Down},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_LEFT], &g_beControllerSettingsMenuItem_Action_Wolf3D_Left},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_RIGHT], &g_beControllerSettingsMenuItem_Action_Wolf3D_Right},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_FIRE], &g_beControllerSettingsMenuItem_Action_Wolf3D_Fire},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_STRAFE], &g_beControllerSettingsMenuItem_Action_Wolf3D_Strafe},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_USE], &g_beControllerSettingsMenuItem_Action_Wolf3D_Use},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_RUN], &g_beControllerSettingsMenuItem_Action_Wolf3D_Run},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_WEAPON1], &g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon1},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_WEAPON2], &g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon2},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_WEAPON3], &g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon3},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_WEAPON4], &g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon4},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_WEAPON5], &g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon5},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_WEAPON6], &g_beControllerSettingsMenuItem_Action_Wolf3D_Weapon6},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_MAP], &g_beControllerSettingsMenuItem_Action_Wolf3D_Map},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_FUNCKEYS], &g_beControllerSettingsMenuItem_Action_Wolf3D_FuncKeys},
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_DEBUGKEYS], &g_beControllerSettingsMenuItem_Action_Wolf3D_DebugKeys},
#endif

	{&g_refKeenCfg.pcSpkVol, &g_beDeviceVolumesMenuItem_PCSpkVol,
		BEL_Launcher_ReadSoundVolume, BEL_Launcher_WriteSoundVolume},
	{&g_refKeenCfg.oplVol, &g_beDeviceVolumesMenuItem_OPLVol,
		BEL_Launcher_ReadSoundVolume, BEL_Launcher_WriteSoundVolume},
#ifdef BE_ST_ENABLE_SETTING_DIGIVOL
	{&g_refKeenCfg.digiVol, &g_beDeviceVolumesMenuItem_DigiVol,
		BEL_Launcher_ReadSoundVolume, BEL_Launcher_WriteSoundVolume},
#endif

#ifdef BE_ST_ENABLE_SETTING_LOWFPS
	{&g_refKeenCfg.lowFPS, &g_beMiscSettingsMenuItem_LowFPS},
#endif
};

void BE_Launcher_ReadSettings(void)
{
	for (unsigned i = 0; i < BE_Cross_ArrayLen(g_be_setting_pairs); ++i)
		if (g_be_setting_pairs[i].readSetting)
			g_be_setting_pairs[i].readSetting(&g_be_setting_pairs[i]);
		else
			g_be_setting_pairs[i].menuItem->choice = *g_be_setting_pairs[i].setting;
}

void BE_Launcher_UpdateSettings(void)
{
	for (unsigned i = 0; i < BE_Cross_ArrayLen(g_be_setting_pairs); ++i)
		if (g_be_setting_pairs[i].writeSetting)
			g_be_setting_pairs[i].writeSetting(&g_be_setting_pairs[i]);
		else
			*g_be_setting_pairs[i].setting = g_be_setting_pairs[i].menuItem->choice;
}

#endif // REFKEEN_ENABLE_LAUNCHER
