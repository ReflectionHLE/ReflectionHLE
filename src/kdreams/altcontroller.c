/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2015-2020 NY00123
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

#include "../rsrc/button_confirm.xpm"
#include "../rsrc/button_back.xpm"
#include "../rsrc/button_debug_keys.xpm"
#include "../rsrc/button_function_keys.xpm"
#include "../rsrc/button_jump.xpm"
#include "../rsrc/button_shoot.xpm"
#include "../rsrc/button_pause.xpm"
#include "../rsrc/button_stats.xpm"
#include "../rsrc/button_pageup.xpm"
#include "../rsrc/button_pagedown.xpm"

#include "../rsrc/pad_dpad.xpm"
#include "../rsrc/pad_dpad_left_input.xpm"
#include "../rsrc/pad_dpad_right_input.xpm"
#include "../rsrc/pad_dpad_up_input.xpm"
#include "../rsrc/pad_dpad_down_input.xpm"

#include "../rsrc/analog_circle.xpm"
#include "../rsrc/analog_circle_left_input.xpm"
#include "../rsrc/analog_circle_right_input.xpm"
#include "../rsrc/analog_circle_top_input.xpm"
#include "../rsrc/analog_circle_bottom_input.xpm"
#include "../rsrc/analog_circle_topleft_input.xpm"
#include "../rsrc/analog_circle_topright_input.xpm"
#include "../rsrc/analog_circle_bottomleft_input.xpm"
#include "../rsrc/analog_circle_bottomright_input.xpm"

