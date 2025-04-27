/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
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

#ifndef __GFX_H__
#define __GFX_H__

#include "version.h"

//////////////////////////////////////
//
// Graphics .H file for .BM? (all 3 episodes)
// not IGRAB-ed :)
//
//////////////////////////////////////

//
// Lump creation macros
//

#define START_LUMP(x) x##_LUMP_START, _##x##_S=x##_LUMP_START-1,
#define END_LUMP(x) _##x##_E, x##_LUMP_END=_##x##_E-1,

// macros for beacon and plasma lumps, since their position varies greatly
#define __BEACONLUMP__ \
	START_LUMP(BEACON) \
	BEACON1SPR, \
	BEACON2SPR, \
	BEACON3SPR, \
	BEACON4SPR, \
	BEACON5SPR, \
	END_LUMP(BEACON)

#define __PLASMALUMP__ \
	START_LUMP(PLASMA) \
	PLASMA1SPR, \
	PLASMA2SPR, \
	END_LUMP(PLASMA)

#define __CATERPILLARLUMP__ \
	START_LUMP(CATERPILLAR) \
	CATERPILLARHEAD1SPR, \
	CATERPILLARHEAD2SPR, \
	CATERPILLARBODYSPR, \
	END_LUMP(CATERPILLAR)

//
// Amount of each data item
//

//common numbers:
#define NUMCHUNKS    NUMGRCHUNKS
#define NUMFONT      2
#define NUMFONTM     0
#define NUMPICM      1
#define NUMTILE8     144  //was 144 in the original code, but only 108 are in the EGAGRAPH file
#define NUMTILE8M    36   //was 36 in the original code, but only 12 are in the EGAGRAPH file
#define NUMTILE32    0
#define NUMTILE32M   0

//episode-specific numbers:
#if (EPISODE == 1)
#if (defined GMS_VERSION)
#define NUMPICS      51
#elif (defined VERSION_1_1)
#define NUMPICS      50
#else
#define NUMPICS      49
#endif
#define NUMSPRITES   355
#define NUMTILE16    1512
#define NUMTILE16M   2682
#define NUMEXTERNS   14
#elif (EPISODE == 2)
#define NUMPICS      45
#ifdef BETA
#define NUMSPRITES   350
#else
#define NUMSPRITES   389
#endif
#define NUMTILE16    1728
#define NUMTILE16M   2466
#define NUMEXTERNS   12
#elif (EPISODE == 3)
#define NUMPICS      41
#define NUMSPRITES   408
#define NUMTILE16    1080
#define NUMTILE16M   2682
#define NUMEXTERNS   11
#endif

//
// File offsets for data items
//
#define STRUCTPIC    0
#define STRUCTPICM   1
#define STRUCTSPRITE 2

#define STARTFONT    3
#define STARTFONTM   (STARTFONT+NUMFONT)
#define STARTPICS    (STARTFONTM+NUMFONTM)
#define STARTPICM    (STARTPICS+NUMPICS)
#define STARTSPRITES (STARTPICM+NUMPICM)
#define STARTTILE8   (STARTSPRITES+NUMSPRITES)
#define STARTTILE8M  (STARTTILE8+1)
#define STARTTILE16  (STARTTILE8M+1)
#define STARTTILE16M (STARTTILE16+NUMTILE16)
#define STARTTILE32  (STARTTILE16M+NUMTILE16M)
#define STARTTILE32M (STARTTILE32+NUMTILE32)
#define STARTEXTERNS (STARTTILE32M+NUMTILE32M)

REFKEEN_NS_B

