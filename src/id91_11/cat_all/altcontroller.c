/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 * Copyright (C) 2015-2024 NY00123
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

int g_binding_value_button[2],
    g_binding_value_axisx, g_binding_value_axisy,
    g_binding_value_up, g_binding_value_down, g_binding_value_left, g_binding_value_right,
    g_binding_value_drink, g_binding_value_bolt, g_binding_value_nuke, g_binding_value_fastturn;

bool g_keybind_used_button[2],
     g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right,
     g_keybind_used_drink, g_keybind_used_bolt, g_keybind_used_nuke, g_keybind_used_fastturn;

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_scrolls;
extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys;

// Using macros for static variable definitions in compound structures,
// along with separate mapping variables used elsewhere
#define BUT_FIRE_MAP       &g_binding_value_button[0], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_STRAFE_MAP     &g_binding_value_button[1], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_UP_MAP         &g_binding_value_up, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_DOWN_MAP       &g_binding_value_down, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_LEFT_MAP       &g_binding_value_left, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_RIGHT_MAP      &g_binding_value_right, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define AXIS_UP_MAP        &g_binding_value_axisy, 0, -127, BE_ST_CTRL_MAP_VALUESET
#define AXIS_DOWN_MAP      &g_binding_value_axisy, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define AXIS_LEFT_MAP      &g_binding_value_axisx, 0, -127, BE_ST_CTRL_MAP_VALUESET
#define AXIS_RIGHT_MAP     &g_binding_value_axisx, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_BACK_MAP       NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_DRINK_MAP      &g_binding_value_drink, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_BOLT_MAP       &g_binding_value_bolt, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_NUKE_MAP       &g_binding_value_nuke, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_FASTTURN_MAP   &g_binding_value_fastturn, 0, 127, BE_ST_CTRL_MAP_VALUESET

#define BUT_FUNC_KEYS_MAP  &g_ingame_altcontrol_mapping_funckeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING
#define BUT_PAUSE_MAP      NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_SCROLLS_MAP    &g_ingame_altcontrol_mapping_scrolls, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING
#define BUT_DEBUG_KEYS_MAP &g_beStControllerMappingDebugKeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING

static const BE_ST_ControllerSingleMap
       g_ingame_but_fire_map       = {BUT_FIRE_MAP},
       g_ingame_but_strafe_map     = {BUT_STRAFE_MAP},
       g_ingame_but_up_map         = {BUT_UP_MAP},
       g_ingame_but_down_map       = {BUT_DOWN_MAP},
       g_ingame_but_left_map       = {BUT_LEFT_MAP},
       g_ingame_but_right_map      = {BUT_RIGHT_MAP},
       g_ingame_axis_up_map        = {AXIS_UP_MAP},
       g_ingame_axis_down_map      = {AXIS_DOWN_MAP},
       g_ingame_axis_left_map      = {AXIS_LEFT_MAP},
       g_ingame_axis_right_map     = {AXIS_RIGHT_MAP},
       g_ingame_but_drink_map      = {BUT_DRINK_MAP},
       g_ingame_but_nuke_map       = {BUT_NUKE_MAP},
       g_ingame_but_bolt_map       = {BUT_BOLT_MAP},
       g_ingame_but_fastturn_map   = {BUT_FASTTURN_MAP},
#ifdef REFKEEN_VER_CATADVENTURES
       g_ingame_but_func_keys_map  = {BUT_FUNC_KEYS_MAP},
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
       g_ingame_but_scrolls_map    = {BUT_SCROLLS_MAP},
#endif
       g_ingame_but_debug_keys_map = {BUT_DEBUG_KEYS_MAP};

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
	{{BUT_FIRE_MAP},
		button_shoot_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8
	},
	{{BUT_STRAFE_MAP},
		button_strafe_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
	},
	{{BUT_LEFT_MAP},
		analog_circle_left_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_RIGHT_MAP},
		analog_circle_right_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_UP_MAP},
		analog_circle_top_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_DOWN_MAP},
		analog_circle_bottom_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_UP_MAP,
	  BUT_LEFT_MAP},
		analog_circle_topleft_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_UP_MAP,
	  BUT_RIGHT_MAP},
		analog_circle_topright_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_DOWN_MAP,
	  BUT_LEFT_MAP},
		analog_circle_bottomleft_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_DOWN_MAP,
	  BUT_RIGHT_MAP},
		analog_circle_bottomright_input_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_DRINK_MAP},
		button_heal_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
	},
	{{BUT_NUKE_MAP},
		button_nuke_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, 8
	},
	{{BUT_BOLT_MAP},
		button_bolt_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, 8
	},
	{{BUT_BACK_MAP},
		button_back_xpm+5, 22, 22, 8, 2+8
	},
