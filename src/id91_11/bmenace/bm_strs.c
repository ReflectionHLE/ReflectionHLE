/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2026 K1n9_Duk3
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

#include "version.h"
//#pragma hdrstop
#include "id_heads.h" // Only for definitions like type being used

// In C++, const implies static, so we need to specify extern
// TODO: Use backend definition
#ifdef __cplusplus
#define BUFFLINKAGE extern
#else
#define BUFFLINKAGE
#endif

REFKEEN_NS_B

BUFFLINKAGE const id0_char_t id0_far str_easy[] = "easy";
BUFFLINKAGE const id0_char_t id0_far str_normal[] = "normal";
BUFFLINKAGE const id0_char_t id0_far str_hard[] = "hard";
BUFFLINKAGE const id0_char_t id0_far str_gameover[] = "Game Over!";

BUFFLINKAGE const id0_char_t id0_far str_nomemgame[] =
"Not enough memory\n"
"to load game!";

BUFFLINKAGE const id0_char_t id0_far str_didntmakeit[] = "You didn't make it past";
BUFFLINKAGE const id0_char_t id0_far str_tryagain[] = "Try Again";

BUFFLINKAGE const id0_char_t id0_far str_nomemlevel[] =
"Insufficient memory\n"
"to load level!";

BUFFLINKAGE const id0_char_t id0_far str_onemoment[] = "One moment";
BUFFLINKAGE const id0_char_t id0_far str_godmodeon[] = "God mode ON";
BUFFLINKAGE const id0_char_t id0_far str_godmodeoff[] = "God mode OFF";
BUFFLINKAGE const id0_char_t id0_far str_freeitems[] = "Free items!";
BUFFLINKAGE const id0_char_t id0_far str_jumpcheaton[] = "Jump cheat ON";
BUFFLINKAGE const id0_char_t id0_far str_jumpcheatoff[] = "Jump cheat OFF";
#ifdef BETA
BUFFLINKAGE const id0_char_t id0_far str_warpprompt[] = "  Warp to which level(1-12): ";
#else
BUFFLINKAGE const id0_char_t id0_far str_warpprompt[] = "  Warp to which level(1-17): ";
BUFFLINKAGE const id0_char_t id0_far str_practiceprompt[] = "  Practice which level (1-11): ";
#endif
BUFFLINKAGE const id0_char_t id0_far str_paused[] = "PAUSED";
BUFFLINKAGE const id0_char_t id0_far str_location[] = "LOCATION";
BUFFLINKAGE const id0_char_t id0_far str_score[] = "SCORE";
BUFFLINKAGE const id0_char_t id0_far str_extra[] = "EXTRA";
BUFFLINKAGE const id0_char_t id0_far str_keycards[] = "KEYCARDS";
BUFFLINKAGE const id0_char_t id0_far str_shards[] = "SHARDS";
BUFFLINKAGE const id0_char_t id0_far str_level[] = "LEVEL";
BUFFLINKAGE const id0_char_t id0_far str_leasy[] = "Easy";
BUFFLINKAGE const id0_char_t id0_far str_lnormal[] = "Normal";
BUFFLINKAGE const id0_char_t id0_far str_lhard[] = "Hard";
BUFFLINKAGE const id0_char_t id0_far str_keys[] = "KEYS";
BUFFLINKAGE const id0_char_t id0_far str_ammo[] = "AMMO";
BUFFLINKAGE const id0_char_t id0_far str_lives[] = "LIVES";
#ifdef BETA
BUFFLINKAGE const id0_char_t id0_far str_potions[] = "POTIONS";
BUFFLINKAGE const id0_char_t id0_far str_food[] = "FOOD";
#else
BUFFLINKAGE const id0_char_t id0_far str_clips[] = "CLIPS";
BUFFLINKAGE const id0_char_t id0_far str_gems[] = "GEMS";
#endif
BUFFLINKAGE const id0_char_t id0_far str_question[] = "???";

BUFFLINKAGE const id0_char_t id0_far str_nomemmusic[] =
"Insufficient memory\n"
"for background music!";

BUFFLINKAGE const id0_char_t id0_far str_forgothostage[] =
"Oh NO! You forgot to rescue the\n"
"scientist being held prisoner\n"
"here. You'd better go get him!\n"
"\n";

REFKEEN_NS_E
