/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 * Copyright (C) 2015-2021 NY00123
 *
 * This program is free software; you can redistribute it and/or modify
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

// New file used for preparing and defining alternative controller schemes,
// in case their definitions are configurable (possibly in more than one way)

#include <string.h>
#include "id_heads.h"

/*** Touch input UI resource definitions ***/

#include "../rsrc/button_back.xpm"
#include "../rsrc/button_debug_keys.xpm"
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
#include "../rsrc/button_scrolls.xpm"
#endif
#include "../rsrc/button_shoot.xpm"
#include "../rsrc/button_strafe.xpm"
#include "../rsrc/button_heal.xpm"
#include "../rsrc/button_nuke.xpm"
#include "../rsrc/button_bolt.xpm"
#ifdef REFKEEN_VER_CAT3D
#include "../rsrc/button_run.xpm"
#include "../rsrc/button_pause.xpm"
#else
#include "../rsrc/button_quickturn.xpm"
#include "../rsrc/button_function_keys.xpm"
#endif

#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
#include "../rsrc/pad_dpad.xpm"
#include "../rsrc/pad_dpad_left_input.xpm"
#include "../rsrc/pad_dpad_right_input.xpm"
#ifdef REFKEEN_VER_CATABYSS
#include "../rsrc/pad_dpad_up_input.xpm"
#include "../rsrc/pad_dpad_down_input.xpm"
#endif
#endif

#include "../rsrc/analog_circle.xpm"
#include "../rsrc/analog_circle_left_input.xpm"
#include "../rsrc/analog_circle_right_input.xpm"
#include "../rsrc/analog_circle_top_input.xpm"
#include "../rsrc/analog_circle_bottom_input.xpm"
#include "../rsrc/analog_circle_topleft_input.xpm"
#include "../rsrc/analog_circle_topright_input.xpm"
#include "../rsrc/analog_circle_bottomleft_input.xpm"
#include "../rsrc/analog_circle_bottomright_input.xpm"

REFKEEN_NS_B

