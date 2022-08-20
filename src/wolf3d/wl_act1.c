/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2022 NY00123
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

// WL_ACT1.C

#include "wl_def.h"
#include "wl_dobjl.h" // REFKEEN: Used for buffer overflow emulation
//#pragma hdrstop

REFKEEN_NS_B

/*
=============================================================================

							STATICS

=============================================================================
*/


statobj_t	statobjlist[MAXSTATS],*laststatobj;


// REFKEEN: Just define the struct as static.
// Fixed a possible compilation warning for unnamed type.
#if 0
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
struct _statinfo
#else
struct
#endif
#endif
static struct
{
	id0_int_t		picnum;
	stat_t	type;
} statinfo[] =
{
{SPR_STAT_0},					// puddle          spr1v
{SPR_STAT_1,block},				// Green Barrel    "
{SPR_STAT_2,block},				// Table/chairs    "
{SPR_STAT_3,block},				// Floor lamp      "
{SPR_STAT_4},					// Chandelier      "
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
{SPR_QUIZ,bo_alpo},
{SPR_STAT_6,block},
#else
{SPR_STAT_5,block},				// Hanged man      "
{SPR_STAT_6,bo_alpo},			// Bad food        "
#endif
{SPR_STAT_7,block},				// Red pillar      "
//
// NEW PAGE
//
{SPR_STAT_8,block},				// Tree            spr2v
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
{SPR_STAT_9,block},
{SPR_STAT_10,block},
{SPR_STAT_11},
{SPR_STAT_12},
{SPR_STAT_13},
{SPR_STAT_14,block},
{SPR_STAT_15,block},
#else
{SPR_STAT_9},					// Skeleton flat   "
{SPR_STAT_10,block},			// Sink            " (SOD:gibs)
{SPR_STAT_11,block},			// Potted plant    "
{SPR_STAT_12,block},			// Urn             "
{SPR_STAT_13,block},			// Bare table      "
{SPR_STAT_14},					// Ceiling light   "
#ifndef SPEAR
{SPR_STAT_15},					// Kitchen stuff   "
#else
{SPR_STAT_15,block},			// Gibs!
#endif
#endif
//
// NEW PAGE
//
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
{SPR_STAT_16,bo_key1},
{SPR_STAT_17,bo_key2},
{SPR_STAT_18,bo_bandolier},
{SPR_STAT_19,bo_25clip},
{SPR_STAT_20,bo_food},
{SPR_STAT_21,bo_firstaid},
{SPR_STAT_22,bo_clip},
{SPR_STAT_23,bo_machinegun},
#else
{SPR_STAT_16,block},			// suit of armor   spr3v
{SPR_STAT_17,block},			// Hanging cage    "
{SPR_STAT_18,block},			// SkeletoninCage  "
{SPR_STAT_19},					// Skeleton relax  "
{SPR_STAT_20,bo_key1},			// Key 1           "
{SPR_STAT_21,bo_key2},			// Key 2           "
{SPR_STAT_22,block},			// stuff				(SOD:gibs)
{SPR_STAT_23},					// stuff
#endif
//
// NEW PAGE
//
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
{SPR_STAT_24,bo_chaingun},
{SPR_STAT_25,bo_cross},
{SPR_STAT_26,bo_chalice},
{SPR_STAT_27,bo_chest},
{SPR_STAT_28,bo_crown},
{SPR_STAT_29,bo_fullheal},
{SPR_STAT_30,block},
{SPR_STAT_31,block},
#else
{SPR_STAT_24,bo_food}, 			// Good food       spr4v
{SPR_STAT_25,bo_firstaid},		// First aid       "
{SPR_STAT_26,bo_clip},			// Clip            "
{SPR_STAT_27,bo_machinegun},	// Machine gun     "
{SPR_STAT_28,bo_chaingun},		// Gatling gun     "
{SPR_STAT_29,bo_cross},			// Cross           "
{SPR_STAT_30,bo_chalice},		// Chalice         "
{SPR_STAT_31,bo_bible},			// Bible           "
#endif
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
//
// NEW PAGE
//
{SPR_STAT_32,bo_flamethrower},
{SPR_STAT_33,bo_gascan},
{SPR_STAT_34,bo_launcher},
{SPR_STAT_35,bo_missiles},
{SPR_STAT_36,bo_map},
#else
//
// NEW PAGE
//
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
{SPR_STAT_32},				// crown           spr5v
{SPR_STAT_33},				// one up          "
#else
{SPR_STAT_32,bo_crown},			// crown           spr5v
{SPR_STAT_33,bo_fullheal},		// one up          "
#endif
{SPR_STAT_34,bo_gibs},			// gibs            "
{SPR_STAT_35,block},			// barrel          "
{SPR_STAT_36,block},			// well            "
{SPR_STAT_37,block},			// Empty well      "
{SPR_STAT_38,bo_gibs},			// Gibs 2          "
{SPR_STAT_39,block},			// flag				"
//
// NEW PAGE
//
#ifndef SPEAR
{SPR_STAT_40,block},			// Call Apogee		spr7v
#else
{SPR_STAT_40},					// Red light
#endif
//
// NEW PAGE
//
{SPR_STAT_41},					// junk            "
{SPR_STAT_42},					// junk 		   "
{SPR_STAT_43},					// junk            "
#ifndef SPEAR
{SPR_STAT_44},					// pots            "
#else
{SPR_STAT_44,block},			// Gibs!
#endif
{SPR_STAT_45,block},			// stove           " (SOD:gibs)
{SPR_STAT_46,block},			// spears          " (SOD:gibs)
// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
{SPR_STAT_47,block},				// vines			"
#else
{SPR_STAT_47},					// vines			"
#endif

//
// NEW PAGE
//
#ifdef SPEAR
{SPR_STAT_48,block},			// marble pillar
{SPR_STAT_49,bo_25clip},		// bonus 25 clip
{SPR_STAT_50,block},			// truck
{SPR_STAT_51,bo_spear},			// SPEAR OF DESTINY!
#endif

{SPR_STAT_26,bo_clip2},			// Clip            "
{-1}							// terminator
#endif // GAMEVER_NOAH3D
};

