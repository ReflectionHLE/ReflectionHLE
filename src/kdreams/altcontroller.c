/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
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

#include "kd_def.h"

BE_ST_ControllerSingleMap *g_ingame_altcontrol_button0mapping, *g_ingame_altcontrol_button1mapping,
	*g_ingame_altcontrol_upmapping, *g_ingame_altcontrol_downmapping, *g_ingame_altcontrol_leftmapping, *g_ingame_altcontrol_rightmapping;

// This one is incomplete and partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_gameplay = {
	NULL,
	{
		{0},
		{0},
		{0},
		{0},
		// Back button
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
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
	&g_ingame_altcontrol_mapping_gameplay,
	{
		// Face buttons
		{NULL, BE_ST_SC_F6, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F7, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// Start Button
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

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback = {
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_Control, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_Alt, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		// Back button
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Shoulder buttons
		{NULL, sc_PgUp, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_PgDn, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		// D-pad
		{NULL, sc_UpArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_DownArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_LeftArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_RightArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
		{0},
		{0},
		{0},
		{0},
		// Triggers
	},
	false
};

// This one may also be partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu = {
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start Button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		{0},
		{0},
		// D-pad
		{NULL, sc_UpArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_DownArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_LeftArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_RightArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	false
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_help = {
	NULL,
	{
		// Face buttons
		{0},
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, sc_Escape, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start Button
		{NULL, BE_ST_SC_PAUSE, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Shoulder buttons
		{NULL, sc_PgUp, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_PgDn, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		// D-pad
		{NULL, sc_UpArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_DownArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_LeftArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, sc_RightArrow, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
	},
	{
	},
	false
};

static void CheckButtonMapping(int actionmapping, BE_ST_ControllerSingleMap **outmapptr, const BE_ST_ControllerSingleMap *inputmap)
{
	if ((actionmapping >= 0) && (actionmapping < BE_ST_CTRL_BUT_MAX))
		*outmapptr = &g_ingame_altcontrol_mapping_gameplay.buttons[actionmapping];
	else if ((actionmapping >= BE_ST_CTRL_BUT_MAX) && (BE_ST_CTRL_BUT_MAX < BE_ST_CTRL_BUT_MAX+2))
		*outmapptr = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LTRIGGER+actionmapping-BE_ST_CTRL_BUT_MAX][1];
	else
		return;

	**outmapptr = *inputmap;
}

void RefKeen_PrepareAltControllerScheme(void)
{
	BE_ST_ControllerSingleMap *tempsinglemappingptr;
	BE_ST_ControllerSingleMap tempsinglemapping = {0};

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	tempsinglemapping.val = KbdDefs[0].button0;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[0], &g_ingame_altcontrol_button0mapping, &tempsinglemapping);
	tempsinglemapping.val = KbdDefs[0].button1;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[1], &g_ingame_altcontrol_button1mapping, &tempsinglemapping);
	tempsinglemapping.val = sc_Space;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[2], &tempsinglemappingptr, &tempsinglemapping);
	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_funckeys;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[3], &tempsinglemappingptr, &tempsinglemapping);

	switch (g_refKeenCfg.altControlScheme.movementMapping)
	{
	case 0:
		g_ingame_altcontrol_upmapping = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_UP];
		g_ingame_altcontrol_downmapping = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_DOWN];
		g_ingame_altcontrol_leftmapping = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		g_ingame_altcontrol_rightmapping = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
		break;
	case 1:
		g_ingame_altcontrol_upmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LY][0];
		g_ingame_altcontrol_downmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LY][1];
		g_ingame_altcontrol_leftmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LX][0];
		g_ingame_altcontrol_rightmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_LX][1];
		break;
	case 2:
		g_ingame_altcontrol_upmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][0];
		g_ingame_altcontrol_downmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RY][1];
		g_ingame_altcontrol_leftmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][0];
		g_ingame_altcontrol_rightmapping = &g_ingame_altcontrol_mapping_gameplay.axes[BE_ST_CTRL_AXIS_RX][1];
		break;
	}
	g_ingame_altcontrol_upmapping->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_ingame_altcontrol_downmapping->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_ingame_altcontrol_leftmapping->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	g_ingame_altcontrol_rightmapping->mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;

	BE_ST_ControllerSingleMap mouseup    = {NULL, 1, 4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap mousedown  = {NULL, 1, 4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap mouseleft  = {NULL, 0, 4, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap mouseright = {NULL, 0, 4, BE_ST_CTRL_MAP_MOUSEMOTION};
	switch (g_refKeenCfg.altControlScheme.menuMouseMapping)
	{
		case 1:
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][0], &mouseup, sizeof(mouseup));
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LY][1], &mousedown, sizeof(mousedown));
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][0], &mouseleft, sizeof(mouseleft));
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_LX][1], &mouseright, sizeof(mouseright));
			break;
		case 2:
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][0], &mouseup, sizeof(mouseup));
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RY][1], &mousedown, sizeof(mousedown));
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][0], &mouseleft, sizeof(mouseleft));
			memcpy(&g_ingame_altcontrol_mapping_menu.axes[BE_ST_CTRL_AXIS_RX][1], &mouseright, sizeof(mouseright));
			break;
	}

	// Since it's our first time we can do this now
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);
}
