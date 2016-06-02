/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
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

BE_ST_ControllerSingleMap *g_ingame_altcontrol_button0mappings[2], *g_ingame_altcontrol_button1mappings[2],
	*g_ingame_altcontrol_upmappings[4], *g_ingame_altcontrol_downmappings[4], *g_ingame_altcontrol_leftmappings[4], *g_ingame_altcontrol_rightmappings[4];

// This one is incomplete and partially filled on startup
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_gameplay = {
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
		{0},
	},
	{
	},
	false
};

#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_scrolls = {
	&g_ingame_altcontrol_mapping_gameplay,
	{
		// Face buttons
		{NULL, BE_ST_SC_7, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_6, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_8, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_5, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Start Button
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
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_help = {
	NULL,
	{
		// Face buttons
		{0},
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start Button
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
	&g_ingame_altcontrol_mapping_gameplay,
	{
		// Face buttons
		{NULL, BE_ST_SC_F3, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F2, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F4, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F1, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Start Button
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
		// Start Button
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
	NULL,
	{
		{0},
		{0},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start Button
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
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_ENTER, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_F4, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
#ifdef REFKEEN_VER_CATABYSS
		{NULL, BE_ST_SC_F10, 0, BE_ST_CTRL_MAP_KEYSCANCODE}, // This one depends on game episode and version
#else
		{0},
#endif
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start Button
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
	NULL,
	{
		// Face buttons
		{NULL, BE_ST_SC_N, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{NULL, BE_ST_SC_W, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		{0},
		// Back button
		{NULL, BE_ST_SC_ESC, 0, BE_ST_CTRL_MAP_KEYSCANCODE},
		{0},
		// Start Button
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
#endif

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback = {
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
		// Start Button
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

// Similarly this
BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_paddle = {
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
		// Start Button
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
		// Start Button
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
	BE_ST_ControllerSingleMap **currupmappingptr = g_ingame_altcontrol_upmappings;
	BE_ST_ControllerSingleMap **currdownmappingptr = g_ingame_altcontrol_downmappings;
	BE_ST_ControllerSingleMap **currleftmappingptr = g_ingame_altcontrol_leftmappings;
	BE_ST_ControllerSingleMap **currrightmappingptr = g_ingame_altcontrol_rightmappings;

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_KEYSCANCODE;
	tempsinglemapping.val = BE_ST_SC_LCTRL; // Actually adjusted in game
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE], &g_ingame_altcontrol_button0mappings[0], &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_LALT; // Actually adjusted in game
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE], &g_ingame_altcontrol_button1mappings[0], &tempsinglemapping);
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

	tempsinglemapping.val = BE_ST_SC_SPACE;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DRINK], &tempsinglemappingptr, &tempsinglemapping);
#ifdef REFKEEN_VER_CAT3D
	tempsinglemapping.val = BE_ST_SC_B;
#else
	tempsinglemapping.val = BE_ST_SC_Z;
#endif
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_BOLT], &tempsinglemappingptr, &tempsinglemapping);
	tempsinglemapping.val = BE_ST_SC_ENTER;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_NUKE], &tempsinglemappingptr, &tempsinglemapping);
#ifdef REFKEEN_VER_CAT3D
	tempsinglemapping.val = BE_ST_SC_RSHIFT;
#else
	tempsinglemapping.val = BE_ST_SC_TAB;
#endif
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FASTTURN], &tempsinglemappingptr, &tempsinglemapping);

	tempsinglemapping.mapClass = BE_ST_CTRL_MAP_OTHERMAPPING;
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_scrolls;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_SCROLLS], &tempsinglemappingptr, &tempsinglemapping);
#endif
#ifdef REFKEEN_VER_CATADVENTURES
	tempsinglemapping.otherMappingPtr = &g_ingame_altcontrol_mapping_funckeys;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS], &tempsinglemappingptr, &tempsinglemapping);
#endif
	tempsinglemapping.otherMappingPtr = &g_beStControllerMappingDebugKeys;
	CheckButtonMapping(g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS], &tempsinglemappingptr, &tempsinglemapping);

	// HACK (hide debug keys by pressing outside keyboard with pointer)
	g_beStControllerMappingDebugKeys.prevMapping = &g_ingame_altcontrol_mapping_gameplay;

	BE_ST_ControllerSingleMap gamemouseup    = {NULL, 1, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemousedown  = {NULL, 1,  16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseleft  = {NULL, 0, -16, BE_ST_CTRL_MAP_MOUSEMOTION};
	BE_ST_ControllerSingleMap gamemouseright = {NULL, 0,  16, BE_ST_CTRL_MAP_MOUSEMOTION};

	if (g_refKeenCfg.altControlScheme.useDpad)
	{
		*currupmappingptr = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_UP];
		*currdownmappingptr = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_DOWN];
		*currleftmappingptr = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_LEFT];
		*currrightmappingptr = &g_ingame_altcontrol_mapping_gameplay.buttons[BE_ST_CTRL_BUT_DPAD_RIGHT];
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

	// Version-specific patch
#ifdef REFKEEN_VER_CATABYSS
	g_ingame_altcontrol_mapping_intro.buttons[BE_ST_CTRL_BUT_X].mapClass = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? BE_ST_CTRL_MAP_KEYSCANCODE : BE_ST_CTRL_MAP_NONE;
#endif
	// Since it's our first time we can do this now
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);
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

	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_gameplay);
}