BE_ST_ControllerSingleMap *g_ingame_altcontrol_button0mappings[2], *g_ingame_altcontrol_button1mappings[2],
	*g_ingame_altcontrol_upmappings[4], *g_ingame_altcontrol_downmappings[4], *g_ingame_altcontrol_leftmappings[4], *g_ingame_altcontrol_rightmappings[4];

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys;

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_gameplay_onscreentouchcontrols[] = {
	{button_jump_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8},
	{button_shoot_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8},
	{button_debug_keys_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 26 + 8},
	{button_stats_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 56 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8},
	{button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 2 + 8},
	{button_pause_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 46 - 8, 2 + 8},
	{analog_circle_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8},
	{button_function_keys_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 46 - 8, 26 + 8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_gameplay_touchmappings[] = {
	/*** Actually adjusted in-game ***/
	{{NULL, BE_ST_SC_LCTRL, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_jump_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8
	},
	{{NULL, BE_ST_SC_LALT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_shoot_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8
	},
	{{NULL, BE_ST_SC_LEFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_left_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_right_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_top_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_bottom_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_HOME, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_topleft_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_PAGEUP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_topright_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_END, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_bottomleft_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_PAGEDOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 analog_circle_bottomright_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	/*** Not adjusted as above ***/
	{{NULL, BE_ST_SC_SPACE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_stats_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 56 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8
	},
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 2 + 8
	},
	{{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_pause_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 46 - 8, 2 + 8
	},
	{{&g_ingame_altcontrol_mapping_funckeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	 button_function_keys_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 46 - 8, 26 + 8
	},
	{{&g_beStControllerMappingDebugKeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	 button_debug_keys_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 26 + 8
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

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_F6, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F7, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{NULL, BE_ST_SC_F1, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F3, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F5, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F2, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	true
};

// Used only in EGA mode while running out of memory, and it should never
// happen anyway (unless a few changes are done)
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_notenoughmemorytostart = {
	{0},
	NULL,
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_Q, 0, BE_ST_CTRL_MAP_KEYSCANCODE}, // Quit
		{NULL, BE_ST_SC_C, 0, BE_ST_CTRL_MAP_KEYSCANCODE}, // Continue
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

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_simpledialog_onscreentouchcontrols[] = {
	{button_confirm_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8},
	{button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8},
	{pad_dpad_xpm + 8, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_simpledialog_touchmappings[] = {
	{{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_confirm_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8
	},
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8
	},
	{{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 pad_dpad_up_input_xpm + 4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 pad_dpad_down_input_xpm + 4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{0}
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_simpledialog = {
	{0},
	g_ingame_altcontrol_mapping_simpledialog_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_simpledialog_touchmappings,
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
	false
};

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_demoloop_onscreentouchcontrols[] = {
	{button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_demoloop_touchmappings[] = {
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8
	},
	{0}
};

// This one exists for the sake of showing the "Esc" touch button,
// for consistency with the menu and usage of the "Esc" key under DOS.
// Based on "inackback" mapping.
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_demoloop = {
	{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	g_ingame_altcontrol_mapping_demoloop_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_demoloop_touchmappings,
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

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_menu_onscreentouchcontrols_keyboardemu[] = {
	{button_confirm_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8},
	{pad_dpad_xpm+8, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_menu_touchmappings_keyboardemu[] = {
	{{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_confirm_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
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

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_menu_onscreentouchcontrols_mouseemu[] = {
	{button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_menu_touchmappings_mouseemu[] = {
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{0}
};

// This one may also be partially filled on startup, for two purposes:
// - Game controller mappings.
// - Keyboard vs mouse emulation for touch input.
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
	false
};

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_menu_help_onscreentouchcontrols[] = {
	{button_pageup_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 8},
	{button_pagedown_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 26 + 8},
	{button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8},
	{pad_dpad_xpm + 8, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_menu_help_touchmappings[] = {
	{{NULL, BE_ST_SC_PAGEUP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_pageup_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 8
	},
	{{NULL, BE_ST_SC_PAGEDOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_pagedown_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 26 + 8
	},
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8
	},
	{{NULL, BE_ST_SC_UP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 pad_dpad_up_input_xpm + 4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{NULL, BE_ST_SC_DOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	 pad_dpad_down_input_xpm + 4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{0}
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_help = {
	{0},
	g_ingame_altcontrol_mapping_menu_help_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_menu_help_touchmappings,
	{
		// Face buttons
		{0},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_PAGEDOWN, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_PAGEUP, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
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
	},
	false
};

static bool CheckButtonMapping(int actionmapping, BE_ST_ControllerSingleMap **outmapptr, const BE_ST_ControllerSingleMap *inputmap)
{
	if ((actionmapping >= 0) && (actionmapping < BE_ST_CTRL_BUT_MAX))
		*outmapptr = &g_ingame_altcontrol_mapping_gameplay.buttons[actionmapping];
	else if ((actionmapping >= BE_ST_CTRL_BUT_MAX) && (BE_ST_CTRL_BUT_MAX < BE_ST_CTRL_BUT_MAX+2))
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
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_JUMP], &g_ingame_altcontrol_button0mappings[0], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_LALT; // Actually adjusted in game
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_THROW], &g_ingame_altcontrol_button1mappings[0], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_SPACE;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STATS], &tempsinglemappingptr, &tempsinglemapping);
	// These ones are also adjusted in game
	tempsinglemapping.val = BE_ST_SC_UP;
	if (CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_UP], currupmappingptr, &tempsinglemapping))
		++currupmappingptr;
	tempsinglemapping.val = BE_ST_SC_DOWN;
	if (CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DOWN], currdownmappingptr, &tempsinglemapping))
		++currdownmappingptr;
	tempsinglemapping.val = BE_ST_SC_LEFT;
	if (CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_LEFT], currleftmappingptr, &tempsinglemapping))
		++currleftmappingptr;
	tempsinglemapping.val = BE_ST_SC_RIGHT;
	if (CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RIGHT], currrightmappingptr, &tempsinglemapping))
		++currrightmappingptr;

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_funckeys;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS], &tempsinglemappingptr, &tempsinglemapping);
	tempsinglemapping.otherMappingPtr = &g_beStControllerMappingDebugKeys;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS], &tempsinglemappingptr, &tempsinglemapping);

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
		(*currupmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
		(*currdownmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
		(*currleftmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
		(*currrightmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	}
	if (g_refKeenCfg.altControlScheme.useRightStick)
	{
		*currupmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][0];
		*currdownmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][1];
		*currleftmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][0];
		*currrightmappingptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][1];
		(*currupmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
		(*currdownmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
		(*currleftmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
		(*currrightmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	}

	BE_ST_ControllerSingleMap mouseup    = {NULL, 1, -4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap mousedown  = {NULL, 1,  4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap mouseleft  = {NULL, 0, -4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap mouseright = {NULL, 0,  4, BE_ST_CTRL_MAP_MOUSEMOTION};

	if (g_refKeenCfg.altControlScheme.useLeftStick)
	{
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][0] = mouseup;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][1] = mousedown;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][0] = mouseleft;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][1] = mouseright;
	}
	if (g_refKeenCfg.altControlScheme.useRightStick)
	{
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][0] = mouseup;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][1] = mousedown;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][0] = mouseleft;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][1] = mouseright;
	}

	// Init touch controls UI (excludes UI picked in FinalizeControlPanelMappingsByMousePresence)
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_gameplay.onScreenTouchControls);
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_simpledialog.onScreenTouchControls);
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_demoloop.onScreenTouchControls);
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_menu_help.onScreenTouchControls);
}

void FinalizeControlPanelMappingsByMousePresence(bool withmouse)
{
	g_ingame_altcontrol_mapping_menu.grabMouse = withmouse;
	g_ingame_altcontrol_mapping_menu_help.grabMouse = withmouse;

	if (withmouse)
	{
		g_ingame_altcontrol_mapping_menu.onScreenTouchControls = g_ingame_altcontrol_mapping_menu_onscreentouchcontrols_mouseemu;
		g_ingame_altcontrol_mapping_menu.touchMappings = g_ingame_altcontrol_mapping_menu_touchmappings_mouseemu;
	}
	else
	{
		g_ingame_altcontrol_mapping_menu.onScreenTouchControls = g_ingame_altcontrol_mapping_menu_onscreentouchcontrols_keyboardemu;
		g_ingame_altcontrol_mapping_menu.touchMappings = g_ingame_altcontrol_mapping_menu_touchmappings_keyboardemu;
	}
	g_ingame_altcontrol_mapping_menu.absoluteFingerPositioning = withmouse;
	// Init touch control UIs for this
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_menu.onScreenTouchControls);
}

void UpdateGameplayMappingsByMousePresence(bool withmouse)
{
	g_ingame_altcontrol_mapping_gameplay.grabMouse = withmouse;
	g_ingame_altcontrol_mapping_funckeys.grabMouse = withmouse;
}

void PrepareGamePlayControllerMapping(void)
{
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
	g_ingame_altcontrol_mapping_gameplay.touchMappings[0].mapping.val = KbdDefs[0].button0;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[1].mapping.val = KbdDefs[0].button1;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[2].mapping.val = KbdDefs[0].left;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[3].mapping.val = KbdDefs[0].right;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[4].mapping.val = KbdDefs[0].up;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[5].mapping.val = KbdDefs[0].down;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[6].mapping.val = KbdDefs[0].upleft;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[7].mapping.val = KbdDefs[0].upright;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[8].mapping.val = KbdDefs[0].downleft;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[9].mapping.val = KbdDefs[0].downright;

	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}
