/* Copyright (C) 2014-2021 NY00123
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

#include "../audio/be_audio_private.h"
#include "../video/be_video.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h"
#include "be_st.h"

#define BE_ST_DEFAULT_FARPTRSEGOFFSET 0x14

// The index is taken off the button mappings enum, so ENSURE THESE ARE CONSISTENT!
//
// HACK: If this is updated, also check g_sdlCfgEntries!!!
static const char *g_sdlControlSchemeKeyMapCfgKeyPrefixes[] = {
	"altcontrolscheme_up=",
	"altcontrolscheme_down=",
	"altcontrolscheme_left=",
	"altcontrolscheme_right=",
#ifdef REFKEEN_HAS_VER_KDREAMS
	"altcontrolscheme_jump=",
	"altcontrolscheme_throw=",
	"altcontrolscheme_stats=",
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	"altcontrolscheme_fire=",
	"altcontrolscheme_strafe=",
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	"altcontrolscheme_drink=",
	"altcontrolscheme_bolt=", // Zapper in the Adventures Series
	"altcontrolscheme_nuke=", // Xterminator in the Adventures Series
	"altcontrolscheme_fastturn=",
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	"altcontrolscheme_scrolls=",
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	"altcontrolscheme_use=",
	"altcontrolscheme_run=",
	"altcontrolscheme_weapon1=",
	"altcontrolscheme_weapon2=",
	"altcontrolscheme_weapon3=",
	"altcontrolscheme_weapon4=",
	"altcontrolscheme_weapon5=",
	"altcontrolscheme_weapon6=",
	"altcontrolscheme_map=",
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	"altcontrolscheme_funckeys=",
#endif
	"altcontrolscheme_debugkeys=",
	0,
};

// Enumerated by SDL_GameControllerButton, for most
static const char *g_sdlControlSchemeKeyMapCfgVals[] = {
	"a", "b", "x", "y", 0, 0, 0, "lstick", "rstick", "lshoulder", "rshoulder", "dpadup", "dpaddown", "dpadleft", "dpadright", 0, "paddle1", "paddle2", "paddle3", "paddle4",
	"ltrigger", "rtrigger", // Actually axes but these are added as extras
	"" // for any entry which is not set
};

RefKeenConfig g_refKeenCfg;

#ifdef REFKEEN_VER_KDREAMS
#define REFKEEN_CONFIG_FILENAME "reflection-kdreams.cfg"
#elif defined REFKEEN_VER_CAT3D
#define REFKEEN_CONFIG_FILENAME "reflection-cat3d.cfg"
#elif defined REFKEEN_VER_CATABYSS
#define REFKEEN_CONFIG_FILENAME "reflection-catabyss.cfg"
#elif defined REFKEEN_VER_CATARM
#define REFKEEN_CONFIG_FILENAME "reflection-catarm.cfg"
#elif defined REFKEEN_VER_CATAPOC
#define REFKEEN_CONFIG_FILENAME "reflection-catapoc.cfg"
#elif defined REFKEEN_HAS_VER_CATACOMB_ALL
#define REFKEEN_CONFIG_FILENAME "reflection-catacomb.cfg"
#elif defined REFKEEN_HAS_VER_WOLF3D_ALL // TODO: Optionally separate cfg per version?
#define REFKEEN_CONFIG_FILENAME "reflection-wolf3d.cfg"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
static void BEL_ST_ParseSetting_FullScreen(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.isFullscreen = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.isFullscreen = false;
	}
}
#endif

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
static void BEL_ST_ParseSetting_FullRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.fullWidth, &g_refKeenCfg.fullHeight);
}
#endif

static void BEL_ST_ParseSetting_WindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.winWidth, &g_refKeenCfg.winHeight);
}

#ifdef REFKEEN_ENABLE_LAUNCHER
/*
static void BEL_ST_ParseSetting_LauncherWindowRes(const char *keyprefix, const char *buffer)
{
	sscanf(buffer, "%dx%d", &g_refKeenCfg.launcherWinWidth, &g_refKeenCfg.launcherWinHeight);
}
*/
static void BEL_ST_ParseSetting_LauncherWindowType(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "default"))
	{
		g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_DEFAULT;
	}
	else if (!strcmp(buffer, "full"))
	{
		g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_FULL;
	}
	else if (!strcmp(buffer, "software"))
	{
		g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_SOFTWARE;
	}
}

