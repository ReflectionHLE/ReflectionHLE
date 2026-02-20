/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
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

// The lump list has been moved into LUMPLIST.H to make sure the lumps in the
// enum and the start and end values in the lumpstart and lumpend arrays are
// always in the correct order, even if a modder decides to add or remove some
// lumps.

enum
{
#define MAKELUMP(x) x##_LUMP
#include "lumplist.h"
#undef MAKELUMP
	NUMLUMPS
};

static Uint16 lumpstart[] = {
#define MAKELUMP(x) x##_LUMP_START
#include "lumplist.h"
#undef MAKELUMP
};
static Uint16 lumpend[] = {
#define MAKELUMP(x) x##_LUMP_END
#include "lumplist.h"
#undef MAKELUMP
};


#ifdef BETA
const id0_char_t *levelnames[18] =
#else
const id0_char_t *levelnames[25] =
#endif
{
#if EPISODE == 1
	"DownTown Metro",
	"High Rise",
	"City Park",
	"South Side",
	"Sewer",
	"Cave",
	"Skull Man",
	"Elevator",
	"No Man's Land",
	"Lab Entrance",
	"Computer Core",
	"Mangle's Last Stand",
#elif EPISODE == 2
	"Outer Woods",
	"Main Corridor",
	"Ant Caves",
	"Ant Town",
	"The Queen",
	"Trash Dump",
	"Trash Boss",
	"Electronics Lab",
	"Genetics Lab",
	"Specimens Lab",
	"Killer Pillers",
#ifdef BETA
	"Dr. Mangle",
#else
	"The Enforcer",
#endif
#elif EPISODE == 3
	"Barricade",
	"Fort Entrance",
	"Guard Tower",
	"Cyborg Trouble",
	"Goliath!",
	"Armory",
	"Mutants Attack",
	"Robots Attack",
	"Utility Maintenance",
	"Main Control Center",
	"Circle of Death",
	"Master Cain",
#endif
};

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

static boolean lumpneeded[NUMLUMPS];

#define NEED(x) lumpneeded[x##_LUMP] = true

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

//==========================================================================

/*
==========================
=
= ScanInfoPlane
=
= Spawn all actors and mark down special places
=
==========================
*/

