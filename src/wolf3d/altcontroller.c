/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2025-2021 NY00123
 *
 * This file is part of Wolfenstein 3D.
 *
 * Wolfenstein 3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Wolfenstein 3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an addendum, you can further use the Wolfenstein 3D Source Code under
 * the terms of the limited use software license agreement for Wolfenstein 3D.
 * See id-wolf3d.txt for these terms.
 */

// New file used for preparing and defining alternative controller schemes,
// in case their definitions are configurable (possibly in more than one way)

#include <string.h>
#include "wl_def.h"

/*** Touch input UI resource definitions ***/

#include "../rsrc/button_confirm.xpm"
#include "../rsrc/button_back.xpm"
#include "../rsrc/button_debug_keys.xpm"
#include "../rsrc/button_shoot.xpm"
#include "../rsrc/button_strafe.xpm"
#include "../rsrc/button_use.xpm"
#include "../rsrc/button_run.xpm"
#include "../rsrc/button_weapons.xpm"
#include "../rsrc/button_map.xpm"
#include "../rsrc/button_pause.xpm"
#include "../rsrc/button_function_keys.xpm"

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

REFKEEN_NS_B

int g_binding_value_button[NUMBUTTONS],
    g_binding_value_motionx, g_binding_value_motiony;

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_weapons;
extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys;

// Using macros for static variable definitions in compound structures,
// along with separate mapping variables used elsewhere
#define BUT_FIRE_MAP       &g_binding_value_button[0], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_STRAFE_MAP     &g_binding_value_button[1], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_RUN_MAP        &g_binding_value_button[2], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_USE_MAP        &g_binding_value_button[3], 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_UP_MAP         &g_binding_value_motiony, 0, -127, BE_ST_CTRL_MAP_VALUESET
#define BUT_DOWN_MAP       &g_binding_value_motiony, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_LEFT_MAP       &g_binding_value_motionx, 0, -127, BE_ST_CTRL_MAP_VALUESET
#define BUT_RIGHT_MAP      &g_binding_value_motionx, 0, 127, BE_ST_CTRL_MAP_VALUESET
#define BUT_MAP_MAP        NULL, BE_ST_SC_TAB, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_BACK_MAP       NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_PAUSE_MAP      NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE
#define BUT_WEAPONS_MAP    &g_ingame_altcontrol_mapping_weapons, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING
#define BUT_FUNC_KEYS_MAP  &g_ingame_altcontrol_mapping_funckeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING
#define BUT_DEBUG_KEYS_MAP &g_beStControllerMappingDebugKeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING

static const BE_ST_ControllerSingleMap
       g_ingame_but_fire_map       = {BUT_FIRE_MAP},
       g_ingame_but_strafe_map     = {BUT_STRAFE_MAP},
       g_ingame_but_run_map        = {BUT_RUN_MAP},
       g_ingame_but_use_map        = {BUT_USE_MAP},
       g_ingame_but_weapon_map[] = {
        {&g_binding_value_button[4], 0, 127, BE_ST_CTRL_MAP_VALUESET},
        {&g_binding_value_button[5], 0, 127, BE_ST_CTRL_MAP_VALUESET},
        {&g_binding_value_button[6], 0, 127, BE_ST_CTRL_MAP_VALUESET},
        {&g_binding_value_button[7], 0, 127, BE_ST_CTRL_MAP_VALUESET},
#ifdef GAMEVER_NOAH3D
        {&g_binding_value_button[8], 0, 127, BE_ST_CTRL_MAP_VALUESET},
        {&g_binding_value_button[9], 0, 127, BE_ST_CTRL_MAP_VALUESET}
#endif
       },
       g_ingame_but_up_map         = {BUT_UP_MAP},
       g_ingame_but_down_map       = {BUT_DOWN_MAP},
       g_ingame_but_left_map       = {BUT_LEFT_MAP},
       g_ingame_but_right_map      = {BUT_RIGHT_MAP},