BE_ST_ControllerSingleMap *g_ingame_altcontrol_button0mappings[2], *g_ingame_altcontrol_button1mappings[2],
	*g_ingame_altcontrol_upmappings[4], *g_ingame_altcontrol_downmappings[4], *g_ingame_altcontrol_leftmappings[4], *g_ingame_altcontrol_rightmappings[4];

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_scrolls;
extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys;

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_gameplay_onscreentouchcontrols[] = {
	{button_shoot_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{button_strafe_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8},
	{analog_circle_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8},
	{button_heal_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8},
	{button_nuke_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, 8},
	{button_bolt_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, 8},
	{button_back_xpm+5, 22, 22, 8, 2+8},
#ifdef REFKEEN_VER_CAT3D
	{button_run_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 17+8},
	{button_pause_xpm+5, 22, 22, 24+8, 2+8},
#else
	{button_quickturn_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 17+8},
	{button_function_keys_xpm+5, 22, 22, 24+8, 2+8},
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	{button_scrolls_xpm+5, 22, 22, 24+8, 26+8},
#endif
	{button_debug_keys_xpm+5, 22, 22, 8, 26+8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_gameplay_touchmappings[] = {
	/*** Actually adjusted in-game ***/
	{{NULL, BE_ST_SC_LCTRL, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_shoot_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{{NULL, BE_ST_SC_LALT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_strafe_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
	},
	{{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_left_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_right_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_top_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_bottom_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_HOME, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_topleft_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_PAGEUP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_topright_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_END, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_bottomleft_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_PAGEDOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		analog_circle_bottomright_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	/*** Not adjusted as above ***/
	{{NULL, BE_ST_SC_SPACE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_heal_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
	},
	{{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_nuke_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, 8
	},
	{
#ifdef REFKEEN_VER_CAT3D
	{NULL, BE_ST_SC_B, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
#else
	{NULL, BE_ST_SC_Z, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
#endif
		button_bolt_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, 8
	},
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_back_xpm+5, 22, 22, 8, 2+8
	},
#ifdef REFKEEN_VER_CAT3D
	{{NULL, BE_ST_SC_RSHIFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_run_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 17+8
	},
	{{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_pause_xpm+5, 22, 22, 24+8, 2+8
	},
#else
	{{NULL, BE_ST_SC_TAB, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_quickturn_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 17+8
	},
	{{&g_ingame_altcontrol_mapping_funckeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
		button_function_keys_xpm+5, 22, 22, 24+8, 2+8
	},
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	{{&g_ingame_altcontrol_mapping_scrolls, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
		button_scrolls_xpm+5, 22, 22, 24+8, 26+8
	},
#endif
	{{&g_beStControllerMappingDebugKeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
		button_debug_keys_xpm+5, 22, 22, 8, 26+8
	},
	{0}
};

// This one is incomplete and partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_gameplay = {
	{0},
	g_ingame_altcontrol_mapping_gameplay_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_gameplay_touchmappings,
	{
		{0},
		{0},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
	},
	{
	},
	false
};

#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_scrolls = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_7, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_6, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_8, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_5, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{NULL, BE_ST_SC_1, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_3, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_4, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_2, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	true
};
#endif

#ifdef REFKEEN_VER_CATABYSS
static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_help_onscreentouchcontrols[] = {
	{button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{pad_dpad_xpm+8, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_help_touchmappings[] = {
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		pad_dpad_left_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		pad_dpad_right_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		pad_dpad_up_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		pad_dpad_down_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{0}
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_help = {
	{0},
	g_ingame_altcontrol_mapping_help_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_help_touchmappings,
	{
		// Face buttons
		{0},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	false
};
#endif

#ifdef REFKEEN_VER_CATADVENTURES
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_F3, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F2, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F4, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F1, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_soundoptions = {
	{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_2, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_1, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_3, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_saveoverwriteconfirm = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_Y, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_N, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_RBRACKET, 0, BE_ST_CTRL_MAP_KEYSCANCODE}, // This one is weird, but GE_SaveGame checks this
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_waitforspace = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_SPACE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

// This one is dynamically changed during gameplay
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_keychoice = {
	{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	NULL,
	NULL,
	{
		{0},
		{0},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_intro = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F4, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
#ifdef REFKEEN_VER_CATABYSS
		{NULL, BE_ST_SC_F10, 0, BE_ST_CTRL_MAP_KEYSCANCODE}, // This one depends on game episode and version
#else
		{0},
#endif
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_intro_skillselection = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_N, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_W, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_intro_skillconfirm = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_slidecat_init_onscreentouchcontrols[] = {
	{button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_slidecat_init_touchmappings[] = {
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{0}
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_slidecat_init = {
	{0},
	g_ingame_altcontrol_mapping_slidecat_init_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_slidecat_init_touchmappings,
	{
		// Face buttons
		{0},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	false
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_slidecat = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_P, 0, BE_ST_CTRL_MAP_NONE}, // This depends on the game EXE in use
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{0},
		{0},
		{0},
		// Shoulder buttons
		{NULL, BE_ST_SC_PAGEUP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_PAGEDOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		// D-pad
		{NULL, BE_ST_SC_HOME, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_END, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
		{0},
		{0},
		{0},
		{0},
		// Triggers
		{{0}, {NULL, BE_ST_SC_HOME, 0, BE_ST_CTRL_MAP_KEYSCANCODE}},
		{{0}, {NULL, BE_ST_SC_END, 0, BE_ST_CTRL_MAP_KEYSCANCODE}},
	},
	true
};

#ifdef REFKEEN_VER_CATABYSS
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_printerdialog = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0}
	},
	{
	},
	true
};
#endif // REFKEEN_VER_CATABYSS
#endif // REFKEEN_VER_CATADVENTURES

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback = {
	{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_LCTRL, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_LALT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Shoulder buttons
		{NULL, BE_ST_SC_PAGEUP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_PAGEDOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		// D-pad
		{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
		{0},
		{0},
		{0},
		{0},
		// Triggers
		{{0}, {NULL, BE_ST_SC_HOME, 0, BE_ST_CTRL_MAP_KEYSCANCODE}},
		{{0}, {NULL, BE_ST_SC_END, 0, BE_ST_CTRL_MAP_KEYSCANCODE}},
	},
	false
};

#ifdef REFKEEN_VER_CAT3D
// This one may also be partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	true
};

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_menu_paddle_onscreentouchcontrols[] = {
	{button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{pad_dpad_xpm+8, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_menu_paddle_touchmappings[] = {
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		pad_dpad_left_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		pad_dpad_right_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{0}
};

// Similarly this
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_paddle = {
	{0},
	g_ingame_altcontrol_mapping_menu_paddle_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_menu_paddle_touchmappings,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{0},
		{0},
		{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	false
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_confirm = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_Y, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_N, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
		{0},
	},
	{
	},
	true
};
#endif

static bool CheckButtonMapping(int actionmapping, BE_ST_ControllerSingleMap **outmapptr, const BE_ST_ControllerSingleMap *inputmap)
{
	if ((actionmapping >= 0) && (actionmapping < BE_ST_CTRL_BUT_MAX))
		*outmapptr = &g_ingame_altcontrol_mapping_gameplay.buttons[actionmapping];
	else if ((actionmapping >= BE_ST_CTRL_BUT_MAX) && (actionmapping < BE_ST_CTRL_BUT_MAX+2))
		*outmapptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LTRIGGER+actionmapping-BE_ST_CTRL_BUT_MAX][1];
	else
		return false;

	**outmapptr = *inputmap;
	return true;
}

void RefKeen_PrepareAltControllerScheme(void)
{
	BE_ST_ControllerSingleMap *tempsinglemappingptr;
	BE_ST_ControllerSingleMap tempsinglemapping = {0};
	BE_ST_ControllerSingleMap **currupmappingptr = g_ingame_altcontrol_upmappings;
	BE_ST_ControllerSingleMap **currdownmappingptr = g_ingame_altcontrol_downmappings;
	BE_ST_ControllerSingleMap **currleftmappingptr = g_ingame_altcontrol_leftmappings;
	BE_ST_ControllerSingleMap **currrightmappingptr = g_ingame_altcontrol_rightmappings;

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	tempsinglemapping.val = BE_ST_SC_LCTRL; // Actually adjusted in game
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FIRE], &g_ingame_altcontrol_button0mappings[0], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_LALT; // Actually adjusted in game
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_STRAFE], &g_ingame_altcontrol_button1mappings[0], &tempsinglemapping);
	// HACK: These two buttons overridden by mouse button mappings due to analog motion (hack for The Catacomb Adventures Series)
	if (g_refKeenCfg.altControlScheme.analogMotion)
	{
		if (g_ingame_altcontrol_button0mappings[0])
		{
			g_ingame_altcontrol_button0mappings[0]->mapClass = BE_ST_CTRL_MAP_MOUSEBUTTON;
			g_ingame_altcontrol_button0mappings[0]->val = 1;
			// Game code shouldn't touch this
			g_ingame_altcontrol_button0mappings[0] = NULL;
		}
		if (g_ingame_altcontrol_button1mappings[0])
		{
			g_ingame_altcontrol_button1mappings[0]->mapClass = BE_ST_CTRL_MAP_MOUSEBUTTON;
			g_ingame_altcontrol_button1mappings[0]->val = 2;
			// Game code shouldn't touch this
			g_ingame_altcontrol_button1mappings[0] = NULL;
		}
	}

	BE_ST_ControllerSingleMap gamekeyup    = {NULL, BE_ST_SC_UP,    0, BE_ST_CTRL_MAP_KEYSCANCODE};
	BE_ST_ControllerSingleMap gamekeydown  = {NULL, BE_ST_SC_DOWN,  0, BE_ST_CTRL_MAP_KEYSCANCODE};
	BE_ST_ControllerSingleMap gamekeyleft  = {NULL, BE_ST_SC_LEFT,  0, BE_ST_CTRL_MAP_KEYSCANCODE};
	BE_ST_ControllerSingleMap gamekeyright = {NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE};

	BE_ST_ControllerSingleMap gamemouseup    = {NULL, 1, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemousedown  = {NULL, 1,  16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseleft  = {NULL, 0, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseright = {NULL, 0,  16, BE_ST_CTRL_MAP_MOUSEMOTION};

	// These ones are also adjusted in game
	tempsinglemapping.val = BE_ST_SC_UP;
	if (CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_UP], currupmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currupmappingptr = gamemouseup;
		++currupmappingptr;
	}
	tempsinglemapping.val = BE_ST_SC_DOWN;
	if (CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_DOWN], currdownmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currdownmappingptr = gamemousedown;
		++currdownmappingptr;
	}
	tempsinglemapping.val = BE_ST_SC_LEFT;
	if (CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_LEFT], currleftmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currleftmappingptr = gamemouseleft;
		++currleftmappingptr;
	}
	tempsinglemapping.val = BE_ST_SC_RIGHT;
	if (CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_RIGHT], currrightmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currrightmappingptr = gamemouseright;
		++currrightmappingptr;
	}

	tempsinglemapping.val = BE_ST_SC_SPACE;
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_DRINK], &tempsinglemappingptr, &tempsinglemapping);
#ifdef REFKEEN_VER_CAT3D
	tempsinglemapping.val = BE_ST_SC_B;
#else
	tempsinglemapping.val = BE_ST_SC_Z;
#endif
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_BOLT], &tempsinglemappingptr, &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_ENTER;
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_NUKE], &tempsinglemappingptr, &tempsinglemapping);
#ifdef REFKEEN_VER_CAT3D
	tempsinglemapping.val = BE_ST_SC_RSHIFT;
#else
	tempsinglemapping.val = BE_ST_SC_TAB;
#endif
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FASTTURN], &tempsinglemappingptr, &tempsinglemapping);

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_scrolls;
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_SCROLLS], &tempsinglemappingptr, &tempsinglemapping);
#endif
#ifdef REFKEEN_VER_CATADVENTURES
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_funckeys;
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FUNCKEYS], &tempsinglemappingptr, &tempsinglemapping);
#endif
	tempsinglemapping.otherMappingPtr = &g_beStControllerMappingDebugKeys;
	CheckButtonMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_DEBUGKEYS], &tempsinglemappingptr, &tempsinglemapping);

	// HACK (leave US_LineInput this way)
	g_beStControllerMappingTextInput.defaultMapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_beStControllerMappingTextInput.defaultMapping.val = BE_ST_SC_ESC;
	// HACK (hide debug keys this way)
	g_beStControllerMappingDebugKeys.defaultMapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	g_beStControllerMappingDebugKeys.defaultMapping.otherMappingPtr = &g_ingame_altcontrol_mapping_gameplay;

	if (g_refKeenCfg.altControlScheme.useLeftStick)
	{
		*currupmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LY][0];
		*currdownmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LY][1];
		*currleftmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LX][0];
		*currrightmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LX][1];
		if (g_refKeenCfg.altControlScheme.analogMotion)
		{
			**currupmappingptr++ = gamemouseup;
			**currdownmappingptr++ = gamemousedown;
			**currleftmappingptr++ = gamemouseleft;
			**currrightmappingptr++ = gamemouseright;
		}
		else
		{
			**currupmappingptr++ = gamekeyup;
			**currdownmappingptr++ = gamekeydown;
			**currleftmappingptr++ = gamekeyleft;
			**currrightmappingptr++ = gamekeyright;
		}
	}
	if (g_refKeenCfg.altControlScheme.useRightStick)
	{
		*currupmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][0];
		*currdownmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][1];
		*currleftmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][0];
		*currrightmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][1];
		if (g_refKeenCfg.altControlScheme.analogMotion)
		{
			**currupmappingptr++ = gamemouseup;
			**currdownmappingptr++ = gamemousedown;
			**currleftmappingptr++ = gamemouseleft;
			**currrightmappingptr++ = gamemouseright;
		}
		else
		{
			**currupmappingptr++ = gamekeyup;
			**currdownmappingptr++ = gamekeydown;
			**currleftmappingptr++ = gamekeyleft;
			**currrightmappingptr++ = gamekeyright;
		}
	}
	// Game code shouldn't touch these
	if (g_refKeenCfg.altControlScheme.analogMotion)
	{
		g_ingame_altcontrol_upmappings[0] = NULL;
		g_ingame_altcontrol_downmappings[0] = NULL;
		g_ingame_altcontrol_leftmappings[0] = NULL;
		g_ingame_altcontrol_rightmappings[0] = NULL;
	}

#ifdef REFKEEN_VER_CAT3D
	BE_ST_ControllerSingleMap menumouseup    = {NULL, 1, -4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap menumousedown  = {NULL, 1,  4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap menumouseleft  = {NULL, 0, -4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap menumouseright = {NULL, 0,  4, BE_ST_CTRL_MAP_MOUSEMOTION};

	if (g_refKeenCfg.altControlScheme.useLeftStick)
	{
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][0] = menumouseup;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][1] = menumousedown;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][0] = menumouseleft;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][1] = menumouseright;

		if (g_refKeenCfg.altControlScheme.analogMotion)
		{
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_LX][0] = gamemouseleft;
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_LX][1] = gamemouseright;
		}
		else
		{
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_altcontrol_mapping_menu_paddle.buttons[BE_ST_CTRL_BUT_DPAD_LEFT];
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_altcontrol_mapping_menu_paddle.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
		}
	}
	if (g_refKeenCfg.altControlScheme.useRightStick)
	{
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][0] = menumouseup;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][1] = menumousedown;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][0] = menumouseleft;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][1] = menumouseright;

		if (g_refKeenCfg.altControlScheme.analogMotion)
		{
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_RX][0] = gamemouseleft;
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_RX][1] = gamemouseright;
		}
		else
		{
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_RX][0] = g_ingame_altcontrol_mapping_menu_paddle.buttons[BE_ST_CTRL_BUT_DPAD_LEFT];
			g_ingame_altcontrol_mapping_menu_paddle.axes[BE_ST_CTRL_AXIS_RX][1] = g_ingame_altcontrol_mapping_menu_paddle.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
		}
	}
