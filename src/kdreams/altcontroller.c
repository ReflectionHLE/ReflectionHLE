/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2015-2022 NY00123
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

int g_binding_value_button[2],
    g_binding_value_motionx, g_binding_value_motiony;

bool g_keybind_used_button[2],
     g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right;

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys;

// Using macros for static variable definitions in compound structures,
// along with separate mapping variables used elsewhere
#define BUT_JUMP_MAP       &g_binding_value_button[0], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_THROW_MAP      &g_binding_value_button[1], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_STATS_MAP      NULL, BE_ST_SC_SPACE, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_UP_MAP         &g_binding_value_motiony, 0, -127, BE_ST_CTRL_MAP_VALUESET
#define BUT_DOWN_MAP       &g_binding_value_motiony, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_LEFT_MAP       &g_binding_value_motionx, 0, -127, BE_ST_CTRL_MAP_VALUESET
#define BUT_RIGHT_MAP      &g_binding_value_motionx, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_BACK_MAP       NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_PAUSE_MAP      NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_FUNC_KEYS_MAP  &g_ingame_altcontrol_mapping_funckeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING
#define BUT_DEBUG_KEYS_MAP &g_beStControllerMappingDebugKeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING

static const BE_ST_ControllerSingleMap
       g_ingame_but_jump_map       = {BUT_JUMP_MAP},
       g_ingame_but_throw_map      = {BUT_THROW_MAP},
       g_ingame_but_stats_map      = {BUT_STATS_MAP},
       g_ingame_but_up_map         = {BUT_UP_MAP},
       g_ingame_but_down_map       = {BUT_DOWN_MAP},
       g_ingame_but_left_map       = {BUT_LEFT_MAP},
       g_ingame_but_right_map      = {BUT_RIGHT_MAP},
       g_ingame_but_func_keys_map  = {BUT_FUNC_KEYS_MAP},
       g_ingame_but_debug_keys_map = {BUT_DEBUG_KEYS_MAP},
       g_ingame_mouse_up_map       = {NULL, 1, -4, BE_ST_CTRL_MAP_MOUSEMOTION},
       g_ingame_mouse_down_map     = {NULL, 1,  4, BE_ST_CTRL_MAP_MOUSEMOTION},
       g_ingame_mouse_left_map     = {NULL, 0, -4, BE_ST_CTRL_MAP_MOUSEMOTION},
       g_ingame_mouse_right_map    = {NULL, 0,  4, BE_ST_CTRL_MAP_MOUSEMOTION};


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
	{{BUT_JUMP_MAP},
	 button_jump_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8
	},
	{{BUT_THROW_MAP},
	 button_shoot_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8
	},
	{{BUT_LEFT_MAP},
	 analog_circle_left_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_RIGHT_MAP},
	 analog_circle_right_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_UP_MAP},
	 analog_circle_top_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_DOWN_MAP},
	 analog_circle_bottom_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_UP_MAP,
	  BUT_LEFT_MAP},
	 analog_circle_topleft_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_UP_MAP,
	  BUT_RIGHT_MAP},
	 analog_circle_topright_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_DOWN_MAP,
	  BUT_LEFT_MAP},
	 analog_circle_bottomleft_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_DOWN_MAP,
	  BUT_RIGHT_MAP},
	 analog_circle_bottomright_input_xpm + 5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 48 - 8
	},
	{{BUT_STATS_MAP},
	 button_stats_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 56 - 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 39 - 8
	},
	{{BUT_BACK_MAP},
	 button_back_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 22 - 8, 2 + 8
	},
	{{BUT_PAUSE_MAP},
	 button_pause_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 46 - 8, 2 + 8
	},
	{{BUT_FUNC_KEYS_MAP},
	 button_function_keys_xpm + 5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM - 46 - 8, 26 + 8
	},
	{{BUT_DEBUG_KEYS_MAP},
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
	},
	{
	},
	{
		{0},
		{0},
		{0},
		{0},
		// Back button
		{BUT_BACK_MAP},
		{0},
		// Start button
		{BUT_PAUSE_MAP},
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
	},
	{
	},
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
	},
	{
	},
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
	},
	{
	},
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
	},
	{
	},
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
	},
	{
	},
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
	},
	{
	},
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
	},
	{
	},
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

