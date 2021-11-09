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

#include <limits.h>

#include "../audio/be_audio_private.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h"
#include "be_st.h"

#define BE_ST_DEFAULT_FARPTRSEGOFFSET 0x14

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

typedef enum {
	BE_ST_CFG_VAL_ENUM,
	BE_ST_CFG_VAL_HEX_INT,
	BE_ST_CFG_VAL_INT,
	BE_ST_CFG_VAL_DIMS,
	BE_ST_CFG_VAL_STR,
	BE_ST_CFG_VAL_DISPLAY_NUM,
	BE_ST_CFG_VAL_SDL_RENDERER,
} BE_ST_CFG_VAL_T;

typedef struct {
	// Pointer to internal configuration value; Width of window dimensions.
	void *setting;
	// Height of window dimensions; Hidden setting marker otherwise.
	void *ptraux;
	// Configuration key
	const char *key;
	// Classification of configuration value
	BE_ST_CFG_VAL_T valType;
	// Values of variable meanings; An intptr one is
	// allowed to hold a pointer, but doesn't have to.
	int aux0;
	intptr_t aux1, aux2;
} BE_ST_CFG_Setting_T;

// Enumerated by SDL_GameControllerButton, for most
static const char *g_sdlControlSchemeKeyMapCfgVals[] = {
	"a", "b", "x", "y", 0, 0, 0, "lstick", "rstick", "lshoulder", "rshoulder", "dpadup", "dpaddown", "dpadleft", "dpadright", 0, "paddle1", "paddle2", "paddle3", "paddle4",
	"ltrigger", "rtrigger", // Actually axes but these are added as extras
	"" // for any entry which is not set
};

// Enumerated by other enums, as well as "bool"
static const char *g_be_setting_bool_vals[] = {"false", "true"};
#ifdef REFKEEN_ENABLE_LAUNCHER
static const char *g_be_setting_wintype_vals[] = {"default", "full", "software"};
#endif
static const char *g_be_setting_vsync_vals[] = {"auto", "off", "on"};
static const char *g_be_setting_scaletype_vals[] = {"aspect", "fill"};
static const char *g_be_setting_mousegrab_vals[] = {"auto", "off", "commonly"};
#ifdef BE_ST_ENABLE_SETTING_SB
static const char *g_be_setting_sb_vals[] = {"off", "sb", "sbpro" ,"sb16"};
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
static const char *g_be_setting_touchinput_vals[] = {"auto", "off", "forced"};
#endif

#define DEF_ENUM(setting, key, strs, def) \
	{&g_refKeenCfg.setting, 0, key, BE_ST_CFG_VAL_ENUM, def, (intptr_t)strs, BE_Cross_ArrayLen(strs)},
#define DEF_INT(setting, key, def, min, max) \
	{&g_refKeenCfg.setting, 0, key, BE_ST_CFG_VAL_INT, def, min, max},
#define DEF_DIMS(width, height, key, defw, defh) \
	{&g_refKeenCfg.width, &g_refKeenCfg.height, key, BE_ST_CFG_VAL_DIMS, defw, defh},
#define DEF_STR(setting, key) \
	{&g_refKeenCfg.setting, 0, key, BE_ST_CFG_VAL_STR, sizeof(g_refKeenCfg.setting)},
#define DEF_CUSTOM_INT(setting, key, type, def) \
	{&g_refKeenCfg.setting, 0, key, type, def},

#define DEF_BOOL(setting, key, def) \
	DEF_ENUM(setting, key, g_be_setting_bool_vals, def)

#define DEF_ACTIONMAP_ENUM(index, key, def) \
	DEF_ENUM(altControlScheme.actionMappings[index], key, g_sdlControlSchemeKeyMapCfgVals, def)

#define DEF_HIDDEN_ENUM(setting, key, strs, def) \
	{&g_refKeenCfg.setting, &g_refKeenCfg.setting, key, BE_ST_CFG_VAL_ENUM, def, (intptr_t)strs, BE_Cross_ArrayLen(strs)},