void ScanInfoPlane(void)
{
	Uint16 x, y, i, j, value;
	Uint16 id0_far *infoptr;

	InitObjArray();
	memset(lumpneeded, 0, sizeof(lumpneeded));
	infoptr = mapsegs[2];
	for (y=0; y<mapheight; y++)
	{
		for (x=0; x<mapwidth; x++)
		{
			value = *(infoptr++);
			if (value == 0)
			{
				continue;
			}
			switch (value)
			{
			case 1:
				SpawnPlayer(x, y, -1);
				NEED(PLAYER);
				break;
			case 2:
				SpawnPlayer(x, y, 1);
				NEED(PLAYER);
				break;
			case 3:
				SpawnWorm(x, y);
				NEED(PINKWORM);
				newobj->active = no;
				break;
			case 4:
#if (EPISODE == 1)
				SpawnDevil(x, y);
				NEED(DEVIL);
#else
				SpawnKamikaze(x, y);
				NEED(KAMIKAZE);
#endif
				newobj->active = no;
				break;
			case 5:
#if (EPISODE == 1)
				SpawnPlasmaWorm(x, y);
				NEED(PLASMAWORM);
				newobj->active = no;
#elif (EPISODE == 2)
				SpawnTrashBoss(x, y);
				NEED(TRASHBOSS);
				newobj->active = no;
#elif (EPISODE == 3)
				SpawnHairy(x, y);
				NEED(HAIRY);
#endif
				break;
#if (EPISODE != 1)
			case 6:
				SpawnBug(x, y);
				NEED(BUG);
				newobj->active = no;
				break;
#endif
			case 7:
#if (EPISODE == 1)
				SpawnSewerman(x, y);
				NEED(SEWERMAN);
#else
				SpawnBee(x, y);
				NEED(BEE);
#endif
				break;
			case 8:
#if (EPISODE == 1)
				SpawnSlimer(x, y);
				NEED(SLIMER);
#else
				SpawnBlueDevil(x, y);
				NEED(BLUEDEVIL);
#endif
				break;
			case 9:
				SpawnBonus(x, y, 9);
				NEED(PLAYER);
				newobj->active = no;
				break;
			case 10:
				SpawnPunchy(x, y);
				NEED(PUNCHY);
				break;
			case 11:
				SpawnGorp(x, y);
				NEED(GORP);
				break;
			case 12:
				SpawnCobra(x, y);
				NEED(COBRA);
				break;
			case 13:
#if (EPISODE == 1)
				SpawnHairy(x, y);
				NEED(HAIRY);
#elif (EPISODE == 2)
				SpawnJim(x, y);
				NEED(PEOPLE);
#elif (EPISODE == 3)
				SpawnGoliath(x, y);
				NEED(GOLIATH);
#endif
				break;
#if (EPISODE == 2)
			case 14:
				SpawnGeorge(x, y);
				NEED(PEOPLE);
				break;
			case 15:
				SpawnKeen(x, y);
				NEED(PEOPLE);
				break;
#elif (EPISODE == 3)
			case 14:
				SpawnGrenadeBot(x, y);
				NEED(GRENADEBOT);
				break;
			case 15:
				SpawnCyborg(x, y);
				NEED(CYBORG);
				break;
#endif
			case 16:
				SpawnBonus(x, y, 16);
				newobj->active = no;
				break;
			case 17:
				SpawnBonus(x, y, 17);
				newobj->active = no;
				break;
			case 18:
				SpawnBonus(x, y, 18);
				newobj->active = no;
				break;
			case 19:
				SpawnBonus(x, y, 19);
				newobj->active = no;
				break;
			case 20:
				SpawnBonus(x, y, 20);
				newobj->active = no;
				break;
			case 21:
				SpawnBonus(x, y, 21);
				newobj->active = no;
				break;
			case 22:
				SpawnBonus(x, y, 22);
				newobj->active = no;
				break;
			case 23:
				SpawnBonus(x, y, 23);
				newobj->active = no;
				break;
			case 24:
				SpawnBonus(x, y, 24);
				newobj->active = no;
				break;
			case 25:
				SpawnBonus(x, y, 25);
				newobj->active = no;
				break;
			case 26:
				SpawnBonus(x, y, 26);
				newobj->active = no;
				break;
			case 27:
				SpawnBonus(x, y, 27);
				newobj->active = no;
				break;
			case 28:
				SpawnBonus(x, y, 28);
				newobj->active = no;
				break;
			case 29:
				SpawnBonus(x, y, 29);
				newobj->active = no;
				break;
			case 30:
				SpawnBonus(x, y, 30);
				newobj->active = no;
				break;
#if (EPISODE == 1)
			case 31:
				SpawnHostage(x, y, 0);
				NEED(HOSTAGE);
				break;
			case 32:
				SpawnHostage(x, y, 1);
				NEED(HOSTAGE);
				break;
			case 33:
				SpawnHostage(x, y, 2);
				NEED(HOSTAGE);
				break;
#else
			case 31:
				SpawnHostage1(x, y);
				NEED(HOSTAGE);
				break;
			case 32:
				SpawnHostage2(x, y);
				NEED(HOSTAGE);
				break;
			case 33:
				SpawnHostage3(x, y);
				NEED(HOSTAGE);
				break;
#endif
			case 34:
				RF_SetScrollBlock(x, y, true);
				break;
			case 35:
				RF_SetScrollBlock(x, y, false);
				break;
#ifndef BETA
			case 36:
				SpawnBonus(x, y, 36);
				NEED(PLAYER);
				break;
			case 37:
				SpawnBonus(x, y, 37);
				NEED(PLAYER);
				break;
			case 38:
				SpawnBonus(x, y, 38);
				NEED(PLAYER);
				break;
			case 39:
				SpawnBonus(x, y, 39);
				NEED(PLAYER);
				break;
#if (EPISODE == 1)
			case 40:
				SpawnSkullman(x, y);
				NEED(SKULLMAN);
				break;
			case 41:
				SpawnSkullHand(x, y, 0);
				NEED(SKULLMAN);
				break;
			case 42:
				SpawnSkullHand(x, y, 1);
				NEED(SKULLMAN);
				break;
#elif (EPISODE == 3)
			case 40:
				SpawnDevil(x, y);
				NEED(DEVIL);
				break;
			case 41:
				SpawnSlimer(x, y);
				NEED(SLIMER);
				break;
			case 42:
				SpawnEnemyLandmine(x, y);
				NEED(EXPLOSIVES);
				break;
#endif
			case 43:
				SpawnAsteroid(x, y);
				NEED(INTRO);
				break;
#endif	//ifndef BETA
			case 44:
				SpawnBosshead(x, y);
				NEED(BOSS);
				break;
			case 45:
#if (EPISODE == 1)
				SpawnMangle(x, y);
				NEED(BOSS);
#elif (EPISODE == 2)
				SpawnAnt(x, y);
				NEED(ANT);
#elif (EPISODE == 3)
				SpawnBarrel(x, y);
				NEED(EXPLOSIVES);
#endif
				break;
			case 46:
				SpawnBouncer(x, y);
				NEED(BOUNCER);
				break;
#if (EPISODE != 1)
			case 47:
				SpawnTrashTruck(x, y);
				NEED(TRASHTRUCK);
				break;
#endif
			case 48:
				SpawnTurret(x, y);
				NEED(TURRET);
				break;
			case 49:
				SpawnGuard(x, y);
				NEED(GUARD);
				break;
#if (EPISODE != 2)
			case 51:
				SpawnJumpbomb(x, y);
				NEED(JUMPBOMB);
				break;
#endif
			case 52:
				SpawnBonus(x, y, 52);
				newobj->active = no;
				break;
			case 53:
				SpawnBonus(x, y, 53);
				newobj->active = no;
				break;
#ifdef BETA
			case 54:
			case 55:
			case 56:
			case 57:
				SpawnShuttle(x, y, (arrowdirtype)(value+(arrow_North-54)));
				NEED(SHUTTLE);
				break;
#else
			case 54:
				SpawnBonus(x, y, 54);
				newobj->active = no;
				break;
			case 55:
				SpawnBeacon(x, y);
				NEED(BEACON);
				break;
#if (EPISODE == 3)
			case 56:
			case 57:
				SpawnShuttle(x, y, (arrowdirtype)(value+(arrow_North-56)));
				NEED(SHUTTLE);
				break;
#endif
#endif
			case 66:
			case 67:
			case 68:
			case 69:
				SpawnPlatform(x, y, (arrowdirtype)(value+(arrow_North-66)));
				NEED(PLATFORM);
				break;
#if (EPISODE != 2)
			case 72:
				SpawnPlasmashooter(x, y);
				NEED(PLASMA);
				break;
#endif
			case 74:
				SpawnRobopal(x, y);
				NEED(ROBOPAL);
				break;
			case 75:
				SpawnBonus(x, y, 75);
				newobj->active = no;
				break;
			case 76:
				SpawnBonus(x, y, 76);
				newobj->active = no;
				break;
			case 77:
				SpawnBonus(x, y, 77);
				newobj->active = no;
				break;
			case 78:
				SpawnBonus(x, y, 78);
				newobj->active = no;
				break;
			case 79:
				SpawnBonus(x, y, 79);
				newobj->active = no;
				break;
			case 80:
				SpawnTank(x, y);
				NEED(TANK);
				break;
			case 81:
				SpawnPushableBlock(x, y);
				NEED(BLOCK);
				break;
			case 82:
				SpawnDrool(x, y);
				NEED(ACIDMONSTER);
				break;
			case 83:
#if (EPISODE == 1)
				SpawnHelicopter(x, y, arrow_North);
				NEED(HELICOPTER);
#elif (EPISODE == 2)
				SpawnQueen(x, y, arrow_North);
				NEED(QUEENANT);
#elif (EPISODE == 3)
				SpawnCain(x, y);
				NEED(CAIN);
#endif
				break;
#if (EPISODE != 1)
			case 84:
				SpawnCaterpillar(x, y, arrow_North, 0);
				NEED(CATERPILLAR);
				break;
			case 85:
				SpawnCaterpillar(x, y, arrow_North, 1);
				NEED(CATERPILLAR);
				break;
			case 86:
				SpawnSpikey(x, y);
				NEED(SPIKEY);
				break;
#endif
			case 87:
				SpawnCrusher(x, y);
				NEED(CRUSHER);
				break;
			case 88:
				SpawnFallingBrick(x, y);
				NEED(BRICK);
				break;
#ifndef BETA
			case 89:
				SpawnApogee(x, y, arrow_North);
				NEED(INTRO);
				break;
#endif
			}
			if (newobj->active != allways)
			{
				newobj->active = no;
			}
		}
	}
	CA_MarkGrChunk(SCOREBOXSPR);
	SpawnScore();
	for (i=0; i<NUMLUMPS; i++)
	{
		if (lumpneeded[i])
		{
			for (j=lumpstart[i]; j<=lumpend[i]; j++)
			{
				CA_MarkGrChunk(j);
			}
		}
	}
}