/*
===============
=
= InitStaticList
=
===============
*/

void InitStaticList (void)
{
	laststatobj = &statobjlist[0];
}



// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void SpawnExit (id0_int_t tilex, id0_int_t tiley, id0_boolean_t secret)
{
	laststatobj->shapenum = -2;
	laststatobj->tilex = tilex;
	laststatobj->tiley = tiley;
	laststatobj->visspot = &spotvis[tilex][tiley];
	laststatobj->flags = FL_BONUS;
	laststatobj->itemnumber = secret ? bo_secret : bo_exit;

	laststatobj++;

	if (laststatobj == &statobjlist[MAXSTATS])
		Quit ("Too many static objects!\n");
}
#endif

/*
===============
=
= SpawnStatic
=
===============
*/

void SpawnStatic (id0_int_t tilex, id0_int_t tiley, id0_int_t type)
{
	laststatobj->shapenum = statinfo[type].picnum;
	laststatobj->tilex = tilex;
	laststatobj->tiley = tiley;
	laststatobj->visspot = &spotvis[tilex][tiley];

	switch (statinfo[type].type)
	{
	case block:
		/*(id0_unsigned_t)*/actorat[tilex][tiley] = 1;		// consider it a blocking tile
	case dressing:
		laststatobj->flags = 0;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		laststatobj->itemnumber = statinfo[type].type;
#endif
		break;

	case	bo_cross:
	case	bo_chalice:
	case	bo_bible:
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case	bo_crown:
	case	bo_fullheal:
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	case	bo_chest:
#endif
		if (!loadedgame)
		  gamestate.treasuretotal++;
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	case	bo_alpo:
#endif
	case	bo_firstaid:
	case	bo_key1:
	case	bo_key2:
	case	bo_key3:
	case	bo_key4:
	case	bo_clip:
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	case	bo_25clip:
#endif
	case	bo_machinegun:
	case	bo_chaingun:
	case	bo_food:
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	case	bo_bandolier:
	case	bo_flamethrower:
	case	bo_gascan:
	case	bo_launcher:
	case	bo_missiles:
	case	bo_map:
#else
	case	bo_alpo:
	case	bo_gibs:
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	case	bo_spear:
#endif
#endif
		laststatobj->flags = FL_BONUS;
		laststatobj->itemnumber = statinfo[type].type;
		break;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	default:
		sprintf (str,"SpawnStatic: Invalid object $%02X at %d, %d!\n",
			type,tilex,tiley);
		Quit (str);
#endif
	}

	laststatobj++;

	if (laststatobj == &statobjlist[MAXSTATS])
		Quit ("Too many static objects!\n");
}


