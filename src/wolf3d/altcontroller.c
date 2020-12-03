/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2015-2020 NY00123
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

BE_ST_ControllerSingleMap *g_ingame_altcontrol_buttonmappings[NUMBUTTONS],
	*g_ingame_altcontrol_upmappings[4], *g_ingame_altcontrol_downmappings[4], *g_ingame_altcontrol_leftmappings[4], *g_ingame_altcontrol_rightmappings[4];

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_weapons;
extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_funckeys;

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
#if 0
... // TODO need to use double mappings here, diagonal ones won't do it
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
...
#endif
	{{NULL, BE_ST_SC_SPACE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_use_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8
	},
	{{NULL, BE_ST_SC_RSHIFT, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_run_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-22-8, 17+8
	},
	/*** Not adjusted as above ***/
	{{&g_ingame_altcontrol_mapping_weapons, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
		button_weapons_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-39-8, 8
	},
#ifdef GAMEVER_NOAH3D
	{{NULL, BE_ST_SC_TAB, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_map_xpm+5, 22, 22, BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM-56-8, 17+8
	},
#endif
	{{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_back_xpm+5, 22, 22, 8, 2+8
	},
	{{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		button_pause_xpm+5, 22, 22, 24+8, 2+8
	},
	{{&g_ingame_altcontrol_mapping_funckeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
		button_function_keys_xpm+5, 22, 22, 8, 26+8
	},
	{{&g_beStControllerMappingDebugKeys, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
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

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_weapons = {
	{&g_ingame_altcontrol_mapping_gameplay, 0, 0, BE_ST_CTRL_MAP_OTHERMAPPING},
	NULL,
	NULL,
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

static bool CheckButtonMapping(
	int actionmapping, BE_ST_ControllerMapping *controllerMapping,
	 BE_ST_ControllerSingleMap **outmapptr,	const BE_ST_ControllerSingleMap *inputmap)
{
	if ((actionmapping >= 0) && (actionmapping < BE_ST_CTRL_BUT_MAX))
		*outmapptr = &controllerMapping->buttons[actionmapping];
	else if ((actionmapping >= BE_ST_CTRL_BUT_MAX) && (actionmapping < BE_ST_CTRL_BUT_MAX+2))
		*outmapptr = &controllerMapping->axes[BE_ST_CTRL_AXIS_LTRIGGER+actionmapping-BE_ST_CTRL_BUT_MAX][1];
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

	// These ones are actually adjusted in game
	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	tempsinglemapping.val = BE_ST_SC_LCTRL;
	CheckButtonMapping(
		g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE],
		&g_ingame_altcontrol_mapping_gameplay,
		&g_ingame_altcontrol_buttonmappings[bt_attack], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_LALT;
	CheckButtonMapping(
		g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE],
		&g_ingame_altcontrol_mapping_gameplay,
		&g_ingame_altcontrol_buttonmappings[bt_strafe], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_RSHIFT;
	CheckButtonMapping(
		g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RUN],
		&g_ingame_altcontrol_mapping_gameplay,
		&g_ingame_altcontrol_buttonmappings[bt_run], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_SPACE;
	CheckButtonMapping(
		g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_USE],
		&g_ingame_altcontrol_mapping_gameplay,
		&g_ingame_altcontrol_buttonmappings[bt_use], &tempsinglemapping);

	// Albeit the button assignments for the weapons in Wolf3D can't be
	// directly changed in the game, hex-editing CONFIG.EXT is a possibility
	for (int i = 0; i < NUMBUTTONS - bt_readyknife; ++i)
	{
		tempsinglemapping.val = BE_ST_SC_1 + i;
		CheckButtonMapping(
			g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_WEAPON1 + i],
			&g_ingame_altcontrol_mapping_gameplay,
			&g_ingame_altcontrol_buttonmappings[i + bt_readyknife], &tempsinglemapping);

	}

	BE_ST_ControllerSingleMap gamemouseup    = {NULL, 1, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemousedown  = {NULL, 1,  16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseleft  = {NULL, 0, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseright = {NULL, 0,  16, BE_ST_CTRL_MAP_MOUSEMOTION};

	// These ones are also adjusted in game
	tempsinglemapping.val = BE_ST_SC_UP;
	if (CheckButtonMapping(
	    g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_UP],
	    &g_ingame_altcontrol_mapping_gameplay,
	    currupmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currupmappingptr = gamemouseup;
		++currupmappingptr;
	}
	tempsinglemapping.val = BE_ST_SC_DOWN;
	if (CheckButtonMapping(
	    g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DOWN],
	    &g_ingame_altcontrol_mapping_gameplay,
	    currdownmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currdownmappingptr = gamemousedown;
		++currdownmappingptr;
	}
	tempsinglemapping.val = BE_ST_SC_LEFT;
	if (CheckButtonMapping(
	    g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_LEFT],
	    &g_ingame_altcontrol_mapping_gameplay,
	    currleftmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currleftmappingptr = gamemouseleft;
		++currleftmappingptr;
	}
	tempsinglemapping.val = BE_ST_SC_RIGHT;
	if (CheckButtonMapping(
	    g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_RIGHT],
	    &g_ingame_altcontrol_mapping_gameplay,
	    currrightmappingptr, &tempsinglemapping))
	{
		if (g_refKeenCfg.altControlScheme.analogMotion)
			**currrightmappingptr = gamemouseright;
		++currrightmappingptr;
	}

	// Not adjusted in-game
#ifdef GAMEVER_NOAH3D
	tempsinglemapping.val = BE_ST_SC_TAB;
	CheckButtonMapping(
		g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_MAP],
		&g_ingame_altcontrol_mapping_gameplay,
		&tempsinglemappingptr, &tempsinglemapping);
#endif

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_funckeys;
	CheckButtonMapping(
		g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS],
		&g_ingame_altcontrol_mapping_gameplay,
		&tempsinglemappingptr, &tempsinglemapping);
	tempsinglemapping.otherMappingPtr = &g_beStControllerMappingDebugKeys;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS],
		&g_ingame_altcontrol_mapping_gameplay,
		&tempsinglemappingptr, &tempsinglemapping);

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
			(*currupmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
			(*currdownmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
			(*currleftmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
			(*currrightmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
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
			(*currupmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
			(*currdownmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
			(*currleftmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
			(*currrightmappingptr++)->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
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
	}
	if (g_refKeenCfg.altControlScheme.useRightStick)
	{
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][0] = menumouseup;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][1] = menumousedown;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][0] = menumouseleft;
		g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][1] = menumouseright;
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
	BE_ST_ControllerSingleMap **singlemappingptr;

	for (int i = 0; i < NUMBUTTONS; ++i)
		if (g_ingame_altcontrol_buttonmappings[i])
			g_ingame_altcontrol_buttonmappings[i]->val = buttonscan[i];

	for (singlemappingptr = g_ingame_altcontrol_upmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = dirscan[di_north];
	for (singlemappingptr = g_ingame_altcontrol_downmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = dirscan[di_south];
	for (singlemappingptr = g_ingame_altcontrol_leftmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = dirscan[di_west];
	for (singlemappingptr = g_ingame_altcontrol_rightmappings; *singlemappingptr; ++singlemappingptr)
		(*singlemappingptr)->val = dirscan[di_east];
	// HACK - The indices are hardcoded
	g_ingame_altcontrol_mapping_gameplay.touchMappings[0].mappings[0].val = buttonscan[bt_attack];
	g_ingame_altcontrol_mapping_gameplay.touchMappings[1].mappings[0].val = buttonscan[bt_strafe];
	g_ingame_altcontrol_mapping_gameplay.touchMappings[2].mappings[0].val = dirscan[di_west];
	g_ingame_altcontrol_mapping_gameplay.touchMappings[3].mappings[0].val = dirscan[di_east];
	g_ingame_altcontrol_mapping_gameplay.touchMappings[4].mappings[0].val = dirscan[di_north];
	g_ingame_altcontrol_mapping_gameplay.touchMappings[5].mappings[0].val = dirscan[di_south];
#if 0 // Diagonal ...
	g_ingame_altcontrol_mapping_gameplay.touchMappings[6].mappings[0].val = KbdDefs[0].upleft;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[7].mappings[0].val = KbdDefs[0].upright;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[8].mappings[0].val = KbdDefs[0].downleft;
	g_ingame_altcontrol_mapping_gameplay.touchMappings[9].mappings[0].val = KbdDefs[0].downright;
#endif
	g_ingame_altcontrol_mapping_gameplay.touchMappings[6].mappings[0].val = buttonscan[bt_use];
	g_ingame_altcontrol_mapping_gameplay.touchMappings[7].mappings[0].val = buttonscan[bt_run];
	// HACK - Also hardcoded
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_DPAD_DOWN] =
		*g_ingame_altcontrol_buttonmappings[bt_readyknife];
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT] =
		*g_ingame_altcontrol_buttonmappings[bt_readypistol];
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_DPAD_LEFT] =
		*g_ingame_altcontrol_buttonmappings[bt_readymachinegun];
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_DPAD_UP] =
		*g_ingame_altcontrol_buttonmappings[bt_readychaingun];
#if (NUMBUTTONS >= 10)
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_Y] =
		*g_ingame_altcontrol_buttonmappings[bt_readycantaloupe];
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_B] =
		*g_ingame_altcontrol_buttonmappings[bt_readywatermelon];
#else
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_Y].mapClass = BE_ST_CTRL_MAP_NONE;
	g_ingame_altcontrol_mapping_weapons.buttons[BE_ST_CTRL_BUT_B].mapClass = BE_ST_CTRL_MAP_NONE;
#endif

	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}

REFKEEN_NS_E