#define DEF_HIDDEN_BOOL(setting, key, def) \
	DEF_HIDDEN_ENUM(setting, key, g_be_setting_bool_vals, def)

#define DEF_HIDDEN_HEX_INT(setting, key, def, min, max) \
	{&g_refKeenCfg.setting, &g_refKeenCfg.setting, key, BE_ST_CFG_VAL_HEX_INT, def, min, max},


static BE_ST_CFG_Setting_T g_be_st_settings[] = {
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	DEF_BOOL(isFullscreen, "fullscreen", false)
#endif
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_RES_SETTING
	DEF_DIMS(fullWidth, fullHeight, "fullres", 0, 0)
#endif
	DEF_DIMS(winWidth, winHeight, "windowres", 0, 0)
#ifdef REFKEEN_ENABLE_LAUNCHER
//	DEF_DIMS(launcherWinWidth, launcherWinHeight, "launcherwindowres", 0, 0)
	DEF_ENUM(launcherWinType, "launcherwindowtype", g_be_setting_wintype_vals, LAUNCHER_WINDOW_DEFAULT)
	DEF_STR(launcherExeArgs, "launcherexeargs")
#endif
	DEF_STR(lastSelectedGameExe, "lastselectedgameexe")
	DEF_ENUM(lastSelectedGameVer, "lastselectedgamever", refkeen_gamever_strs, BE_GAMEVER_LAST)
	DEF_CUSTOM_INT(displayNum, "displaynum", BE_ST_CFG_VAL_DISPLAY_NUM, 0)
	DEF_BOOL(rememberDisplayNum, "rememberdisplaynum", true)
	DEF_CUSTOM_INT(sdlRendererDriver, "sdlrenderer", BE_ST_CFG_VAL_SDL_RENDERER, -1)
	DEF_ENUM(vSync, "vsync", g_be_setting_vsync_vals, VSYNC_OFF)
	DEF_BOOL(isBilinear, "bilinear", true)
	DEF_ENUM(scaleType, "scaletype", g_be_setting_scaletype_vals, SCALE_ASPECT)
	DEF_INT(scaleFactor, "scalefactor", 2, 1, INT_MAX)
	DEF_BOOL(forceFullSoftScaling, "forcefullsoftscaling", false)
	DEF_ENUM(mouseGrab, "mousegrab", g_be_setting_mousegrab_vals, MOUSEGRAB_AUTO)
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	DEF_BOOL(absMouseMotion, "absmousemotion", false)
#endif
	DEF_INT(sndInterThreadBufferRatio, "sndinterthreadbufferratio", 2, 1, INT_MAX)
	// 49716 may lead to unexpected behaviors on Android
	DEF_INT(sndSampleRate, "sndsamplerate", 48000, 1, INT_MAX)
	DEF_BOOL(sndSubSystem, "sndsubsystem", true)
	DEF_BOOL(oplEmulation, "oplemulation", true)
#ifdef BE_ST_ENABLE_SETTING_SB
	DEF_ENUM(sb, "sbemu", g_be_setting_sb_vals, SOUNDBLASTER_SB16)
#endif
	DEF_INT(pcSpkVol, "pcspkvol", BE_AUDIO_VOL_MAX, BE_AUDIO_VOL_MIN, BE_AUDIO_VOL_MAX)
	DEF_INT(oplVol, "oplvol", BE_AUDIO_VOL_MAX, BE_AUDIO_VOL_MIN, BE_AUDIO_VOL_MAX)
#ifdef BE_ST_ENABLE_SETTING_DIGIVOL
	DEF_INT(digiVol, "digivol", BE_AUDIO_VOL_MAX, BE_AUDIO_VOL_MIN, BE_AUDIO_VOL_MAX)
#endif
#ifndef REFKEEN_RESAMPLER_NONE
	DEF_BOOL(useResampler, "useresampler", true)
#endif
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	#ifdef REFKEEN_CONFIG_AUTODETECT_TOUCHINPUT_BY_DEFAULT
	DEF_ENUM(touchInputToggle, "touchinput", g_be_setting_touchinput_vals, TOUCHINPUT_AUTO)
	#else
	DEF_ENUM(touchInputToggle, "touchinput", g_be_setting_touchinput_vals, TOUCHINPUT_OFF)
	#endif
	DEF_BOOL(touchInputDebugging, "touchinputdebugging", false)
#endif
	DEF_BOOL(altControlScheme.isEnabled, "altcontrolscheme", true)

#ifdef REFKEEN_HAS_VER_KDREAMS // Reserve the d-pad for weapons/feeds in Wolf3D/S3DNA
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_UP, "altcontrolscheme_up", BE_ST_CTRL_BUT_DPAD_UP)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_DOWN, "altcontrolscheme_down", BE_ST_CTRL_BUT_DPAD_DOWN)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_LEFT, "altcontrolscheme_left", BE_ST_CTRL_BUT_DPAD_LEFT)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_RIGHT, "altcontrolscheme_right", BE_ST_CTRL_BUT_DPAD_RIGHT)
#else // FIXME: HACK (extra 2 are for triggers)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_UP, "altcontrolscheme_up", BE_ST_CTRL_BUT_MAX+2)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_DOWN, "altcontrolscheme_down", BE_ST_CTRL_BUT_MAX+2)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_LEFT, "altcontrolscheme_left", BE_ST_CTRL_BUT_MAX+2)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_RIGHT, "altcontrolscheme_right", BE_ST_CTRL_BUT_MAX+2)
#endif
#ifdef REFKEEN_HAS_VER_KDREAMS
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_JUMP, "altcontrolscheme_jump", BE_ST_CTRL_BUT_A)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_THROW, "altcontrolscheme_throw", BE_ST_CTRL_BUT_B)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_STATS, "altcontrolscheme_stats", BE_ST_CTRL_BUT_X)
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_FIRE, "altcontrolscheme_fire", BE_ST_CTRL_BUT_LSHOULDER)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_STRAFE, "altcontrolscheme_strafe", BE_ST_CTRL_BUT_B)
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_DRINK, "altcontrolscheme_drink", BE_ST_CTRL_BUT_A)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_BOLT, "altcontrolscheme_bolt", BE_ST_CTRL_BUT_X)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_NUKE, "altcontrolscheme_nuke", BE_ST_CTRL_BUT_Y)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_FASTTURN, "altcontrolscheme_fastturn", BE_ST_CTRL_BUT_RSHOULDER)
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	// HACK for getting right trigger (technically an axis)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_SCROLLS, "altcontrolscheme_scrolls", BE_ST_CTRL_BUT_MAX+1)
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	// HACK for getting right trigger (technically an axis)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_USE, "altcontrolscheme_use", BE_ST_CTRL_BUT_MAX+1)

	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_RUN, "altcontrolscheme_run", BE_ST_CTRL_BUT_RSHOULDER)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_WEAPON1, "altcontrolscheme_weapon1", BE_ST_CTRL_BUT_DPAD_DOWN)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_WEAPON2, "altcontrolscheme_weapon2", BE_ST_CTRL_BUT_DPAD_RIGHT)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_WEAPON3, "altcontrolscheme_weapon3", BE_ST_CTRL_BUT_DPAD_LEFT)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_WEAPON4, "altcontrolscheme_weapon4", BE_ST_CTRL_BUT_DPAD_UP)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_WEAPON5, "altcontrolscheme_weapon5", BE_ST_CTRL_BUT_X)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_WEAPON6, "altcontrolscheme_weapon6", BE_ST_CTRL_BUT_Y)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_MAP, "altcontrolscheme_map", BE_ST_CTRL_BUT_A)
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	// HACK for getting left trigger (technically an axis)
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS, "altcontrolscheme_funckeys", BE_ST_CTRL_BUT_MAX)
#endif
	DEF_ACTIONMAP_ENUM(BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS, "altcontrolscheme_debugkeys", BE_ST_CTRL_BUT_LSTICK)

	DEF_BOOL(altControlScheme.useLeftStick, "altcontrolscheme_lstick", true)
	DEF_BOOL(altControlScheme.useRightStick, "altcontrolscheme_rstick", false)
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	DEF_BOOL(altControlScheme.analogMotion, "altcontrolscheme_analogmotion", false)
#endif

