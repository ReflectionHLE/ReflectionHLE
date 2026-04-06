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

Uint16 bounceangle[8][8] = {
	{ 0,  0, 0, 0,  0,  0,  0,  0},
	{ 7,  6, 5, 4,  3,  2,  1,  0},
	{ 5,  4, 3, 2,  1,  0, 15, 14},
	{ 5,  4, 3, 2,  1,  0, 15, 14},
	{ 3,  2, 1, 0, 15, 14, 13, 12},
	{ 9,  8, 7, 6,  5,  4,  3,  2},
	{ 9,  8, 7, 6,  5,  4,  3,  2},
	{11, 10, 9, 8,  7,  6,  5,  4}
};

#if (EPISODE == 1)
Uint16 bonusscores[10] = {
	100, 200, 500, 800, 1000, 1500, 2000, 5000, 20000, 50000
};
#endif

/////////////////////////////////////////////////////////////////////////////
// state declarations
/////////////////////////////////////////////////////////////////////////////

FARSTATE s_score = {0, 0, think, false, push_none, 0, 0, 0, NULL, NULL, NULL, NULL};
FARSTATE s_smallexplosion1 = {TINYEXPLOSION1SPR, TINYEXPLOSION1SPR, step, false, push_none, 5, 0, 0, NULL, NULL, DrawReact, &s_smallexplosion2};
FARSTATE s_smallexplosion2 = {TINYEXPLOSION2SPR, TINYEXPLOSION2SPR, step, false, push_none, 5, 0, 0, NULL, NULL, DrawReact, &s_smallexplosion3};
FARSTATE s_smallexplosion3 = {TINYEXPLOSION3SPR, TINYEXPLOSION3SPR, step, false, push_none, 5, 0, 0, NULL, NULL, DrawReact, NULL};
#ifdef BETA
extern FARSTATE s_chunk1;
extern FARSTATE s_chunk2;
extern FARSTATE s_chunk3;
extern FARSTATE s_chunk4;
FARSTATE s_chunk1 = {MEAT1SPR, MEAT1SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk2};
FARSTATE s_chunk2 = {MEAT2SPR, MEAT2SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk3};
FARSTATE s_chunk3 = {MEAT1SPR, MEAT1SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk4};
FARSTATE s_chunk4 = {MEAT2SPR, MEAT2SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk1};
#else
FARSTATE s_chunk1fly1 = {MEAT1SPR, MEAT1SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk1fly2};
FARSTATE s_chunk1fly2 = {MEAT2SPR, MEAT2SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk1fly1};
FARSTATE s_chunkeye1 = {EYEBALL1SPR, EYEBALL1SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunkeye2};
FARSTATE s_chunkeye2 = {EYEBALL2SPR, EYEBALL2SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunkeye1};
FARSTATE s_chunk3fly1 = {MEAT3SPR, MEAT3SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk3fly2};
FARSTATE s_chunk3fly2 = {MEAT4SPR, MEAT4SPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, NULL, ChunkReact, &s_chunk3fly1};
#endif
FARSTATE s_chunkground = {CHUNKGROUNDLSPR, CHUNKGROUNDRSPR, stepthink, false, push_none, 100, 0, 0, NULL, NULL, DrawReact, NULL};
FARSTATE s_metalchunk1 = {METALCHUNK1LSPR, METALCHUNK1RSPR, stepthink, false, push_none, 8, 0, 0, ProjectileThink, NULL, ChunkReact, &s_metalchunk2};
FARSTATE s_metalchunk2 = {METALCHUNK2LSPR, METALCHUNK2RSPR, stepthink, false, push_none, 8, 0, 0, ProjectileThink, NULL, ChunkReact, &s_metalchunk3};
FARSTATE s_metalchunk3 = {METALCHUNK3LSPR, METALCHUNK3RSPR, stepthink, false, push_none, 8, 0, 0, ProjectileThink, NULL, ChunkReact, &s_metalchunk4};
FARSTATE s_metalchunk4 = {METALCHUNK4LSPR, METALCHUNK4RSPR, stepthink, false, push_none, 8, 0, 0, ProjectileThink, NULL, ChunkReact, &s_metalchunk1};
FARSTATE s_metalchunkground = {METALCHUNK4LSPR, METALCHUNK4RSPR, stepthink, false, push_none, 100, 0, 0, NULL, NULL, DrawReact, NULL};
FARSTATE s_frag1 = {FRAG1LSPR, FRAG1RSPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, FragContact, FragReact, &s_frag2};
FARSTATE s_frag2 = {FRAG2LSPR, FRAG2RSPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, FragContact, FragReact, &s_frag3};
FARSTATE s_frag3 = {FRAG3LSPR, FRAG3RSPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, FragContact, FragReact, &s_frag4};
FARSTATE s_frag4 = {FRAG4LSPR, FRAG4RSPR, stepthink, false, push_none, 6, 0, 0, ProjectileThink, FragContact, FragReact, &s_frag1};
FARSTATE s_grenadeg1 = {GRENADEGREEN1SPR, GRENADEGREEN1SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenadeg2};
FARSTATE s_grenadeg2 = {GRENADEGREEN2SPR, GRENADEGREEN2SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenadeg3};
FARSTATE s_grenadeg3 = {GRENADEGREEN3SPR, GRENADEGREEN3SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenadeg4};
FARSTATE s_grenadeg4 = {GRENADEGREEN4SPR, GRENADEGREEN4SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenadeg1};
#ifndef BETA
FARSTATE s_grenader1 = {GRENADERED1SPR, GRENADERED1SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenader2};
FARSTATE s_grenader2 = {GRENADERED2SPR, GRENADERED2SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenader3};
FARSTATE s_grenader3 = {GRENADERED3SPR, GRENADERED3SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenader4};
FARSTATE s_grenader4 = {GRENADERED4SPR, GRENADERED4SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, GrenadeContact, GrenadeReact, &s_grenader1};
#endif
FARSTATE s_grenadeexplosion1 = {GRENADEEXPLOSION1SPR, GRENADEEXPLOSION1SPR, step, false, push_none, 10, 0, 0, GrenadeExplosionThink, GrenadeExplosionContact, DrawReact, &s_grenadeexplosion2};
FARSTATE s_grenadeexplosion2 = {GRENADEEXPLOSION2SPR, GRENADEEXPLOSION2SPR, step, false, push_none, 10, 0, 0, NULL, GrenadeExplosionContact, DrawReact, &s_grenadeexplosion3};
FARSTATE s_grenadeexplosion3 = {GRENADEEXPLOSION3SPR, GRENADEEXPLOSION3SPR, step, false, push_none, 10, 0, 0, NULL, GrenadeExplosionContact, DrawReact, &s_grenadeexplosion4};
FARSTATE s_grenadeexplosion4 = {GRENADEEXPLOSION4SPR, GRENADEEXPLOSION4SPR, step, false, push_none, 10, 0, 0, NULL, GrenadeExplosionContact, DrawReact, &s_grenadeexplosion5};
FARSTATE s_grenadeexplosion5 = {GRENADEEXPLOSION5SPR, GRENADEEXPLOSION5SPR, step, false, push_none, 10, 0, 0, NULL, GrenadeExplosionContact, DrawReact, &s_grenadeexplosion6};
FARSTATE s_grenadeexplosion6 = {GRENADEEXPLOSION6SPR, GRENADEEXPLOSION6SPR, step, false, push_none, 10, 0, 0, NULL, GrenadeExplosionContact, DrawReact, NULL};
FARSTATE s_robopalshot1 = {ROBOPALSHOT1LSPR, ROBOPALSHOT1RSPR, slide, false, push_none, 6, 64, 64, ShotThink, RobopalShotContact, ShotReact, &s_robopalshot2};
FARSTATE s_robopalshot2 = {ROBOPALSHOT2LSPR, ROBOPALSHOT2RSPR, slide, false, push_none, 6, 64, 64, ShotThink, RobopalShotContact, ShotReact, &s_robopalshot1};
#ifndef BETA
FARSTATE s_plasmabolt = {LASERSHOTSPR, LASERSHOTSPR, slide, false, push_none, 6, 64, 64, ShotThink, PlasmaboltContact, ShotReact, &s_plasmabolt};
FARSTATE s_superplasmabolt = {SUPERPLASMABOLTLSPR, SUPERPLASMABOLTRSPR, slide, false, push_none, 2, 64, 64, ShotThink, PlasmaboltContact, SuperPlasmaboltReact, &s_superplasmabolt};
#endif
FARSTATE s_explosion_a1 = {ROBOPALSHOTEXPL1SPR, ROBOPALSHOTEXPL1SPR, step, false, push_none, 12, 0, 0, NULL, NULL, DrawReact, &s_explosion_a2};
FARSTATE s_explosion_a2 = {ROBOPALSHOTEXPL2SPR, ROBOPALSHOTEXPL2SPR, step, false, push_none, 12, 0, 0, NULL, NULL, DrawReact, NULL};
#ifndef BETA
FARSTATE s_explosion_b1 = {MEDIUMEXPLOSION1SPR, MEDIUMEXPLOSION1SPR, step, false, push_none, 12, 0, 0, NULL, NULL, DrawReact, &s_explosion_b2};
FARSTATE s_explosion_b2 = {MEDIUMEXPLOSION2SPR, MEDIUMEXPLOSION2SPR, step, false, push_none, 12, 0, 0, NULL, NULL, DrawReact, NULL};
#endif

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

#if (EPISODE == 1)
/*
=================
=
= SnakeContact
=
=================
*/