//==========================================================================

/*
NOTE:

All of the dialog functions below copy the message string from far memory into
a local near buffer (on the stack). This would have been necessary for the
original ID Engine as used in Keen 4-6, but BioMenace uses a modified version
of the US_CPrint() function that can accept far strings. BioMenace's version of
US_CPrint() also allocates a local 200 byte buffer and copies the string into
that buffer, allowing the string to be processed by the other print routines.
That means allocating a local string buffer and copying the string into that
buffer is not necessary in these dialog functions. The code effectively wastes
200 bytes of stack space for an unnecessary copy of the string.

###############################################################################
# WARNING: You cannot use US_CPrint() on strings that are longer than 199     #
# characters, no matter if that string is near or far! That will cause severe #
# problems and will most likely result in a system crash.                     #
#                                                                             #
# I STRONGLY recommend that you modify the US_CPrint() and US_CPrintLine()    #
# code in ID_US_1.C to accept far pointers without copying the string into a  #
# temporary local (near) buffer. That would get rid of the length limitation. #
# See ID_US_1.C for details.                                                  #
###############################################################################

If you want to optimize the code a bit, you could replace
	char messagetext[200];
with
	char far *messagetext;

After this change, you must also replace
	_fstrcpy(messagetext, msg_...);
with
	messagetext = msg_...;

But it might be better to use the implementation from ALT_SPEC.C instead of the
code found in here if you want to write your own customized dialog for the game.
The code in that file boils everything down to a single switch statement without
any loops. The code will get a bit smaller and easier to modify. And the macros
used in that file allow you to simply type the string as a parameter directly
where it will be used instead of having to declare it as a separate far array
of chars.

I think this would make modding the game a LOT easier, since the order of the
messages, the images and the actual contents of the message strings are all
defined in one place. And you don't need to worry about variables not being
initialized, since there are no loop variables anymore and unhandled map numbers
simply would not show any message windows at all.
*/

//==========================================================================

#if (EPISODE == 1)
/*
=============================================================================

						 DIALOG MESSAGES -- EPISODE ONE

=============================================================================
*/

/*
====================
=
= HostageDialog
=
====================
*/

void HostageDialog(void)
{
	static const id0_char_t id0_far msg_hostage0[] =
		"Somewhere in each level, there\n"
		"is someone like me who holds a key\n"
		"to each exit.  You must find them!\n";
	static const id0_char_t id0_far msg_hostage1[] =
		"Thanks for the key.  I'll use\n"
		"this to go on to the next level\n"
		"and rescue your friends.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage2[] =
		"Thank you. Dr. Mangle has captured\n"
		"others like me. He said it would be\n"
		"a trail for you to follow so that\n"
		"you could find him and meet your\n"
		"final fate. Be careful!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage3[] =
		"I'm gonna dust that little dweeb!\n"
		"He can't do this and escape!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage4[] =
		"Thank you, Sir!\n"
		"I can't believe what has\n"
		"happened to our beautiful city!\n"
		"They've destroyed everything!\n"
		"Please!  You have to stop them!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage5[] =
		"Ya, it's really a pile!  I'll do\n"
		"my best to clear these mutants out\n"
		"of here. Mangle WON'T succeed!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage6[] =
		"Hey, be careful buddy! These\n"
		"buildings are, excuse the pun,\n"
		"crawling with, like, these weird\n"
		"creature-things, man! Hey, WOW!\n"
		"What a cool gun! Can I try it out?\n"
		"\n";
	static const id0_char_t id0_far msg_hostage7[] =
		"Na, you'd probably shoot your foot\n"
		"off or something. You'd better get\n"
		"out of here before the mutants\n"
		"come creeping back this way....\n"
		"\n";
	static const id0_char_t id0_far msg_hostage8[] =
		"Aaaaaaaahhhhhhh! AAAAAAaaaahhhh!\n"
		"Help! Those green things spit\n"
		"slime on me!  I just know I will\n"
		"dissolve or something, right?\n"
		"\n";
	static const id0_char_t id0_far msg_hostage9[] =
		"Geez, take a valium or something!\n"
		"You look all right to me....\n"
		"\n";
	static const id0_char_t id0_far msg_hostage10[] =
		"Thank you, Mr. Logan!  I can't\n"
		"wait to get home and take a nice,\n"
		"hot bath!  I've been locked in\n"
		"here for days waiting for\n"
		"someone to show up and get\n"
		"me out of here.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage11[] =
		"Heh...Um, well, I hate to be the\n"
		"one to give you the news...but...\n"
		"there are no houses left in this\n"
		"entire city.  Sorry...\n"
		"\n";
	static const id0_char_t id0_far msg_hostage12[] =
		"WOW! What a light show!  I saw you\n"
		"kick'n robotic butt a mile away!\n"
		"I love this destruction stuff!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage13[] =
		"Ya, me too.  Those parachuting\n"
		"robots were a new one on me.\n"
		"Fun to kill though.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage14[] =
		"Mangle is somewhere in this\n"
		"complex. Be very careful!  I've\n"
		"seen him setting up all kinds of\n"
		"traps around here.\n"
		"He must be expecting you.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage15[] =
		"Yes, he knows I'm coming for him.\n"
		"But he's not expecting what I'm\n"
		"going to do to him when I\n"
		"get my hands on him!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage16[] =
		"Thank God you're here!  I know\n"
		"that Dr. Mangle is in the next\n"
		"room waiting for you.  But be\n"
		"careful!  Not everything is as\n"
		"it seems.  I think it's a trap!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage17[] =
		"Thanks for the tip....  But I've\n"
		"learned the way this creep works.\n"
		"He's not going to get away with\n"
		"this!  His luck's just run out!\n"
		"\n";

	Sint16 i, first, count, oldmusic;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	oldmusic = currentmusic;
	switch (gamestate.mapon)
	{
	case 0:
		first = 0;
		count = 2;
		break;
	case 1:
		first = 2;
		count = 2;
		break;
	case 2:
		first = 4;
		count = 2;
		break;
	case 3:
		first = 6;
		count = 2;
		break;
	case 4:
		first = 8;
		count = 2;
		break;
	case 5:
		first = 10;
		count = 2;
		break;
	case 8:
		first = 12;
		count = 2;
		break;
	case 9:
		first = 14;
		count = 2;
		break;
	case 10:
		first = 16;
		count = 2;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	CA_UpLevel();
	// BUG: should call CA_ClearMarks() here to make sure we don't cache more than we need
	CA_MarkGrChunk(HOSTAGE1PIC);
	CA_MarkGrChunk(HOSTAGE2PIC);
	CA_MarkGrChunk(HOSTAGE3PIC);
	CA_MarkGrChunk(SNAKELOGANPIC);
	CA_CacheMarks(NULL);
	VW_FixRefreshBuffer();
	StartMusic(MUS_HOSTAGE);
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(36, 9);
		switch (i)
		{
		case 0:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 1:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 2:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 3:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 4:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 5:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 6:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 7:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 8:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 9:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 10:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 11:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 12:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 13:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 14:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 15:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 16:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 17:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		}
		PrintY += 6;
		WindowW -= 48;
		WindowX += 48;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_hostage0);
			break;
		case 1:
			strcpy(messagetext, msg_hostage1);
			break;
		case 2:
			strcpy(messagetext, msg_hostage2);
			break;
		case 3:
			strcpy(messagetext, msg_hostage3);
			break;
		case 4:
			strcpy(messagetext, msg_hostage4);
			break;
		case 5:
			strcpy(messagetext, msg_hostage5);
			break;
		case 6:
			strcpy(messagetext, msg_hostage6);
			break;
		case 7:
			strcpy(messagetext, msg_hostage7);
			break;
		case 8:
			strcpy(messagetext, msg_hostage8);
			break;
		case 9:
			strcpy(messagetext, msg_hostage9);
			break;
		case 10:
			strcpy(messagetext, msg_hostage10);
			break;
		case 11:
			strcpy(messagetext, msg_hostage11);
			break;
		case 12:
			strcpy(messagetext, msg_hostage12);
			break;
		case 13:
			strcpy(messagetext, msg_hostage13);
			break;
		case 14:
			strcpy(messagetext, msg_hostage14);
			break;
		case 15:
			strcpy(messagetext, msg_hostage15);
			break;
		case 16:
			strcpy(messagetext, msg_hostage16);
			break;
		case 17:
			strcpy(messagetext, msg_hostage17);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
	}
	CA_DownLevel();
	RF_ForceRefresh();
	ResetScoreObj();
