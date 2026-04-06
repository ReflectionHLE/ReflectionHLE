/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2026 K1n9_Duk3
 * Copyright (C) 2025-2026 NY00123
 *
 * This file is loosely based on:
 * Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
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

#include "bm_def.h"
//#pragma hdrstop

REFKEEN_NS_B

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

#ifndef BETA
static id0_char_t piracymessage[] =
	"Critical file FILE_ID.DIZ missing\n"
	"or modified.  If you purchased the\n"
	"game from Apogee simply reinstall\n"
	"the game.  You can call Apogee at\n"
	"(214) 278-5655 or refer to\n"
	"Ordering Info for more details.\n";
#endif

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

Sint16 mapon;	//originally declared in ID_CA.C
static Uint16 fadecount;

/////////////////////////////////////////////////////////////////////////////
// local prototypes:
/////////////////////////////////////////////////////////////////////////////

static void FadeAndUnhook(void);

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
============================
=
= FreeGraphics
=
============================
*/

static void FreeGraphics(void)
{
	/*register*/ Sint16 i;
	for (i=STARTSPRITES; i<STARTSPRITES+NUMSPRITES; i++)
	{
		if (grsegs[i])
		{
			MM_SetPurge(&grsegs[i], true);
		}
	}
	for (i=STARTTILE16; i<STARTEXTERNS; i++)
	{
		if (grsegs[i])
		{
			MM_SetPurge(&grsegs[i], true);
		}
	}
}

//===========================================================================

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame(void)
{
#ifdef BETA
	gamestate.mapon = 0;
	gamestate.score = 0;
	gamestate.nextextra = 20000;
	gamestate.lives = 3;
	gamestate.food = 0;
	gamestate.autofire = 0;
	memset(&gamestate.grenades, 0, sizeof(gamestate.grenades));
	gamestate.ammo = 0;
	gamestate.clips = 0;
	gamestate.gotshard[0] = 0;
	gamestate.gotshard[1] = 0;
	gamestate.gotshard[2] = 0;
	gamestate.gotshard[3] = 0;
	gamestate.got_exitkey = false;
	gamestate.specialkeys = 0;
	gamestate.nukestate = 0;
	gamestate.potions = 0;
	gamestate.got_pill = false;
	gamestate.savedhostage = false;
#else
	if (playstate != ex_loadedgame)
	{
		gamestate.mapon = 0;
		gamestate.score = 0;
		gamestate.nextextra = 20000;
		gamestate.lives = 3;
		gamestate.gems = 0;
		gamestate.nukestate = 0;
		gamestate.potions = 0;
	}
	gamestate.health = 4;
	gamestate.autofire = 0;
	gamestate.weapon = 0;
	memset(&gamestate.grenades, 0, sizeof(gamestate.grenades));
	gamestate.ammo = 0;
	gamestate.clips = 0;
	gamestate.gotshard[0] = 0;
	gamestate.gotshard[1] = 0;
	gamestate.gotshard[2] = 0;
	gamestate.gotshard[3] = 0;
	gamestate.got_exitkey = false;
	gamestate.trianglekey = 0;
	gamestate.specialkeys = 0;
	gamestate.got_pill = false;
	gamestate.savedhostage = false;
	gamestate.got_warpgem = false;
#endif
}

//===========================================================================

/*
============================
=
= GameOver
=
============================
*/

static void GameOver(void)
{
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	VW_FixRefreshBuffer();
	US_CenterWindow(16, 3);
	US_PrintCentered("GAME OVER");
	VW_UpdateScreen();
	IN_ClearKeysDown();
	IN_UserInput(4*TickBase, false);
}

//===========================================================================

/*
============================
=
= ResetGame
=
============================
*/

void ResetGame(void)
{
	Sint16 num;
	boolean esc;

	NewGame();
	ca_levelnum--;
	ca_levelbit >>= 1;
	CA_ClearMarks();
	ca_levelbit <<= 1;
	ca_levelnum++;
#ifndef BETA
	if (restartgame != gd_Continue)
	{
		if (practicemode == true)
		{
			
			oldmapon = gamestate.mapon;
			VW_FixRefreshBuffer();
			US_CenterWindow(26, 3);
			PrintY += 6;
			strcpy(str, str_practiceprompt);
//			_fstrcpy(str, str_practiceprompt);
			US_Print(str);
			VW_UpdateScreen();
			esc = !US_LineInput(px, py, str, NULL, true, 2, 0);
			if (!esc)
			{
				num = atoi(str);
				if (num > 0 && num <= 11)
				{
					gamestate.mapon = num-1;
				}
			}
			practicemode = false;
			practiceTimeLeft = 500;
		}
		else if (practiceTimeLeft >= 0)
		{
			practiceTimeLeft = -1;
		}
		playstate = ex_resetgame;
	}
	else if (loadedgame)
	{
		playstate = ex_loadedgame;
		practiceTimeLeft = -1;
	}
	VW_FadeOut();
#endif
}