void SnakeContact(objtype *ob, objtype *hit)
{
	switch (hit->obclass)
	{
	case bonusobj:
		switch (hit->temp1)	//temp1 is the infoplane icon number of the bonus object
		{
		case 9:
			SD_PlaySound(POWERUPSND);
			hit->shapenum = GRENADEGREEN1SPR;
			gamestate.grenades.green++;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_grenade)
			{
				HelpMessage("Throw grenades to blow up monsters!\n");
				gamestate.hint_grenade = true;
			}
			break;
			
		case 16:
			SD_PlaySound(POWERUPSND);
			hit->shapenum = ITEMMACHINEGUN1SPR;
			if (gamestate.ammo > 0 || gamestate.weapon != 0)
			{
				gamestate.clips++;
			}
			else
			{
				gamestate.autofire = true;
				if (gamestate.difficulty == gd_Hard)
				{
					gamestate.ammo = 45;
				}
				else
				{
					gamestate.ammo = 90;
				}
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_machinegun)
			{
				HelpMessage("Collect machine guns for automatic fire.\n");
				gamestate.hint_machinegun = true;
			}
			break;
			
		case 17:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMKEYCARD1SPR;
				gamestate.keys.keycards++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
				if (!gamestate.hint_keycard)
				{
					HelpMessage("Use keycards to turn off electric beams.\n");
					gamestate.hint_keycard = true;
				}
			}
			break;
			
		case 18:
			SD_PlaySound(GETKEYSND);
			hit->shapenum = ITEMKEY1SPR;
			gamestate.keys.keys++;
			hit->obclass = decoobj;
			hit->priority = 3;
			if (!gamestate.hint_key)
			{
				HelpMessage("Push <UP> and use key to open door.\n");
				gamestate.hint_key = true;
			}
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
			if (c.yaxis == -1)
			{
				SD_PlaySound(POWERUPSND);
				hit->shapenum = hit->temp1 + BONUS100SPR - 19;
				GivePoints(bonusscores[hit->temp1 - 19]);
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
			}
			break;
			
		case 25:
			if (c.yaxis == -1)
			{
				SD_PlaySound(EXTRASNAKESND);
				hit->shapenum = BONUS1UPSPR;
				if (gamestate.lives < 9)	//BUG? you can't get more than 9 lives by picking up extra lives, but you can get more than 9 lives by collecting gems and getting points
				{
					gamestate.lives++;
				}
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
			}
			break;
			
		case 26:
			if (c.yaxis == -1 || gamestate.mapon == 6 || gamestate.mapon == 11)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMBLUESHARD1SPR;
				gamestate.gotshard[0]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
			}
			break;
			
		case 27:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMGREENSHARD1SPR;
				gamestate.gotshard[1]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
			}
			break;
			
		case 28:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMREDSHARD1SPR;
				gamestate.gotshard[2]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
			}
			break;
			
		case 29:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMCYANSHARD1SPR;
				gamestate.gotshard[3]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
			}
			break;
			
		case 30:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMSPECIALKEY1SPR;
				gamestate.specialkeys++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
				HelpMessage("Hmm....What a strange key!\n");
			}
			break;
			
		case 36:
			hit->shapenum = ITEMREDGRENADE1SPR;
			SD_PlaySound(POWERUPSND);
			gamestate.grenades.red++;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_grenade)
			{
				HelpMessage("Throw grenades to blow up monsters!\n");
				gamestate.hint_grenade = true;
			}
			break;
			
		case 37:
			SD_PlaySound(GETKEYSND);
			hit->shapenum = ITEMSUPERGUN1SPR;
			gamestate.autofire = true;
			gamestate.weapon = 3;
			if (gamestate.ammo > 0)
			{
				gamestate.clips++;
			}
			if (gamestate.difficulty == gd_Hard)
			{
				gamestate.ammo = 45;
			}
			else
			{
				gamestate.ammo = 90;
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_supergun)
			{
				HelpMessage("Super Gun!  Bullets do 5x the damage!\n");	//BUG? super gun only does 2 damage
				gamestate.hint_supergun = true;
			}
			break;
			
		case 38:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMLANDMINES1SPR;
			if (gamestate.difficulty == gd_Hard)
			{
				gamestate.grenades.landmines = 5;
			}
			else
			{
				gamestate.grenades.landmines = 10;
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_landmine)
			{
				HelpMessage("Cool!  Land mines!\n");
				gamestate.hint_landmine = true;
			}
			break;
			
		case 39:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMPLASMABOLTS1SPR;
			gamestate.autofire = false;
			gamestate.weapon = 4;
			if (gamestate.ammo > 0)
			{
				gamestate.clips++;
			}
			if (gamestate.difficulty == gd_Hard)
			{
				gamestate.ammo = 10;
			}
			else
			{
				gamestate.ammo = 20;
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_plasmabolts)
			{
				HelpMessage("Plasma Bolts!  Burn holes through enemies!\n");
				gamestate.hint_plasmabolts = true;
			}
			break;
			
		case 52:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMGEM1SPR;
			gamestate.gems++;
			hit->obclass = decoobj;
			hit->priority = 3;
			if (gamestate.gems == 50)
			{
				gamestate.lives++;
				gamestate.gems = 0;
				SD_PlaySound(EXTRASNAKESND);
				hit->shapenum = BONUS1UPSPR;
			}
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_gem)
			{
				HelpMessage("Collect 50 gems for an extra life!\n");
				gamestate.hint_gem = true;
			}
			break;
			
		case 53:
			SD_PlaySound(YUMSND);	//probably useless
			hit->shapenum = ITEMFIRSTAID1SPR;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			SD_PlaySound(LIFEPOINTSND);
			switch (gamestate.difficulty)
			{
			case gd_Easy:
				player->health = 8;
				break;
				
			case gd_Normal:
				player->health = 4;
				break;
				
			case gd_Hard:
				player->health = 2;	//BUG: player starts with 4 health on hard mode, so picking up a first aid kit might reduce player's health!
				break;
				
			default:
				player->health = 4;
				break;
			}
			gamestate.health = player->health;
			if (!gamestate.hint_firstaid)
			{
				HelpMessage("First Aid Kit restores you to full health.\n");
				gamestate.hint_firstaid = true;
			}
			break;
			
		case 54:
			SD_PlaySound(EXTRASNAKESND);
			hit->shapenum = ITEMWARPGEM1SPR;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			gamestate.got_warpgem = true;
			if (!gamestate.hint_warpgem)
			{
				HelpMessage("You found a secret level warp gem!\n");
				gamestate.hint_warpgem = true;
			}
			break;
			
		case 75:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMNUKE1SPR;
			gamestate.nukestate = 1;
			hit->obclass = decoobj;
			hit->priority = 3;
			HelpMessage("Oh look! What a cute little nuclear bomb!\n");
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 76:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMANTIRADIATIONPILL1SPR;
			gamestate.got_pill = true;
			hit->obclass = decoobj;
			hit->priority = 3;
			HelpMessage("It's an anti-radiation pill!\n");
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 77:
			SD_PlaySound(EGGSND);
			hit->shapenum = ITEMINVINCIBILITYPOTION1SPR;
			gamestate.potions++;
			if (gamestate.potions >= 1)
			{
				gamestate.potions = 0;
				invincibility = 1500;
				StartMusic(MUS_INVINCIBLE);
				// BUG: Starting a new music track might move memory buffers around!
				// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
				// with animated tiles.

				// Note: The original IMF music data contains some instructions that
				// interfere with the playback of AdLib sound effects. The IMF data
				// overwrites the instrument data for the first OPL channel (the one
				// reserved for AdLib sound effects), which means any AdLib sound
				// effect that is playing when a music track was just (re-)started
				// will use the wrong instrument data for the rest of its duration.
				// Since this code starts a new music track right after playing a
				// sound effect, the music can prevent that effect from being played
				// correctly when the game is set to AdLib sounds effects. The same
				// problem also occurs when the music loops around to the start, so
				// this is really a bug in the data, not a bug in the code.
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_invincible)
			{
				HelpMessage("Now you're invincible to monster attacks!\n");
				gamestate.hint_invincible = true;
			}
			break;
			
		case 78:
			SD_PlaySound(ICON5000SND);
			hit->shapenum = BONUS5000SPR;
			GivePoints(5000);
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 79:
			SD_PlaySound(ICON5000SND);
			hit->shapenum = BONUS50000SPR;
			GivePoints(50000);
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 80:
			SD_PlaySound(GETKEYSND);
			gamestate.got_exitkey = true;
			hit->shapenum = ITEMEXITKEY1SPR;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			break;
		}
		break;	//end of case bonusobj
		
	case platformobj:
		if (!gamestate.riding)
		{
			ClipToSpriteTop(ob, hit);
		}
		break;
		
	case hostageobj:
		gamestate.savedhostage = true;
		HostageDialog();
		SD_PlaySound(POWERUPSND);
		hit->shapenum = BONUS5000SPR;
		GivePoints(bonusscores[BONUS5000SPR-BONUS100SPR]);
		hit->obclass = bonusobj;
		hit->ydir = -1;
		ChangeState(hit, &s_bonusrise);
		SpawnBonus(ob->tilemidx, ob->tiletop, 80);	//spawn exit key
		break;
		
	case triangleobj:		//never actually used!
		gamestate.trianglekey++;
		SD_PlaySound(POWERUPSND);
		hit->shapenum = ITEMGREENTRIANGLE2SPR;
		hit->obclass = bonusobj;	//BUG: this should probably be set to 'decoobj'
		hit->ydir = -1;
		ChangeState(hit, &s_bonusrise);
		break;
		
	case robopalobj:
		if (!gamestate.got_robopal && (invincibility == 0 || invincibility > 50))
		{
			SD_PlaySound(ICON5000SND);
			gamestate.got_robopal = true;
			ChangeState(hit, &s_robopalfollow1);
		}
		if (!gamestate.hint_robopal)
		{
			HelpMessage("RoboPal gives you extra fire power!\n");
			gamestate.hint_robopal = true;
		}
		break;
		
	case tankobj:
		HurtPlayer(ob, 50);	//kill the player
		break;
		
	case pushableobj:
		if (gamestate.mapon != 13)
		{
			ClipPushSprite(ob, hit, false);
		}
		break;
		
	case blockedobj:
		ClipToSprite(ob, hit, false);
		break;
		
	case fallingbrickobj:
		ClipToSprite(ob, hit, true);
		break;
		
	case explosionobj:
	case wormobj:
	case bugobj:
	case punchyobj:
	case gorpobj:
	case cobraobj:
	case bossobj:
	case guardobj:
	case bouncerobj:
	case acidmonsterobj:
	case enemyshotobj:
	case caterpillarobj:
	case beeobj:
	case sewermanobj:
	case hairyobj:
	case skullheadobj:
	case skullhandobj:
	case jumpbombobj:
	case slimerobj:
	case devilobj:
	case helicopterobj:
	case parabotobj:
		if (hit->spareplayer == true)
		{
			return;
		}
		if (!invincibility)
		{
			HurtPlayer(ob, 1);
		}
		break;
	}
}
#endif	// (EPISODE == 1)