/*
===============
=
= PlaceItemType
=
= Called during game play to drop actors' items.  It finds the proper
= item number based on the item type (bo_???).  If there are no free item
= spots, nothing is done.
=
===============
*/

void PlaceItemType (id0_int_t itemtype, id0_int_t tilex, id0_int_t tiley)
{
	id0_int_t			type;
	statobj_t	*spot;

//
// find the item number
//
	for (type=0 ;  ; type++)
	{
		if (statinfo[type].picnum == -1)		// end of list
			Quit ("PlaceItemType: couldn't find type!");
		if (statinfo[type].type == itemtype)
			break;
	}

//
// find a spot in statobjlist to put it in
//
	for (spot=&statobjlist[0] ; ; spot++)
	{
		if (spot==laststatobj)
		{
			if (spot == &statobjlist[MAXSTATS])
				return;							// no free spots
			laststatobj++;						// space at end
			break;
		}

		if (spot->shapenum == -1)				// -1 is a free spot
			break;
	}
//
// place it
//
	spot->shapenum = statinfo[type].picnum;
	spot->tilex = tilex;
	spot->tiley = tiley;
	spot->visspot = &spotvis[tilex][tiley];
	spot->flags = FL_BONUS;
	spot->itemnumber = statinfo[type].type;
}



/*
=============================================================================

							DOORS

doorobjlist[] holds most of the information for the doors

doorposition[] holds the amount the door is open, ranging from 0 to 0xffff
	this is directly accessed by AsmRefresh during rendering

The number of doors is limited to 64 because a spot in tilemap holds the
	door number in the low 6 bits, with the high bit meaning a door center
	and bit 6 meaning a door side tile

Open doors conect two areas, so sounds will travel between them and sight
	will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
	An area is true if it connects with the player's current spor.

=============================================================================
*/

#define DOORWIDTH	0x7800
#define OPENTICS	300

doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_unsigned_t		pwallnoise;
#endif
id0_int_t			doornum;

id0_unsigned_t	doorposition[MAXDOORS];		// leading edge of door 0=closed
										// 0xffff = fully open

id0_byte_t		id0_far areaconnect[NUMAREAS][NUMAREAS];

id0_boolean_t		areabyplayer[NUMAREAS];


/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/

void RecursiveConnect (id0_int_t areanumber)
{
	id0_int_t	i;

	for (i=0;i<NUMAREAS;i++)
	{
		if (areaconnect[areanumber][i] && !areabyplayer[i])
		{
			areabyplayer[i] = true;
			RecursiveConnect (i);
		}
	}
}


void ConnectAreas (void)
{
	memset (areabyplayer,0,sizeof(areabyplayer));
	areabyplayer[player->areanumber] = true;
	RecursiveConnect (player->areanumber);
}


void InitAreas (void)
{
	memset (areabyplayer,0,sizeof(areabyplayer));
	areabyplayer[player->areanumber] = true;
}



/*
===============
=
= InitDoorList
=
===============
*/

void InitDoorList (void)
{
	memset (areabyplayer,0,sizeof(areabyplayer));
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	memset (areaconnect,0,sizeof(areaconnect));
//	_fmemset (areaconnect,0,sizeof(areaconnect));
#endif

	lastdoorobj = &doorobjlist[0];
	doornum = 0;
}


/*
===============
=
= SpawnDoor
=
===============
*/