/*
	Note:
	It would be better to call StopMusic() before calling CA_DownLevel()
	to avoid "out of memory" crashes. The only downside of this is that
	there would be no music while the game re-caches the graphics.
	However, that is just a minor inconvenience compared to having the
	game crash.
*/
	StopMusic();
	StartMusic(oldmusic);
	// BUG: Starting a new music track might move memory buffers around!
	// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
	// with animated tiles.

	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

//==========================================================================

/*
====================
=
= HintDialog
=
====================
*/

// This hint message is also used in BossDialog() below, so it must be declared
// outside the HintDialog() function. This might indicate that the all the
// message strings in this file were originally declared outside the functions.
static const id0_char_t id0_far msg_hint0[] =
	"If you have a SuperVGA graphics card\n"
	"and experience video problems such\n"
	"as garbage on the screen, select\n"
	"the <compatability mode> option from\n"
	"the main menu CONFIGURE/OPTIONS.\n"
	"\n";

void HintDialog(void)
{
	Sint16 i, first, count;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	switch (gamestate.mapon)
	{
	case 0:
		first = 0;
		count = 1;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	VW_FixRefreshBuffer();
	if (gamestate.mapon != 6)
	{
		StartMusic(MUS_HOSTAGE);
	}
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(30, 8);
		PrintY += 2;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_hint0);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
		RF_ForceRefresh();
	}
	if (gamestate.mapon != 6)
	{
		StopMusic();
		StartMusic(gamestate.mapon);
		// BUG: Starting a new music track might move memory buffers around!
		// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
		// with animated tiles.
	}
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

//==========================================================================

/*
====================
=
= BossDialog
=
====================
*/

void BossDialog(void)
{
	static const id0_char_t id0_far msg_boss0[] =
		"Greetings Mr. Logan!\n"
		"You will never stop me!\n"
		"I will kill you and toss\n"
		"your bones to my critters.\n"
		"\n";
	static const id0_char_t id0_far msg_boss1[] =
		"And then the world will bow to the\n"
		"will of Master Cain and Dr. Mangle!\n"
		"Ha ha ha ha ha....\n"
		"\n";
	static const id0_char_t id0_far msg_boss2[] =
		"Well, well, well....Here we are,\n"
		"Mr. Logan.  It has come down to just\n"
		"you...and to me.  I hope you have\n"
		"enjoyed the little party I've thrown\n"
		"\n";
	static const id0_char_t id0_far msg_boss3[] =
		"for you today.  I hope you've had fun\n"
		"with my little friends. But now, I'm\n"
		"afraid, it is time for you to die!\n"
		"\n";
	static const id0_char_t id0_far msg_boss4[] =
		"Mr. Logan, listen to me carefully.\n"
		" *-COUGH-* I am beaten, I'm going to\n"
		"die. *-ACK!-*, so I have nothing to\n"
		"hide and I owe nothing to that creep,\n";
	static const id0_char_t id0_far msg_boss5[] =
		"Master Cain. I only did this because\n"
		"he promised me wealth and power, and\n"
		"he also said he'd kill me if I didn't\n"
		"help him. So you can see how I might\n";
	static const id0_char_t id0_far msg_boss6[] =
		"be compelled to create these mutants\n"
		"for him.  *-COUGH-* I have gotten\n"
		"what I deserve for my crimes. But you\n"
		"must stop Cain to stop this madness!\n";
	static const id0_char_t id0_far msg_boss7[] =
		"There is an escape jet just through\n"
		"that door. I've uploaded navigation\n"
		"coordinates to get you to the next\n"
		"lab.  Enter and destroy it to stop\n";
	static const id0_char_t id0_far msg_boss8[] =
		"the production of the mutant army.\n"
		"But be careful, a deadly computer\n"
		"guards the whole complex.\n"
		"You must then go on and find Cain's\n";
	static const id0_char_t id0_far msg_boss9[] =
		"secret fortress. Seek him out and\n"
		"destroy him or he will be a plague\n"
		"of destruction to this earth!\n"
		"Goodbye, Snake Logan, *-Hack-*,\n";
	static const id0_char_t id0_far msg_boss10[] =
		"and good luck...you will need it.\n"
		"*-Aaaaaarrrggg!-*  <sigh>.......\n"
		"\n";
	static const id0_char_t id0_far msg_boss11[] =
		"I am Skullman!  I shall kill\n"
		"you and collect your skull as I\n"
		"have with all these others!\n"
		"Prepare to die, sub-creature!\n"
		"\n";

	Sint16 i, first, count;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	switch (gamestate.mapon)
	{
	case 6:
		first = 11;
		count = 1;
		break;
	case 9:
		first = 0;
		count = 2;
		break;
	case 11:
		first = 2;
		count = 2;
		break;
	case 12:
		first = 4;
		count = 7;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	CA_UpLevel();
	VW_FixRefreshBuffer();
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(32, 6);
		PrintY += 2;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_boss0);
			break;
		case 1:
			strcpy(messagetext, msg_boss1);
			break;
		case 2:
			strcpy(messagetext, msg_boss2);
			break;
		case 3:
			strcpy(messagetext, msg_boss3);
			SD_PlaySound(GROWLSND);
			break;
		case 4:
			strcpy(messagetext, msg_boss4);
			break;
		case 5:
			strcpy(messagetext, msg_boss5);
			break;
		case 6:
			strcpy(messagetext, msg_boss6);
			break;
		case 7:
			strcpy(messagetext, msg_boss7);
			break;
		case 8:
			strcpy(messagetext, msg_boss8);
			break;
		case 9:
			strcpy(messagetext, msg_boss9);
			break;
		case 10:
			strcpy(messagetext, msg_boss10);
			break;
		case 11:
			strcpy(messagetext, msg_boss11);
			break;
		case 12:	// unreachable case
			strcpy(messagetext, msg_hint0);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
	}
	CA_DownLevel();
	ResetScoreObj();
	RF_ForceRefresh();
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