/*
=============================================================================

						 SCORE BOX ROUTINES

=============================================================================
*/

/*
=================
=
= SpawnScore
=
=================
*/

void SpawnScore(void)
{
	scoreobj->obclass = decoobj;
	scoreobj->active = allways;
	scoreobj->needtoclip = cl_noclip;
#if 0	/* REFKEEN: Indeed, let's use safer and more portable code here. */
	//original code (might be unsafe):
	*(Sint32*)(&scoreobj->temp1) = -1;
#else
	//safe code:
	scoreobj->temp1 = scoreobj->temp2 = -1;
#endif
	scoreobj->temp3 = -1;
	scoreobj->temp4 = -1;
	scoreobj->temp5 = -1;
	scoreobj->temp6 = -1;
	NewState(scoreobj, &s_score);
}

/*
======================
=
= MemDrawChar		(taken from KEEN DREAMS)
=
======================
*/

static void MemDrawChar(Sint16 char8, Uint8 id0_far *dest, Uint16 width, Uint16 planesize)
{
	// Ported from ASM

	Uint8 *srcPtr = (Uint8 *)grsegs[STARTTILE8] + 32*char8;
	Uint8 *destPtr;

	for (Uint16 planeLoopVar = 4; planeLoopVar; --planeLoopVar)
	{
		destPtr = dest;

		*destPtr = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);
		*(destPtr += width) = *(srcPtr++);

		dest += planesize;
	}
}

/*
====================
=
= ShiftScore		(taken from KEEN DREAMS)
=
====================
*/

static void ShiftScore (void)
{
	spritetabletype id0_far *spr;
	spritetype id0_seg *dest;

	spr = &spritetable[SCOREBOXSPR-STARTSPRITES];
	dest = (spritetype id0_seg *)grsegs[SCOREBOXSPR];

	CAL_ShiftSprite ((id0_byte_t *)dest+dest->sourceoffset[0],
		(id0_byte_t *)dest+dest->sourceoffset[1],spr->width,spr->height,2);

	CAL_ShiftSprite ((id0_byte_t *)dest+dest->sourceoffset[0],
		(id0_byte_t *)dest+dest->sourceoffset[2],spr->width,spr->height,4);

	CAL_ShiftSprite ((id0_byte_t *)dest+dest->sourceoffset[0],
		(id0_byte_t *)dest+dest->sourceoffset[3],spr->width,spr->height,6);
#if 0
	CAL_ShiftSprite (FP_SEG(dest),dest->sourceoffset[0],
		dest->sourceoffset[1],spr->width,spr->height,2);

	CAL_ShiftSprite (FP_SEG(dest),dest->sourceoffset[0],
		dest->sourceoffset[2],spr->width,spr->height,4);

	CAL_ShiftSprite (FP_SEG(dest),dest->sourceoffset[0],
		dest->sourceoffset[3],spr->width,spr->height,6);
#endif
}

void ResetScoreObj(void)
{
#ifdef BETA
	scoreobj->temp1=scoreobj->temp2=scoreobj->temp3=scoreobj->temp4=
		scoreobj->temp5=scoreobj->temp6 = -1;
#else
	scoreobj->temp1=scoreobj->temp2=scoreobj->temp3=scoreobj->temp4=
		scoreobj->temp5=scoreobj->temp6=scoreobj->temp7 = -1;
#endif
}

/*
===============
=
= UpdateScorebox
=
===============
*/