#endif

	// Version-specific patches
#ifdef REFKEEN_VER_CATABYSS
	g_ingame_altcontrol_mapping_intro.buttons[BE_ST_CTRL_BUT_Y].mapClass = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? BE_ST_CTRL_MAP_KEYSCANCODE : BE_ST_CTRL_MAP_NONE;
	g_ingame_altcontrol_mapping_slidecat.buttons[BE_ST_CTRL_BUT_X].mapClass = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? BE_ST_CTRL_MAP_KEYSCANCODE : BE_ST_CTRL_MAP_NONE;
#endif

	// Init touch controls UI
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_gameplay.onScreenTouchControls);
#ifdef REFKEEN_VER_CATADVENTURES
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_slidecat_init.onScreenTouchControls);
#ifdef REFKEEN_VER_CATABYSS
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_help.onScreenTouchControls);
#endif
#endif

#ifdef REFKEEN_VER_CAT3D
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_menu_paddle.onScreenTouchControls);
#endif
}

void UpdateAltControllerMappingsByMousePresence(bool withmouse)
{
	g_ingame_altcontrol_mapping_gameplay.grabMouse = withmouse;
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	g_ingame_altcontrol_mapping_scrolls.grabMouse = withmouse;
#endif
#ifdef REFKEEN_VER_CATADVENTURES
	g_ingame_altcontrol_mapping_funckeys.grabMouse = withmouse;
#endif
#ifdef REFKEEN_VER_CAT3D
	g_ingame_altcontrol_mapping_menu_paddle.grabMouse = withmouse;
#endif
}