//===========================================================================

// REFKEEN - New cross-platform methods for reading/writing objects from/to saved games

BE_CROSS_IMPLEMENT_FP_READWRITE_U16LE_FUNCS(activetype)
BE_CROSS_IMPLEMENT_FP_READWRITE_U16LE_FUNCS(classtype)
BE_CROSS_IMPLEMENT_FP_READWRITE_U16LE_FUNCS(cliptype)

#ifdef BETA
static boolean SaveObject(BE_FILE_T file, objtype *o)
{
	id0_int_t dummy = 0;
	// BACKWARDS COMPATIBILITY
	Uint16 statedosoffset = o->state ? o->state->compatdospointer : 0;
	// Just tells if "o->next" is zero or not
	Sint16 isnext = o->next ? 1 : 0;
	// Now writing
	return ((BE_Cross_write_classtype_ToU16LE(file, &o->obclass) == 2)
	        && (BE_Cross_write_activetype_ToU16LE(file, &o->active) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &o->needtoreact) == 2)
	        && (BE_Cross_write_cliptype_ToU16LE(file, &o->needtoclip) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->nothink) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->x) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->y) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->xdir) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->ydir) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->xmove) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->ymove) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->xspeed) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->yspeed) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->ticcount) == 2)
	        && (BE_Cross_writeInt16LE(file, &statedosoffset) == 2) // BACKWARDS COMPATIBILITY
	        && (BE_Cross_writeInt16LE(file, &o->shapenum) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->priority) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->left) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->top) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->right) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->bottom) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->midx) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tileleft) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tiletop) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tileright) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tilebottom) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tilemidx) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->hitnorth) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->hiteast) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->hitsouth) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->hitwest) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &o->shootable) == 2)
#ifdef BETA
	        && (BE_Cross_writeInt16LE(file, &o->health) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->white) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->_unknown) == 2)
#else
	        && (BE_Cross_write_boolean_To16LE(file, &o->spareplayer) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->health) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->white) == 2)
#endif
	        && (BE_Cross_writeInt16LE(file, &o->temp1) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp2) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp3) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp4) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp5) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp6) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp7) == 2)
	        // No need to write sprite, prev pointers as-is,
	        // these are ignored on loading. So write dummy value.
	        // Furthermore, all we need to know about next on loading is
	        // if it's zero or not.
	        && (BE_Cross_writeInt16LE(file, &dummy) == 2) // sprite
	        && (BE_Cross_writeInt16LE(file, &isnext) == 2) // next
	        && (BE_Cross_writeInt16LE(file, &dummy) == 2) // prev
	);
}