#elif (EPISODE == 2)

/*
=============================================================================

						 DIALOG MESSAGES -- EPISODE TWO

=============================================================================
*/

/*
====================
=
= HostageDialog
=
====================
*/

void HostageDialog(void)
{
	static const id0_char_t id0_far msg_hostage0[] =
		"Hey, thanks for rescuing me!\n"
		"I thought those creatures were\n"
		"going to have me for dinner!\n"
#ifdef BETA
		"Dr. Mangle has locked my friends\n"
		"up on the other floors.\n"
#else
		"The Enforcer has locked my\n"
		"friends up on the other floors.\n"
#endif
		"Can you help them?\n"
		"\n";
	static const id0_char_t id0_far msg_hostage1[] =
#ifdef BETA
		"I think I can manage it...\n"
#else
		"I think I can manage it....\n"
#endif
		"\n";
	static const id0_char_t id0_far msg_hostage2[] =	// never actually used
		"A hero's life is filled with\n"
#ifdef BETA
		"many hardships...\n"
#else
		"many hardships....\n"
#endif
		"\n";
	static const id0_char_t id0_far msg_hostage3[] =
#ifdef BETA
		"Thankyou for rescuing me!\n"
#else
		"Thank you for rescuing me!\n"
#endif
		"I am the engineer in this\n"
		"facility. I have a plan that\n"
		"could help you to destroy\n"
#ifdef BETA
		"Dr. Mangle's evil work...but...\n"
#else
		"Cain's evil work...but...\n"
#endif
		"it's not going to be easy.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage4[] =
		"Don't worry, I'm getting used\n"
#ifdef BETA
		"to things not being easy.\n"
		"So what's the plan?\n"
#else
		"to things not being easy!\n"
		"So, what's the plan?\n"
#endif
		"\n";
	static const id0_char_t id0_far msg_hostage5[] =
		"Before I was captured by these\n"
		"ants, I had just completed a\n"
#ifdef BETA
		"small nuclear device. It is\n"
		"still in the engineering level.\n"
#else
		"small nuclear device.\n"
#endif
		"You must find it!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage6[] =
		"Ok, I'm getting the general idea\n"
		"here. Once I find this thing,\n"
		"where should I detonate it?\n"
		"\n";
	static const id0_char_t id0_far msg_hostage7[] =
#ifdef BETA
		"The computer core is where the\n"
		"main reactor is housed. Detonate\n"
		"it there! The bomb I'm talking\n"
		"about is metal, round, and has\n"
#else
		"The main lab is where the\n"
		"self-destruct receptor is housed.\n"
		"Detonate it there! The bomb\n"
		"is metal, round, and has\n"
#endif
		"two glowing red slots in it.\n"
		"GOOD LUCK!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage8[] =
#ifdef BETA
		"Hey, thanks buddy...\n"
#else
		"Hey, thanks buddy....\n"
#endif
		"When you get to the next chamber\n"
		"be careful, it's the queen ant!\n"
		"She has tiny flying nymphs that\n"
		"always protect her. Their sting\n"
		"is deadly!";
	static const id0_char_t id0_far msg_hostage9[] =
		"Thanks for the tip!\n"
		"I'll be careful.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage10[] =
#ifdef BETA
		"Thanks for getting me outa here\n"
#else
		"Thanks for getting me outa here,\n"
#endif
		"Mr. Logan! I gotta take off\n"
		"and go stop the fiendish plot of\n"
		"Mortimer McMire!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage11[] =
#ifdef BETA
		"No sweat Captain Keen....\n";
#else
		"No sweat, Captain Keen....\n";
#endif
	static const id0_char_t id0_far msg_hostage12[] =
		"That's Commander Keen!\n"
		"COMMANDER KEEN!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage13[] =
		"I think there is a nuclear bomb\n"
		"somewhere in the next level!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage14[] =
		"Thanks, I've been looking for that!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage15[] =
		"Don't forget to find the nuclear\n"
		"bomb before you leave this level.\n"
#ifdef BETA
		"Without it you won't be able to\n"
		"stop Dr. Mangle!\n"
#else
		"Without it, you won't be able to\n"
		"destroy this facility!\n"
#endif
		"\n";
	static const id0_char_t id0_far msg_hostage16[] =
#ifdef BETA
		"Thanks sonny! I'm out of here.\n"
		"Dr. Mangle said to tell you that\n"
#else
		"Thanks, Sonny! I'm out of here!\n"
		"The Enforcer said to tell you that\n"
#endif
		"he is waiting for your wimpy self\n"
		"in the next room!\n"
		"\n";
	static const id0_char_t id0_far msg_hostage17[] =
		"WIMPY!!!!!!!?\n"
		"\n";

	Sint16 i, first, count, oldmusic;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

#ifndef BETA
	oldmusic = currentmusic;
#endif
	switch (gamestate.mapon)
	{
	case 1:
		first = 0;
		count = 2;
		break;
	case 2:
		first = 3;
		count = 5;
		break;
	case 3:
		first = 8;
		count = 2;
		break;
	case 5:
		first = 10;
		count = 3;
		break;
	case 7:
		first = 13;
		count = 2;
		break;
	case 8:
		first = 15;
		count = 1;
		break;
	case 10:
		first = 16;
		count = 2;
		break;
	}
	CA_UpLevel();
	// BUG: should call CA_ClearMarks() here to make sure we don't cache more than we need
	CA_MarkGrChunk(HOSTAGE1PIC);
	CA_MarkGrChunk(HOSTAGE2PIC);
	CA_MarkGrChunk(HOSTAGE3PIC);
	CA_MarkGrChunk(KEENPIC);
	CA_MarkGrChunk(SNAKELOGANPIC);
	CA_CacheMarks(NULL);
	VW_FixRefreshBuffer();
#ifdef BETA
	StartMusic(MUS_NO_MUSIC);
#else
	StartMusic(MUS_HOSTAGE);
#endif
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(34, 9);
		switch (i)
		{
		case 0:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 1:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 2:	// unreachable case
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 3:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 4:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 5:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 6:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 7:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 8:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 9:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 10:
			VWB_DrawPic(WindowX, WindowY+4, KEENPIC);
			break;
		case 11:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 12:
			VWB_DrawPic(WindowX, WindowY+4, KEENPIC);
			break;
		case 13:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 14:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 15:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 16:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 17:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		}
		PrintY += 6;
		WindowW -= 48;
		WindowX += 48;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_hostage0);
			break;
		case 1:
			strcpy(messagetext, msg_hostage1);
			break;
		case 2:	// unreachable case
			strcpy(messagetext, msg_hostage2);
			break;
		case 3:
			strcpy(messagetext, msg_hostage3);
			break;
		case 4:
			strcpy(messagetext, msg_hostage4);
			break;
		case 5:
			strcpy(messagetext, msg_hostage5);
			break;
		case 6:
			strcpy(messagetext, msg_hostage6);
			break;
		case 7:
			strcpy(messagetext, msg_hostage7);
			break;
		case 8:
			strcpy(messagetext, msg_hostage8);
			break;
		case 9:
			strcpy(messagetext, msg_hostage9);
			break;
		case 10:
			strcpy(messagetext, msg_hostage10);
			break;
		case 11:
			strcpy(messagetext, msg_hostage11);
			break;
		case 12:
			strcpy(messagetext, msg_hostage12);
			break;
		case 13:
			strcpy(messagetext, msg_hostage13);
			break;
		case 14:
			strcpy(messagetext, msg_hostage14);
			break;
		case 15:
			strcpy(messagetext, msg_hostage15);
			break;
		case 16:
			strcpy(messagetext, msg_hostage16);
			break;
		case 17:
			strcpy(messagetext, msg_hostage17);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
	}
	CA_DownLevel();
	RF_ForceRefresh();
	ResetScoreObj();
	StopMusic();