typedef enum {
	LASTFONT = STARTPICS-1,
	// Lump Start
	START_LUMP(CONTROLS)
	CP_MAINMENUPIC,              // 5
	CP_NEWGAMEMENUPIC,           // 6
	CP_LOADMENUPIC,              // 7
	CP_SAVEMENUPIC,              // 8
	CP_CONFIGMENUPIC,            // 9
	CP_SOUNDMENUPIC,             // 10
	CP_MUSICMENUPIC,             // 11
	CP_KEYBOARDMENUPIC,          // 12
	CP_KEYMOVEMENTPIC,           // 13
	CP_KEYBUTTONPIC,             // 14
	CP_JOYSTICKMENUPIC,          // 15
	CP_OPTIONSMENUPIC,           // 16
	CP_QUITPIC,                  // 17
	CP_JOYSTICKPIC,              // 18
	CP_MENUSCREENPIC,            // 19
	END_LUMP(CONTROLS)

	TITLEPIC,                    // 20
	HELPMENUPIC,                 // 21
	HELPCURSORPIC,               // 22
	TEXTARROW1PIC,               // 23
	TEXTARROW2PIC,               // 24
	TEXTBORDERTOPPIC,            // 25
	TEXTBORDERLEFTPIC,           // 26
	TEXTBORDERRIGHTPIC,          // 27
	TEXTBORDERBOTTOMPIC,         // 28
	ESCAPEKEYPIC,                // 29
	LEFTKEYPIC,                  // 30
	RIGHTKEYPIC,                 // 31
	ENTERKEYPIC,                 // 32
	ENDOFTEXTPIC,                // 33
	VISAPIC,                     // 34
	MASTERCARDPIC,               // 35

	HOSTAGE1PIC,                 // 36
	HOSTAGE2PIC,                 // 37
	HOSTAGE3PIC,                 // 38
#if (EPISODE == 2)
	KEENPIC,
#endif
	SNAKELOGANPIC,               // 39

#ifdef BETA
	STORY_JETPIC,
	STORY_NUKEPIC,
	STORY_HANGARPIC,
	STORY_OFFICEPIC,
	STORY_MUTANTSATTACKPIC,
	STORY_MASTERCAINPIC,
	STORY_CESSNAPIC,
	STORY_ARTILLERYPIC,
#elif (EPISODE == 1)
	ITEMSPIC,                    // 40
	STORY_JETPIC,                // 41
	STORY_NUKEPIC,               // 42
	STORY_OFFICEPIC,             // 43
	STORY_MASTERCAINPIC,         // 44
	STORY_CESSNAPIC,             // 45
	STORY_ARTILLERYPIC,          // 46
	STORY_MANGLEPIC,             // 47
	STORY_FILEPIC,               // 48
	STORY_LASERPIC,              // 49
#elif (EPISODE == 2)
	STORY_ENFORCERPIC,
	STORY_NUKEPIC,
	STORY_MUTANTSPIC,
	STORY_GRAVEPIC,
	STORY_OFFICEPIC,
	STORY_MASTERCAINPIC,
	STORY_MUTANTSATTACKPIC,
#elif (EPISODE == 3)
	STORY_CAINUNMASKEDPIC,
	STORY_CAINTALKPIC,
	STORY_SNAKERUNPIC,
	STORY_CAINEXPLODEPIC,
#endif

#ifndef BETA
	CREDITSPIC,                  // 50
#if (EPISODE == 1)
	PREVIEWPIC,                  // 51
	REGTITLEPIC,                 // 52
#endif
#endif
	PIRACYPIC,                   // 53
#if (defined VERSION_1_1) && (EPISODE == 1)
	TECHHELPPIC,                 // 54
#endif
#if (defined GMS_VERSION)
	GMSPIC,
#endif

	CP_MENUMASKPICM,             // 55

	//
	// SPRITES
	//
	CURSORARROWSPR,              // 56

	//player lump:
	START_LUMP(PLAYER)
	MEDIUMEXPLOSION1SPR,         // 57
	MEDIUMEXPLOSION2SPR,         // 58
	SMALLFIRE1SPR,               // 59
	SMALLFIRE2SPR,               // 60
	SMALLFIRE3SPR,               // 61
	ENEMYSHOTSPR,                // 62
#ifndef BETA
	LASERSHOTSPR,                // 63
	SUPERPLASMABOLTRSPR,         // 64
	SUPERPLASMABOLTLSPR,         // 65
#endif
	SNAKESTANDRSPR,              // 66
	SNAKEWALK1RSPR,              // 67
	SNAKEWALK2RSPR,              // 68
	SNAKEWALK3RSPR,              // 69
	SNAKEWALK4RSPR,              // 70
	SNAKESTANDLSPR,              // 71
	SNAKEWALK1LSPR,              // 72
	SNAKEWALK2LSPR,              // 73
	SNAKEWALK3LSPR,              // 74
	SNAKEWALK4LSPR,              // 75
#ifndef BETA
	SNAKESHIELD1RSPR,            // 76
	SNAKESHIELD2RSPR,            // 77
	SNAKESHIELD1LSPR,            // 78
	SNAKESHIELD2LSPR,            // 79
#endif
	SNAKEUSESPR,                 // 80
	SNAKECLIMB1SPR,              // 81
	SNAKECLIMB2SPR,              // 82
#ifdef BETA
	SNAKEJUMPRSPR,
	SNAKEJUMPLSPR,
#endif
	SNAKEDIE1SPR,                // 83
	SNAKEDIE2SPR,                // 84
#if (EPISODE != 1)
	SNAKEWORM1RSPR,
	SNAKEWORM1LSPR,
	SNAKEWORM2RSPR,
	SNAKEWORM2LSPR,
	SNAKEWORMATTACKRSPR,
	SNAKEWORMATTACKLSPR,
	SNAKEWORMJUMPRSPR,
	SNAKEWORMJUMPLSPR,
#endif
	SNAKESTANDTHROW1RSPR,        // 85
	SNAKESTANDTHROW2RSPR,        // 86
	SNAKESTANDTHROW1LSPR,        // 87
	SNAKESTANDTHROW2LSPR,        // 88
	SNAKEAIRTHROW1RSPR,          // 89
	SNAKEAIRTHROW2RSPR,          // 90
	SNAKEAIRTHROW1LSPR,          // 91
	SNAKEAIRTHROW2LSPR,          // 92
	SNAKESTANDFIRE1RSPR,         // 93
	SNAKESTANDFIRE2RSPR,         // 94
	SNAKESTANDFIRE1LSPR,         // 95
	SNAKESTANDFIRE2LSPR,         // 96
	SNAKEDUCKFIRE1RSPR,          // 97
	SNAKEDUCKFIRE2RSPR,          // 98
	SNAKEDUCKFIRE1LSPR,          // 99
	SNAKEDUCKFIRE2LSPR,          //100
	SNAKEAIRFIRE1RSPR,           //101
	SNAKEAIRFIRE2RSPR,           //102
	SNAKEAIRFIRE1LSPR,           //103
	SNAKEAIRFIRE2LSPR,           //104
	GRENADEGREEN1SPR,            //105
	GRENADEGREEN2SPR,            //106
	GRENADEGREEN3SPR,            //107
	GRENADEGREEN4SPR,            //108
#ifndef BETA
	GRENADERED1SPR,              //109
	GRENADERED2SPR,              //110
	GRENADERED3SPR,              //111
	GRENADERED4SPR,              //112
	BIGFIRE1SPR,                 //113
	BIGFIRE2SPR,                 //114
#endif
	GRENADEEXPLOSION1SPR,        //115
	GRENADEEXPLOSION2SPR,        //116
	GRENADEEXPLOSION3SPR,        //117
	GRENADEEXPLOSION4SPR,        //118
	GRENADEEXPLOSION5SPR,        //119
	GRENADEEXPLOSION6SPR,        //120
	FRAG1LSPR,                   //121
	FRAG2LSPR,                   //122
	FRAG3LSPR,                   //123
	FRAG4LSPR,                   //124
	FRAG1RSPR,                   //125
	FRAG2RSPR,                   //126
	FRAG3RSPR,                   //127
	FRAG4RSPR,                   //128
	TINYEXPLOSION1SPR,           //129
	TINYEXPLOSION2SPR,           //130
	TINYEXPLOSION3SPR,           //131
	CHUNKGROUNDLSPR,             //132
	CHUNKGROUNDRSPR,             //133
	MEAT1SPR,                    //134
	MEAT2SPR,                    //135
#ifndef BETA
	EYEBALL1SPR,                 //136
	EYEBALL2SPR,                 //137
	MEAT3SPR,                    //138
	MEAT4SPR,                    //139
#endif
	ITEMGREENGRENADE1SPR,        //140
	ITEMGREENGRENADE2SPR,        //141
#ifndef BETA
	ITEMREDGRENADE1SPR,          //142
	ITEMREDGRENADE2SPR,          //143
#endif
	ITEMMACHINEGUN1SPR,          //144
	ITEMMACHINEGUN2SPR,          //145
	METALCHUNK1LSPR,             //146
	METALCHUNK2LSPR,             //147
	METALCHUNK3LSPR,             //148
	METALCHUNK4LSPR,             //149
	METALCHUNK1RSPR,             //150
	METALCHUNK2RSPR,             //151
	METALCHUNK3RSPR,             //152
	METALCHUNK4RSPR,             //153
	ITEMKEYCARD1SPR,             //154
	ITEMKEYCARD2SPR,             //155
	ITEMKEY1SPR,                 //156
	ITEMKEY2SPR,                 //157
	ITEMBOTTLE1SPR,              //158
	ITEMBOTTLE2SPR,              //159
	ITEMVIAL1SPR,                //160
	ITEMVIAL2SPR,                //161
	ITEMGIZMO1SPR,               //162
	ITEMGIZMO2SPR,               //163
	ITEMBOOK1SPR,                //164
	ITEMBOOK2SPR,                //165
#ifdef BETA
	ITEMBEER1SPR,
	ITEMBEER2SPR,
	ITEMMEAL1SPR,
	ITEMMEAL2SPR,
#else
	ITEMGEM1SPR,                 //166
	ITEMGEM2SPR,                 //167
	ITEMFIRSTAID1SPR,            //168
	ITEMFIRSTAID2SPR,            //169
#endif
	ITEMNUKE1SPR,                //170
	ITEMNUKE2SPR,                //171
	ITEMANTIRADIATIONPILL1SPR,   //172
	ITEMANTIRADIATIONPILL2SPR,   //173
	ITEMINVINCIBILITYPOTION1SPR, //174
	ITEMINVINCIBILITYPOTION2SPR, //175
	ITEMDIAMOND1SPR,             //176
	ITEMDIAMOND2SPR,             //177
	ITEMORB1SPR,                 //178
	ITEMORB2SPR,                 //179
#ifndef BETA
	ITEMWARPGEM1SPR,             //180
	ITEMWARPGEM2SPR,             //181
#endif
	BIGEXPLOSION1SPR,            //182
	BIGEXPLOSION2SPR,            //183
	BIGEXPLOSION3SPR,            //184
	ITEMBLUESHARD1SPR,           //185
	ITEMBLUESHARD2SPR,           //186
	ITEMGREENSHARD1SPR,          //187
	ITEMGREENSHARD2SPR,          //188
	ITEMREDSHARD1SPR,            //189
	ITEMREDSHARD2SPR,            //190
	ITEMCYANSHARD1SPR,           //191
	ITEMCYANSHARD2SPR,           //192
	ITEMEXTRALIFE1SPR,           //193
	ITEMEXTRALIFE2SPR,           //194
	ITEMTAPEDECK1SPR,            //195
	ITEMTAPEDECK2SPR,            //196
	ITEMCOMPUTER1SPR,            //197
	ITEMCOMPUTER2SPR,            //198
	BONUS1UPSPR,                 //199
	BONUS100SPR,                 //200
	BONUS200SPR,                 //201
	BONUS500SPR,                 //202
	BONUS800SPR,                 //203
	BONUS1000SPR,                //204
	BONUS1500SPR,                //205
	BONUS2000SPR,                //206
	BONUS5000SPR,                //207
	BONUS50000SPR,               //208
	ITEMSPECIALKEY1SPR,          //209
	ITEMSPECIALKEY2SPR,          //210
	ITEMEXITKEY1SPR,             //211
	ITEMEXITKEY2SPR,             //212
#ifndef BETA
	ITEMGREENTRIANGLE1SPR,       //213
	ITEMGREENTRIANGLE2SPR,       //214
	ITEMPLASMABOLTS1SPR,         //215
	ITEMPLASMABOLTS2SPR,         //216
	ITEMSUPERGUN1SPR,            //217
	ITEMSUPERGUN2SPR,            //218
	ITEMLANDMINES1SPR,           //219
	ITEMLANDMINES2SPR,           //220
	LANDMINE1SPR,                //221
	LANDMINE2SPR,                //222
#endif
	END_LUMP(PLAYER)

	//hostage lump:
	START_LUMP(HOSTAGE)
#if (EPISODE == 1)
	HOSTAGE11SPR,                //223
	HOSTAGE12SPR,                //224
#else
	HOSTAGE11LSPR,
	HOSTAGE12LSPR,
	HOSTAGE11RSPR,
	HOSTAGE12RSPR,
#endif
	HOSTAGE21SPR,                //225
	HOSTAGE22SPR,                //226
#if (EPISODE == 1)
	HOSTAGE31SPR,                //227
	HOSTAGE32SPR,                //228
#else
	HOSTAGE31LSPR,
	HOSTAGE32LSPR,
	HOSTAGE31RSPR,
	HOSTAGE32RSPR,
#endif
	END_LUMP(HOSTAGE)

	//pinkworm lump:
	START_LUMP(PINKWORM)
	PINKWORM1LSPR,               //229
	PINKWORM2LSPR,               //230
	PINKWORM3LSPR,               //231
	PINKWORM1RSPR,               //232
	PINKWORM2RSPR,               //233
	PINKWORM3RSPR,               //234
	END_LUMP(PINKWORM)

	// the BUG lump is present as an empty lump in episode 1
	START_LUMP(BUG)
#if (EPISODE != 1)
	BUG1LSPR,
	BUG2LSPR,
	BUG1RSPR,
	BUG2RSPR,
#endif
	END_LUMP(BUG)

	//punchy lump:
	START_LUMP(PUNCHY)
	PUNCHYSTANDLSPR,             //235
	PUNCHYSTANDRSPR,             //236
	PUNCHYWALK1LSPR,             //237
	PUNCHYWALK2LSPR,             //238
	PUNCHYWALK3LSPR,             //239
	PUNCHYWALK4LSPR,             //240
	PUNCHYATTACK1LSPR,           //241
	PUNCHYATTACK2LSPR,           //242
	PUNCHYWALK1RSPR,             //243
	PUNCHYWALK2RSPR,             //244
	PUNCHYWALK3RSPR,             //245
	PUNCHYWALK4RSPR,             //246
	PUNCHYATTACK1RSPR,           //247
	PUNCHYATTACK2RSPR,           //248
	END_LUMP(PUNCHY)

#if (EPISODE != 1)
	START_LUMP(BEE)
	BEE1LSPR,
	BEE2LSPR,
	BEE1RSPR,
	BEE2RSPR,
	END_LUMP(BEE)
#endif

	//garg lump:
	START_LUMP(GORP)
	GORPCWALK1LSPR,              //249
	GORPCWALK2LSPR,              //250
	GORPCWALK3LSPR,              //251
	GORPCWALK4LSPR,              //252
	GORPFALLLSPR,                //253
	GORPCWALK1RSPR,              //254
	GORPCWALK2RSPR,              //255
	GORPCWALK3RSPR,              //256
	GORPCWALK4RSPR,              //257
	GORPFALLRSPR,                //258
	GORPWALK1LSPR,               //259
	GORPWALK2LSPR,               //260
	GORPWALK3LSPR,               //261
	GORPWALK4LSPR,               //262
	GORPWALK1RSPR,               //263
	GORPWALK2RSPR,               //264
	GORPWALK3RSPR,               //265
	GORPWALK4RSPR,               //266
	END_LUMP(GORP)

	//jumpbomb lump:
	START_LUMP(JUMPBOMB)
	JUMPBOMB1LSPR,               //267
	JUMPBOMB2LSPR,               //268
	JUMPBOMB3LSPR,               //269
	JUMPBOMB4LSPR,               //270
	JUMPBOMB1RSPR,               //271
	JUMPBOMB2RSPR,               //272
	JUMPBOMB3RSPR,               //273
	JUMPBOMB4RSPR,               //274
	END_LUMP(JUMPBOMB)

#if (EPISODE == 2)
	START_LUMP(ANT)
	ANTWALK1LSPR,
	ANTWALK2LSPR,
	ANTWALK3LSPR,
	ANTATTACK1LSPR,
	ANTATTACK2LSPR,
	ANTWALK1RSPR,
	ANTWALK2RSPR,
	ANTWALK3RSPR,
	ANTATTACK1RSPR,
	ANTATTACK2RSPR,
	END_LUMP(ANT)
#endif

	//cobra lump:
	START_LUMP(COBRA)
	COBRAWALK1LSPR,              //275
	COBRAWALK2LSPR,              //276
	COBRAWALK1RSPR,              //277
	COBRAWALK2RSPR,              //278
	COBRAATTACKLSPR,             //279
	COBRAATTACKRSPR,             //280
	COBRASPIT1LSPR,              //281
	COBRASPIT2LSPR,              //282
	COBRASPIT1RSPR,              //283
	COBRASPIT2RSPR,              //284
	END_LUMP(COBRA)

	//devil lump:
#if (EPISODE != 2)
	START_LUMP(DEVIL)
	DEVILWALK1LSPR,              //285
	DEVILWALK2LSPR,              //286
	DEVILWALK3LSPR,              //287
	DEVILWALK4LSPR,              //288
	DEVILWALK1RSPR,              //289
	DEVILWALK2RSPR,              //290
	DEVILWALK3RSPR,              //291
	DEVILWALK4RSPR,              //292
	DEVILSTANDSPR,               //293
	END_LUMP(DEVIL)
#endif

	//plasmawom lump:
#if (EPISODE != 2)
	START_LUMP(PLASMAWORM)
	PLASMAWORM1LSPR,             //294
	PLASMAWORM2LSPR,             //295
	PLASMAWORM1RSPR,             //296
	PLASMAWORM2RSPR,             //297
	END_LUMP(PLASMAWORM)
#endif

#if (EPISODE != 1)
	START_LUMP(BLUEDEVIL)
	BLUEDEVILWALK1LSPR,
	BLUEDEVILWALK2LSPR,
	BLUEDEVILWALK3LSPR,
	BLUEDEVILWALK4LSPR,
	BLUEDEVILJUMPLSPR,
	BLUEDEVILWALK1RSPR,
	BLUEDEVILWALK2RSPR,
	BLUEDEVILWALK3RSPR,
	BLUEDEVILWALK4RSPR,
	BLUEDEVILJUMPRSPR,
	END_LUMP(BLUEDEVIL)
#endif

	//sewerman lump:
#if (EPISODE == 1)
	START_LUMP(SEWERMAN)
	SEWERMANWALK1LSPR,           //298
	SEWERMANWALK2LSPR,           //299
	SEWERMANATTACKLSPR,          //300
	SEWERMANWALK1RSPR,           //301
	SEWERMANWALK2RSPR,           //302
	SEWERMANATTACKRSPR,          //303
	SEWERMANSPIT1SPR,            //304
	SEWERMANSPIT2SPR,            //305
	END_LUMP(SEWERMAN)
#endif

	//acidmonster lump:
	START_LUMP(ACIDMONSTER)
	ACIDMONSTER1SPR,             //306
	ACIDMONSTER2SPR,             //307
	ACIDMONSTER3SPR,             //308
	ACIDDROP1SPR,                //309
	ACIDDROP2SPR,                //310
	ACIDDROP3SPR,                //311
	ACIDLAND1SPR,                //312
	ACIDLAND2SPR,                //313
	ACIDLAND3SPR,                //314
	ACIDLAND4SPR,                //315
	END_LUMP(ACIDMONSTER)

	//caterpillar lump:
#if (EPISODE == 2)
	__CATERPILLARLUMP__
#endif

	//queen lump:
#if (EPISODE == 2)
	START_LUMP(QUEENANT)
	NYMPH1SPR,
	NYMPH2SPR,
	QUEEN1SPR,
	QUEEN2SPR,
	END_LUMP(QUEENANT)
#endif

	//spikey lump:
#if (EPISODE != 1)
	START_LUMP(SPIKEY)
	SPIKEY1SPR,
	SPIKEY2SPR,
	END_LUMP(SPIKEY)
#endif

	//kamikaze lump:
#if (EPISODE != 1)
	START_LUMP(KAMIKAZE)
	KAMIKAZE1LSPR,
	KAMIKAZE2LSPR,
	KAMIKAZE3LSPR,
	KAMIKAZE4LSPR,
	KAMIKAZE1RSPR,
	KAMIKAZE2RSPR,
	KAMIKAZE3RSPR,
	KAMIKAZE4RSPR,
	KAMIKAZEDIE1SPR,
#if (EPISODE == 2)
	KAMIKAZEDIE2SPR,
#else
	KAMIKAZEDIE2SPR = KAMIKAZEDIE1SPR,	//E3 uses DIE1 where E2 used DIE2
#endif
	END_LUMP(KAMIKAZE)
#endif

	//slimer lump:
#if (EPISODE != 2)
	START_LUMP(SLIMER)
	SLIMERWALK1LSPR,             //316
	SLIMERWALK2LSPR,             //317
	SLIMERWALK1RSPR,             //318
	SLIMERWALK2RSPR,             //319
	SLIMERATTACKSPR,             //320
	END_LUMP(SLIMER)
#endif

	//hairy lump:
#if (EPISODE != 2)
	START_LUMP(HAIRY)
#if (EPISODE == 1)
	HAIRYSTANDRSPR,              //321
	HAIRYSTANDLSPR,              //322
#else
	HAIRYSTANDLSPR,
	HAIRYSTANDRSPR,
#endif
	HAIRYWALK1LSPR,              //323
	HAIRYWALK2LSPR,              //324
	HAIRYWALK3LSPR,              //325
	HAIRYWALK4LSPR,              //326
	HAIRYWALK1RSPR,              //327
	HAIRYWALK2RSPR,              //328
	HAIRYWALK3RSPR,              //329
	HAIRYWALK4RSPR,              //330
	END_LUMP(HAIRY)
#endif
	
	// Either the PEOPLE lump or the TRASHBOSS lump is present as an empty
	// lump in the lumpstart and lumpend arrays in BM_SPEC.C in episode 1,
	// but not in episode 3. I could add an extra #ifdef to exclude the
	// start and end values for episode 3, but all it would do is reduce
	// the symbol table size by a few bytes while compiling the code. The
	// resulting executable would be the same either way.
	START_LUMP(PEOPLE)
#if (EPISODE == 2)
	JIM1LSPR,
	JIM1RSPR,
	JIM2LSPR,
	JIM2RSPR,
	GEORGE1LSPR,
	GEORGE2LSPR,
	GEORGE1RSPR,
	GEORGE2RSPR,
	KEEN1SPR,
	KEEN2SPR,
#endif
	END_LUMP(PEOPLE)

	//trashboss lump:
#if (EPISODE == 2)
	START_LUMP(TRASHBOSS)
	TRASHBOSS1LSPR,
	TRASHBOSS1RSPR,
	TRASHBOSS2LSPR,
	TRASHBOSS2RSPR,
	TRASHBOSS3LSPR,
	TRASHBOSS3RSPR,
	TRASHCAN1SPR,
	TRASHCAN2SPR,
	TRASHCAN3SPR,
	TRASHCAN4SPR,
	TRASHAPPLE1SPR,
	TRASHAPPLE2SPR,
	TRASHAPPLE3SPR,
	TRASHAPPLE4SPR,
	TRASHBIRD1LSPR,
	TRASHBIRD2LSPR,
	TRASHBIRD1RSPR,
	TRASHBIRD2RSPR,
	END_LUMP(TRASHBOSS)
#endif

#if (EPISODE == 2) && !(defined BETA)
	__BEACONLUMP__
#endif

	//skullman lump:
#if (EPISODE == 1)
	START_LUMP(SKULLMAN)
	SKULLMAN1SPR,                //331
	SKULLMAN2SPR,                //332
	SKULLMANHANDRSPR,            //333
	SKULLMANHANDLSPR,            //334
	SKULLMANSWIPE1SPR,           //335
	SKULLMANSWIPE2SPR,           //336
	END_LUMP(SKULLMAN)
#endif

	//boss lump:
	START_LUMP(BOSS)
	BOSSHEAD1SPR,                //337
	BOSSHEAD2SPR,                //338
	BOSSHEAD3SPR,                //339
	BOSSHEAD1FADESPR,            //340
	BOSSHEAD2FADESPR,            //341
	BOSSHEAD3FADESPR,            //342
	BOSSHEADMETALSPR,            //343
#if (EPISODE == 1)
	//mangle should be a lump on its own, but it is not, which wastes memory
	MANGLESTAND1SPR,             //344
	MANGLESTAND2SPR,             //345
	MANGLEDEADSPR,               //346
	MANGLEWALK1LSPR,             //347
	MANGLEWALK2LSPR,             //348
	MANGLEWALK1RSPR,             //349
	MANGLEWALK2RSPR,             //350
#endif
	END_LUMP(BOSS)

	//helicopter lump:
#if (EPISODE == 1)
	START_LUMP(HELICOPTER)
	HELICOPTER1SPR,              //351
	HELICOPTER2SPR,              //352
	PARABOTFALLLSPR,             //353
	PARABOTFALLRSPR,             //354
	PARABOTWALK1LSPR,            //355
	PARABOTWALK2LSPR,            //356
	PARABOTWALK1RSPR,            //357
	PARABOTWALK2RSPR,            //358
	END_LUMP(HELICOPTER)
#endif

#if (EPISODE == 1)
	__BEACONLUMP__
#endif

	//bouncer lump:
	START_LUMP(BOUNCER)
	BOUNCER1SPR,                 //364
	BOUNCER2SPR,                 //365
	BOUNCER3SPR,                 //366
	BOUNCER4SPR,                 //367
	BOUNCER5SPR,                 //368
	END_LUMP(BOUNCER)

	//platform_lump:
	START_LUMP(PLATFORM)
	PLATFORMSPR,                 //369
	END_LUMP(PLATFORM)

	//shuttle lump:
	START_LUMP(SHUTTLE)
#if (EPISODE != 1)
	SHUTTLESPR,
	SNAKESHUTTLESPR,
#endif
	END_LUMP(SHUTTLE)

	//trashtruck lump:
#if (EPISODE != 1)
	START_LUMP(TRASHTRUCK)
	TRASHTRUCK1RSPR,
	TRASHTRUCK2RSPR,
	TRASHTRUCK1LSPR,
	TRASHTRUCK2LSPR,
	END_LUMP(TRASHTRUCK)
#endif

	//turret lump:
	START_LUMP(TURRET)
	TURRET1LSPR,                 //370
	TURRET2LSPR,                 //371
	TURRET3LSPR,                 //372
	TURRET1RSPR,                 //373
	TURRET2RSPR,                 //374
	TURRET3RSPR,                 //375
	END_LUMP(TURRET)

	//guard lump:
	START_LUMP(GUARD)
	GUARD1LSPR,                   //376
	GUARD2LSPR,                   //377
	GUARD1RSPR,                   //378
	GUARD2RSPR,                   //379
	END_LUMP(GUARD)

#if (EPISODE == 3)
	START_LUMP(GRENADEBOT)
	GRENADEBOT1LSPR,
	GRENADEBOT2LSPR,
	GRENADEBOT1RSPR,
	GRENADEBOT2RSPR,
	PLASMAGRENADESPR,
	END_LUMP(GRENADEBOT)
#endif

#if (EPISODE == 3)
	__PLASMALUMP__
	__CATERPILLARLUMP__
#else
	//sparkbot lump: (never used as such!)
	START_LUMP(SPARKBOT)
	SPARKBOTA1SPR,          //380
	SPARKBOTA2SPR,          //381
	SPARKBOTB1SPR,          //382
	SPARKBOTB2SPR,          //383
	END_LUMP(SPARKBOT)
	// The sparkbot sprites are present in episodes 1 and 2 (including the
	// beta) but are never actually used in those episodes. Episode 3 used
	// the sprites to replace the caterpillar sprites.
#endif

#if (EPISODE == 1)
	__PLASMALUMP__
#endif

	//block lump:
	START_LUMP(BLOCK)
	BLOCKSPR,                    //386
	END_LUMP(BLOCK)

	//robopal lump:
	START_LUMP(ROBOPAL)
	ROBOPAL1SPR,                 //387
	ROBOPAL2SPR,                 //388
	ROBOPAL3SPR,                 //389
	ROBOPAL4SPR,                 //390
	ROBOPALSHOT1RSPR,            //391
	ROBOPALSHOT2RSPR,            //392
	ROBOPALSHOT1LSPR,            //393
	ROBOPALSHOT2LSPR,            //394
	ROBOPALSHOTEXPL1SPR,         //395
	ROBOPALSHOTEXPL2SPR,         //396
	END_LUMP(ROBOPAL)

#if (EPISODE == 3)
	START_LUMP(EXPLOSIVES)
	BIGLANDMINE1SPR,
	BIGLANDMINE2SPR,
	BIGLANDMINE3SPR,
	BARRELSPR,
	END_LUMP(EXPLOSIVES)
#endif

	//crusher lump:
	START_LUMP(CRUSHER)
	CRUSHERSPR,                  //397
	END_LUMP(CRUSHER)

	//brick lump:
	START_LUMP(BRICK)
	BRICKSPR,                    //398
	END_LUMP(BRICK)

	//tank lump:
	START_LUMP(TANK)
	TANK1SPR,                    //399
	TANK2SPR,                    //400
	TANK3SPR,                    //401
	TANKSHOTSPR,                 //402
	SMALLROCKETFLYSPR,           //403
	SMALLROCKETFALLSPR,          //404
	END_LUMP(TANK)

	//cyborg lump:
#if (EPISODE == 3)
	START_LUMP(CYBORG)
	CYBORG1RSPR,
	CYBORG2RSPR,
	CYBORG3RSPR,
	CYBORG4RSPR,
	CYBORG1LSPR,
	CYBORG2LSPR,
	CYBORG3LSPR,
	CYBORG4LSPR,
	CYBORGSHOTSPR,
	END_LUMP(CYBORG)
#endif

#if (EPISODE == 3)
	__BEACONLUMP__
#endif

	//goliath lump:
#if (EPISODE == 3)
	START_LUMP(GOLIATH)
	GOLIATH1SPR,
	GOLIATH2SPR,
	GOLIATH3SPR,
	GOLIATHSHOT1SPR,
	GOLIATHSHOT2SPR,
	END_LUMP(GOLIATH)
#endif

	//cain lump:
#if (EPISODE == 3)
	START_LUMP(CAIN)
	CAINSTANDLSPR,
	CAINSTANDRSPR,
	CAINDUCKLSPR,
	CAINDUCKRSPR,
	CAINGHOST1LSPR,
	CAINGHOST2LSPR,
	CAINGHOST1RSPR,
	CAINGHOST2RSPR,
	CAINSHOT1SPR,
	CAINSHOT2SPR,
	CAINSHOT3SPR,
	CAINDEAD1SPR,
	CAINDEAD2SPR,
	END_LUMP(CAIN)
#endif

	//not part of a lump:
	SCOREBOXSPR,                 //405

	//intro lump:
#ifndef BETA
	START_LUMP(INTRO)
	APOGEESPR,                   //406
	ASTEROID1SPR,                //407
	ASTEROID2SPR,                //408
	ASTEROID3SPR,                //409
	ASTEROID4SPR,                //410
	END_LUMP(INTRO)
#endif

	//
	// TILES (don't need names)
	//
	FIRSTTILE,
	LASTTILE = STARTEXTERNS-1,
	
	//
	// EXTERNS
	//

#ifndef BETA
	//texts
	HIGHSCORETEXT,               // 4607
	GAMEHELPTEXT,                // 4608
	ENDINGTEXT,                  // 4609
	MENUHELPTEXT,                // 4610
	STORYTEXT,                   // 4611
	ORDERTEXT,                   // 4612
#if (EPISODE == 1)
	ANNOYINGTEXT,                // 4613
#elif (EPISODE == 2)
	BADENDINGTEXT,
#endif

	//demos
	HIGHSCOREDEMO,               // 4614
	APOGEEDEMO,                  // 4615
#if (EPISODE == 1)
	FIRSTDEMO,                   // 4616
	SECONDDEMO,                  // 4617
#endif
	LEVELWARPDEMO,               // 4618
	
	//exit screens
	ORDERSCREEN,                 // 4619
	OUTOFMEM,                    // 4620
#endif

	NUMGRCHUNKS
} graphicnums;

#undef START_LUMP
#undef END_LUMP

#undef __BEACONLUMP__
#undef __PLASMALUMP__
#undef __CATERPILLARLUMP__

REFKEEN_NS_E

#endif //__GFX_H__