void SpawnDoor (id0_int_t tilex, id0_int_t tiley, id0_boolean_t vertical, id0_int_t lock)
{
	id0_int_t	areanumber;
	id0_unsigned_t	id0_far *map;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (doornum>=MAXDOORS)
	{
		sprintf (str,"SpawnDoor(): Too many doors on level %d!",gamestate.mapon);
		Quit (str);
	}
#else
	if (doornum==64)
		Quit ("64+ doors on level!");
#endif

	doorposition[doornum] = 0;		// doors start out fully closed
	lastdoorobj->tilex = tilex;
	lastdoorobj->tiley = tiley;
	lastdoorobj->vertical = vertical;
	lastdoorobj->lock = lock;
	lastdoorobj->action = dr_closed;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	lastdoorobj->seen = false;
#endif

	/*(id0_unsigned_t)*/actorat[tilex][tiley] = doornum | 0x80;	// consider it a solid wall

//
// make the door tile a special tile, and mark the adjacent tiles
// for door sides
//
	tilemap[tilex][tiley] = doornum | 0x80;
	map = mapsegs[0] + farmapylookup[tiley]+tilex;
	if (vertical)
	{
		*map = *(map-1);                        // set area number
		tilemap[tilex][tiley-1] |= 0x40;
		tilemap[tilex][tiley+1] |= 0x40;
	}
	else
	{
		*map = *(map-mapwidth);					// set area number
		tilemap[tilex-1][tiley] |= 0x40;
		tilemap[tilex+1][tiley] |= 0x40;
	}

	doornum++;
	lastdoorobj++;
}

//===========================================================================

/*
=====================
=
= OpenDoor
=
=====================
*/

void OpenDoor (id0_int_t door)
{
#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS
	if (DoorObjList_GetAction(door) == dr_open)
		DoorObjList_SetTicCount(door, 0);			// reset open time
	else
		DoorObjList_SetAction(door, dr_opening);	// start it opening
#else
	if (doorobjlist[door].action == dr_open)
		doorobjlist[door].ticcount = 0;			// reset open time
	else
		doorobjlist[door].action = dr_opening;	// start it opening
#endif
}


/*
=====================
=
= CloseDoor
=
=====================
*/

void CloseDoor (id0_int_t door)
{
	id0_int_t	tilex,tiley,area;
	objtype *check;

//
// don't close on anything solid
//
#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS
	tilex = DoorObjList_GetTileX(door);
	tiley = DoorObjList_GetTileY(door);
#else
	tilex = doorobjlist[door].tilex;
	tiley = doorobjlist[door].tiley;
#endif

	if (actorat[tilex][tiley])
		return;

	if (player->tilex == tilex && player->tiley == tiley)
		return;

#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS
	if (DoorObjList_GetVertical(door))
#else
	if (doorobjlist[door].vertical)
#endif
	{
		if ( player->tiley == tiley )
		{
			if ( ((player->x+MINDIST) >>TILESHIFT) == tilex )
				return;
			if ( ((player->x-MINDIST) >>TILESHIFT) == tilex )
				return;
		}
		check = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(actorat[tilex-1][tiley]);
		if (check && ((check->x+MINDIST) >> TILESHIFT) == tilex )
			return;
		check = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(actorat[tilex+1][tiley]);
		if (check && ((check->x-MINDIST) >> TILESHIFT) == tilex )
			return;
	}
	else // REFKEEN: No need for this check
//	else if (!doorobjlist[door].vertical)
	{
		if (player->tilex == tilex)
		{
			if ( ((player->y+MINDIST) >>TILESHIFT) == tiley )
				return;
			if ( ((player->y-MINDIST) >>TILESHIFT) == tiley )
				return;
		}
		check = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(actorat[tilex][tiley-1]);
		if (check && ((check->y+MINDIST) >> TILESHIFT) == tiley )
			return;
		check = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(actorat[tilex][tiley+1]);
		if (check && ((check->y-MINDIST) >> TILESHIFT) == tiley )
			return;
	}


//
// play door sound if in a connected area
//
#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS
	area = *(mapsegs[0] + farmapylookup[tiley] + tilex) - AREATILE;
	if (areabyplayer[area])
		PlaySoundLocTile(CLOSEDOORSND,tilex,tiley);	// JAB

	DoorObjList_SetAction(door, dr_closing);
#else
	area = *(mapsegs[0] + farmapylookup[doorobjlist[door].tiley]
			+doorobjlist[door].tilex)-AREATILE;
	if (areabyplayer[area])
	{
		PlaySoundLocTile(CLOSEDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);	// JAB
	}

	doorobjlist[door].action = dr_closing;
#endif
//
// make the door space solid
//
	/*(id0_unsigned_t)*/actorat[tilex][tiley]
		= door | 0x80;
}