#ifdef BETA
	StartMusic(gamestate.mapon);
#else
	StartMusic(oldmusic);
#endif
	// BUG: Starting a new music track might move memory buffers around!
	// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
	// with animated tiles.

	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

//==========================================================================

/*
====================
=
= HintDialog
=
====================
*/

void HintDialog(void)
{
	static const id0_char_t id0_far msg_hint3[] =
		"Hey! How did you get in here!\n"
		"George! Did you leave that key\n"
		"lying around again?\n"
		"Well, as long as you're here\n"
		"we might as well introduce\n"
		"ourselves.\n"
		"\n";
	static const id0_char_t id0_far msg_hint4[] =
		"I'm Jim, and behind me is\n"
		"George, and Scott is at the\n"
		"computer. We run this game\n"
		"from in here. If you promise\n";
	static const id0_char_t id0_far msg_hint5[] =
		"not to tell about this place,\n"
		"there are plenty of supplies\n"
		"upstairs you can have to help\n"
		"you on your way....\n"
		"\n";
	static const id0_char_t id0_far msg_hint6[] =
		"Who do you think you are\n"
		"shooting at US!\n"
		"We made this game!\n"
		"\n"
#ifdef BETA
		"EAT HOT LEAD SUCKER!!!\n"
#else
		"EAT THIS LOGAN!!!\n"
#endif
		"\n";
	static const id0_char_t id0_far msg_hint0[] =
		"Remember this color sequence:\n"
		"Cyan, Red, Magenta, Green, Blue.\n";
	static const id0_char_t id0_far msg_hint1[] =
		"You may want to write it down\n"
		"on a piece of paper if you think\n"
#ifdef BETA
		"that you might not remember it...\n"
#else
		"that you might not remember it.\n"
#endif
		"\n";
#ifndef BETA
	static const id0_char_t id0_far msg_hint7[] =
		"If you have a SuperVGA graphics card\n"
		"and experience video problems such\n"
		"as garbage on the screen, select\n"
		"the <compatability mode> option from\n"
		"the main menu CONFIGURE/OPTIONS.\n"
		"\n";
#endif

	Sint16 i, first, count;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	switch (gamestate.mapon)
	{
	case 0:
#ifndef BETA
		first = 7;
		count = 1;
		break;
	case 99:
#endif
		first = 6;
		count = 1;
		break;
	case 2:
		first = 0;
		count = 2;
		break;
	case 9:
		first = 3;
		count = 3;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	VW_FixRefreshBuffer();
#ifdef BETA
	if (gamestate.mapon != 0)
	{
		// REFKEEN: Fix invalid music assignment leading to a crash,
		// if option is enabled.
		// It's possible the invalid assignment was correct before
		// reducing episode 2 from 18 levels to 12, according to
		// old beta testing notes found in README2.TXT file.
		if (g_refKeenCfg.bmenace.betaFixes)
			StartMusic(MUS_HOSTAGE);
		else
			StartMusic(20);	// BUG: This will cause a crash!
	}
#else
	if (gamestate.mapon == 99)
	{
		StartMusic(MUS_HIGHSCORELEVEL);
	}
	else if (gamestate.mapon != 0)
	{
		StartMusic(MUS_HOSTAGE);
	}
#endif
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(30, 8);
		PrintY += 2;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_hint0);
			break;
		case 1:
			strcpy(messagetext, msg_hint1);
			break;
		case 3:
			strcpy(messagetext, msg_hint3);
			hintstate = 3;
			break;
		case 4:
			strcpy(messagetext, msg_hint4);
			break;
		case 5:
			strcpy(messagetext, msg_hint5);
			break;
		case 6:
			strcpy(messagetext, msg_hint6);
			break;
#ifndef BETA
		case 7:
			strcpy(messagetext, msg_hint7);
			break;
#endif
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
		RF_ForceRefresh();
	}
#ifdef BETA
	if (gamestate.mapon != 0)
#else
	if (gamestate.mapon != 0 && gamestate.mapon != 99)