static void CheckKeyMapping(int actionmapping, const BE_ST_ControllerSingleMap *inputmap, bool *indicator)
{
	if (actionmapping > 0)
	{
		g_ingame_altcontrol_mapping_gameplay.keys[actionmapping].map = *inputmap;
		g_ingame_altcontrol_mapping_gameplay.keys[actionmapping].indicator = indicator;
	}
}

static void CheckMouseMapping(int actionmapping, const BE_ST_ControllerSingleMap *inputmap)
{
	if ((actionmapping >= 0) && (actionmapping < BE_ST_CTRL_MOUSE_BUT_MAX))
		g_ingame_altcontrol_mapping_gameplay.mbuttons[actionmapping] = *inputmap;
}

static void CheckPadMapping(int actionmapping, const BE_ST_ControllerSingleMap *inputmap)
{
	if (BE_ST_IsValidPadButton(actionmapping))
		g_ingame_altcontrol_mapping_gameplay.pbuttons[actionmapping] = *inputmap;
	else if (BE_ST_IsValidPadAxis(actionmapping))
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_LTRIGGER+actionmapping-BE_ST_CTRL_BUT_MAX][1] = *inputmap;
}

static void CheckNonKeyMappings(int bind, const BE_ST_ControllerSingleMap *inputmap)
{
	CheckMouseMapping(g_refKeenCfg.kdreams.binds[bind].mouse, inputmap);
	CheckPadMapping(g_refKeenCfg.kdreams.binds[bind].pad, inputmap);
}

static void CheckMappings(int bind, const BE_ST_ControllerSingleMap *inputmap, bool *keyindicator)
{
	CheckKeyMapping(g_refKeenCfg.kdreams.binds[bind].key, inputmap, keyindicator);
	CheckNonKeyMappings(bind, inputmap);
}

void RefKeen_PrepareAltControllerScheme(void)
{
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_JUMP, &g_ingame_but_jump_map, &g_keybind_used_button[0]);
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_THROW, &g_ingame_but_throw_map ,&g_keybind_used_button[1]);
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_STATS, &g_ingame_but_stats_map, 0);
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_UP, &g_ingame_but_up_map, &g_keybind_used_up);
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_DOWN, &g_ingame_but_down_map, &g_keybind_used_down);
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_LEFT, &g_ingame_but_left_map, &g_keybind_used_left);
	CheckMappings(BE_ST_CTRL_BIND_KDREAMS_RIGHT, &g_ingame_but_right_map, &g_keybind_used_right);
	CheckNonKeyMappings(BE_ST_CTRL_BIND_KDREAMS_FUNCKEYS, &g_ingame_but_func_keys_map);
	CheckNonKeyMappings(BE_ST_CTRL_BIND_KDREAMS_DEBUGKEYS, &g_ingame_but_debug_keys_map);

	// HACK (leave US_LineInput this way)
	g_beStControllerMappingTextInput.defaultMapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_beStControllerMappingTextInput.defaultMapping.val = BE_ST_SC_ESC;
	// HACK (hide debug keys this way)
	g_beStControllerMappingDebugKeys.defaultMapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	g_beStControllerMappingDebugKeys.defaultMapping.miscPtr = &g_ingame_altcontrol_mapping_gameplay;

	if (g_refKeenCfg.kdreams.useLeftStick)
	{
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_LY][0] = g_ingame_but_up_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_LY][1] = g_ingame_but_down_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_but_left_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_but_right_map;

		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LY][0] = g_ingame_mouse_up_map;
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LY][1] = g_ingame_mouse_down_map;
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_mouse_left_map;
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_mouse_right_map;
	}

	if (g_refKeenCfg.kdreams.useRightStick)
	{
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_RY][0] = g_ingame_but_up_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_RY][1] = g_ingame_but_down_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_RX][0] = g_ingame_but_left_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[BE_ST_CTRL_AXIS_RX][1] = g_ingame_but_right_map;

		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RY][0] = g_ingame_mouse_up_map;
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RY][1] = g_ingame_mouse_down_map;
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RX][0] = g_ingame_mouse_left_map;
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RX][1] = g_ingame_mouse_right_map;
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
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}