#ifdef BE_ST_ENABLE_SETTING_NOVERT
	DEF_BOOL(novert, "novert", false)
#endif
#ifdef BE_ST_ENABLE_SETTING_LOWFPS
	DEF_BOOL(lowFPS, "lowfps", false)
#endif
	DEF_HIDDEN_BOOL(manualGameVerMode, "manualgamevermode", false)
#ifdef BE_CROSS_ENABLE_FARPTR_CFG
	DEF_HIDDEN_HEX_INT(farPtrSegOffset, "farptrsegoffset", BE_ST_DEFAULT_FARPTRSEGOFFSET, 0U, 65535U)
#endif
};

// These ones are implementation-defined
void BEL_ST_ParseSetting_DisplayNum(int *displayNum, const char *buffer);
void BEL_ST_ParseSetting_SDLRendererDriver(int *driver, const char *buffer);
void BEL_ST_SaveSDLRendererDriverToConfig(FILE *fp, const char *key, int driver);

static void BEL_ST_SetConfigDefaults(void)
{
	for (unsigned i = 0; i < BE_Cross_ArrayLen(g_be_st_settings); ++i)
		switch (g_be_st_settings[i].valType)
		{
		case BE_ST_CFG_VAL_STR:
			break;
		case BE_ST_CFG_VAL_DIMS:
			*(int *)g_be_st_settings[i].setting = g_be_st_settings[i].aux0;
			*(int *)g_be_st_settings[i].ptraux = g_be_st_settings[i].aux1;
			break;
		default:
			*(int *)g_be_st_settings[i].setting = g_be_st_settings[i].aux0;
		}
}