void UpdateScorebox(objtype *ob)
{
	id0_char_t buffer[10], *bufptr;	// BUG: score might need up to 12 chars! ("-2147483648")
	spritetype id0_seg *block;
	Uint8 id0_far *dest;
	Uint8 id0_far *dest2;
	Uint16 isodd, len, width, planesize;
	Uint16 amount;
	boolean changed;
	/*register*/ Uint16 i;

	changed = false;
	
	if (!showscorebox
#ifndef BETA
		 || gamestate.mapon == 12 || gamestate.mapon == 13	// no score box in the highscore and the Apogee level
#endif
		)
	{
		return;
	}
	
//
// check if score (or practice timer) changed
//
	if (gamestate.score >> 16 != ob->temp1
		 || (Uint16)gamestate.score != ob->temp2
#ifndef BETA
		 || practiceTimeLeft > 0
#endif
		)
	{
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
#ifdef BETA
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*4 + 1;
		BE_Cross_ltoa_dec(gamestate.score, buffer);
		len = strlen(buffer);
		for (i=6; i>len; i--)
		{
			MemDrawChar(0, ++dest, width, planesize);
		}
		for (bufptr=buffer; *bufptr;)
		{
			MemDrawChar((*(bufptr++))+1-'0', ++dest, width, planesize);
		}
#else
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*4;
		if (practiceTimeLeft > 0)
		{
			BE_Cross_ltoa_dec(practiceTimeLeft/33, buffer);
		}
		else
		{
			BE_Cross_ltoa_dec(gamestate.score, buffer);
		}
		len = strlen(buffer);
		for (i=7; i>len; i--)
		{
			MemDrawChar(0, ++dest, width, planesize);
		}
		for (bufptr=buffer; *bufptr;)
		{
			MemDrawChar((*(bufptr++))+1-'0', ++dest, width, planesize);
		}
		if (practiceTimeLeft > 0)
		{
			dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*4 + 1;
			dest--;
			for (i=1; i<6; i++)
			{
				MemDrawChar(59+i, ++dest, width, planesize);
			}
		}
#endif
		ShiftScore();
		ob->needtoreact = true;
		ob->temp1 = gamestate.score>>16;
		ob->temp2 = gamestate.score;
		changed = true;
	}

//
// check if health changed
//
	amount = player->health;
	if (ob->temp6 != amount)
	{
		if (amount > 8)
		{
			amount = 8;
		}
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
		dest2 = dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*4 + 8;
		for (i=4; i>0; i--)
		{
			MemDrawChar(0, ++dest2, width, planesize);
		}
		if (amount > 0)
		{
#if 1
			//original code:
			for (i=1; i<=amount; i++)
			{
				isodd = i&1;
				if (isodd)
				{
					if (i==1)
					{
						dest++;
					}
					MemDrawChar(31, dest, width, planesize);
				}
				else
				{
					MemDrawChar(21, dest, width, planesize);
					dest++;
				}
			}
#else
			//faster implementation:
			while (amount >= 2)
			{
				amount -= 2;
				MemDrawChar(21, ++dest, width, planesize);
			}
			if (amount)
			{
				MemDrawChar(31, ++dest, width, planesize);
			}
#endif
		}
		ShiftScore();
		ob->needtoreact = true;
		ob->temp6 = player->health;
		changed = true;
	}

#ifndef BETA
//
// update grenade icon, if necessary
//
	if (gamestate.grenades.landmines > 0)
	{
		amount = gamestate.grenades.landmines;
	}
	else if (gamestate.grenades.red > 0)
	{
		amount = gamestate.grenades.red;
	}
	else
	{
		amount = gamestate.grenades.green;
	}
	if (ob->temp3 != amount)
	{
		// Note: The temp3 check above and the preparation code before that is
		// exactly the same as for the grenade number check below, so you could
		// just merge this with the code below to save some space and avoid an
		// extra sprite shift.
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*20 + 4;
		if (gamestate.grenades.landmines > 0)
		{
			MemDrawChar(35, dest, width, planesize);
		}
		else if (gamestate.grenades.red > 0)
		{
			MemDrawChar(57, dest, width, planesize);
		}
		else
		{
			MemDrawChar(58, dest, width, planesize);
		}
		ShiftScore();
		ob->needtoreact = true;
		changed = true;
	}
#endif

//
// check if number of grenades changed
//
#ifdef BETA
	if (mapon == 15)
	{
		amount = gamestate.grenades.green;
	}
	else
	{
		amount = gamestate.grenades.green;
	}
	// As you can see, both branches do exactly the same, which makes the mapon
	// check rather pointless. It's not entirely clear what this was supposed to
	// accomplish, since map 15 doesn't exist in the beta. The beta's original
	// README2.TXT mentions that the game's size had been reduced from 18 to 12
	// levels, so this might refer to a level that got cut before this beta was
	// sent to the beta testers. Maybe there was a level where Snake Logan was
	// supposed to throw a different kind of grenade or maybe they just wanted
	// to make the score box show a different number for the grenades, like
	// during the boss battle where the boss basically steals the player's
	// grenades and uses them against the player.
	//
	// It could also have been inherited from the Keen Dreams source. Map 15 is
	// the boss level in Keen Dreams and displays the number of Boobus Bombs
	// instead of the number of Flower Power pellets.
#else
	if (gamestate.grenades.landmines > 0)
	{
		amount = gamestate.grenades.landmines;
	}
	else if (gamestate.grenades.red > 0)
	{
		amount = gamestate.grenades.red;
	}
	else
	{
		amount = gamestate.grenades.green;
	}
#endif
	if (ob->temp3 != amount)
	{
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*20 + 5;
		if (amount > 99)
		{
			strcpy(buffer, "99");
		}
		else
		{
			BE_Cross_ltoa_dec(amount, buffer);
		}
		len = strlen(buffer);
		for (i=2; i>len; i--)
		{
			MemDrawChar(0, ++dest, width, planesize);
		}
		bufptr = &buffer[0];
		while (*bufptr)
		{
			MemDrawChar(*(bufptr++)+1-'0', ++dest, width, planesize);
		}
		ShiftScore();
		ob->needtoreact = true;
		// Note: The number that is going to be stored in temp3 is already stored
		// in the amount variable, so you could just set temp3 to amount here.
#ifdef BETA
		ob->temp3 = gamestate.grenades.green;
#else
		if (gamestate.grenades.landmines != 0)
		{
			ob->temp3 = gamestate.grenades.landmines;
		}
		else if (gamestate.grenades.red != 0)
		{
			ob->temp3 = gamestate.grenades.red;
		}
		else
		{
			ob->temp3 = gamestate.grenades.green;
		}
#endif
		changed = true;
	}

#ifndef BETA
//
// check if weapon changed
//
	if (gamestate.weapon != ob->temp7)
	{
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*20 + 9;
		if (gamestate.weapon == 0)
		{
			MemDrawChar(33, dest, width, planesize);
		}
		else if (gamestate.weapon == 3)
		{
			MemDrawChar(34, dest, width, planesize);
		}
		else if (gamestate.weapon == 4)
		{
			MemDrawChar(59, dest, width, planesize);
		}
		ShiftScore();
		ob->needtoreact = true;
		ob->temp7 = gamestate.weapon;
		changed = true;
	}
#endif

//
// check if ammo changed
//
	amount = gamestate.ammo;
	if (gamestate.difficulty == gd_Easy && gamestate.ammo <= 3
#ifndef BETA
		 && gamestate.weapon == 0
#endif
		)
	{
		amount = 0;
	}
	if (ob->temp5 != amount)
	{
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*20 + 10;
		if (amount > 99)
		{
			strcpy(buffer, "99");
		}
		else
		{
			BE_Cross_ltoa_dec(amount, buffer);
		}
		len = strlen(buffer);
		for (i=2; i>len; i--)
		{
			MemDrawChar(0, ++dest, width, planesize);
		}
		bufptr = &buffer[0];
		if (amount > 0)
		{
			while (*bufptr)
			{
				MemDrawChar(*(bufptr++)+1-'0', ++dest, width, planesize);
			}
		}
		else
		{
			MemDrawChar(*(bufptr++)+20-'0', dest, width, planesize);	// always draws tile 20 (amount is 0)
			MemDrawChar(0, ++dest, width, planesize);
		}
		ShiftScore();
		ob->needtoreact = true;
		if (gamestate.difficulty == gd_Easy)
		{
#ifdef BETA
			ob->temp5 = 0;
#else
			ob->temp5 = -1;
			// BUG? This causes this code to constantly update the ammo display when
			// playing on easy, which just wrecks the overall performance unless the
			// score box is turned off. You should just set temp5 to amount here,
			// and not just when playing on easy.
#endif
		}
		else
		{
			ob->temp5 = gamestate.ammo;
		}
		changed = true;
	}

//
// check if lives changed
//
	if (gamestate.lives != ob->temp4)
	{
		block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
		width = block->width[0];
		planesize = block->planesize[0];
		dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + width*20 + 2;
		if (gamestate.lives > 9)
		{
			MemDrawChar(9+1, dest, width, planesize);
		}
		else
		{
			MemDrawChar(gamestate.lives+1, dest, width, planesize);
		}
		ShiftScore();
		ob->needtoreact = true;
		ob->temp4 = gamestate.lives;
		changed = true;
	}
	
//
// check if boss health changed
//
#ifndef BETA
	if (bosshealth > 0)
	{
		if (bossdiv == 0)
		{
			bossdiv = 1;	//don't divide by zero
		}
		amount = bosshealth / bossdiv;
		if (amount != oldhealthbarlength)	//Note: this is (almost) always true because the bosses set 'oldhealthbarlength' to 'bosshealth'
		{
			/*
			Note:
			Since bossdiv is often initialized to bosshealth/20 - 1, the amount
			value can be greater than 20 here. For example, if the initial boss
			health is 200, the divisor is set to 9 and the amount calculated from
			that will initially be 22. This means the boss health bar will not
			change until bosshealth drops below 180. It gets even worse when the
			initial boss health is 125, because that will set the divisor to 5 and
			will not change the health bar until the health value drops below 100.

			In general, the lower the initial boss health is, the more damage has
			to be done to the boss before the health bar changes for the first
			time. Players need to cause at least 20 to 40 points of damage before
			the health bar changes for the first time.

			The best way to avoid this would be to store the initial boss health
			as the divisor and then calculate the health bar length like this:

				amount = (20*bosshealth)/bossdiv;

			Since the game uses the value 999 to indicate that the boss is dead,
			the boss can never have more than 998 health without causing issues.
			This code is perfectly safe even when using 16 bit integers because
			20*998 is 19960, which is a perfectly valid signed 16 bit integer.

			Using the formula given above will result in the health bar being
			updated as soon as the boss takes 1 damage, so the players can see
			immediately that they hurt the boss.

			Of course, there is also the fact that this code requires amount to
			be at least 1, so the health bar does not update in the same intervals
			when the boss is nearly dead. The simple solution is to use this code:

				amount = (19*bosshealth)/bossdiv + 1;

			This makes sure the amount value is always in range 1 to 20 and also
			lets the boss health bar decrease in constant intervals.

			And while you're at it, you might also want to remove the code that
			updates oldhealthbarlength from each boss's code and instead set the
			variable in here, so the game doesn't waste CPU power updating the
			boss health bar EVERY SINGLE FRAME!
			*/
			if (amount > 20)
			{
				amount = 20;
			}
			if (amount <= 0)
			{
				amount = 1;
			}
			block = (spritetype id0_seg *)grsegs[SCOREBOXSPR];
			width = block->width[0];
			planesize = block->planesize[0];
			dest2 = dest = (Uint8 id0_far *)grsegs[SCOREBOXSPR] + block->sourceoffset[0] + planesize + (width << 5) - 1;
			dest2 += 3;
			if (bosshealth == 999)
			{
				bosshealth = -1;
				//erase "BOSS" label and health bar:
				MemDrawChar(72, ++dest, width, planesize);
				for (i=0; i<12; i++)
				{
					MemDrawChar(71, ++dest, width, planesize);
				}
			}
			else
			{
				//draw "BOSS" label:
				for (i=0; i<3; i++)
				{
					MemDrawChar(i+66, ++dest, width, planesize);
				}
				dest = dest2;
				//erase old health bar:
				for (i=0; i<10; i++)
				{
					MemDrawChar(71, ++dest2, width, planesize);
				}
				//draw new health bar:
				if (amount > 0)	//always true
				{
#if 1
					//original code:
					for (i=1; i<=amount; i++)
					{
						isodd = i&1;
						if (isodd)
						{
							if (i==1)
							{
								dest++;
							}
							MemDrawChar(70, dest, width, planesize);
						}
						else
						{
							MemDrawChar(69, dest, width, planesize);
							dest++;
						}
					}
#else
					//faster implementation:
					while (amount >= 2)
					{
						amount -= 2;
						MemDrawChar(69, ++dest, width, planesize);
					}
					if (amount)
					{
						MemDrawChar(70, ++dest, width, planesize);
					}
					//bosshealth--;	// just for testing this code without having to hurt the boss
#endif
				}
			}
			ShiftScore();
			ob->needtoreact = true;
			changed = true;
		}
	}
#endif	// ifndef BETA
	/*
	Note:

	This code is quite wasteful when it comes to shifting the scorebox
	sprite. Since the weapons in this game are mostly hitscan, this
	often causes situations where more than one part of the scorebox
	changes, which means the scorebox will be shifted multiple times
	in one frame.

	For example, if you kill an enemy with a hitscan weapon, you lose
	ammo and you gain points in one frame. Which means the score and
	the ammo numbers need to be updated. Increasing the score could
	also give the player an extra life, so the lives number might need
	to be updated as well. And if you killed the enemy with the last
	bullet of your super gun, the gun icon also needs to be updated.
	And if you happen to pick up a better type of grenade during the
	same frame, the grenade number and icon also need updating,
	which means the game may perform up to 6 shifts in one frame.

	To avoid performing multiple shifts in one frame, simply use the
	following code in here and remove all calls to ShiftScore() above.

	if (changed)
	{
		ShiftScore();
	}
	*/
	if (ob->x != originxglobal || ob->y != originyglobal)
	{
		ob->x = originxglobal;
		ob->y = originyglobal;
		changed = true;
	}
	if (changed)
	{
		RF_PlaceSprite(&ob->sprite, ob->x+4*PIXGLOBAL, ob->y+4*PIXGLOBAL, SCOREBOXSPR, spritedraw, MAXPRIORITY);
	}
}

/*
=============================================================================

						 PROJECTILES AND EXPLOSIVES

=============================================================================
*/

/*
======================
=
= HurtObject
=
======================
*/

