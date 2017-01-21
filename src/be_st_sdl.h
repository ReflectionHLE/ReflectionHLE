/* Copyright (C) 2014-2017 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BE_ST_SDL_H
#define BE_ST_SDL_H

#include "be_st.h"

// WARNING: This struct is used by BE_ST_SDL ONLY (except for controller stuff)

#ifdef REFKEEN_VER_KDREAMS
#define BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
#endif

typedef enum { VSYNC_AUTO, VSYNC_OFF, VSYNC_ON } VSyncSettingType;
typedef enum { SCALE_ASPECT, SCALE_FILL } ScaleTypeSettingType;
typedef enum { TOUCHINPUT_AUTO, TOUCHINPUT_OFF, TOUCHINPUT_FORCED } TouchInputSettingType;
typedef enum { MOUSEGRAB_AUTO, MOUSEGRAB_OFF, MOUSEGRAB_COMMONLY } MouseGrabSettingType;

#ifdef REFKEEN_ENABLE_LAUNCHER
#define LAUNCHER_EXE_ARGS_BUFFERLEN 80
typedef enum { LAUNCHER_WINDOW_DEFAULT, LAUNCHER_WINDOW_FULL, LAUNCHER_WINDOW_SOFTWARE } LauncherWindowSettingType;
#endif

typedef struct
{
	bool isFullscreen;
	int fullWidth, fullHeight;
	int winWidth, winHeight;
#ifdef REFKEEN_ENABLE_LAUNCHER
	// Now using just winWidth and winHeight due to seamless launcher->game transitions
	//int launcherWinWidth, launcherWinHeight;
	LauncherWindowSettingType launcherWinType;
	char launcherExeArgs[LAUNCHER_EXE_ARGS_BUFFERLEN];
#endif
	int lastSelectedGameVer;
	int displayNum;
	int sdlRendererDriver;
	VSyncSettingType vSync;
	bool isBilinear;
	ScaleTypeSettingType scaleType;
	int scaleFactor;
	bool forceFullSoftScaling;
	MouseGrabSettingType mouseGrab;
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	bool absMouseMotion;
#endif
	int sndSampleRate;
	bool sndSubSystem;
	bool oplEmulation;
#ifndef REFKEEN_RESAMPLER_NONE
	bool useResampler;
#endif
	TouchInputSettingType touchInputToggle;
	bool touchInputDebugging;
	struct
	{
		int actionMappings[BE_ST_CTRL_CFG_BUTMAP_AFTERLAST]; // Buttons/triggers
		bool useDpad;
		bool useLeftStick;
		bool useRightStick;
		bool analogMotion;
		bool isEnabled;
	} altControlScheme;
	bool manualGameVerMode;
	unsigned int farPtrSegOffset; // Actually used just in The Catacomb Armageddon/Apocalypse
} RefKeenConfig;

extern RefKeenConfig g_refKeenCfg;

#endif // BE_ST_SDL_H