/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
=
=====================
*/

void OperateDoor (id0_int_t door)
{
	id0_int_t	lock;

#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS
	lock = DoorObjList_GetLock(door);
#else
	lock = doorobjlist[door].lock;
#endif
	if (lock >= dr_lock1 && lock <= dr_lock4)
	{
		if ( ! (gamestate.keys & (1 << (lock-dr_lock1) ) ) )
		{
			SD_PlaySound (NOWAYSND);		// locked
			return;
		}
	}

#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS
	switch (DoorObjList_GetAction(door))
#else
	switch (doorobjlist[door].action)
#endif
	{
	case dr_closed:
	case dr_closing:
		OpenDoor (door);
		break;
	case dr_open:
	case dr_opening:
		CloseDoor (door);
		break;
	}
}


//===========================================================================

/*
===============
=
= DoorOpen
=
= Close the door after three seconds
=
===============
*/

// REFKEEN: Function is static, clarifying it's called with a valid door index
static void DoorOpen (id0_int_t door)
{
	if ( (doorobjlist[door].ticcount += tics) >= OPENTICS)
		CloseDoor (door);
}



/*
===============
=
= DoorOpening
=
===============
*/

// REFKEEN: Function is static, clarifying it's called with a valid door index
static void DoorOpening (id0_int_t door)
{
	id0_int_t		area1,area2;
	id0_unsigned_t	id0_far	*map;
	id0_long_t	position;

	position = doorposition[door];
	if (!position)
	{
	//
	// door is just starting to open, so connect the areas
	//
		map = mapsegs[0] + farmapylookup[doorobjlist[door].tiley]
			+doorobjlist[door].tilex;

		if (doorobjlist[door].vertical)
		{
			area1 =	*(map+1);
			area2 =	*(map-1);
		}
		else
		{
			area1 =	*(map-mapwidth);
			area2 =	*(map+mapwidth);
		}
		area1 -= AREATILE;
		area2 -= AREATILE;
		areaconnect[area1][area2]++;
		areaconnect[area2][area1]++;
		ConnectAreas ();
		if (areabyplayer[area1])
		{
			PlaySoundLocTile(OPENDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);	// JAB
		}
	}

//
// slide the door by an adaptive amount
//
	position += tics<<10;
	if (position >= 0xffff)
	{
	//
	// door is all the way open
	//
		position = 0xffff;
		doorobjlist[door].ticcount = 0;
		doorobjlist[door].action = dr_open;
		actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
	}

	doorposition[door] = position;
}


/*
===============
=
= DoorClosing
=
===============
*/

// REFKEEN: Function is static, clarifying it's called with a valid door index
static void DoorClosing (id0_int_t door)
{
	id0_int_t		area1,area2,move;
	id0_unsigned_t	id0_far	*map;
	id0_long_t	position;
	id0_int_t		tilex,tiley;

	tilex = doorobjlist[door].tilex;
	tiley = doorobjlist[door].tiley;

	if ( ((id0_unsigned_t)actorat[tilex][tiley] != (door | 0x80))
	|| (player->tilex == tilex && player->tiley == tiley) )
	{			// something got inside the door
		OpenDoor (door);
		return;
	};

	position = doorposition[door];

//
// slide the door by an adaptive amount
//
	position -= tics<<10;
	if (position <= 0)
	{
	//
	// door is closed all the way, so disconnect the areas
	//
		position = 0;

		doorobjlist[door].action = dr_closed;

		map = mapsegs[0] + farmapylookup[doorobjlist[door].tiley]
			+doorobjlist[door].tilex;

		if (doorobjlist[door].vertical)
		{
			area1 =	*(map+1);
			area2 =	*(map-1);
		}
		else
		{
			area1 =	*(map-mapwidth);
			area2 =	*(map+mapwidth);
		}
		area1 -= AREATILE;
		area2 -= AREATILE;
		areaconnect[area1][area2]--;
		areaconnect[area2][area1]--;

		ConnectAreas ();
	}

	doorposition[door] = position;
}