void HurtObject(objtype *ob, Sint16 damage)
{
	Uint16 points;

	points = 0;
#if (EPISODE == 3)
	if (ob->obclass == cainobj)
	{
		// count the damage done to Cain:
		ob->temp6 = ob->temp6 + damage;
	}
#endif
	ob->health = ob->health - damage;
	ob->needtoreact = true;	// for the white flashing damage visualization
	if (ob->health <= 0)
	{
		// object got killed:
		switch (ob->obclass)
		{
		case wormobj:
		case beeobj:
		case gorpobj:
#if (EPISODE != 2)
		case triangleobj:	//BUG? Isn't this supposed to be a bonus object, not an enemy?
		case slimerobj:
#endif
			points = 50;
			break;
			
		case bugobj:
#if (EPISODE == 2)
		case antobj:
#else
		case hairyobj:
#endif
			points = 100;
			break;
			
#if (EPISODE != 1)
		case kamikazeobj:
			points = 250;
			break;
			
#endif
		case cobraobj:
#if (EPISODE == 3)
		case jumpbombobj:
#endif
			points = 200;
			break;
			
		case punchyobj:
#if (EPISODE == 1)
		case sewermanobj:
#endif
#if (EPISODE != 2)
		case devilobj:
#endif
			points = 500;
			break;
			
#if (EPISODE == 2)
		case trashbossobj:
			points = 10000;
			ob->shootable = false;
#ifndef BETA
			bosshealth = 999;
#endif
			SpawnBonus(ob->tilemidx, ob->tiletop, 18);	//regular key
			StartMusic(MUS_VICTORY);
			// BUG: Starting a new music track might move memory buffers around!
			// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
			// with animated tiles.
			break;
			
#endif
		case tankobj:
#if (EPISODE == 3)
		case grenadebotobj:
#endif
			points = 2000;
			ob->shootable = false;
			break;
			
		case guardobj:
			points = 800;
			ob->shootable = false;
			break;
			
		case bouncerobj:
			points = 350;
			ob->shootable = false;
			break;
			
#if (EPISODE == 2)
		case queenobj:
			points = 10000;
#ifndef BETA
			bosshealth = 999;
#endif
			SpawnBonus(ob->tilemidx, ob->tiletop, 18);	//regular key
#if 0
			// bugfix in case the key spawns where the player can't reach it:
			// (this code assumes s_bonus2 uses FlyBonusThink, see BM_ACT2.C)
			newobj->state = &s_bonus2;			// make the key fall
			newobj->needtoclip = cl_midclip;	// don't let it fall through the floor
#endif
			StartMusic(MUS_VICTORY);
			// BUG: Starting a new music track might move memory buffers around!
			// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
			// with animated tiles.
			break;
			
#endif
#if (EPISODE != 1)
		case caterpillarobj:
			points = 1000;
			ob->shootable = false;
			break;
#endif

		case turretobj:
			points = 500;
			ob->shootable = false;
			break;
			
#if (EPISODE == 1)
		case acidmonsterobj:
			points = 500;
			break;
			
		case bossobj:
			points = 100000;	//BUG: 'points' is a 16 bit variable, so the value is clipped to 34464!
			ob->shootable = false;
			SD_PlaySound(GROWLSND);
			ob->obclass = decoobj;
			bosshealth = 999;
			break;
			
		case skullheadobj:
			points = 5000;
			ob->shootable = false;
			SpawnBonus(ob->tilemidx, ob->tiletop, 26);	//blue shard
			StartMusic(MUS_VICTORY);
			// BUG: Starting a new music track might move memory buffers around!
			// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
			// with animated tiles.
			skullmanstate = 1;	// Skullman is dead
			bosshealth = 999;
			break;
			
		case skullhandobj:
			points = 1000;
			ob->shootable = false;
			break;
			
		case helicopterobj:
			points = 1000;
			ob->shootable = false;
			break;
			
#else
		case trashtruckobj:
			points = 200;
			ob->shootable = false;
			break;
			
		case bluedevilobj:
#if (EPISODE == 3)
		case cyborgobj:
#endif
			points = 500;
			ob->shootable = false;
			break;
#endif
#ifdef BETA
		case acidmonsterobj:
			points = 500;
			break;
			
		case jimobj:
			{
				ob->shootable = false;
				ChangeState(ob, &s_jimattack1);
				points = gamestate.mapon;
				gamestate.mapon = 0;
				HintDialog();
				gamestate.mapon = points;
				points = 0;
				StopMusic();
				StartMusic(gamestate.mapon);
				// BUG: Starting a new music track might move memory buffers around!
				// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
				// with animated tiles.
				return;
			}
			
		case bossobj:
			points = 100000;	//BUG: 'points' is a 16 bit variable, so the value is clipped to 34464!
			ob->shootable = false;
			SpawnBonus(player->tilemidx, player->tiletop, 26);	//blue shard
			StartMusic(MUS_VICTORY);
			// BUG: Starting a new music track might move memory buffers around!
			// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
			// with animated tiles.
			break;

#elif (EPISODE == 2)
		case bossobj:
			points = 10000;
			ob->shootable = false;
			bosshealth = 999;
			SpawnBonus(player->tilemidx, player->tiletop, 26);	//blue shard
			StartMusic(MUS_VICTORY);
			// BUG: Starting a new music track might move memory buffers around!
			// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
			// with animated tiles.
			break;
			
		case jimobj:
			{
				Sint16 oldmap;
				
				ob->shootable = false;
				oldmap = gamestate.mapon;
				gamestate.mapon = 99;
				HintDialog();
				gamestate.mapon = oldmap;
				ChangeState(ob, &s_jimattack1);
				hintstate = 1;
				return;
			}
			
#elif (EPISODE == 3)
		case goliathobj:
			points = 10000;
			ob->shootable = false;
			bosshealth = 999;
			SpawnBonus(player->tilemidx, player->tiletop, 26);	//blue shard
			StartMusic(MUS_VICTORY);
			// BUG: Starting a new music track might move memory buffers around!
			// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
			// with animated tiles.
			break;
			
		case cainobj:
			points = 100000;	//BUG: 'points' is a 16 bit variable, so the value is clipped to 34464!
			ob->shootable = false;
			bosshealth = 999;
			SpawnBonus(player->tilemidx, player->tiletop, 26);	//blue shard
			break;
#endif

		}
#ifdef BETA
		if (ob->obclass == bouncerobj
			|| ob->obclass == turretobj
			|| ob->obclass == trashtruckobj
			|| ob->obclass == bluedevilobj
			|| ob->obclass == caterpillarobj
			|| ob->obclass == bossobj)
#elif (EPISODE == 1)
		if (ob->obclass == bouncerobj
			|| ob->obclass == turretobj
			|| ob->obclass == bossobj
			|| ob->obclass == jumpbombobj
			|| ob->obclass == helicopterobj)
#elif (EPISODE == 2)
		if (ob->obclass == bouncerobj
			|| ob->obclass == turretobj
			|| ob->obclass == trashtruckobj
			|| ob->obclass == bluedevilobj
			|| ob->obclass == bossobj)
#elif (EPISODE == 3)
		if (ob->obclass == bouncerobj
			|| ob->obclass == turretobj
			|| ob->obclass == trashtruckobj
			|| ob->obclass == bluedevilobj
			|| ob->obclass == bossobj
			|| ob->obclass == jumpbombobj
			|| ob->obclass == cyborgobj
			|| ob->obclass == goliathobj
			|| ob->obclass == grenadebotobj
			|| ob->obclass == caterpillarobj
			|| ob->obclass == cainobj)
#endif
		{
			SpawnBigExplosion(ob->x, ob->y+TILEGLOBAL);
#if (EPISODE == 3) || (defined BETA)
#ifdef BETA
			if (ob->obclass != bluedevilobj && ob->obclass != bossobj)
#else
			if (ob->obclass != bossobj && ob->obclass != bluedevilobj)
#endif
			{
				goto chunks;
			}
			else
			{
				RemoveObj(ob);
			}
#else
			goto chunks;
#endif
		}
		else
		{
chunks:
			ChunkBloom(ob, ob->midx, ob->y, 0);
			ChunkBloom(ob, ob->midx, ob->y, 4);
			ChunkBloom(ob, ob->midx, ob->y, 2);
			ChunkBloom(ob, ob->midx, ob->y, 6);
#if (EPISODE == 3)
			if (ob->obclass != tankobj
				&& ob->obclass != guardobj
				&& ob->obclass != grenadebotobj
				&& ob->obclass != barrelobj
				&& ob->obclass != goliathobj
				&& ob->obclass != cainobj)
#elif (defined BETA)
			if (ob->obclass != tankobj && ob->obclass != guardobj && ob->obclass != trashbossobj)
#else
			if (ob->obclass != tankobj && ob->obclass != decoobj)
#endif
			{
				RemoveObj(ob);
			}
		}
		GivePoints(points);
	}
	else
	{
		// object took damage but didn't get killed yet
		if (player->obclass == playerwormobj)
		{
			ob->white = 10;
		}
		else
		{
			ob->white = 4;
		}
		
#if (EPISODE == 1)
		// make Skull Man start attacking after getting shot
		if ((ob->obclass == skullheadobj || ob->obclass == skullhandobj) && skullmanstate < 0)
		{
			skullmanstate++;
		}
#endif
	}
}

/*
======================
=
= SpawnExplosion
=
======================
*/

void SpawnExplosion(Uint16 x, Uint16 y)
{
	GetNewObj(true);
	newobj->obclass = decoobj;
	newobj->active = removable;
	newobj->x = x - 4*PIXGLOBAL;
	newobj->y = y - 4*PIXGLOBAL;
	newobj->priority = 3;
#ifdef BETA
	NewState(newobj, &s_smallexplosion1);
#else
	if (gamestate.weapon == 3 || gamestate.weapon == 4)
	{
		NewState(newobj, &s_explosion1);
	}
	else
	{
		NewState(newobj, &s_smallexplosion1);
	}
#endif
}

/*
======================
=
= DoShooting
=
======================
*/