#ifdef REFKEEN_VER_CAT3D
	{{BUT_FASTTURN_MAP},
		button_run_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 17+8
	},
	{{BUT_PAUSE_MAP},
		button_pause_xpm+5, 22, 22, 24+8, 2+8
	},
#else
	{{BUT_FASTTURN_MAP},
		button_quickturn_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 17+8
	},
	{{BUT_FUNC_KEYS_MAP},
		button_function_keys_xpm+5, 22, 22, 24+8, 2+8
	},
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	{{BUT_SCROLLS_MAP},
		button_scrolls_xpm+5, 22, 22, 24+8, 26+8
	},
#endif
	{{BUT_DEBUG_KEYS_MAP},
		button_debug_keys_xpm+5, 22, 22, 8, 26+8
	},
	{0}
};

// This one is incomplete and partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_gameplay = {
	{0},
	g_ingame_altcontrol_mapping_gameplay_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_gameplay_touchmappings,
	{0},
	{0},
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
	{0},
	false
};

#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_scrolls = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
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
	{{BUT_LEFT_MAP},
		pad_dpad_left_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_RIGHT_MAP},
		pad_dpad_right_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_UP_MAP},
		pad_dpad_up_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{{BUT_DOWN_MAP},
		pad_dpad_down_input_xpm+4, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8
	},
	{0}
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_help = {
	{0},
	g_ingame_altcontrol_mapping_help_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_help_touchmappings,
	{0},
	{0},
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
		{BUT_UP_MAP},
		{BUT_DOWN_MAP},
		{BUT_LEFT_MAP},
		{BUT_RIGHT_MAP},
	},
	{0},
	false
};
#endif

#ifdef REFKEEN_VER_CATADVENTURES
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_soundoptions = {
	{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_saveoverwriteconfirm = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_waitforspace = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

// This one is dynamically changed during gameplay
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_keychoice = {
	{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_intro = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_intro_skillselection = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_intro_skillconfirm = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
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
	{0},
	{0},
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
	{0},
	false
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_slidecat = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	{0},
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
	{0},
	true
};
#endif // REFKEEN_VER_CATABYSS
#endif // REFKEEN_VER_CATADVENTURES

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback = {
	{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	{0},
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
	{0},
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
	{0},
	{0},
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
	{0},
	false
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_confirm = {
	{0},
	NULL,
	NULL,
	{0},
	{0},
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
	{0},
	true
};
#endif

static void SetMappingsForAxes(int x, int y)
{
	if (g_refKeenCfg.cat3d.analogMotion)
	{
		g_ingame_altcontrol_mapping_gameplay.paxes[y][0] = g_ingame_axis_up_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[y][1] = g_ingame_axis_down_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[x][0] = g_ingame_axis_left_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[x][1] = g_ingame_axis_right_map;
	}
	else
	{
		g_ingame_altcontrol_mapping_gameplay.paxes[y][0] = g_ingame_but_up_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[y][1] = g_ingame_but_down_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[x][0] = g_ingame_but_left_map;
		g_ingame_altcontrol_mapping_gameplay.paxes[x][1] = g_ingame_but_right_map;
	}
}

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
	CheckMouseMapping(g_refKeenCfg.cat3d.binds[bind].mouse, inputmap);
	CheckPadMapping(g_refKeenCfg.cat3d.binds[bind].pad, inputmap);
}

static void CheckMappings(int bind, const BE_ST_ControllerSingleMap *inputmap, bool *keyindicator)
{
	CheckKeyMapping(g_refKeenCfg.cat3d.binds[bind].key, inputmap, keyindicator);
	CheckNonKeyMappings(bind, inputmap);
}

void RefKeen_PrepareAltControllerScheme(void)
{
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_FIRE, &g_ingame_but_fire_map, &g_keybind_used_button[0]);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_STRAFE, &g_ingame_but_strafe_map, &g_keybind_used_button[1]);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_UP, &g_ingame_but_up_map, &g_keybind_used_up);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_DOWN, &g_ingame_but_down_map, &g_keybind_used_down);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_LEFT, &g_ingame_but_left_map, &g_keybind_used_left);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_RIGHT, &g_ingame_but_right_map, &g_keybind_used_right);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_DRINK, &g_ingame_but_drink_map, &g_keybind_used_drink);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_BOLT, &g_ingame_but_bolt_map, &g_keybind_used_bolt);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_NUKE, &g_ingame_but_nuke_map, &g_keybind_used_nuke);
	CheckMappings(BE_ST_CTRL_BIND_CAT3D_FASTTURN, &g_ingame_but_fastturn_map, &g_keybind_used_fastturn);
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
  #ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	CheckNonKeyMappings(BE_ST_CTRL_BIND_CAT3D_SCROLLS, &g_ingame_but_scrolls_map);
  #else
	CheckPadMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_SCROLLS].pad,
	                &g_ingame_but_scrolls_map);
  #endif