static boolean LoadObject(BE_FILE_T file, objtype *o)
{
	id0_int_t dummy;
	// BACKWARDS COMPATIBILITY
	id0_word_t statedosoffset;
	// Just tells if "o->next" is zero or not
	id0_int_t isnext;
	// Now reading
	if ((BE_Cross_read_classtype_FromU16LE(file, &o->obclass) != 2)
	    || (BE_Cross_read_activetype_FromU16LE(file, &o->active) != 2)
	    || (BE_Cross_read_boolean_From16LE(file, &o->needtoreact) != 2)
	    || (BE_Cross_read_cliptype_FromU16LE(file, &o->needtoclip) != 2)
	    || (BE_Cross_readInt16LE(file, &o->nothink) != 2)
	    || (BE_Cross_readInt16LE(file, &o->x) != 2)
	    || (BE_Cross_readInt16LE(file, &o->y) != 2)
	    || (BE_Cross_readInt16LE(file, &o->xdir) != 2)
	    || (BE_Cross_readInt16LE(file, &o->ydir) != 2)
	    || (BE_Cross_readInt16LE(file, &o->xmove) != 2)
	    || (BE_Cross_readInt16LE(file, &o->ymove) != 2)
	    || (BE_Cross_readInt16LE(file, &o->xspeed) != 2)
	    || (BE_Cross_readInt16LE(file, &o->yspeed) != 2)
	    || (BE_Cross_readInt16LE(file, &o->ticcount) != 2)
	    || (BE_Cross_readInt16LE(file, &statedosoffset) != 2) // BACKWARDS COMPATIBILITY
	    || (BE_Cross_readInt16LE(file, &o->shapenum) != 2)
	    || (BE_Cross_readInt16LE(file, &o->priority) != 2)
	    || (BE_Cross_readInt16LE(file, &o->left) != 2)
	    || (BE_Cross_readInt16LE(file, &o->top) != 2)
	    || (BE_Cross_readInt16LE(file, &o->right) != 2)
	    || (BE_Cross_readInt16LE(file, &o->bottom) != 2)
	    || (BE_Cross_readInt16LE(file, &o->midx) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tileleft) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tiletop) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tileright) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tilebottom) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tilemidx) != 2)
	    || (BE_Cross_readInt16LE(file, &o->hitnorth) != 2)
	    || (BE_Cross_readInt16LE(file, &o->hiteast) != 2)
	    || (BE_Cross_readInt16LE(file, &o->hitsouth) != 2)
	    || (BE_Cross_readInt16LE(file, &o->hitwest) != 2)
	    || (BE_Cross_read_boolean_From16LE(file, &o->shootable) != 2)
#ifdef BETA
	    || (BE_Cross_readInt16LE(file, &o->health) != 2)
	    || (BE_Cross_readInt16LE(file, &o->white) != 2)
	    || (BE_Cross_readInt16LE(file, &o->_unknown) != 2)
#else
	    || (BE_Cross_read_boolean_From16LE(file, &o->spareplayer) != 2)
	    || (BE_Cross_readInt16LE(file, &o->health) != 2)
	    || (BE_Cross_ReadInt16LE(file, &o->white) != 2)
#endif
	    || (BE_Cross_readInt16LE(file, &o->temp1) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp2) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp3) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp4) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp5) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp6) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp7) != 2)
	    // No need to read sprite, prev pointers as-is,
	    // these are ignored on loading. So read dummy value.
	    // Furthermore, all we need to know about next on loading is
	    // if it's zero or not.
	    || (BE_Cross_readInt16LE(file, &dummy) != 2) // sprite
	    || (BE_Cross_readInt16LE(file, &isnext) != 2) // next
	    || (BE_Cross_readInt16LE(file, &dummy) != 2) // prev
	)
		return false;

	o->state = RefKeen_GetObjStatePtrFromDOSPointer(statedosoffset);
	// HACK: All we need to know is if next was originally NULL or not
	o->next = isnext ? o : NULL;
	return true;
}
#endif // BETA

// Similar new methods for writing/reading game state
static boolean SaveGameState(BE_FILE_T file, gametype *state)
{
	// TODO: We don't seem to have a real need to convert pointers
	// to objects, at least outside of the "riding" field here,
	// so basically support only the value of 0 being written
	// to the saved game (as occurring with the original executables).
	Uint16 ridingdosoffset = 0;
	return ((BE_Cross_writeInt32LE(file, &state->score) == 4)
	        && (BE_Cross_writeInt32LE(file, &state->nextextra) == 4)
	        && (BE_Cross_writeInt16LE(file, &state->mapon) == 2)
#ifdef BETA
	        && (BE_Cross_writeInt16LE(file, &state->grenades.landmines) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->grenades.green) == 2)
#else
	        && (BE_Cross_writeInt8LE(file, &state->grenades.landmines) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->grenades.green) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->grenades.red) == 1)
#endif
	        && (BE_Cross_writeInt16LE(file, &state->keys.keycards) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->keys.keys) == 2)
	        && (BE_Cross_writeInt16LEBuffer(file, state->gotshard, sizeof(state->gotshard)) == sizeof(state->gotshard))
	        && (BE_Cross_writeInt16LE(file, &state->specialkeys) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->nukestate) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->got_pill) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->got_exitkey) == 2)
#ifndef BETA
	        && (BE_Cross_writeInt16LE(file, &state->trianglekey) == 2)
#endif
	        && (BE_Cross_write_boolean_To16LE(file, &state->got_robopal) == 2)
#ifdef BETA
	        && (BE_Cross_writeInt16LE(file, &state->potions) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->lives) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->food) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->difficulty) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->autofire) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->ammo) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->clips) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->savedhostage) == 2)