void DoShooting(Uint16 x, Uint16 y, Sint16 xdir, Uint16 damage)
{
	objtype *ob;
	Uint16 tilex, tiley, tile, top, bottom, checks;
	
	if (gamestate.ammo > 0)
	{
#ifdef BETA
		SD_PlaySound(MACHINEGUNSND);
		gamestate.ammo--;
		// BUG: gamestate.ammo almost never reaches 0 on easy. It will be set to
		// 3 shortly after it reached 0 (see SnakeAutofireThink() and
		// SnakeAirAutofireThink() in BM_SNAKE.C), which means the clips are
		// never used. The later code "fixed" this by using a clip when there is
		// only 1 shot left (see below), but that means the player never gets to
		// fire the last Plasma Bolt on easy, so that's not a perfect solution.
		// The better fix might be to make the autofire think routines use a clip
		// instead of setting gamestate.ammo to 3 on easy. Move the code into a
		// new subroutine to reduce redundancy if you want.
#else
		if (gamestate.difficulty == gd_Easy && gamestate.ammo <= 1)
		{
			if (gamestate.weapon == 3 || gamestate.weapon == 4)
			{
				gamestate.weapon = 0;
			}
			if (gamestate.clips > 0)
			{
				gamestate.ammo = 90;
				gamestate.clips--;
			}
		}
		if (gamestate.weapon != 4)
		{
			SD_PlaySound(MACHINEGUNSND);
		}
		gamestate.ammo--;
		if (gamestate.weapon == 0)
		{
			gamestate.autofire = true;
		}
#endif
		if (gamestate.ammo & 1 && gamestate.got_robopal)
		{
			robopal_mustfire = true;
		}
	}
	else
	{
#ifndef BETA
		if (gamestate.weapon == 3 || gamestate.weapon == 4)
		{
			gamestate.weapon = 0;
		}
#endif
		if (gamestate.clips > 0)
		{
			gamestate.ammo = 90;	//BUG? this should be set to 45 on hard difficulty
			gamestate.clips--;
			return;
		}
#ifdef BETA
		SD_PlaySound(MACHINEGUNSND);
#else
		if (gamestate.weapon != 4)
		{
			SD_PlaySound(MACHINEGUNSND);
		}
#endif
		gamestate.autofire = false;
		if (gamestate.got_robopal)
		{
			robopal_mustfire = true;
		}
	}
	tilex = CONVERT_GLOBAL_TO_TILE(x);
	tiley = CONVERT_GLOBAL_TO_TILE(y);
	top = y - 4*PIXGLOBAL;
	bottom = y + 4*PIXGLOBAL;
	checks = 20;
#ifndef BETA
	if (gamestate.weapon == 4)
	{
		SpawnLaserShot(x, y - 2*PIXGLOBAL, xdir);
		return;
	}
#endif
	//perform hitscan check:
	/*
	Note: It might be more efficient to change the order of the hitscan check.
	First check if there are any objects that could potentially be hit by the
	shot, then check if there are any blocking tiles in between the origin of
	the shot and the nearest object in the line of fire. That means the code
	would only need to iterate through the object list (up to 125 entries) once
	instead of up to 20 times.
	*/
	do
	{
		//see if the shot hits an actor:
		for (ob=player->next; ob; ob=ob->next)
		{
			if (ob->active && ob->shootable 
				&& ob->tileleft <= tilex && ob->tileright >= tilex
				&& ob->top <= bottom && ob->bottom >= top)
			{
				SpawnExplosion(ob->midx, y);
				switch (ob->obclass)
				{
#if (EPISODE != 2)
				case devilobj:
					if (ob->state == &s_devilburn1 || ob->state == &s_devilburn2)
					{
						return;
					}
					break;
#endif
				case pushableobj:	//BUG: should also handle blockedobj here!
				case fallingbrickobj:
#if (EPISODE != 1)
				case bluedevilobj:
#endif
#if (EPISODE != 2)
				case jumpbombobj:
				case slimerobj:
#endif
					return;
				}
#ifdef BETA
				HurtObject(ob, damage);	//Note: damage is always 1
#else
				if (gamestate.weapon == 3)
				{
					HurtObject(ob, 2);
				}
				else
				{
					HurtObject(ob, damage);	//Note: damage is always 1
				}
#endif
				return;
			}
		}
		//see if the shot hits a wall:
		tile = FORESPOT(tilex, tiley);
		if ((tinf[tile+NORTHWALL]&SLOPEMASK) > 1 || (tinf[tile+SOUTHWALL]&SLOPEMASK) > 1)
		{
			SpawnExplosion(CONVERT_TILE_TO_GLOBAL(tilex) + 8*PIXGLOBAL, y);
			if (US_RndT() < 16)
			{
				SD_PlaySound(RICOCHET1SND);
			}
			return;
		}
		if (xdir == -1)
		{
			if (tinf[tile+EASTWALL])
			{
				SpawnExplosion(TILEGLOBAL*(tilex+1), y);
				if (US_RndT() < 16)
				{
					SD_PlaySound(RICOCHET1SND);
				}
				return;
			}
		}
		else
		{
			if (tinf[tile+WESTWALL])
			{
				SpawnExplosion(CONVERT_TILE_TO_GLOBAL(tilex), y);
				if (US_RndT() < 16)
				{
					SD_PlaySound(RICOCHET1SND);
				}
				return;
			}
		}
		//move the shot:
		tilex += xdir;
		checks--;
	} while (checks);
}

/*
======================
=
= ChunkBloom
=
======================
*/

void ChunkBloom(objtype *ob, Uint16 x, Uint16 y, Uint16 dir)
{
	Sint16 value;

	SD_PlaySound(CHUNKSPLATSND);
	GetNewObj(true);
	newobj->active = removable;
	newobj->obclass = decoobj;
	newobj->x = x;
	newobj->y = y;
	newobj->ydir = -1;
	switch (dir)
	{
	case 0:
		newobj->xdir = 1;
		newobj->xspeed = 6;
		newobj->yspeed = -30;
		break;
		
	case 2:
		newobj->xdir = 1;
		newobj->xspeed = (US_RndT() / 8) + 10;	// (10..41)
		newobj->yspeed = (US_RndT() / 8) - 20;	// (-20..11)
		break;
		
	case 4:
		newobj->xdir = -1;
		newobj->xspeed = -6;
		newobj->yspeed = -30;
		break;
		
	case 6:
		newobj->xdir = -1;
		newobj->xspeed = -10 - (US_RndT() / 8);	// (-41..-10)
		newobj->yspeed = (US_RndT() / 8) - 20; 	// (-20..11)
		break;
		
	default:
		Quit("ChunkBloom: Bad dir!");
	}
	switch (ob->obclass)
	{
	case tankobj:
	case bouncerobj:
	case turretobj:
#if (EPISODE == 1)
	case jumpbombobj:
	case helicopterobj:
	case parabotobj:
#elif (EPISODE == 2)
	case guardobj:
	case caterpillarobj:
	case trashtruckobj:
	case trashbossobj:
#ifndef BETA
	case bossobj:
#endif
#elif (EPISODE == 3)
	case guardobj:
	case caterpillarobj:
	case trashtruckobj:
	case jumpbombobj:
	case cyborgobj:
	case goliathobj:
	case grenadebotobj:
#endif
		newobj->temp1 = 1;	// let ChunkReact know that it's a metal chunk
		NewState(newobj, &s_metalchunk1);
		break;
	default:
#ifdef BETA
		NewState(newobj, &s_chunk1);
#else
		value = US_RndT();
		if (value < 85)
		{
			NewState(newobj, &s_chunk1fly1);
		}
		else if (value > 85 && value < 170)
		{
			NewState(newobj, &s_chunkeye1);
		}
		else
		{
			NewState(newobj, &s_chunk3fly1);
		}
#endif
		break;
	}
}

/*
======================
=
= ChunkReact		(based on PowerReact() from KEEN DREAMS)
=
======================
*/

void ChunkReact(objtype *ob)
{
	Uint16 wall, absx, absy, angle, newangle;
	Uint32 speed;
	
	PLACESPRITE;
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = -ob->xspeed/2;
		ob->obclass = bonusobj;	//BUG?
		/*
		Note: Setting obclass to bonusobj was inherited from PowerReact() from
		Keen Dreams. In that game, Keen is able to pick up the thrown power
		pellets again if they didn't hit anything.
		
		Turning the chunk into a bonus object might obviously cause problems when
		Snake (the player) touches them. The only reason why this doesn't cause
		problems in BioMenace is that the chunk's temp1 value is either 0 or 1,
		which is not a valid temp1 value for bonus objects in BioMenace.
		BioMenace stores the info layer icon number in the bonus object's temp1
		field (see SpawnBonus() in BM_ACT2.C and ScanInfoPlane() in BM_SPEC.C).
		*/
	}
	wall = ob->hitnorth;
	if (wall)
	{
		ob->obclass = bonusobj;	//BUG?
		if (ob->yspeed < 0)
		{
			ob->yspeed = 0;
		}
		absx = abs(ob->xspeed);
		absy = ob->yspeed;
		if (absx>absy)
		{
			if (absx>absy*2)	// 22 degrees
			{
				angle = 0;
				speed = absx*286;	// x*sqrt(5)/2
			}
			else				// 45 degrees
			{
				angle = 1;
				speed = absx*362;	// x*sqrt(2)
			}
		}
		else
		{
			if (absy>absx*2)	// 90 degrees
			{
				angle = 3;
				speed = absy*256;
			}
			else
			{
				angle = 2;		// 67 degrees
				speed = absy*286;	// y*sqrt(5)/2
			}
		}
		if (ob->xspeed > 0)
			angle = 7-angle;

		speed >>= 1;
		newangle = bounceangle[ob->hitnorth][angle];	//I think this should use (ob->hitnorth&SLOPEMASK) just to be safe.
		switch (newangle)
		{
		case 0:
			ob->xspeed = speed / 286;
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 1:
			ob->xspeed = speed / 362;
			ob->yspeed = -ob->xspeed;
			break;
		case 2:
			ob->yspeed = -(speed / 286);
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 3:

		case 4:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 5:
			ob->yspeed = -(speed / 286);
			ob->xspeed = ob->yspeed / 2;
			break;
		case 6:
			ob->xspeed = ob->yspeed = -(speed / 362);
			break;
		case 7:
			ob->xspeed = -(speed / 286);
			ob->yspeed = ob->xspeed / 2;
			break;

		case 8:
			ob->xspeed = -(speed / 286);
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 9:
			ob->xspeed = -(speed / 362);
			ob->yspeed = -ob->xspeed;
			break;
		case 10:
			ob->yspeed = speed / 286;
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 11:

		case 12:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 13:
			ob->yspeed = speed / 286;
			ob->xspeed = ob->yspeed / 2;
			break;
		case 14:
			ob->xspeed = speed / 362;
			ob->yspeed = speed / 362;
			break;
		case 15:
			ob->xspeed = speed / 286;
			ob->yspeed = ob->xspeed / 2;
			break;
		}

		if (speed < 256*16)
		{
			ob->y += 8*PIXGLOBAL;
			if (!ob->temp1)
			{
				ChangeState (ob, &s_chunkground);
			}
			else
			{
				ChangeState (ob, &s_metalchunkground);
			}
			return;
		}
	}
}