#ifdef GAMEVER_NOAH3D
       g_ingame_but_map_map        = {BUT_MAP_MAP},
#endif
       g_ingame_but_func_keys_map  = {BUT_FUNC_KEYS_MAP},
       g_ingame_but_debug_keys_map = {BUT_DEBUG_KEYS_MAP};

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_gameplay_onscreentouchcontrols[] = {
	{button_shoot_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{button_strafe_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8},
	{analog_circle_xpm+5, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8},
	{button_use_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8},
	{button_run_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, 17+8},
	{button_weapons_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 8},
#ifdef GAMEVER_NOAH3D
	{button_map_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, 17+8},
#endif
	{button_back_xpm+5, 22, 22, 8, 2+8},
	{button_pause_xpm+5, 22, 22, 24+8, 2+8},
	{button_function_keys_xpm+5, 22, 22, 8, 26+8},
	{button_debug_keys_xpm+5, 22, 22, 24+8, 26+8},
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
	{{BUT_USE_MAP},
		button_use_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
	},
	{{BUT_RUN_MAP},
		button_run_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, 17+8
	},
	{{BUT_WEAPONS_MAP},
		button_weapons_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 8
	},
#ifdef GAMEVER_NOAH3D
	{{BUT_MAP_MAP},
		button_map_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, 17+8
	},
#endif
	{{BUT_BACK_MAP},
		button_back_xpm+5, 22, 22, 8, 2+8
	},
	{{BUT_PAUSE_MAP},
		button_pause_xpm+5, 22, 22, 24+8, 2+8
	},
	{{BUT_FUNC_KEYS_MAP},
		button_function_keys_xpm+5, 22, 22, 8, 26+8
	},
	{{BUT_DEBUG_KEYS_MAP},
		button_debug_keys_xpm+5, 22, 22, 24+8, 26+8
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

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_weapons = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{
	},
	{
		// Face buttons
		{0},
		{NULL, BE_ST_SC_6, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
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
	},
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

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
	{
	},
	{
		// Face buttons
		{NULL, BE_ST_SC_F7, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F8, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F9, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F10, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{0},
		{NULL, BE_ST_SC_F1, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F3, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F2, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
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

static BE_ST_OnscreenTouchControl g_ingame_altcontrol_mapping_menu_onscreentouchcontrols[] = {
	{button_confirm_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8},
	{button_back_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8},
	{pad_dpad_xpm+8, 48, 48, 8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-48-8},
	{0}
};

static BE_ST_TouchControlSingleMap g_ingame_altcontrol_mapping_menu_touchmappings[] = {
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

// This one may also be partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu = {
	{0},
	g_ingame_altcontrol_mapping_menu_onscreentouchcontrols,
	g_ingame_altcontrol_mapping_menu_touchmappings,
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

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_confirm = {
	{0},
	NULL,
	NULL,
	{
	},
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

static void CheckKeyMapping(int actionmapping, const BE_ST_ControllerSingleMap *inputmap)
{
	if (actionmapping > 0)
		g_ingame_altcontrol_mapping_gameplay.keys[actionmapping] = *inputmap;
}

static void CheckButtonMapping(int actionmapping, const BE_ST_ControllerSingleMap *inputmap)
{
	if ((actionmapping >= 0) && (actionmapping < BE_ST_CTRL_BUT_MAX))
		g_ingame_altcontrol_mapping_gameplay.buttons[actionmapping] = *inputmap;
	else if ((actionmapping >= BE_ST_CTRL_BUT_MAX) && (actionmapping < BE_ST_CTRL_BUT_MAX+2))
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LTRIGGER+actionmapping-BE_ST_CTRL_BUT_MAX][1] = *inputmap;
}

static void CheckMappings(int bind, const BE_ST_ControllerSingleMap *inputmap)
{
	CheckKeyMapping(g_refKeenCfg.wolf3d.binds[bind].key, inputmap);
	CheckButtonMapping(g_refKeenCfg.wolf3d.binds[bind].pad, inputmap);
}

void RefKeen_PrepareAltControllerScheme(void)
{
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_FIRE, &g_ingame_but_fire_map);
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_STRAFE, &g_ingame_but_strafe_map);
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_RUN, &g_ingame_but_run_map);
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_USE, &g_ingame_but_use_map);

	for (int i = 0; i < NUMBUTTONS - bt_readyknife; ++i)
		CheckMappings(BE_ST_CTRL_BIND_WOLF3D_WEAPON1 + i, &g_ingame_but_weapon_map[i]);

	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_UP, &g_ingame_but_up_map);
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_DOWN, &g_ingame_but_down_map);
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_LEFT, &g_ingame_but_left_map);
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_RIGHT, &g_ingame_but_right_map);
#ifdef GAMEVER_NOAH3D
	CheckMappings(BE_ST_CTRL_BIND_WOLF3D_MAP, &g_ingame_but_map_map);
#endif
	CheckButtonMapping(g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_FUNCKEYS].pad, &g_ingame_but_func_keys_map);
	CheckButtonMapping(g_refKeenCfg.wolf3d.binds[BE_ST_CTRL_BIND_WOLF3D_DEBUGKEYS].pad, &g_ingame_but_debug_keys_map);

	BE_ST_ControllerSingleMap gamekeyup    = {NULL, BE_ST_SC_UP,    0, BE_ST_CTRL_MAP_KEYSCANCODE};
	BE_ST_ControllerSingleMap gamekeydown  = {NULL, BE_ST_SC_DOWN,  0, BE_ST_CTRL_MAP_KEYSCANCODE};
	BE_ST_ControllerSingleMap gamekeyleft  = {NULL, BE_ST_SC_LEFT,  0, BE_ST_CTRL_MAP_KEYSCANCODE};
	BE_ST_ControllerSingleMap gamekeyright = {NULL, BE_ST_SC_RIGHT, 0, BE_ST_CTRL_MAP_KEYSCANCODE};

	BE_ST_ControllerSingleMap gamemouseup    = {NULL, 1, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemousedown  = {NULL, 1,  16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseleft  = {NULL, 0, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseright = {NULL, 0,  16, BE_ST_CTRL_MAP_MOUSEMOTION};

	// HACK (leave US_LineInput this way)
	g_beStControllerMappingTextInput.defaultMapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_beStControllerMappingTextInput.defaultMapping.val = BE_ST_SC_ESC;
	// HACK (hide debug keys this way)
	g_beStControllerMappingDebugKeys.defaultMapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	g_beStControllerMappingDebugKeys.defaultMapping.miscPtr = &g_ingame_altcontrol_mapping_gameplay;

	if (g_refKeenCfg.wolf3d.useLeftStick)
	{
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LY][0] = g_ingame_but_up_map;
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LY][1] = g_ingame_but_down_map;
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_but_left_map;
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_but_right_map;

		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][0] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_UP];
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][1] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_DOWN];
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
	}
	if (g_refKeenCfg.wolf3d.useRightStick)
	{
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][0] = g_ingame_but_up_map;
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][1] = g_ingame_but_down_map;
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][0] = g_ingame_but_left_map;
		g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][1] = g_ingame_but_right_map;

		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][0] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_UP];
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][1] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_DOWN];
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][0] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][1] = g_ingame_altcontrol_mapping_menu.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
	}

	// Init touch controls UI
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_gameplay.onScreenTouchControls);
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_help.onScreenTouchControls);
	BE_ST_AltControlScheme_InitTouchControlsUI(g_ingame_altcontrol_mapping_menu.onScreenTouchControls);
}

void UpdateAltControllerMappingsByMousePresence(bool withmouse)
{
	g_ingame_altcontrol_mapping_gameplay.grabMouse = withmouse;
	g_ingame_altcontrol_mapping_weapons.grabMouse = withmouse;
	g_ingame_altcontrol_mapping_funckeys.grabMouse = withmouse;
}

void PrepareGamePlayControllerMapping(void)
{
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}

REFKEEN_NS_E