#else
	        && (BE_Cross_writeInt8LE(file, &state->potions) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->lives) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->gems) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->health) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->difficulty) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->autofire) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->ammo) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->clips) == 1)
	        && (BE_Cross_writeInt8LE(file, &state->weapon) == 1)
	        && (BE_Cross_write_boolean_To16LE(file, &state->savedhostage) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->got_warpgem) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_grenade) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_machinegun) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_keycard) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_key) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_crystalshard) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_supergun) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_landmine) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_gem) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_firstaid) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_invincible) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_robopal) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_switchbridge) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_switchplat) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_colorsequence) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_opendoor) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_plasmabolts) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_warpgem) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->hint_beacon) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->_zero) == 2)
#endif
	        && (BE_Cross_writeInt16LE(file, &ridingdosoffset) == 2)
	);
}

static boolean LoadGameState(BE_FILE_T file, gametype *state)
{
	// TODO: We don't seem to have a real need to convert pointers
	// to objects, at least outside of the "riding" field here,
	// so basically support only the value of 0 being written
	// to the saved game (as occurring with the original executables).
	state->riding = NULL;
	Uint16 ridingdosoffset = 0;
	return ((BE_Cross_readInt32LE(file, &state->score) == 4)
	        && (BE_Cross_readInt32LE(file, &state->nextextra) == 4)
	        && (BE_Cross_readInt16LE(file, &state->mapon) == 2)
#ifdef BETA
	        && (BE_Cross_readInt16LE(file, &state->grenades.landmines) == 2)
	        && (BE_Cross_readInt16LE(file, &state->grenades.green) == 2)
#else
	        && (BE_Cross_readInt8LE(file, &state->grenades.landmines) == 1)
	        && (BE_Cross_readInt8LE(file, &state->grenades.green) == 1)
	        && (BE_Cross_readInt8LE(file, &state->grenades.red) == 1)
#endif
	        && (BE_Cross_readInt16LE(file, &state->keys.keycards) == 2)
	        && (BE_Cross_readInt16LE(file, &state->keys.keys) == 2)
	        && (BE_Cross_readInt16LEBuffer(file, state->gotshard, sizeof(state->gotshard)) == sizeof(state->gotshard))
	        && (BE_Cross_readInt16LE(file, &state->specialkeys) == 2)
	        && (BE_Cross_readInt16LE(file, &state->nukestate) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->got_pill) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->got_exitkey) == 2)
#ifndef BETA
	        && (BE_Cross_readInt16LE(file, &state->trianglekey) == 2)
#endif
	        && (BE_Cross_read_boolean_From16LE(file, &state->got_robopal) == 2)
#ifdef BETA
	        && (BE_Cross_readInt16LE(file, &state->potions) == 2)
	        && (BE_Cross_readInt16LE(file, &state->lives) == 2)
	        && (BE_Cross_readInt16LE(file, &state->food) == 2)
	        && (BE_Cross_readInt16LE(file, &state->difficulty) == 2)
	        && (BE_Cross_readInt16LE(file, &state->autofire) == 2)
	        && (BE_Cross_readInt16LE(file, &state->ammo) == 2)
	        && (BE_Cross_readInt16LE(file, &state->clips) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->savedhostage) == 2)
#else
	        && (BE_Cross_readInt8LE(file, &state->potions) == 1)
	        && (BE_Cross_readInt8LE(file, &state->lives) == 1)
	        && (BE_Cross_readInt8LE(file, &state->gems) == 1)
	        && (BE_Cross_readInt8LE(file, &state->health) == 1)
	        && (BE_Cross_readInt8LE(file, &state->difficulty) == 1)
	        && (BE_Cross_readInt8LE(file, &state->autofire) == 1)
	        && (BE_Cross_readInt8LE(file, &state->ammo) == 1)
	        && (BE_Cross_readInt8LE(file, &state->clips) == 1)
	        && (BE_Cross_readInt8LE(file, &state->weapon) == 1)
	        && (BE_Cross_read_boolean_From16LE(file, &state->savedhostage) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->got_warpgem) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_grenade) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_machinegun) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_keycard) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_key) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_crystalshard) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_supergun) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_landmine) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_gem) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_firstaid) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_invincible) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_robopal) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_switchbridge) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_switchplat) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_colorsequence) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_opendoor) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_plasmabolts) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_warpgem) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->hint_beacon) == 2)
	        && (BE_Cross_readInt16LE(file, &state->_zero) == 2)