/*
======================
=
= FragBloom
=
======================
*/

void FragBloom(Uint16 x, Uint16 y, Sint16 dir)
{
#ifdef BETA
#define speed 0
#else
	Uint16 speed = 1;
#endif

	GetNewObj(true);
	newobj->active = removable;
	newobj->obclass = explosionobj;
	newobj->temp1 = -1;
	newobj->temp2 = dir;
	newobj->x = x;
	newobj->y = y;
	newobj->ydir = -1;
#ifndef BETA
	if (dir >= FRAG_SIDEWAYS)
	{
		dir -= FRAG_SIDEWAYS;
		newobj->spareplayer = true;
		speed = US_RndT()/4 + 15;
	}
	if (dir >= FRAG_SPAREPLAYER)
	{
		dir -= FRAG_SPAREPLAYER;
		newobj->spareplayer = true;
	}
#endif
	switch (dir)
	{
	case dir_North:
		newobj->xspeed = 0;
		newobj->yspeed = -20;
		break;
	case dir_South:
		newobj->xspeed = 0;
		newobj->yspeed = 20;
		break;
	case dir_East:
		newobj->xdir = 1;
		newobj->xspeed = speed + 20;
		newobj->yspeed = -20;
		break;
	case dir_West:
		newobj->xdir = -1;
		newobj->xspeed = -20 - speed;
		newobj->yspeed = -20;
		break;
	default:
		Quit("FragBloom: Bad dir!");
	}
	NewState(newobj, &s_frag1);
#ifdef BETA
#undef speed
#endif
}

/*
======================
=
= FragContact
=
======================
*/

void FragContact(objtype *ob, objtype *hit)
{
	if (hit->shootable && !hit->white
		&& hit->obclass != pushableobj	//BUG: should also handle blockedobj here!
		&& hit->obclass != tankobj
		&& hit->obclass != fallingbrickobj
#if (EPISODE == 1)
		&& hit->obclass != devilobj
#elif (EPISODE == 2)
		&& hit->obclass != bossobj
#elif (EPISODE == 3)
		&& hit->obclass != bossobj
		&& hit->obclass != devilobj
		&& hit->obclass != goliathobj
		&& hit->obclass != grenadebotobj
#endif
		)
	{
#if (EPISODE != 3) && !(defined BETA)
		if (hit->obclass == playerobj && ob->spareplayer == true)
		{
			return;
		}
#endif
		HurtObject(hit, 1);
		hit->white = 25;
		RemoveObj(ob);
	}
}

/*
======================
=
= FragReact		(based on ProjectileReact() from KEEN DREAMS)
=
======================
*/

void FragReact(objtype *ob)
{
	Uint16 wall, absx, absy, angle, newangle;
	Uint32 speed;
	
	PLACESPRITE;
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = -ob->xspeed/2;
	}
	if (ob->hitsouth)
	{
		ob->yspeed = -ob->yspeed/2;
	}
	wall = ob->hitnorth;
	if (wall)
	{
		if (ob->yspeed < 0)
		{
			ob->yspeed = 0;
		}
		absx = abs(ob->xspeed);
		absy = ob->yspeed;
		if (absx>absy)
		{
			if (absx>absy*2)	// 22 degrees
			{
				angle = 0;
				speed = absx*286;	// x*sqrt(5)/2
			}
			else				// 45 degrees
			{
				angle = 1;
				speed = absx*362;	// x*sqrt(2)
			}
		}
		else
		{
			if (absy>absx*2)	// 90 degrees
			{
				angle = 3;
				speed = absy*256;
			}
			else
			{
				angle = 2;		// 67 degrees
				speed = absy*286;	// y*sqrt(5)/2
			}
		}
		if (ob->xspeed > 0)
			angle = 7-angle;

		speed >>= 1;
		newangle = bounceangle[ob->hitnorth][angle];	//I think this should use (ob->hitnorth&SLOPEMASK) just to be safe.
		switch (newangle)
		{
		case 0:
			ob->xspeed = speed / 286;
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 1:
			ob->xspeed = speed / 362;
			ob->yspeed = -ob->xspeed;
			break;
		case 2:
			ob->yspeed = -(speed / 286);
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 3:

		case 4:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 5:
			ob->yspeed = -(speed / 286);
			ob->xspeed = ob->yspeed / 2;
			break;
		case 6:
			ob->xspeed = ob->yspeed = -(speed / 362);
			break;
		case 7:
			ob->xspeed = -(speed / 286);
			ob->yspeed = ob->xspeed / 2;
			break;

		case 8:
			ob->xspeed = -(speed / 286);
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 9:
			ob->xspeed = -(speed / 362);
			ob->yspeed = -ob->xspeed;
			break;
		case 10:
			ob->yspeed = speed / 286;
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 11:

		case 12:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 13:
			ob->yspeed = speed / 286;
			ob->xspeed = ob->yspeed / 2;
			break;
		case 14:
			ob->xspeed = speed / 362;
			ob->yspeed = speed / 362;
			break;
		case 15:
			ob->xspeed = speed / 286;
			ob->yspeed = ob->xspeed / 2;
			break;
		}

		if (speed < 256*16)
		{
			RemoveObj(ob);
			return;
		}
	}
}

/*
======================
=
= GrenadeExplosionThink
=
======================
*/

//#pragma argsused
void GrenadeExplosionThink(objtype *ob)
{
	SD_PlaySound(GRENADEXPLODESND);
}

/*
======================
=
= ThrowPower
=
======================
*/

void ThrowPower(Uint16 x, Uint16 y, Uint16 dir)
{
#ifdef BETA
	if (!gamestate.grenades.green)
	{
		return;
	}
	gamestate.grenades.green--;
#else
	if (!gamestate.grenades.landmines && !gamestate.grenades.green && !gamestate.grenades.red)
	{
		return;
	}
	if (gamestate.grenades.landmines > 0)
	{
		SpawnLandmine();
//		SpawnLandmine(CONVERT_GLOBAL_TO_TILE(x), CONVERT_GLOBAL_TO_TILE(y));	//Note: args are ignored!
		gamestate.grenades.landmines--;
		ChangeState(player, &s_snakeplacemine);
		return;
	}
#endif
	GetNewObj(true);
	newobj->obclass = grenadeobj;
	newobj->active = allways;	//this is overwritten below
	newobj->temp2 = dir;
	newobj->x = x;
	newobj->y = y;
	newobj->ydir = -1;
#ifndef BETA
	if (gamestate.grenades.red > 0)
	{
		gamestate.grenades.red--;
		newobj->temp7 = 1;
	}
	else
	{
		gamestate.grenades.green--;
	}
#endif
	switch (dir)
	{
	case dir_East:
		newobj->xspeed = 45;
		newobj->yspeed = -16;
#ifndef BETA
		newobj->xdir = 1;
#endif
		break;
	case dir_West:
		newobj->xspeed = -45;
		newobj->yspeed = -16;
#ifndef BETA
		newobj->xdir = -1;
#endif
		break;
	default:
		Quit("ThrowPower: Bad dir!");
	}
	newobj->temp1 = 9;
#ifdef BETA
	newobj->active = removable;
	NewState(newobj, &s_grenadeg1);
	if (!StatePositionOk(newobj, &s_grenadeg1))
	{
		newobj->hitnorth = 1;
	}
#else
	newobj->spareplayer = true;
	newobj->active = removable;
	if (newobj->temp7)
	{
		//it's a red grenade
		NewState(newobj, &s_grenader1);
		newobj->active = allways;
		if (!StatePositionOk(newobj, &s_grenader1))
		{
			newobj->hitnorth = 1;
		}
	}
	else
	{
		//it's a green grenade
		NewState(newobj, &s_grenadeg1);
		if (!StatePositionOk(newobj, &s_grenadeg1))
		{
			newobj->hitnorth = 1;
		}
	}
#endif
}

/*
======================
=
= PowerCount		(from Keen Dreams, never used in BioMenace)
=
======================
*/

void PowerCount(objtype *ob)
{
	ob->temp2 += tics;
	ob->shapenum = 0;
	if (ob->temp2 > 50)
	{
		RemoveObj(ob);
	}
}

/*
======================
=
= GrenadeContact
=
======================
*/

void GrenadeContact(objtype *ob, objtype *hit)
{
	if (hit->shootable
		&& hit->obclass != pushableobj	//BUG: should also handle blockedobj here!
#if (EPISODE != 1)
		&& hit->obclass != bossobj
#endif
#if (EPISODE == 3)
		&& hit->obclass != grenadebotobj
#endif
		)
	{
#ifdef BETA
		FragBloom(ob->x + 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, dir_East);
		FragBloom(ob->x - 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, dir_West);
		ob->obclass = explosionobj;
		SD_PlaySound(GRENADEXPLODESND);
		ChangeState(ob, &s_grenadeexplosion1);
#else
		FragBloom(ob->x + 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_East);
		FragBloom(ob->x - 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_West);
		ChangeState(ob, &s_grenadeexplosion1);
		ob->obclass = explosionobj;
		SD_PlaySound(GRENADEXPLODESND);
#endif
		ob->needtoclip = cl_midclip;
	}
}

/*
======================
=
= GrenadeExplosionContact
=
======================
*/