void PrepareGamePlayControllerMapping(void)
{
#if 0 // TODO: Let's try disabling this for now
	BE_ST_ControllerSingleMap **singlemappingptr;

	for (singlemappingptr = g_ingame_altcontrol_button0mappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = KbdDefs[0].button0;
	for (singlemappingptr = g_ingame_altcontrol_button1mappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = KbdDefs[0].button1;
	for (singlemappingptr = g_ingame_altcontrol_upmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = KbdDefs[0].up;
	for (singlemappingptr = g_ingame_altcontrol_downmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = KbdDefs[0].down;
	for (singlemappingptr = g_ingame_altcontrol_leftmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = KbdDefs[0].left;
	for (singlemappingptr = g_ingame_altcontrol_rightmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = KbdDefs[0].right;
	// HACK - The indices are hardcoded
	g_ingame_altcontrol_mapping_gameplay.touchMappings[0].mappings[0].val = KbdDefs[0].button0;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[1].mappings[0].val = KbdDefs[0].button1;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[2].mappings[0].val = KbdDefs[0].left;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[3].mappings[0].val = KbdDefs[0].right;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[4].mappings[0].val = KbdDefs[0].up;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[5].mappings[0].val = KbdDefs[0].down;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[6].mappings[0].val = KbdDefs[0].upleft;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[7].mappings[0].val = KbdDefs[0].upright;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[8].mappings[0].val = KbdDefs[0].downleft;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[9].mappings[0].val = KbdDefs[0].downright;
#endif

	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}

REFKEEN_NS_E