/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/

void MoveDoors (void)
{
	id0_int_t		door;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (gamestate.victoryflag)		// don't move door during victory sequence
		return;
#endif

	for (door = 0 ; door < doornum ; door++)
		switch (doorobjlist[door].action)
		{
		case dr_open:
			DoorOpen (door);
			break;

		case dr_opening:
			DoorOpening(door);
			break;

		case dr_closing:
			DoorClosing(door);
			break;
		}
}


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
=============================================================================

						PUSHABLE WALLS

=============================================================================
*/

id0_unsigned_t	pwallstate;
id0_unsigned_t	pwallpos;			// amount a pushable wall has been moved (0-63)
id0_unsigned_t	pwallx,pwally;
id0_int_t			pwalldir;

/*
===============
=
= PushWall
=
===============
*/

void PushWall (id0_int_t checkx, id0_int_t checky, id0_int_t dir)
{
	id0_int_t		oldtile;

	if (pwallstate)
	  return;


	oldtile = tilemap[checkx][checky];
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	oldtile &= 0xFFDF;
#endif
	if (!oldtile)
		return;

	switch (dir)
	{
	case di_north:
		if (actorat[checkx][checky-1])
		{
			SD_PlaySound (NOWAYSND);
			return;
		}
		/*(id0_unsigned_t)*/actorat[checkx][checky-1] =
		tilemap[checkx][checky-1] = oldtile;
		break;

	case di_east:
		if (actorat[checkx+1][checky])
		{
			SD_PlaySound (NOWAYSND);
			return;
		}
		/*(id0_unsigned_t)*/actorat[checkx+1][checky] =
		tilemap[checkx+1][checky] = oldtile;
		break;

	case di_south:
		if (actorat[checkx][checky+1])
		{
			SD_PlaySound (NOWAYSND);
			return;
		}
		/*(id0_unsigned_t)*/actorat[checkx][checky+1] =
		tilemap[checkx][checky+1] = oldtile;
		break;

	case di_west:
		if (actorat[checkx-1][checky])
		{
			SD_PlaySound (NOWAYSND);
			return;
		}
		/*(id0_unsigned_t)*/actorat[checkx-1][checky] =
		tilemap[checkx-1][checky] = oldtile;
		break;
	}

	gamestate.secretcount++;
	pwallx = checkx;
	pwally = checky;
	pwalldir = dir;
	pwallstate = 1;
	pwallpos = 0;
	tilemap[pwallx][pwally] |= 0xc0;
	*(mapsegs[1]+farmapylookup[pwally]+pwallx) = 0;	// remove P tile info

	SD_PlaySound (PUSHWALLSND);
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	pwallnoise = 0;
#endif
}



/*
=================
=
= MovePWalls
=
=================
*/