static void BEL_ST_ParseSetting_LauncherExeArgs(const char *keyprefix, const char *buffer)
{
	BE_Cross_safeandfastcstringcopy(g_refKeenCfg.launcherExeArgs, g_refKeenCfg.launcherExeArgs+sizeof(g_refKeenCfg.launcherExeArgs), buffer);
}
#endif

static void BEL_ST_ParseSetting_LastSelectedGameExe(const char *keyprefix, const char *buffer)
{
	BE_Cross_safeandfastcstringcopy(g_refKeenCfg.lastSelectedGameExe, g_refKeenCfg.lastSelectedGameExe+sizeof(g_refKeenCfg.lastSelectedGameExe), buffer);
}

static void BEL_ST_ParseSetting_LastSelectedGameVer(const char *keyprefix, const char *buffer)
{
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
		if (!strcmp(buffer, refkeen_gamever_strs[i]))
		{
			g_refKeenCfg.lastSelectedGameVer = i;
			break;
		}
}

static void BEL_ST_ParseSetting_RememberDisplayNum(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.rememberDisplayNum = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.rememberDisplayNum = false;
	}
}

static void BEL_ST_ParseSetting_VSync(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "on"))
	{
		g_refKeenCfg.vSync = VSYNC_ON;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_refKeenCfg.vSync = VSYNC_OFF;
	}
	else if (!strcmp(buffer, "auto"))
	{
		g_refKeenCfg.vSync = VSYNC_AUTO;
	}
}

static void BEL_ST_ParseSetting_Bilinear(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.isBilinear = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.isBilinear = false;
	}
}

static void BEL_ST_ParseSetting_ScaleType(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "aspect"))
	{
		g_refKeenCfg.scaleType = SCALE_ASPECT;
	}
	else if (!strcmp(buffer, "fill"))
	{
		g_refKeenCfg.scaleType = SCALE_FILL;
	}
}

static void BEL_ST_ParseSetting_ScaleFactor(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.scaleFactor = atoi(buffer);
}

static void BEL_ST_ParseSetting_ForceFullSoftScaling(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.forceFullSoftScaling = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.forceFullSoftScaling = false;
	}
}

static void BEL_ST_ParseSetting_MouseGrab(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "commonly"))
	{
		g_refKeenCfg.mouseGrab = MOUSEGRAB_COMMONLY;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_refKeenCfg.mouseGrab = MOUSEGRAB_OFF;
	}
	else if (!strcmp(buffer, "auto"))
	{
		g_refKeenCfg.mouseGrab = MOUSEGRAB_AUTO;
	}
}

#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
static void BEL_ST_ParseSetting_AbsMouseMotion(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.absMouseMotion = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.absMouseMotion = false;
	}
}
#endif

static void BEL_ST_ParseSetting_SndInterThreadBufferRatio(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.sndInterThreadBufferRatio = atoi(buffer);
	if (g_refKeenCfg.sndInterThreadBufferRatio <= 0)
		g_refKeenCfg.sndInterThreadBufferRatio = 2;
}

static void BEL_ST_ParseSetting_SndSampleRate(const char *keyprefix, const char *buffer)
{
	g_refKeenCfg.sndSampleRate = atoi(buffer);
	if (g_refKeenCfg.sndSampleRate <= 0)
		g_refKeenCfg.sndSampleRate = OPL_SAMPLE_RATE;
}

static void BEL_ST_ParseSetting_SoundSubSystem(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.sndSubSystem = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.sndSubSystem = false;
	}
}

static void BEL_ST_ParseSetting_OPLEmulation(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.oplEmulation = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.oplEmulation = false;
	}
}