static void BEL_ST_ParseEnum(int *val, const char *list[], int len, const char *buffer)
{
	for (int i = 0; i < len; ++i)
		if (list[i] && !strcmp(buffer, list[i]))
		{
			*val = i;
			break;
		}
}

static void BEL_ST_WriteEnum(FILE *fp, const char *key, const char *list[], int len, int val)
{
	fprintf(fp, "%s=%s\n", key, ((val >= 0) && (val < len)) ? list[val] : "");
}

static void BEL_ST_ParseHexInt(int *val, int min, int max, const char *buffer)
{
	long ret = strtol(buffer, 0, 16);
	if ((ret >= min) && (ret <= max))
		*val = ret;
}

static void BEL_ST_WriteHexInt(FILE *fp, const char *key, int val)
{
	fprintf(fp, "%s=0x%x\n", key, val);
}

static void BEL_ST_ParseInt(int *val, int min, int max, const char *buffer)
{
	int ret = atoi(buffer);
	if ((ret >= min) && (ret <= max))
		*val = ret;
}

static void BEL_ST_WriteInt(FILE *fp, const char *key, int val)
{
	fprintf(fp, "%s=%d\n", key, val);
}

static void BEL_ST_ParseDims(int *w, int *h, const char *buffer)
{
	sscanf(buffer, "%dx%d", w, h);
}

static void BEL_ST_WriteDims(FILE *fp, const char *key, int w, int h)
{
	fprintf(fp, "%s=%dx%d\n", key, w, h);
}

static void BEL_ST_ParseString(char *val, int len, const char *buffer)
{
	BE_Cross_safeandfastcstringcopy(val, val + len, buffer);
}

static void BEL_ST_WriteString(FILE *fp, const char *key, const char *val)
{
	fprintf(fp, "%s=%s\n", key, val);
}

