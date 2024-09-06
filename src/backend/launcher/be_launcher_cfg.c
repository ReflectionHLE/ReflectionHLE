/* Copyright (C) 2015-2024 NY00123
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

// Helper macros for menu item declarations related to binds
#define DECLARE_MOUSE_BINDS_MENUITEM(Game, Suffix) \
	g_be ## Game ## MouseBindsMenuItem_Action_ ## Suffix,

#define DECLARE_PAD_BINDS_MENUITEM(Game, Suffix) \
	g_be ## Game ## PadBindsMenuItem_Action_ ## Suffix,

#define DECLARE_NONKEY_BINDS_MENUITEMS(Game, Suffix) \
	DECLARE_MOUSE_BINDS_MENUITEM(Game,Suffix) \
	DECLARE_PAD_BINDS_MENUITEM(Game,Suffix)

#define DECLARE_BINDS_MENUITEMS(Game, Suffix) \
	g_be ## Game ## KeyBindsMenuItem_Action_ ## Suffix, \
	DECLARE_NONKEY_BINDS_MENUITEMS(Game, Suffix)

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
	g_beVideoSettingsMenuItem_ShowEndoom,
	g_beSoundSettingsMenuItem_SndSampleRate,
	g_beSoundSettingsMenuItem_SndSubSystem,
	g_beSoundSettingsMenuItem_OPLEmulation,
	g_beSoundSettingsMenuItem_SB,
	g_beSoundSettingsMenuItem_UseResampler,
	g_beInputSettingsMenuItem_ControllerScheme,
	g_beInputSettingsMenuItem_MouseGrab,
	g_beInputSettingsMenuItem_TouchControls,
	g_beInputSettingsMenuItem_TouchInputDebugging,
	g_beKDreamsSettingsMenuItem_AbsMouseMotion,
	DECLARE_BINDS_MENUITEMS(KDreams, Up)
	DECLARE_BINDS_MENUITEMS(KDreams, Down)
	DECLARE_BINDS_MENUITEMS(KDreams, Left)
	DECLARE_BINDS_MENUITEMS(KDreams, Right)
	DECLARE_BINDS_MENUITEMS(KDreams, Jump)
	DECLARE_BINDS_MENUITEMS(KDreams, Throw)
	DECLARE_BINDS_MENUITEMS(KDreams, Stats)
#ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DECLARE_MOUSE_BINDS_MENUITEM(KDreams, FuncKeys)
#endif
	DECLARE_PAD_BINDS_MENUITEM(KDreams, FuncKeys)
	DECLARE_NONKEY_BINDS_MENUITEMS(KDreams, DebugKeys)
	g_beKDreamsPadBindsMenuItem_LeftStick,
	g_beKDreamsPadBindsMenuItem_RightStick,
	g_beCat3DSettingsMenuItem_VertAnalogMotion,
	DECLARE_BINDS_MENUITEMS(Cat3D, Up)
	DECLARE_BINDS_MENUITEMS(Cat3D, Down)
	DECLARE_BINDS_MENUITEMS(Cat3D, Left)
	DECLARE_BINDS_MENUITEMS(Cat3D, Right)
	DECLARE_BINDS_MENUITEMS(Cat3D, Fire)
	DECLARE_BINDS_MENUITEMS(Cat3D, Strafe)
	DECLARE_BINDS_MENUITEMS(Cat3D, Drink)
	DECLARE_BINDS_MENUITEMS(Cat3D, Bolt)
	DECLARE_BINDS_MENUITEMS(Cat3D, Nuke)
	DECLARE_BINDS_MENUITEMS(Cat3D, FastTurn)
#ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DECLARE_MOUSE_BINDS_MENUITEM(Cat3D, Scrolls)
	DECLARE_MOUSE_BINDS_MENUITEM(Cat3D, FuncKeys)
#endif
	DECLARE_PAD_BINDS_MENUITEM(Cat3D, Scrolls)
	DECLARE_PAD_BINDS_MENUITEM(Cat3D, FuncKeys)
	DECLARE_NONKEY_BINDS_MENUITEMS(Cat3D, DebugKeys)
	g_beCat3DPadBindsMenuItem_LeftStick,
	g_beCat3DPadBindsMenuItem_RightStick,
	g_beCat3DPadBindsMenuItem_AnalogMotion,
	g_beWolf3DSettingsMenuItem_LowFPS,
	g_beWolf3DSettingsMenuItem_VertAnalogMotion,
	DECLARE_BINDS_MENUITEMS(Wolf3D, Up)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Down)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Left)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Right)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Fire)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Strafe)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Use)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Run)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Weapon1)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Weapon2)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Weapon3)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Weapon4)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Weapon5)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Weapon6)
	DECLARE_BINDS_MENUITEMS(Wolf3D, Map)
#ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DECLARE_MOUSE_BINDS_MENUITEM(Wolf3D, FuncKeys)
#endif
	DECLARE_PAD_BINDS_MENUITEM(Wolf3D, FuncKeys)
	DECLARE_NONKEY_BINDS_MENUITEMS(Wolf3D, DebugKeys)
	g_beWolf3DPadBindsMenuItem_LeftStick,
	g_beWolf3DPadBindsMenuItem_RightStick,
	g_beWolf3DPadBindsMenuItem_AnalogMotion,
	g_beDeviceVolumesMenuItem_PCSpkVol,
	g_beDeviceVolumesMenuItem_OPLVol,
	g_beDeviceVolumesMenuItem_DigiVol;

// Helper macros for definitions of pairs related to binds
#define DEFINE_KDREAMS_MOUSE_BIND_PAIR(Suffix, SUFFIX) \
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_ ## SUFFIX].mouse, &g_beKDreamsMouseBindsMenuItem_Action_ ## Suffix}, \

#define DEFINE_KDREAMS_PAD_BIND_PAIR(Suffix, SUFFIX) \
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_ ## SUFFIX].pad, &g_beKDreamsPadBindsMenuItem_Action_ ## Suffix},

#define DEFINE_KDREAMS_NONKEY_BIND_PAIRS(Suffix, SUFFIX) \
	DEFINE_KDREAMS_MOUSE_BIND_PAIR(Suffix,SUFFIX) \
	DEFINE_KDREAMS_PAD_BIND_PAIR(Suffix,SUFFIX)

#define DEFINE_KDREAMS_BIND_PAIRS(Suffix, SUFFIX) \
	{&g_refKeenCfg.kdreams.binds[BE_ST_CTRL_BIND_KDREAMS_ ## SUFFIX].key, &g_beKDreamsKeyBindsMenuItem_Action_ ## Suffix}, \
	DEFINE_KDREAMS_NONKEY_BIND_PAIRS(Suffix, SUFFIX)

#define DEFINE_CAT3D_MOUSE_BIND_PAIR(Suffix, SUFFIX) \
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_ ## SUFFIX].mouse, &g_beCat3DMouseBindsMenuItem_Action_ ## Suffix}, \

#define DEFINE_CAT3D_PAD_BIND_PAIR(Suffix, SUFFIX) \
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_ ## SUFFIX].pad, &g_beCat3DPadBindsMenuItem_Action_ ## Suffix},

#define DEFINE_CAT3D_NONKEY_BIND_PAIRS(Suffix, SUFFIX) \
	DEFINE_CAT3D_MOUSE_BIND_PAIR(Suffix,SUFFIX) \
	DEFINE_CAT3D_PAD_BIND_PAIR(Suffix,SUFFIX)

#define DEFINE_CAT3D_BIND_PAIRS(Suffix, SUFFIX) \
	{&g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_ ## SUFFIX].key, &g_beCat3DKeyBindsMenuItem_Action_ ## Suffix}, \
	DEFINE_CAT3D_NONKEY_BIND_PAIRS(Suffix, SUFFIX)

#define DEFINE_WOLF3D_MOUSE_BIND_PAIR(Suffix, SUFFIX) \
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_ ## SUFFIX].mouse, &g_beWolf3DMouseBindsMenuItem_Action_ ## Suffix}, \

#define DEFINE_WOLF3D_PAD_BIND_PAIR(Suffix, SUFFIX) \
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_ ## SUFFIX].pad, &g_beWolf3DPadBindsMenuItem_Action_ ## Suffix},

#define DEFINE_WOLF3D_NONKEY_BIND_PAIRS(Suffix, SUFFIX) \
	DEFINE_WOLF3D_MOUSE_BIND_PAIR(Suffix,SUFFIX) \
	DEFINE_WOLF3D_PAD_BIND_PAIR(Suffix,SUFFIX)

#define DEFINE_WOLF3D_BIND_PAIRS(Suffix, SUFFIX) \
	{&g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_ ## SUFFIX].key, &g_beWolf3DKeyBindsMenuItem_Action_ ## Suffix}, \
	DEFINE_WOLF3D_NONKEY_BIND_PAIRS(Suffix, SUFFIX)


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
	{&g_refKeenCfg.showEndoom, &g_beVideoSettingsMenuItem_ShowEndoom},

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

	{&g_refKeenCfg.altControlScheme, &g_beInputSettingsMenuItem_ControllerScheme},
	{&g_refKeenCfg.mouseGrab, &g_beInputSettingsMenuItem_MouseGrab},
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	{&g_refKeenCfg.touchInputToggle, &g_beInputSettingsMenuItem_TouchControls},
	{&g_refKeenCfg.touchInputDebugging, &g_beInputSettingsMenuItem_TouchInputDebugging},
#endif

#ifdef REFKEEN_HAS_VER_KDREAMS
	{&g_refKeenCfg.kdreams.absMouseMotion, &g_beKDreamsSettingsMenuItem_AbsMouseMotion},
	DEFINE_KDREAMS_BIND_PAIRS(Up, UP)
	DEFINE_KDREAMS_BIND_PAIRS(Down, DOWN)
	DEFINE_KDREAMS_BIND_PAIRS(Left, LEFT)
	DEFINE_KDREAMS_BIND_PAIRS(Right, RIGHT)
	DEFINE_KDREAMS_BIND_PAIRS(Jump, JUMP)
	DEFINE_KDREAMS_BIND_PAIRS(Throw, THROW)
	DEFINE_KDREAMS_BIND_PAIRS(Stats, STATS)
#ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DEFINE_KDREAMS_MOUSE_BIND_PAIR(FuncKeys, FUNCKEYS)
#endif
	DEFINE_KDREAMS_PAD_BIND_PAIR(FuncKeys, FUNCKEYS)
	DEFINE_KDREAMS_NONKEY_BIND_PAIRS(DebugKeys, DEBUGKEYS)
	{&g_refKeenCfg.kdreams.useLeftStick, &g_beKDreamsPadBindsMenuItem_LeftStick},
	{&g_refKeenCfg.kdreams.useRightStick, &g_beKDreamsPadBindsMenuItem_RightStick},
#endif

#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	{&g_refKeenCfg.cat3d.novert, &g_beCat3DSettingsMenuItem_VertAnalogMotion,
		BEL_Launcher_ReadNoVert, BEL_Launcher_WriteNoVert},
	DEFINE_CAT3D_BIND_PAIRS(Up, UP)
	DEFINE_CAT3D_BIND_PAIRS(Down, DOWN)
	DEFINE_CAT3D_BIND_PAIRS(Left, LEFT)
	DEFINE_CAT3D_BIND_PAIRS(Right, RIGHT)
	DEFINE_CAT3D_BIND_PAIRS(Fire, FIRE)
	DEFINE_CAT3D_BIND_PAIRS(Strafe, STRAFE)
	DEFINE_CAT3D_BIND_PAIRS(Drink, DRINK)
	DEFINE_CAT3D_BIND_PAIRS(Bolt, BOLT)
	DEFINE_CAT3D_BIND_PAIRS(Nuke, NUKE)
	DEFINE_CAT3D_BIND_PAIRS(FastTurn, FASTTURN)
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
  #ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DEFINE_CAT3D_MOUSE_BIND_PAIR(Scrolls, SCROLLS)
  #endif
	DEFINE_CAT3D_PAD_BIND_PAIR(Scrolls, SCROLLS)
#endif
#ifdef REFKEEN_HAS_VER_CATADVENTURES
  #ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DEFINE_CAT3D_MOUSE_BIND_PAIR(FuncKeys, FUNCKEYS)
  #endif
	DEFINE_CAT3D_PAD_BIND_PAIR(FuncKeys, FUNCKEYS)
#endif
	DEFINE_CAT3D_NONKEY_BIND_PAIRS(DebugKeys, DEBUGKEYS)
	{&g_refKeenCfg.cat3d.useLeftStick, &g_beCat3DPadBindsMenuItem_LeftStick},
	{&g_refKeenCfg.cat3d.useRightStick, &g_beCat3DPadBindsMenuItem_RightStick},
	{&g_refKeenCfg.cat3d.analogMotion, &g_beCat3DPadBindsMenuItem_AnalogMotion},
#endif

#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	{&g_refKeenCfg.wolf3d.lowFPS, &g_beWolf3DSettingsMenuItem_LowFPS},
	{&g_refKeenCfg.wolf3d.novert, &g_beWolf3DSettingsMenuItem_VertAnalogMotion,
		BEL_Launcher_ReadNoVert, BEL_Launcher_WriteNoVert},
	DEFINE_WOLF3D_BIND_PAIRS(Up, UP)
	DEFINE_WOLF3D_BIND_PAIRS(Down, DOWN)
	DEFINE_WOLF3D_BIND_PAIRS(Left, LEFT)
	DEFINE_WOLF3D_BIND_PAIRS(Right, RIGHT)
	DEFINE_WOLF3D_BIND_PAIRS(Fire, FIRE)
	DEFINE_WOLF3D_BIND_PAIRS(Strafe, STRAFE)
	DEFINE_WOLF3D_BIND_PAIRS(Use, USE)
	DEFINE_WOLF3D_BIND_PAIRS(Run, RUN)
	DEFINE_WOLF3D_BIND_PAIRS(Weapon1, WEAPON1)
	DEFINE_WOLF3D_BIND_PAIRS(Weapon2, WEAPON2)
	DEFINE_WOLF3D_BIND_PAIRS(Weapon3, WEAPON3)
	DEFINE_WOLF3D_BIND_PAIRS(Weapon4, WEAPON4)
	DEFINE_WOLF3D_BIND_PAIRS(Weapon5, WEAPON5)
	DEFINE_WOLF3D_BIND_PAIRS(Weapon6, WEAPON6)
	DEFINE_WOLF3D_BIND_PAIRS(Map, MAP)
#ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	DEFINE_WOLF3D_MOUSE_BIND_PAIR(FuncKeys, FUNCKEYS)
#endif
	DEFINE_WOLF3D_PAD_BIND_PAIR(FuncKeys, FUNCKEYS)
	DEFINE_WOLF3D_NONKEY_BIND_PAIRS(DebugKeys, DEBUGKEYS)
	{&g_refKeenCfg.wolf3d.useLeftStick, &g_beWolf3DPadBindsMenuItem_LeftStick},
	{&g_refKeenCfg.wolf3d.useRightStick, &g_beWolf3DPadBindsMenuItem_RightStick},
	{&g_refKeenCfg.wolf3d.analogMotion, &g_beWolf3DPadBindsMenuItem_AnalogMotion},
#endif

	{&g_refKeenCfg.pcSpkVol, &g_beDeviceVolumesMenuItem_PCSpkVol,
		BEL_Launcher_ReadSoundVolume, BEL_Launcher_WriteSoundVolume},
	{&g_refKeenCfg.oplVol, &g_beDeviceVolumesMenuItem_OPLVol,
		BEL_Launcher_ReadSoundVolume, BEL_Launcher_WriteSoundVolume},
#ifdef BE_ST_ENABLE_SETTING_DIGIVOL
	{&g_refKeenCfg.digiVol, &g_beDeviceVolumesMenuItem_DigiVol,
		BEL_Launcher_ReadSoundVolume, BEL_Launcher_WriteSoundVolume},
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