#endif
	{
		StopMusic();
		StartMusic(gamestate.mapon);
		// BUG: Starting a new music track might move memory buffers around!
		// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
		// with animated tiles.
	}
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
	// Note: This code tries to leave the highscore music playing after the
	// level 99 message (Jim's response after being attacked), but that just
	// won't work. As soon as the player gets hurt, the invincibility will
	// cause the regular level music to be played again (see PlayLoop in
	// BM_PLAY.C). And opening the main menu will also start the regular level
	// music again when returning to the game.
}

//==========================================================================

/*
====================
=
= BossDialog
=
====================
*/

void BossDialog(void)
{
	static const id0_char_t id0_far msg_boss0[] =
		"So, my little visitor has\n"
		"come to see my newest\n"
		"creations....\n"
		"\n";
	static const id0_char_t id0_far msg_boss1[] =
		"Well, take a good look at\n"
		"them because these mutants\n"
		"will be unstoppable when\n"
		"they fully mature in\n"
		"these incubators!\n"
		"\n";
	static const id0_char_t id0_far msg_boss2[] =
		"Don't bother trying to\n"
		"destroy the incubators! You\n"
		"can't! They're hooked up\n"
		"directly into the main\n"
		"core of the super computer!\n"
		"\n";
	static const id0_char_t id0_far msg_boss3[] =
		"But never mind all that!\n"
		"My kamikaze mutants will\n"
		"soon be the end of you!\n"
		"\n";
#ifdef BETA
	static const id0_char_t id0_far msg_boss4[] =
		"So sorry I had to go before we\n"
		"could meet in person! But I\n"
		"had to move on...\n"
		"We can't have you ruining\n"
		"Master Cain's plans now can we?\n"
		"\n";
	static const id0_char_t id0_far msg_boss5[] =
		"Stopping me will not stop\n"
		"Master Cain from taking over\n"
		"this country...and eventually\n"
		"the WORLD!!!\n"
		"\n";
	static const id0_char_t id0_far msg_boss6[] =
		"Soon my mutant army will be\n"
		"unstoppable and your weak,\n"
		"corrupt government will be on\n"
		"it's knees, begging for mercy!\n"
		"And then, all this will belong\n"
		"\n";
	static const id0_char_t id0_far msg_boss7[] =
		"to US! My master wants very much\n"
		"to meet you... He has decided to\n"
		"take care of you personally!\n"
		"Enjoy your small victory now...\n";
	static const id0_char_t id0_far msg_boss8[] =
		"IT WILL NOT BE LONG LIVED!!!\n"
		"\n";
#else
	static const id0_char_t id0_far msg_boss4[] =
		"So, you've made it through all\n"
		"of my traps. Now you must deal\n"
		"with me personally! I am the\n"
		"guardian of this fortress.\n"
		"You will NOT defeat ME!\n"
		"\n";
	static const id0_char_t id0_far msg_boss5[] =
		"Stopping me will not stop\n"
		"Master Cain from taking over\n"
		"this country...and eventually\n"
		"the WORLD!!!\n"
		"\n";
	static const id0_char_t id0_far msg_boss6[] =
		"Soon Cain's mutant army will be\n"
		"unstoppable and your weak,\n"
		"corrupt government will be on\n"
		"its knees, begging for mercy!\n"
		"And then, all this will belong\n"
		"\n";
	static const id0_char_t id0_far msg_boss7[] =
		"to US! Master Cain really wants\n"
		"to meet you...but I'm afraid\n"
		"you'll be quite dead when this\n"
		"is all over with.\n";
#endif

	Sint16 i, first, count;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	switch (gamestate.mapon)
	{
	case 9:
		first = 0;
		count = 4;
		break;
	case 11:
		first = 4;
#ifdef BETA
		count = 5;
#else
		count = 4;
#endif
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	CA_UpLevel();
	VW_FixRefreshBuffer();
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(30, 7);
		PrintY += 2;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_boss0);
			break;
		case 1:
			strcpy(messagetext, msg_boss1);
			break;
		case 2:
			strcpy(messagetext, msg_boss2);
			break;
		case 3:
			strcpy(messagetext, msg_boss3);
			break;
		case 4:
			strcpy(messagetext, msg_boss4);
			break;
		case 5:
			strcpy(messagetext, msg_boss5);
			break;
		case 6:
			strcpy(messagetext, msg_boss6);
			break;
		case 7:
			strcpy(messagetext, msg_boss7);
			break;
#ifdef BETA
		case 8:
			strcpy(messagetext, msg_boss8);
			break;
#endif
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
	}
	CA_DownLevel();
	ResetScoreObj();
	RF_ForceRefresh();
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

#elif (EPISODE == 3)

/*
=============================================================================

						 DIALOG MESSAGES -- EPISODE THREE

=============================================================================
*/

/*
====================
=
= HostageDialog
=
====================
*/