static void BEL_ST_ParseSetting(BE_ST_CFG_Setting_T *setting, const char *valStr)
{
	switch (setting->valType)
	{
	case BE_ST_CFG_VAL_ENUM:
		BEL_ST_ParseEnum((int *)setting->setting, (const char **)setting->aux1, setting->aux2, valStr);
		break;
	case BE_ST_CFG_VAL_DIMS:
		BEL_ST_ParseDims((int *)setting->setting, (int *)setting->ptraux, valStr);
		break;
	case BE_ST_CFG_VAL_HEX_INT:
		BEL_ST_ParseHexInt((int *)setting->setting, setting->aux1, setting->aux2, valStr);
		break;
	case BE_ST_CFG_VAL_INT:
		BEL_ST_ParseInt((int *)setting->setting, setting->aux1, setting->aux2, valStr);
		break;
	case BE_ST_CFG_VAL_DISPLAY_NUM:
		BEL_ST_ParseSetting_DisplayNum((int *)setting->setting, valStr);
		break;
	case BE_ST_CFG_VAL_SDL_RENDERER:
		BEL_ST_ParseSetting_SDLRendererDriver((int *)setting->setting, valStr);
		break;
	case BE_ST_CFG_VAL_STR:
		BEL_ST_ParseString((char *)setting->setting, setting->aux0, valStr);
		break;
	}
}

static void BEL_ST_SaveSetting(FILE *fp, const BE_ST_CFG_Setting_T *setting)
{
	switch (setting->valType)
	{
	case BE_ST_CFG_VAL_ENUM:
		BEL_ST_WriteEnum(fp, setting->key, (const char **)setting->aux1, setting->aux2, *(int *)setting->setting);
		break;
	case BE_ST_CFG_VAL_DIMS:
		BEL_ST_WriteDims(fp, setting->key, *(int *)setting->setting, *(int *)setting->ptraux);
		break;
	case BE_ST_CFG_VAL_HEX_INT:
		BEL_ST_WriteHexInt(fp, setting->key, *(int *)setting->setting);
		break;
	case BE_ST_CFG_VAL_INT:
		BEL_ST_WriteInt(fp, setting->key, *(int *)setting->setting);
		break;
	case BE_ST_CFG_VAL_DISPLAY_NUM:
		BEL_ST_WriteInt(fp, setting->key, *(int *)setting->setting);
		break;
	case BE_ST_CFG_VAL_SDL_RENDERER:
		BEL_ST_SaveSDLRendererDriverToConfig(fp, setting->key, *(int *)setting->setting);
		break;
	case BE_ST_CFG_VAL_STR:
		BEL_ST_WriteString(fp, setting->key, (const char *)setting->setting);
		break;
	}
}

void BEL_ST_ParseConfig(void)
{
	BEL_ST_SetConfigDefaults();
	// Try to load config
	FILE *fp = BE_Cross_open_additionalfile_for_reading(REFKEEN_CONFIG_FILENAME);
	if (!fp)
		return;

	char buffer[80];
	while (fgets(buffer, sizeof(buffer), fp))
	{
		size_t len = strlen(buffer);
		if (!len)
			continue;
		char *sep = strchr(buffer, '=');
		if (!sep)
			continue;
		if (buffer[len-1] == '\n')
			buffer[len-1] = '\0';
		*sep = '\0';
		for (unsigned i = 0; i < BE_Cross_ArrayLen(g_be_st_settings); ++i)
		{
			BE_ST_CFG_Setting_T *setting = &g_be_st_settings[i];
			if (!strcmp(buffer, setting->key))
			{
				// Unhide a setting if it's hidden by default
				if (setting->valType != BE_ST_CFG_VAL_DIMS)
					setting->ptraux = 0;
				BEL_ST_ParseSetting(setting, sep + 1);
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
		return;

	for (unsigned i = 0; i < BE_Cross_ArrayLen(g_be_st_settings); ++i)
	{
		const BE_ST_CFG_Setting_T *setting = &g_be_st_settings[i];
		// Write setting only if it's not hidden
		if ((setting->valType == BE_ST_CFG_VAL_DIMS) || !setting->ptraux)
			BEL_ST_SaveSetting(fp, &g_be_st_settings[i]);
	}
	fclose(fp);
}