#ifndef REFKEEN_RESAMPLER_NONE
static void BEL_ST_ParseSetting_UseResampler(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.useResampler = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.useResampler = false;
	}
}
#endif

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
static void BEL_ST_ParseSetting_TouchInputToggle(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "auto"))
	{
		g_refKeenCfg.touchInputToggle = TOUCHINPUT_AUTO;
	}
	else if (!strcmp(buffer, "off"))
	{
		g_refKeenCfg.touchInputToggle = TOUCHINPUT_OFF;
	}
	else if (!strcmp(buffer, "forced"))
	{
		g_refKeenCfg.touchInputToggle = TOUCHINPUT_FORCED;
	}
}

static void BEL_ST_ParseSetting_TouchInputDebugging(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.touchInputDebugging = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.touchInputDebugging = false;
	}
}
#endif // REFKEEN_CONFIG_ENABLE_TOUCHINPUT

static void BEL_ST_ParseSetting_AlternativeControlScheme(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.isEnabled = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.isEnabled = false;
	}
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap(const char *keyprefix, const char *buffer)
{
	int keyindex, valindex;
	for (keyindex = 0; keyindex < BE_ST_CTRL_CFG_BUTMAP_AFTERLAST; ++keyindex)
	{
		if (!strcmp(keyprefix, g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex]))
			break;
	}
	if (keyindex == BE_ST_CTRL_CFG_BUTMAP_AFTERLAST)
	{
		BE_ST_ExitWithErrorMsg("BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap: Invalid config key!\n");
	}
	for (valindex = 0; valindex < (int)BE_Cross_ArrayLen(g_sdlControlSchemeKeyMapCfgVals); ++valindex)
	{
		// Empty strings are used for SDL game controller buttons we wish to skip
		if (g_sdlControlSchemeKeyMapCfgVals[valindex] && !strcmp(buffer, g_sdlControlSchemeKeyMapCfgVals[valindex]))
		{
			g_refKeenCfg.altControlScheme.actionMappings[keyindex] = valindex;
			return;
		}
	}
	g_refKeenCfg.altControlScheme.actionMappings[keyindex] = BE_Cross_ArrayLen(g_sdlControlSchemeKeyMapCfgVals) - 1; // SPECIAL - A way to toggle this off
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeLeftStick(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.useLeftStick = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.useLeftStick = false;
	}
}

static void BEL_ST_ParseSetting_AlternativeControlSchemeRightStick(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.useRightStick = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.useRightStick = false;
	}
}

#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
static void BEL_ST_ParseSetting_AlternativeControlSchemeAnalogMotion(const char *keyprefix, const char *buffer)
{
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.altControlScheme.analogMotion = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.altControlScheme.analogMotion = false;
	}
}
#endif

// HACK (cfg file may be rewritten and we don't want to remove any setting)
static bool g_sdlIsManualGameVerModeSettingRead = false;
static void BEL_ST_ParseSetting_ManualGameVerMode(const char *keyprefix, const char *buffer)
{
	g_sdlIsManualGameVerModeSettingRead = true;
	if (!strcmp(buffer, "true"))
	{
		g_refKeenCfg.manualGameVerMode = true;
	}
	else if (!strcmp(buffer, "false"))
	{
		g_refKeenCfg.manualGameVerMode = false;
	}
}

#ifdef BE_CROSS_ENABLE_FARPTR_CFG
// Same HACK again
static bool g_sdlIsFarPtrSegOffsetSettingRead = false;

static void BEL_ST_ParseSetting_FarPtrSegOffset(const char *keyprefix, const char *buffer)
{
	unsigned int segOffset;
	g_sdlIsFarPtrSegOffsetSettingRead = true;
	if (sscanf(buffer, "%X", &segOffset) == 1)
	{
		g_refKeenCfg.farPtrSegOffset = segOffset;
	}
}
#endif

// These ones are implementation-defined
void BEL_ST_ParseSetting_DisplayNum(const char *keyprefix, const char *buffer);
void BEL_ST_ParseSetting_SDLRendererDriver(const char *keyprefix, const char *buffer);
void BEL_ST_SaveSDLRendererDriverToConfig(FILE *fp, const char *keyprefix, int driver);