#endif
	        && (BE_Cross_readInt16LE(file, &ridingdosoffset) == 2)
		&& ((gamestate.riding = 0) == 0/*TODO - See comment further above*/)
	);
}

/*
============================
=
= SaveTheGame
=
============================
*/

#define RLETAG 0xABCD

boolean SaveTheGame(BE_FILE_T handle)
{
#ifdef BETA
	Uint16	i,compressed,expanded;
	objtype	*ob;
	memptr	bigbuffer;

	gamestate.riding = NULL;

	// (REFKEEN) Writing fields one-by-one in a cross-platform manner
	if (!SaveGameState(handle, &gamestate))
	{
		return false;
	}

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr(&bigbuffer, expanded);

	for (i = 0; i < MAPPLANES; i++)
	{
		compressed = CA_RLEWCompress(mapsegs[i], expanded, (Uint16 id0_huge *)bigbuffer+1, RLETAG);
		*(Uint16 id0_huge *)bigbuffer = compressed;
		if (!CA_FarWrite(handle, (id0_byte_t id0_far *)bigbuffer, compressed+2))
		{
			MM_FreePtr(&bigbuffer);
			return false;
		}
	}
	for (ob = player; ob; ob=ob->next)
	{
		// (REFKEEN) Writing fields one-by-one in a cross-platform manner
		if (!SaveObject(handle, ob))
		{
			MM_FreePtr(&bigbuffer);
			return false;
		}
	}
	MM_FreePtr(&bigbuffer);
	return true;
#else
	gametype savestate;

	gamestate.riding = NULL;
	practiceTimeLeft = -1;
	practicemode = false;
	memcpy(&savestate, &gamestate, sizeof(savestate));
	memset(&savestate.keys, 0, sizeof(savestate.keys));
	savestate.gotshard[0] = 0;
	savestate.gotshard[1] = 0;
	savestate.gotshard[2] = 0;
	savestate.gotshard[3] = 0;
	savestate.got_exitkey = false;
	savestate.trianglekey = 0;
	savestate.specialkeys = 0;
	savestate.got_pill = false;
	savestate.got_robopal = false;
	savestate.savedhostage = false;
	savestate.clips = 0;
	savestate.ammo = 0;
	savestate.weapon = 0;
	memset(&savestate.grenades, 0, sizeof(savestate.grenades));
	switch (savestate.difficulty)
	{
	case gd_Easy:
		savestate.health = 8;
		break;
	case gd_Normal:
		savestate.health = 4;
		break;
	case gd_Hard:
		savestate.health = 4;
		break;
	default:
		savestate.health = 4;
		break;
	}
	colorseqnum = 0;	// BUG: shouldn't be set to 0 when saving
	// (REFKEEN) Writing fields one-by-one in a cross-platform manner
	if (!SaveGameState(handle, &savestate))
	{
		return false;
	}
	return true;
#endif	// ifdef BETA ... else ...
}

//===========================================================================

/*
============================
=
= LoadTheGame
=
============================
*/