void GrenadeExplosionContact(objtype *ob, objtype *hit)
{
	if (hit->shootable && !hit->white
		&& hit->obclass != pushableobj	//BUG: should also handle blockedobj here!
		&& hit->obclass != fallingbrickobj
#if (EPISODE != 1)
		&& hit->obclass != bossobj
#endif
#if (EPISODE != 2)
		&& hit->obclass != devilobj
#endif
#if (EPISODE == 3)
		&& hit->obclass != grenadebotobj
#endif
		)
	{
#ifndef BETA
		if (hit->obclass == playerobj && ob->spareplayer == true)
		{
			return;
		}
#endif
		HurtObject(hit, 5);
		hit->white = 25;
	}
	ob++;	//was probably used to avoid "unused argument" warnings in earlier versions
}

/*
======================
=
= GrenadeReact		(based on PowerReact() from KEEN DREAMS)
=
======================
*/

void GrenadeReact(objtype *ob)
{
	Uint16 wall, absx, absy, angle, newangle;
	Uint32 speed;
	
	PLACESPRITE;
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = -ob->xspeed/2;
	}
	if (ob->hitsouth)
	{
		ob->yspeed = -ob->yspeed/2;
	}
	wall = ob->hitnorth;
	if (wall)
	{
		if (ob->yspeed < 0)
		{
			ob->yspeed = 0;
		}
		absx = abs(ob->xspeed);
		absy = ob->yspeed;
		if (absx>absy)
		{
			if (absx>absy*2)	// 22 degrees
			{
				angle = 0;
				speed = absx*286;	// x*sqrt(5)/2
			}
			else				// 45 degrees
			{
				angle = 1;
				speed = absx*362;	// x*sqrt(2)
			}
		}
		else
		{
			if (absy>absx*2)	// 90 degrees
			{
				angle = 3;
				speed = absy*256;
			}
			else
			{
				angle = 2;		// 67 degrees
				speed = absy*286;	// y*sqrt(5)/2
			}
		}
		if (ob->xspeed > 0)
			angle = 7-angle;

		speed >>= 1;
		newangle = bounceangle[ob->hitnorth][angle];	//I think this should use (ob->hitnorth&SLOPEMASK) just to be safe.
		switch (newangle)
		{
		case 0:
			ob->xspeed = speed / 286;
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 1:
			ob->xspeed = speed / 362;
			ob->yspeed = -ob->xspeed;
			break;
		case 2:
			ob->yspeed = -(speed / 286);
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 3:

		case 4:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 5:
			ob->yspeed = -(speed / 286);
			ob->xspeed = ob->yspeed / 2;
			break;
		case 6:
			ob->xspeed = ob->yspeed = -(speed / 362);
			break;
		case 7:
			ob->xspeed = -(speed / 286);
			ob->yspeed = ob->xspeed / 2;
			break;

		case 8:
			ob->xspeed = -(speed / 286);
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 9:
			ob->xspeed = -(speed / 362);
			ob->yspeed = -ob->xspeed;
			break;
		case 10:
			ob->yspeed = speed / 286;
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 11:

		case 12:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 13:
			ob->yspeed = speed / 286;
			ob->xspeed = ob->yspeed / 2;
			break;
		case 14:
			ob->xspeed = speed / 362;
			ob->yspeed = speed / 362;
			break;
		case 15:
			ob->xspeed = speed / 286;
			ob->yspeed = ob->xspeed / 2;
			break;
		}

		if (speed < 256*16)
		{
#ifdef BETA
			FragBloom(ob->x + 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, dir_East);
			FragBloom(ob->x - 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, dir_West);
			ob->obclass = explosionobj;
			ChangeState(ob, &s_grenadeexplosion1);
#else
			if (ob->temp7)
			{
				//it's a red grenade
				FragBloom(ob->x + 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_East);
				FragBloom(ob->x - 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_West);
				ob->obclass = explosionobj;
				ob->needtoclip = cl_midclip;
				ob->x = ob->x - 8*PIXGLOBAL;
				ob->y = ob->y - 24*PIXGLOBAL;
				ob->temp7 = 50;
				ChangeState(ob, &s_grenadefire1);
			}
			else
			{
				//it's a green grenade
				FragBloom(ob->x + 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_East);
				FragBloom(ob->x - 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_West);
				ob->obclass = explosionobj;
				ChangeState(ob, &s_grenadeexplosion1);
			}
#endif
		}
		return;
	}
}

/*
======================
=
= SpawnShot
=
======================
*/

void SpawnShot(Uint16 x, Uint16 y, Sint16 xdir)
{
	GetNewObj(true);
	newobj->x = x;
	newobj->y = y;
	newobj->priority = 0;
	newobj->obclass = shotobj;
	newobj->active = allways;
	SD_PlaySound(STUNRAYSND);
	switch (xdir)
	{
	case 1:
		newobj->xdir = 1;
		newobj->ydir = 0;
		break;
	case -1:
		newobj->xdir = -1;
		newobj->ydir = 0;
		break;
	default:
		Quit("SpawnShot: Bad dir!");
	}
	NewState(newobj, &s_robopalshot1);
}

#ifndef BETA
/*
======================
=
= SpawnLaserShot
=
======================
*/

void SpawnLaserShot(Uint16 x, Uint16 y, Sint16 xdir)
{
	GetNewObj(true);
	newobj->x = x;
	newobj->y = y;
	newobj->priority = 0;
	newobj->obclass = shotobj;
	newobj->active = allways;
	SD_PlaySound(LASERSND);
	newobj->temp7 = 1;
	switch (xdir)
	{
	case 1:
		newobj->xdir = 1;
		newobj->ydir = 0;
		break;
	case -1:
		newobj->xdir = -1;
		newobj->ydir = 0;
		break;
	default:
		Quit("SpawnLaserShot: Bad dir!");
	}
	NewState(newobj, &s_plasmabolt);
}

/*
======================
=
= SpawnSuperLaserShot
=
======================
*/

void SpawnSuperLaserShot(Uint16 x, Uint16 y, Sint16 xdir)
{
	GetNewObj(true);
	newobj->x = x;
	newobj->y = y;
	newobj->priority = 2;
	newobj->obclass = shotobj;
	newobj->active = allways;
	SD_PlaySound(GRENADEXPLODESND);
	newobj->temp7 = 0;
	switch(xdir)
	{
	case 1:
		newobj->xdir = 1;
		newobj->ydir = 0;
		break;
	case -1:
		newobj->xdir = -1;
		newobj->ydir = 0;
		break;
	default:
		Quit("SpawnLaserShot: Bad dir!");
	}
	NewState(newobj, &s_superplasmabolt);
}
#endif	// ifndef BETA

/*
======================
=
= RobopalShotContact
=
======================
*/

void RobopalShotContact(objtype *ob, objtype *hit)
{
	if (hit->shootable
		&& hit->obclass != pushableobj	//BUG: should also handle blockedobj here!
		&& hit->obclass != fallingbrickobj)
	{
		HurtObject(hit, 1);
		ChangeState(ob, &s_explosion_a1);
	}
	ob++;	//was probably used to avoid "unused argument" warnings in earlier versions
}

#ifndef BETA
/*
======================
=
= PlasmaboltContact
=
======================
*/

void PlasmaboltContact(objtype *ob, objtype *hit)
{
	if (hit->shootable
		&& hit->obclass != pushableobj	//BUG: should also handle blockedobj here!
		&& hit->obclass != fallingbrickobj)
	{
		if (ob->temp7 == 1)
		{
			HurtObject(hit, 5);
			ChangeState(ob, &s_explosion_b1);
		}
		else
		{
			HurtObject(hit, 2);
		}
	}
	ob++;	//was probably used to avoid "unused argument" warnings in earlier versions
}
#endif

/*
======================
=
= PlasmaboltExplode
=
======================
*/

void PlasmaboltExplode(objtype *ob)
{
	ob->obclass = decoobj;
#ifdef BETA
	ChangeState(ob, &s_explosion_a1);
#else
	if (ob->temp7 == 1)
	{
		ChangeState(ob, &s_explosion_b1);
	}
	else
	{
		ChangeState(ob, &s_explosion_a1);
	}
#endif
	SD_PlaySound(RAYHITSND);
}

/*
======================
=
= ShotThink
=
======================
*/

void ShotThink(objtype *ob)
{
	objtype *hit;

	//do nothing if this shot is visible:
	if (ob->tileright >= originxtile && ob->tilebottom >= originytile
		&& ob->tileleft <= originxtilemax && ob->tiletop <= originytilemax)
	{
		return;
	}
	//remove the shot when off-screen by more than half a screen:
	if (ob->tileright+10 < originxtile || ob->tileleft-10 > originxtilemax
		|| ob->tilebottom+6 < originytile || ob->tiletop-6 > originytilemax)
	{	
		RemoveObj(ob);
		return;
	}
	//check contact with INACTIVE objects:
	for (hit=player->next; hit; hit=hit->next)
	{
		if (!hit->active
			&& ob->right > hit->left && ob->left < hit->right
			&& ob->top < hit->bottom && ob->bottom > hit->top)
		{
			if (hit->state->contactptr)
			{
				hit->state->contactptr(hit, ob);
				hit->needtoreact = true;
				hit->active = yes;
			}
			if (ob->obclass == nothing)	//useless -- obclass is NOT set to 'nothing' by RemoveObj()
			{
				break;
			}
		}
	}
}

/*
======================
=
= ShotReact
=
======================
*/

void ShotReact(objtype *ob)
{
	if (ob->hitnorth || ob->hitsouth || ob->hiteast || ob->hitwest)
	{
		PlasmaboltExplode(ob);
	}
	PLACESPRITE;
}

#ifndef BETA
/*
======================
=
= SuperPlasmaboltReact
=
======================
*/

void SuperPlasmaboltReact(objtype *ob)
{
	if (ob->hitnorth || ob->hitsouth || ob->hiteast || ob->hitwest)
	{
		ChangeState(ob, &s_grenadeexplosion1);
	}
	PLACESPRITE;
}
#endif

REFKEEN_NS_E