typedef struct {
	const char *cfgPrefix; // Includes '=' sign
	void (*handlerPtr)(const char *, const char *);
} BESDLCfgEntry;

static BESDLCfgEntry g_sdlCfgEntries[] = {
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	{"fullscreen=", &BEL_ST_ParseSetting_FullScreen},
#endif
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
	{"fullres=", &BEL_ST_ParseSetting_FullRes},
#endif
	{"windowres=", &BEL_ST_ParseSetting_WindowRes},
#ifdef REFKEEN_ENABLE_LAUNCHER
//	{"launcherwindowres=", &BEL_ST_ParseSetting_LauncherWindowRes},
	{"launcherwindowtype=", &BEL_ST_ParseSetting_LauncherWindowType},
	{"launcherexeargs=", &BEL_ST_ParseSetting_LauncherExeArgs},
#endif
	{"lastselectedgameexe=", &BEL_ST_ParseSetting_LastSelectedGameExe},
	{"lastselectedgamever=", &BEL_ST_ParseSetting_LastSelectedGameVer},
	{"displaynum=", &BEL_ST_ParseSetting_DisplayNum},
	{"rememberdisplaynum=", &BEL_ST_ParseSetting_RememberDisplayNum},
	{"sdlrenderer=", &BEL_ST_ParseSetting_SDLRendererDriver},
	{"vsync=", &BEL_ST_ParseSetting_VSync},
	{"bilinear=", &BEL_ST_ParseSetting_Bilinear},
	{"scaletype=", &BEL_ST_ParseSetting_ScaleType},
	{"scalefactor=", &BEL_ST_ParseSetting_ScaleFactor},
	{"forcefullsoftscaling=", &BEL_ST_ParseSetting_ForceFullSoftScaling},
	{"mousegrab=", &BEL_ST_ParseSetting_MouseGrab},
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	{"absmousemotion=", &BEL_ST_ParseSetting_AbsMouseMotion},
#endif
	{"sndinterthreadbufferratio=", &BEL_ST_ParseSetting_SndInterThreadBufferRatio},
	{"sndsamplerate=", &BEL_ST_ParseSetting_SndSampleRate},
	{"sndsubsystem=", &BEL_ST_ParseSetting_SoundSubSystem},
	{"oplemulation=", &BEL_ST_ParseSetting_OPLEmulation},
#ifndef REFKEEN_RESAMPLER_NONE
	{"useresampler=", &BEL_ST_ParseSetting_UseResampler},
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	{"touchinput=", &BEL_ST_ParseSetting_TouchInputToggle},
	{"touchinputdebugging=", &BEL_ST_ParseSetting_TouchInputDebugging},
#endif
	{"altcontrolscheme=", &BEL_ST_ParseSetting_AlternativeControlScheme},

	// HACK: Copy-paste... if this is updated, check g_sdlControlSchemeKeyMapCfgKeyPrefixes too!!!
	{"altcontrolscheme_up=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_down=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_left=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_right=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#ifdef REFKEEN_HAS_VER_KDREAMS
	{"altcontrolscheme_jump=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_throw=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_stats=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	{"altcontrolscheme_fire=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_strafe=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	{"altcontrolscheme_drink=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_bolt=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_nuke=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_fastturn=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	{"altcontrolscheme_scrolls=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	{"altcontrolscheme_use=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_run=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon1=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon2=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon3=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon4=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon5=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_weapon6=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
	{"altcontrolscheme_map=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	{"altcontrolscheme_funckeys=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},
#endif
	{"altcontrolscheme_debugkeys=", &BEL_ST_ParseSetting_AlternativeControlSchemeKeyMap},

	{"altcontrolscheme_lstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeLeftStick},
	{"altcontrolscheme_rstick=", &BEL_ST_ParseSetting_AlternativeControlSchemeRightStick},
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	{"altcontrolscheme_analogmotion=", &BEL_ST_ParseSetting_AlternativeControlSchemeAnalogMotion},
#endif

	{"manualgamevermode=", &BEL_ST_ParseSetting_ManualGameVerMode},
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	{"farptrsegoffset=", &BEL_ST_ParseSetting_FarPtrSegOffset},
#endif
};



void BEL_ST_ParseConfig(void)
{
	// Defaults
	g_refKeenCfg.isFullscreen = false; // Always exists internally, regardless of REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	g_refKeenCfg.fullWidth = 0;
	g_refKeenCfg.fullHeight = 0;
	g_refKeenCfg.winWidth = 0;
	g_refKeenCfg.winHeight = 0;
#ifdef REFKEEN_ENABLE_LAUNCHER
/*
	g_refKeenCfg.launcherWinWidth = 0;
	g_refKeenCfg.launcherWinHeight = 0;
*/
	g_refKeenCfg.launcherExeArgs[0] = '\0';
	g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_DEFAULT;
#endif
	g_refKeenCfg.lastSelectedGameExe[0] = '\0';
	g_refKeenCfg.lastSelectedGameVer = BE_GAMEVER_LAST;
	g_refKeenCfg.displayNum = 0;
	g_refKeenCfg.rememberDisplayNum = true;
	g_refKeenCfg.sdlRendererDriver = -1;
	g_refKeenCfg.vSync = VSYNC_OFF;
	g_refKeenCfg.isBilinear = true;
	g_refKeenCfg.scaleType = SCALE_ASPECT;
	g_refKeenCfg.scaleFactor = 2;
	g_refKeenCfg.forceFullSoftScaling = false;
	g_refKeenCfg.mouseGrab = MOUSEGRAB_AUTO;
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	g_refKeenCfg.absMouseMotion = false;
#endif
	g_refKeenCfg.sndInterThreadBufferRatio = 2;
	g_refKeenCfg.sndSampleRate = 48000; // 49716 may lead to unexpected behaviors on Android
	g_refKeenCfg.sndSubSystem = true;
	g_refKeenCfg.oplEmulation = true;
#ifndef REFKEEN_RESAMPLER_NONE
	g_refKeenCfg.useResampler = true;
#endif
#if (defined REFKEEN_CONFIG_ENABLE_TOUCHINPUT) && (defined REFKEEN_CONFIG_AUTODETECT_TOUCHINPUT_BY_DEFAULT)
	g_refKeenCfg.touchInputToggle = TOUCHINPUT_AUTO;
#else
	g_refKeenCfg.touchInputToggle = TOUCHINPUT_OFF;
#endif
	g_refKeenCfg.touchInputDebugging = false;
	g_refKeenCfg.altControlScheme.isEnabled = true;

#ifdef REFKEEN_HAS_VER_KDREAMS // Reserve the d-pad for weapons/feeds in Wolf3D/S3DNA
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_UP] = BE_ST_CTRL_BUT_DPAD_UP;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DOWN] = BE_ST_CTRL_BUT_DPAD_DOWN;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_LEFT] = BE_ST_CTRL_BUT_DPAD_LEFT;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RIGHT] = BE_ST_CTRL_BUT_DPAD_RIGHT;
#else // FIXME: HACK (extra 2 are for triggers)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_UP] = BE_ST_CTRL_BUT_MAX+2;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DOWN] = BE_ST_CTRL_BUT_MAX+2;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_LEFT] = BE_ST_CTRL_BUT_MAX+2;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RIGHT] = BE_ST_CTRL_BUT_MAX+2;
#endif
#ifdef REFKEEN_HAS_VER_KDREAMS
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_JUMP] = BE_ST_CTRL_BUT_A;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_THROW] = BE_ST_CTRL_BUT_B;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STATS] = BE_ST_CTRL_BUT_X;
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE] = BE_ST_CTRL_BUT_LSHOULDER;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE] = BE_ST_CTRL_BUT_B;
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DRINK] = BE_ST_CTRL_BUT_A;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_BOLT] = BE_ST_CTRL_BUT_X;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_NUKE] = BE_ST_CTRL_BUT_Y;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FASTTURN] = BE_ST_CTRL_BUT_RSHOULDER;
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_SCROLLS] = BE_ST_CTRL_BUT_MAX+1; // HACK for getting right trigger (technically an axis)
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_USE] = BE_ST_CTRL_BUT_MAX+1; // HACK for getting right trigger (technically an axis)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RUN] = BE_ST_CTRL_BUT_RSHOULDER;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON1] = BE_ST_CTRL_BUT_DPAD_DOWN;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON2] = BE_ST_CTRL_BUT_DPAD_RIGHT;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON3] = BE_ST_CTRL_BUT_DPAD_LEFT;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON4] = BE_ST_CTRL_BUT_DPAD_UP;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON5] = BE_ST_CTRL_BUT_X;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON6] = BE_ST_CTRL_BUT_Y;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_MAP] = BE_ST_CTRL_BUT_A;
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS] = BE_ST_CTRL_BUT_MAX; // HACK for getting left trigger (technically an axis)
#endif
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS] = BE_ST_CTRL_BUT_LSTICK;

	g_refKeenCfg.altControlScheme.useLeftStick = true;
	g_refKeenCfg.altControlScheme.useRightStick = false;
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	g_refKeenCfg.altControlScheme.analogMotion = false;
#endif

	g_refKeenCfg.manualGameVerMode = false;
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	g_refKeenCfg.farPtrSegOffset = BE_ST_DEFAULT_FARPTRSEGOFFSET;
#endif
	// Try to load config
	FILE *fp = BE_Cross_open_additionalfile_for_reading(REFKEEN_CONFIG_FILENAME);
	if (!fp)
	{
		return;
	}
	char buffer[80];
	while (fgets(buffer, sizeof(buffer), fp))
	{
		size_t len = strlen(buffer);
		if (!len)
		{
			continue;
		}
		if (buffer[len-1] == '\n')
		{
			buffer[len-1] = '\0';
		}
		for (int i = 0; i < (int)BE_Cross_ArrayLen(g_sdlCfgEntries); ++i)
		{
			if (!strncmp(g_sdlCfgEntries[i].cfgPrefix, buffer, strlen(g_sdlCfgEntries[i].cfgPrefix)))
			{
				g_sdlCfgEntries[i].handlerPtr(g_sdlCfgEntries[i].cfgPrefix, buffer+strlen(g_sdlCfgEntries[i].cfgPrefix));
				break;
			}
		}
	}
	fclose(fp);
}