boolean LoadTheGame(BE_FILE_T handle)
{
#ifdef BETA
	Uint16	i;
	objtype	*prev,*next,*followed;
	Uint16	compressed,expanded;
	memptr	bigbuffer;

	// (REFKEEN) Reading fields one-by-one in a cross-platform manner
	if (!LoadGameState(handle, &gamestate))
	{
		return false;
	}

	ca_levelbit >>= 1;
	ca_levelnum--;
	SetupGameLevel(false);
	// BUG: ca_levelbit and ca_levelnum should be adjusted here and not after the
	// mmerror check, otherwise a memory error would cause this routine to return
	// with the wrong cache level and probably cause the program to crash with a
	// CA_DownLevel error later on.
	if (mmerror)
	{
		mmerror = false;
		US_CenterWindow(20, 8);
		PrintY += 20;
		US_CPrint(str_nomemgame);
		VW_UpdateScreen();
		IN_Ack();
		return false;
	}
	ca_levelbit <<= 1;
	ca_levelnum++;

	expanded = mapwidth * mapheight * 2;
	MM_BombOnError(true);	// BUG: this should use false to avoid an instant crash
	MM_GetPtr(&bigbuffer, expanded);
	MM_BombOnError(false);	// BUG: this should use true to force an instant crash
	if (mmerror)
	{
		mmerror = false;
		US_CenterWindow(20, 8);
		PrintY += 20;
		US_CPrint(str_nomemgame);
		VW_UpdateScreen();
		IN_Ack();
		return false;
	}
	for (i = 0; i < MAPPLANES; i++)
	{
		if (!CA_FarRead(handle, (id0_byte_t id0_far *)&compressed, sizeof(compressed)))
		{
			MM_FreePtr(&bigbuffer);
			return false;
		}
		if (!CA_FarRead(handle, (id0_byte_t id0_far *)bigbuffer, compressed))
		{
			MM_FreePtr(&bigbuffer);
			return false;
		}
		CA_RLEWexpand((id0_unsigned_t *)bigbuffer, mapsegs[i], expanded, RLETAG);
	}
	MM_FreePtr(&bigbuffer);

	InitObjArray();
	newobj = player;
	prev = newobj->prev;
	next = newobj->next;
	// (REFKEEN) Writing fields one-by-one in a cross-platform manner
	if (!LoadObject(handle, newobj))
	{
		return false;
	}
	newobj->prev = prev;
	newobj->next = next;
	newobj->needtoreact = true;
	newobj->sprite = NULL;
	newobj = scoreobj;
	while (true)
	{
		prev = newobj->prev;
		next = newobj->next;
		// And again
		if (!LoadObject(handle, newobj))
		{
			return false;
		}
		followed = newobj->next;
		newobj->prev = prev;
		newobj->next = next;
		newobj->needtoreact = true;
		newobj->sprite = NULL;
		if (followed)
		{
			GetNewObj(false);
		}
		else
		{
			break;
		}
	}
	*((Sint32*)&scoreobj->temp1) = -1;
	scoreobj->temp3 = -1;
	scoreobj->temp4 = -1;
	return true;
#else
	// (REFKEEN) Reading fields one-by-one in a cross-platform manner
	if (!LoadGameState(handle, &gamestate))
	{
		return false;
	}
	playstate = ex_loadedgame;
	player->health = gamestate.health;
	practiceTimeLeft = -1;
	practicemode = false;
	return true;
#endif	// ifdef BETA ... else ...
}

#undef RLETAG

/*
==========================
=
= DelayedFade
=
= Fades out and latches FadeAndUnhook onto the refresh
=
==========================
*/

static void DelayedFade(void)
{
	VW_FadeOut();
	fadecount = 0;
	RF_SetRefreshHook(FadeAndUnhook);
}

/*
==========================
=
= FadeAndUnhook
=
= Latch this onto the refresh so the screen only gets faded in after two
= refreshes.  This lets all actors draw themselves to both pages before
= fading the screen in.
=
==========================
*/

static void FadeAndUnhook(void)
{
	if (++fadecount == 2)
	{
		VW_FadeIn();
		RF_SetRefreshHook(NULL);
		SD_SetTimeCount(lasttimecount);
	}
}

//===========================================================================


/*
==========================
=
= SetupGameLevel
=
= Load in map mapon and cache everything needed for it
=
==========================
*/

void SetupGameLevel(boolean loadnow)
{
#ifndef BETA
	respawnx = 0;
	respawny = 0;
#endif
	SD_FadeOutMusic();
	CA_SetAllPurge();
//
// randomize if not a demo
//
	if (DemoMode)
	{
		US_InitRndT(false);
		gamestate.difficulty = gd_Normal;
	}
	else
	{
		US_InitRndT(true);
	}
//
// load the level header and three map planes
//
	CA_CacheMap(gamestate.mapon);
//
// let the refresh manager set up some variables
//
	RF_NewMap();
//
// decide which graphics are needed and spawn actors
//
	CA_ClearMarks();
	ScanInfoPlane();
	RF_MarkTileGraphics();
//
// have the caching manager load and purge stuff to make sure all marks
// are in memory
//
	CA_LoadAllSounds();
	if (loadnow)
	{
#ifdef BETA
		if (scorescreenkludge)
#else
		if (scorescreenkludge || gamestate.mapon == 13)
#endif
		{
			CA_CacheMarks(NULL);
		}
#ifndef BETA
		else if (gamestate.mapon >= 20)
		{
			CA_CacheMarks("Secret Level");
		}
#endif
		else if (DemoMode)
		{
			CA_CacheMarks("DEMO");
		}
		else
		{
			CA_CacheMarks(levelnames[mapon]);
		}
	}
	if (loadnow)
	{
		DelayedFade();
	}
}

//==========================================================================

#ifdef BETA
/*
==========================
=
= HandleDeath
=
==========================
*/