void HostageDialog(void)
{
	static const id0_char_t id0_far msg_hostage0[] =
		"Oh, how did you find this place?\n"
		"I thought we'd be stuck here\n"
		"forever. Keep going, there are\n"
		"a lot more of us to rescue.\n";
	static const id0_char_t id0_far msg_hostage1[] =
		"Don't worry, I will.\n"
		"\n";
	static const id0_char_t id0_far msg_hostage2[] =
		"Watch out for cyborgs in the\n"
		"next level. They carry rifles.\n";
	static const id0_char_t id0_far msg_hostage3[] =
		"Thank you, young man. I'm not\n"
		"sure that you should venture any\n"
		"further into this fortress!\n";
	static const id0_char_t id0_far msg_hostage4[] =
		"In the next room is the BIGGEST\n"
		"robot I've ever seen! It will\n"
		"most assuredly squash you into\n"
		"oblivion!\n";
	static const id0_char_t id0_far msg_hostage5[] =
		"There must be a way past it!\n";
	static const id0_char_t id0_far msg_hostage6[] =
		"Don't approach it directly right\n"
		"away. See if you can find\n"
		"something to help you.\n";
	static const id0_char_t id0_far msg_hostage7[] =
		"Be really careful, dude! The\n"
		"next levels are packed with\n"
		"mutants and robots.\n";
	static const id0_char_t id0_far msg_hostage8[] =
		"Hey, I saw some cloaked figure\n"
		"float through here earlier.\n"
		"It looked really sinister and\n"
		"had red glowing eyes.\n";
	static const id0_char_t id0_far msg_hostage9[] =
		"Hmm. I think I know who that is.\n";
	static const id0_char_t id0_far msg_hostage10[] =
		"Watch out! I've detected large\n"
		"amounts of radiation in the\n"
		"next level!\n";
	static const id0_char_t id0_far msg_hostage11[] =
		"Watch out for the computer\n"
		"controlled laser defense systems\n"
		"upstairs from here.\n";
	static const id0_char_t id0_far msg_hostage12[] =
		"Be careful of the robots in the\n"
		"next level. They can kill you\n"
		"even if you have taken an\n"
		"invincibility potion!\n";
	static const id0_char_t id0_far msg_hostage13[] =
		"I saw that Cain creep run out\n"
		"through that exit door over\n"
		"there! I guess he got away.\n"
		"Or...maybe he is waiting.\n";
	static const id0_char_t id0_far msg_hostage14[] =
		"I hope he is waiting, it's time\n"
		"for him to pay for what he's\n"
		"done! I won't let him escape!\n";

	Sint16 i, first, count, oldmusic;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	oldmusic = currentmusic;
	switch (gamestate.mapon)
	{
	case 1:
		first = 0;
		count = 2;
		break;
	case 2:
		first = 2;
		count = 1;
		break;
	case 3:
		first = 3;
		count = 4;
		break;
	case 5:
		first = 7;
		count = 1;
		break;
	case 6:
		first = 8;
		count = 2;
		break;
	case 7:
		first = 10;
		count = 1;
		break;
	case 8:
		first = 11;
		count = 1;
		break;
	case 9:
		first = 12;
		count = 1;
		break;
	case 10:
		first = 13;
		count = 2;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	CA_UpLevel();
	// BUG: should call CA_ClearMarks() here to make sure we don't cache more than we need
	CA_MarkGrChunk(HOSTAGE1PIC);
	CA_MarkGrChunk(HOSTAGE2PIC);
	CA_MarkGrChunk(HOSTAGE3PIC);
	CA_MarkGrChunk(SNAKELOGANPIC);
	CA_CacheMarks(NULL);
	VW_FixRefreshBuffer();
	StartMusic(MUS_HOSTAGE);
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(34, 9);
		switch (i)
		{
		case 0:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 1:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 2:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 3:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 4:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 5:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 6:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 7:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 8:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 9:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		case 10:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 11:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE2PIC);
			break;
		case 12:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE1PIC);
			break;
		case 13:
			VWB_DrawPic(WindowX, WindowY+4, HOSTAGE3PIC);
			break;
		case 14:
			VWB_DrawPic(WindowX, WindowY+4, SNAKELOGANPIC);
			break;
		}
		PrintY += 6;
		WindowW -= 48;
		WindowX += 48;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_hostage0);
			break;
		case 1:
			strcpy(messagetext, msg_hostage1);
			break;
		case 2:
			strcpy(messagetext, msg_hostage2);
			break;
		case 3:
			strcpy(messagetext, msg_hostage3);
			break;
		case 4:
			strcpy(messagetext, msg_hostage4);
			break;
		case 5:
			strcpy(messagetext, msg_hostage5);
			break;
		case 6:
			strcpy(messagetext, msg_hostage6);
			break;
		case 7:
			strcpy(messagetext, msg_hostage7);
			break;
		case 8:
			strcpy(messagetext, msg_hostage8);
			break;
		case 9:
			strcpy(messagetext, msg_hostage9);
			break;
		case 10:
			strcpy(messagetext, msg_hostage10);
			break;
		case 11:
			strcpy(messagetext, msg_hostage11);
			break;
		case 12:
			strcpy(messagetext, msg_hostage12);
			break;
		case 13:
			strcpy(messagetext, msg_hostage13);
			break;
		case 14:
			strcpy(messagetext, msg_hostage14);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
	}
	CA_DownLevel();
	RF_ForceRefresh();
	ResetScoreObj();
	StopMusic();
	StartMusic(oldmusic);
	// BUG: Starting a new music track might move memory buffers around!
	// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
	// with animated tiles.

	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

//==========================================================================

/*
====================
=
= HintDialog
=
====================
*/

void HintDialog(void)
{
	static const id0_char_t id0_far msg_hint0[] =
		"You can't pass the radioactive\n"
		"zone unless you find the anti-\n"
		"radiation pill.\n";

	Sint16 i, first, count;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	switch (gamestate.mapon)
	{
	case 8:
		first = 0;
		count = 1;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	VW_FixRefreshBuffer();
	if (gamestate.mapon != 0)
	{
		StartMusic(MUS_HOSTAGE);
	}
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(30, 8);
		PrintY += 2;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_hint0);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
		RF_ForceRefresh();
	}
	if (gamestate.mapon != 0)
	{
		StopMusic();
		StartMusic(gamestate.mapon);
		// BUG: Starting a new music track might move memory buffers around!
		// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
		// with animated tiles.
	}
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

/*
====================
=
= BossDialog
=
====================
*/

void BossDialog(void)
{
	static const id0_char_t id0_far msg_boss0[] =
		"Ah, Snake Logan! You have proved\n"
		"most troublesome to me, and for\n"
		"that, I shall make sure that you\n"
		"die a most PAINFUL death!\n";
	static const id0_char_t id0_far msg_boss1[] =
		"I don't know how you've survived all\n"
		"of the traps I've laid out for you,\n"
		"but you'll not be escaping from ME!\n"
		"I have been waiting a long time\n";
	static const id0_char_t id0_far msg_boss2[] =
		"for this moment, and I am going to\n"
		"enjoy very much getting rid of you\n"
		"once and for all. Then there will\n"
		"be nothing left to stand in my way!\n";
	static const id0_char_t id0_far msg_boss3[] =
		"Now we shall see just how good\n"
		"your fighting skills really are!\n"
		"Aaahhh Ha Ha Ha Ha Ha Ha!\n";

	Sint16 i, first, count;
	id0_char_t messagetext[200];
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	switch (gamestate.mapon)
	{
	case 11:
		first = 0;
		count = 4;
		break;
	//BUG: there is no default case and 'first' and 'count' are not initialized!
	}
	CA_UpLevel();
	VW_FixRefreshBuffer();
	for (i=first; i<first+count; i++)
	{
		US_CenterWindow(30, 7);
		PrintY += 2;
		switch (i)
		{
		case 0:
			strcpy(messagetext, msg_boss0);
			break;
		case 1:
			strcpy(messagetext, msg_boss1);
			break;
		case 2:
			strcpy(messagetext, msg_boss2);
			break;
		case 3:
			strcpy(messagetext, msg_boss3);
			break;
		}
		US_CPrint(messagetext);
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
	}
	CA_DownLevel();
	ResetScoreObj();
	RF_ForceRefresh();
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}

#endif

REFKEEN_NS_E