#endif
#ifdef REFKEEN_VER_CATADVENTURES
  #ifdef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS
	CheckNonKeyMappings(BE_ST_CTRL_BIND_CAT3D_FUNCKEYS, &g_ingame_but_func_keys_map);
  #else
	CheckPadMapping(g_refKeenCfg.cat3d.binds[BE_ST_CTRL_BIND_CAT3D_FUNCKEYS].pad,
	                &g_ingame_but_func_keys_map);
  #endif
#endif
	CheckNonKeyMappings(BE_ST_CTRL_BIND_CAT3D_DEBUGKEYS, &g_ingame_but_debug_keys_map);

	// HACK (leave US_LineInput this way)
	g_beStControllerMappingTextInput.defaultMapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_beStControllerMappingTextInput.defaultMapping.val = BE_ST_SC_ESC;
	// HACK (hide debug keys this way)
	g_beStControllerMappingDebugKeys.defaultMapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	g_beStControllerMappingDebugKeys.defaultMapping.miscPtr = &g_ingame_altcontrol_mapping_gameplay;

	if (g_refKeenCfg.cat3d.useLeftStick)
		SetMappingsForAxes(BE_ST_CTRL_AXIS_LX, BE_ST_CTRL_AXIS_LY);
	if (g_refKeenCfg.cat3d.useRightStick)
		SetMappingsForAxes(BE_ST_CTRL_AXIS_RX, BE_ST_CTRL_AXIS_RY);

#ifdef REFKEEN_VER_CAT3D
	if (g_refKeenCfg.cat3d.useLeftStick)
	{
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LY][0] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_UP];
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LY][1] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_DOWN];
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_RIGHT];

		g_ingame_altcontrol_mapping_menu_paddle.paxes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_altcontrol_mapping_menu_paddle.pbuttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_mapping_menu_paddle.paxes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_altcontrol_mapping_menu_paddle.pbuttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
	}
	if (g_refKeenCfg.cat3d.useRightStick)
	{
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RY][0] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_UP];
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RY][1] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_DOWN];
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RX][0] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_mapping_menu.paxes[BE_ST_CTRL_AXIS_RX][1] = g_ingame_altcontrol_mapping_menu.pbuttons[BE_ST_CTRL_BUT_DPAD_RIGHT];

		g_ingame_altcontrol_mapping_menu_paddle.paxes[BE_ST_CTRL_AXIS_RX][0] = g_ingame_altcontrol_mapping_menu_paddle.pbuttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_mapping_menu_paddle.paxes[BE_ST_CTRL_AXIS_RX][1] = g_ingame_altcontrol_mapping_menu_paddle.pbuttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
	}
#endif

	// Version-specific patches
#ifdef REFKEEN_VER_CATABYSS
	g_ingame_altcontrol_mapping_intro.pbuttons[BE_ST_CTRL_BUT_Y].mapClass = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? BE_ST_CTRL_MAP_KEYSCANCODE : BE_ST_CTRL_MAP_NONE;
	g_ingame_altcontrol_mapping_slidecat.pbuttons[BE_ST_CTRL_BUT_X].mapClass = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? BE_ST_CTRL_MAP_KEYSCANCODE : BE_ST_CTRL_MAP_NONE;
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
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}

REFKEEN_NS_E