void BEL_ST_SaveConfig(void)
{
	// Try to save current settings just in case (first time file is created or new fields added)
	FILE *fp = BE_Cross_open_additionalfile_for_overwriting(REFKEEN_CONFIG_FILENAME);
	if (!fp)
	{
		return;
	}
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	fprintf(fp, "fullscreen=%s\n", g_refKeenCfg.isFullscreen ? "true" : "false");
#endif
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
	fprintf(fp, "fullres=%dx%d\n", g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight);
#endif
	fprintf(fp, "windowres=%dx%d\n", g_refKeenCfg.winWidth, g_refKeenCfg.winHeight);
#ifdef REFKEEN_ENABLE_LAUNCHER
	//fprintf(fp, "launcherwindowres=%dx%d\n", g_refKeenCfg.launcherWinWidth, g_refKeenCfg.launcherWinHeight);
	fprintf(fp, "launcherwindowtype=%s\n", g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_DEFAULT ? "default" : (g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_FULL ? "full" : "software"));
	fprintf(fp, "launcherexeargs=%s\n", g_refKeenCfg.launcherExeArgs);
#endif
	fprintf(fp, "lastselectedgameexe=%s\n", g_refKeenCfg.lastSelectedGameExe);
	fprintf(fp, "lastselectedgamever=%s\n", (g_refKeenCfg.lastSelectedGameVer != BE_GAMEVER_LAST) ? refkeen_gamever_strs[g_refKeenCfg.lastSelectedGameVer] : "");

	if (g_refKeenCfg.rememberDisplayNum)
		g_refKeenCfg.displayNum = BEL_ST_GetWindowDisplayNum();

	fprintf(fp, "displaynum=%d\n", g_refKeenCfg.displayNum);
	fprintf(fp, "rememberdisplaynum=%s\n", g_refKeenCfg.rememberDisplayNum ? "true" : "false");
	BEL_ST_SaveSDLRendererDriverToConfig(fp, "sdlrenderer=", g_refKeenCfg.sdlRendererDriver);
	fprintf(fp, "vsync=%s\n", (g_refKeenCfg.vSync == VSYNC_AUTO) ? "auto" : ((g_refKeenCfg.vSync == VSYNC_ON) ? "on" : "off"));
	fprintf(fp, "bilinear=%s\n", g_refKeenCfg.isBilinear ? "true" : "false");
	fprintf(fp, "scaletype=%s\n", (g_refKeenCfg.scaleType == SCALE_ASPECT) ? "aspect" : "fill");
	fprintf(fp, "scalefactor=%d\n", g_refKeenCfg.scaleFactor);
	fprintf(fp, "forcefullsoftscaling=%s\n", g_refKeenCfg.forceFullSoftScaling ? "true" : "false");
	fprintf(fp, "mousegrab=%s\n", (g_refKeenCfg.mouseGrab == MOUSEGRAB_AUTO) ? "auto" : ((g_refKeenCfg.mouseGrab == MOUSEGRAB_COMMONLY) ? "commonly" : "off"));
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	fprintf(fp, "absmousemotion=%s\n", g_refKeenCfg.absMouseMotion ? "true" : "false");
#endif
	fprintf(fp, "sndinterthreadbufferratio=%d\n", g_refKeenCfg.sndInterThreadBufferRatio);
	fprintf(fp, "sndsamplerate=%d\n", g_refKeenCfg.sndSampleRate);
	fprintf(fp, "sndsubsystem=%s\n", g_refKeenCfg.sndSubSystem ? "true" : "false");
	fprintf(fp, "oplemulation=%s\n", g_refKeenCfg.oplEmulation ? "true" : "false");
#ifndef REFKEEN_RESAMPLER_NONE
	fprintf(fp, "useresampler=%s\n", g_refKeenCfg.useResampler ? "true" : "false");
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	fprintf(fp, "touchinput=%s\n", (g_refKeenCfg.touchInputToggle == TOUCHINPUT_AUTO) ? "auto" : ((g_refKeenCfg.touchInputToggle == TOUCHINPUT_FORCED) ? "forced" : "off"));
	fprintf(fp, "touchinputdebugging=%s\n", g_refKeenCfg.touchInputDebugging ? "true" : "false");
#endif
	fprintf(fp, "altcontrolscheme=%s\n", g_refKeenCfg.altControlScheme.isEnabled ? "true" : "false");
	// Go through an array of keys
	for (int keyindex = 0; keyindex < BE_ST_CTRL_CFG_BUTMAP_AFTERLAST; ++keyindex)
	{
		fprintf(fp, "%s%s\n", g_sdlControlSchemeKeyMapCfgKeyPrefixes[keyindex], g_sdlControlSchemeKeyMapCfgVals[g_refKeenCfg.altControlScheme.actionMappings[keyindex]]);
	}
	fprintf(fp, "altcontrolscheme_lstick=%s\n", g_refKeenCfg.altControlScheme.useLeftStick ? "true" : "false");
	fprintf(fp, "altcontrolscheme_rstick=%s\n", g_refKeenCfg.altControlScheme.useRightStick ? "true" : "false");
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	fprintf(fp, "altcontrolscheme_analogmotion=%s\n", g_refKeenCfg.altControlScheme.analogMotion ? "true" : "false");
#endif
	if (g_sdlIsManualGameVerModeSettingRead)
	{
		// This should be a relatively hidden setting
		fprintf(fp, "manualgamevermode=%s\n", g_refKeenCfg.manualGameVerMode ? "true" : "false");
	}
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	if (g_sdlIsFarPtrSegOffsetSettingRead)
	{
		// Another hidden setting
		fprintf(fp, "farptrsegoffset=%X\n", g_refKeenCfg.farPtrSegOffset);
	}
#endif
	fclose(fp);
}