void MovePWalls (void)
{
	id0_int_t		oldblock,oldtile;

	if (!pwallstate)
		return;

	oldblock = pwallstate/128;

	pwallstate += tics;

	if (pwallstate/128 != oldblock)
	{
	// block crossed into a new block
		oldtile = tilemap[pwallx][pwally] & 63;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		oldtile &= 0xFFDF;
#endif

		//
		// the tile can now be walked into
		//
		tilemap[pwallx][pwally] = 0;
		/*(id0_unsigned_t)*/actorat[pwallx][pwally] = 0;
		*(mapsegs[0]+farmapylookup[pwally]+pwallx) = player->areanumber+AREATILE;

		//
		// see if it should be pushed farther
		//
		// *** S3DNA RESTORATION ***
		// Actually bugfix for a well-known pushwall bug
#ifdef GAMEVER_NOAH3D
		if (pwallstate>=256)
#else
		if (pwallstate>256)
#endif
		{
		//
		// the block has been pushed two tiles
		//
			pwallstate = 0;
			return;
		}
		else
		{
			switch (pwalldir)
			{
			case di_north:
				pwally--;
				if (actorat[pwallx][pwally-1])
				{
					pwallstate = 0;
					return;
				}
				/*(id0_unsigned_t)*/actorat[pwallx][pwally-1] =
				tilemap[pwallx][pwally-1] = oldtile;
				break;

			case di_east:
				pwallx++;
				if (actorat[pwallx+1][pwally])
				{
					pwallstate = 0;
					return;
				}
				/*(id0_unsigned_t)*/actorat[pwallx+1][pwally] =
				tilemap[pwallx+1][pwally] = oldtile;
				break;

			case di_south:
				pwally++;
				if (actorat[pwallx][pwally+1])
				{
					pwallstate = 0;
					return;
				}
				/*(id0_unsigned_t)*/actorat[pwallx][pwally+1] =
				tilemap[pwallx][pwally+1] = oldtile;
				break;

			case di_west:
				pwallx--;
				if (actorat[pwallx-1][pwally])
				{
					pwallstate = 0;
					return;
				}
				/*(id0_unsigned_t)*/actorat[pwallx-1][pwally] =
				tilemap[pwallx-1][pwally] = oldtile;
				break;
			}

			tilemap[pwallx][pwally] = oldtile | 0xc0;
		}
	}


	pwallpos = (pwallstate/2)&63;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	pwallnoise += tics;
	if (pwallnoise > 8)
	{
		SD_PlaySound (PUSHWALLSND);
		pwallnoise -= 8;
	}
#endif

}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

// (REFKEEN) Used for patching version-specific stuff
id0_word_t refkeen_compat_wl_act1_statobjoffset;

void RefKeen_Patch_wl_act1(void)
{
#ifdef REFKEEN_VER_WL920312
	refkeen_compat_wl_act1_statobjoffset = 0x67B6;
#elif (defined REFKEEN_VER_WL1AP10)
	refkeen_compat_wl_act1_statobjoffset = 0x84CC;
#elif (defined REFKEEN_VER_WL1AP11)
	refkeen_compat_wl_act1_statobjoffset = 0x7B3E;
#elif (defined REFKEEN_VER_WL6AP11)
	refkeen_compat_wl_act1_statobjoffset = UPLOAD ? 0x7B4E : 0x7C2E;
#elif (defined REFKEEN_VER_WL1AP14)
	refkeen_compat_wl_act1_statobjoffset = UPLOAD ? 0x7F5A : 0x803A;
#elif (defined REFKEEN_VER_WL6GT14A) || (defined REFKEEN_VER_WL6GT14B)
	refkeen_compat_wl_act1_statobjoffset = 0x804A;
#elif (defined REFKEEN_VER_WJ6IM14)
	refkeen_compat_wl_act1_statobjoffset = 0x7C7A;
#elif (defined REFKEEN_VER_WL6AC14)
	refkeen_compat_wl_act1_statobjoffset = 0x893C;
#elif (defined REFKEEN_VER_SDMFG10)
	refkeen_compat_wl_act1_statobjoffset = 0x765A;
#elif (defined REFKEEN_VER_SODFG10)
	refkeen_compat_wl_act1_statobjoffset = 0x78BA;
#elif (defined REFKEEN_VER_SODFG14)
	refkeen_compat_wl_act1_statobjoffset = 0x78AA;
#elif (defined REFKEEN_VER_SODAC14)
	refkeen_compat_wl_act1_statobjoffset = 0x853C;
#elif (defined REFKEEN_VER_N3DWT10)
	refkeen_compat_wl_act1_statobjoffset = 0x83C2;
#endif
}

REFKEEN_NS_E