static void HandleDeath(void)
{
	// this is just an empty leftover from Keen Dreams and Keen 4-6
}

#else

/*
==========================
=
= RespawnPlayer
=
==========================
*/

static boolean RespawnPlayer(void)
{
	if (respawnx > 0 && respawny > 0)
	{
		player->y = respawny;
		player->x = respawnx;
		player->priority = 1;
		player->needtoclip = cl_noclip;
		ChangeState(player, &s_snakestand);
		player->needtoclip = cl_midclip;
		CenterActor(player);
		ytry = 15;
		player->obclass = playerobj;
		player->active = yes;
		player->needtoclip = cl_midclip;
		switch (gamestate.difficulty)
		{
		case gd_Easy:
			player->health = 8;
			break;
		case gd_Normal:
			player->health = 4;
			break;
		case gd_Hard:
			player->health = 2;
			break;
		default:
			player->health = 4;
			break;
		}
		gamestate.health = player->health;
		invincibility = 50;
		shuttlecooldown = 0;	//BETA leftovers
		return true;
	}
	return false;
}

#endif

//==========================================================================

/*
============================
=
= GameLoop
=
= A game has just started (after the cinematic or load game)
=
============================
*/

void GameLoop(void)
{
	Uint16 temp;

#ifdef BETA
	if (playstate == ex_loadedgame)
	{
		goto level_loop;
	}
reset_game:
	gamestate.difficulty = restartgame;
	restartgame = gd_Continue;
#else
check_loaded:
	if (playstate != ex_loadedgame)
	{
		gamestate.difficulty = restartgame;
	}
	restartgame = gd_Continue;
#endif

	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	PrepareGamePlayControllerMapping();

start_level:
	do
	{
		SetupGameLevel(true);
level_loop:
		colorseqnum = 0;
		crusherstate = 0;
#ifndef BETA
		hintstate = 0;
		bosshealth = 999;
#endif
		SD_WaitSoundDone();
#ifdef BETA
		if (DebugOk)
		{
			player->health = 8;
		}
#endif
		PlayLoop();
#ifdef BETA
		if (playstate != ex_loadedgame)
		{
			memset(&gamestate.keys, 0, sizeof(gamestate.keys));
			gamestate.gotshard[0] = 0;
			gamestate.gotshard[1] = 0;
			gamestate.gotshard[2] = 0;
			gamestate.gotshard[3] = 0;
			gamestate.got_exitkey = false;
			gamestate.specialkeys = 0;
			gamestate.got_pill = false;
			gamestate.got_robopal = false;
			gamestate.savedhostage = false;
			colorseqnum = 0;
		}
#else
		if (playstate == ex_died && gamestate.lives >= 0)
		{
			if (RespawnPlayer())
			{
				playstate = ex_stillplaying;
				goto level_loop;
			}
		}
		gamestate.health = player->health;
		memset(&gamestate.keys, 0, sizeof(gamestate.keys));
		gamestate.gotshard[0] = 0;
		gamestate.gotshard[1] = 0;
		gamestate.gotshard[2] = 0;
		gamestate.gotshard[3] = 0;
		gamestate.got_exitkey = false;
		gamestate.trianglekey = 0;
		gamestate.specialkeys = 0;
		gamestate.got_pill = false;
		gamestate.got_robopal = false;
		gamestate.savedhostage = false;
		gamestate.clips = 0;
		gamestate.ammo = 0;
		gamestate.weapon = 0;
		memset(&gamestate.grenades, 0, sizeof(gamestate.grenades));
		colorseqnum = 0;
#endif	// ifdef BETA ... else ...
		VW_FixRefreshBuffer();
		if (tedlevel)
		{
			if (playstate == ex_loadedgame)
			{
				goto level_loop;
			}
			else if (playstate == ex_died)
			{
				goto start_level;
			}
			else
			{
				TEDDeath();
			}
		}
		switch (playstate)
		{
#ifdef BETA
		case ex_resetgame:
			goto reset_game;
		case ex_loadedgame:
			goto level_loop;
		case ex_died:
			HandleDeath();
			break;
#else
		case ex_resetgame:
			goto check_loaded;
		case ex_loadedgame:
			goto check_loaded;
#endif

		case ex_completed:
#ifndef BETA
#if (!SHAREWARE) || (defined GMS_VERSION)
			//
			// show an error message and abort game when anti-piracy check failed
			//
			if (pirated)
			{
				// REFKEEN - Alternative controllers support
				BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

				VW_FixRefreshBuffer();
				US_CenterWindow(35, 8);
				PrintY += 2;
				US_CPrint(piracymessage);
				VW_UpdateScreen();
				VW_WaitVBL(50);
				IN_ClearKeysDown();
				IN_Ack();
				RF_ForceRefresh();
				goto abort;
			}
#endif
#if (SHAREWARE) || (defined GMS_VERSION)
#ifdef GMS_VERSION
			goto skip_wait_screen;
#endif
			if (gamestate.mapon >= 4 && gamestate.mapon < 11 && !DebugOk)
			{
				//
				// show the registration screen for at least 30-60 seconds
				//
				timeleft = (gamestate.mapon-4)*5 + 30;
				if (timeleft > 60)
				{
					timeleft = 60;
				}
				HelpScreens();
skip_wait_screen:;
			}
#endif
			if (gamestate.mapon > 13)
			{
				//
				// user has completed a secret level, return to the previous level
				// and continue the regular level sequence
				//
				switch (gamestate.mapon)
				{
#if (EPISODE == 1)
				case 20:
					gamestate.mapon = 2;
					break;
				case 21:
					gamestate.mapon = 5;
					break;
				case 22:
					gamestate.mapon = 8;
					break;
				case 23:
					gamestate.mapon = 10;
					break;
#elif (EPISODE == 2)
				case 20:
					gamestate.mapon = 1;
					break;
				case 21:
					gamestate.mapon = 3;
					break;
				case 22:
					gamestate.mapon = 9;
					break;
#elif (EPISODE == 3)
				case 20:
					gamestate.mapon = 2;
					break;
				case 21:
					gamestate.mapon = 8;
					break;
				case 22:
					gamestate.mapon = 10;
					break;
#endif
				}
			}
			if (gamestate.got_warpgem == true)
			{
				//
				// warp to secret level
				//
				// REMEMBER: use level-1 because gamestate.mapon is increased below
				//
				switch (gamestate.mapon)
				{
#if (EPISODE == 1)
				case 2:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 20-1;
					gamestate.got_warpgem = false;
					break;
				case 5:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 21-1;
					gamestate.got_warpgem = false;
					break;
				case 8:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 22-1;
					gamestate.got_warpgem = false;
					break;
				case 10:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 23-1;
					gamestate.got_warpgem = false;
					break;
#elif (EPISODE == 2)
				case 1:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 20-1;
					gamestate.got_warpgem = false;
					break;
				case 3:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 21-1;
					gamestate.got_warpgem = false;
					break;
				case 9:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 22-1;
					gamestate.got_warpgem = false;
					break;
#elif (EPISODE == 3)
				case 2:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 20-1;
					gamestate.got_warpgem = false;
					break;
				case 8:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 21-1;
					gamestate.got_warpgem = false;
					break;
				case 10:
					playstate = ex_stillplaying;
					RunDemo(DEMO_LEVELWARP);
					gamestate.mapon = 22-1;
					gamestate.got_warpgem = false;
					break;
#endif
				}
			}
#endif	// ifndef BETA
			gamestate.mapon++;
			if (gamestate.mapon == 12)
			{
				FreeGraphics();
				RF_FixOfs();
				VW_FixRefreshBuffer();
				FinaleLayout();
				CheckHighScore(gamestate.score, 0);
				// REFKEEN - Alternative controllers support
				goto popcontrollerscheme;
				//return;
			}
			if (storedemo && mapon == 2)
			{
				IN_ClearKeysDown();
				// REFKEEN - Alternative controllers support
				goto popcontrollerscheme;
				//return;
			}
			temp = bufferofs;
			bufferofs = displayofs;
			US_CenterWindow(15, 2);
			PrintY += 2;
			US_CPrint("One moment...\n\n");
			bufferofs = temp;
			break;
			
		case ex_abort:
abort:
			IN_ClearKeysDown();
#ifndef BETA
			StopMusic();
			CA_SetAllPurge();
#endif
			// REFKEEN - Alternative controllers support
			goto popcontrollerscheme;
			//return;
		}
	} while (gamestate.lives >= 0);
	StopMusic();
	CA_SetAllPurge();
	GameOver();
	CheckHighScore(gamestate.score, mapon);
	// REFKEEN - Alternative controllers support
popcontrollerscheme:
	BE_ST_AltControlScheme_Pop();
}

REFKEEN_NS_E
