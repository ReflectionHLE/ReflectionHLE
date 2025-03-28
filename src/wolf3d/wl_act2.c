/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2025 NY00123
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

// WL_ACT2.C

#include "wl_def.h"
//#pragma hdrstop

REFKEEN_NS_B

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define PROJECTILESIZE	0xc000l

#define BJRUNSPEED	2048
#define BJJUMPSPEED	680


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/



/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


dirtype dirtable[9] = {northwest,north,northeast,west,nodir,east,
	southwest,south,southeast};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
id0_int_t	starthitpoints[NUMENEMIES] =
	 {6,	// sheeps
	  12,	// ostriches
	  25,	// antelopes
	  1,	// goats
	  18,	// oxen
	  150,	// Carl
	  350,	// Melvin
	  300,	// Ginny
	  400,	// Kerry
	  450,	// Ernie
	  500	// Burt
	  };
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
id0_int_t	starthitpoints[NUMENEMIES] =
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs
	// *** ALPHA RESTORATION ***
	// Yes, there's a buffer overflow when Hans is spawned in the alpha
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	  850,	// Hans
	  850,	// Schabbs
	  300,	// fake hitler
	  900,	// mecha hitler
	  75,	// mutants
	  25,	// ghosts
	  0,	// ghosts
	  0,	// ghosts
	  0	// ghosts
#endif
	  };
#else
id0_int_t	starthitpoints[4][NUMENEMIES] =
	 //
	 // BABY MODE
	 //
	 {
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs
	  850,	// Hans
	  850,	// Schabbs
	  200,	// fake hitler
	  800,	// mecha hitler
	  45,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  850,	// Gretel
	  850,	// Gift
	  850	// Fat
	  // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	  ,
	  5,	// en_spectre,
	  1450,	// en_angel,
	  850,	// en_trans,
	  1050,	// en_uber,
	  950,	// en_will,
	  1250	// en_death
#endif
	  },
	 //
	 // DON'T HURT ME MODE
	 //
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs
	  950,	// Hans
	  950,	// Schabbs
	  300,	// fake hitler
	  950,	// mecha hitler
	  55,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  950,	// Gretel
	  950,	// Gift
	  950 	// Fat
	  // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	  ,
	  10,	// en_spectre,
	  1550,	// en_angel,
	  950,	// en_trans,
	  1150,	// en_uber,
	  1050,	// en_will,
	  1350	// en_death
#endif
	  },
	 //
	 // BRING 'EM ON MODE
	 //
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs

	  1050,	// Hans
	  1550,	// Schabbs
	  400,	// fake hitler
	  1050,	// mecha hitler

	  55,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  1050,	// Gretel
	  1050,	// Gift
	  1050 	// Fat
	  // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	  ,
	  15,	// en_spectre,
	  1650,	// en_angel,
	  1050,	// en_trans,
	  1250,	// en_uber,
	  1150,	// en_will,
	  1450	// en_death
#endif
	  },
	 //
	 // DEATH INCARNATE MODE
	 //
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs

	  1200,	// Hans
	  2400,	// Schabbs
	  500,	// fake hitler
	  1200,	// mecha hitler

	  65,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  1200,	// Gretel
	  1200,	// Gift
	  1200	// Fat
	  // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	  ,
	  25,	// en_spectre,
	  2000,	// en_angel,
	  1200,	// en_trans,
	  1400,	// en_uber,
	  1300,	// en_will,
	  1600	// en_death
#endif
	  }}
	  ;
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
void	A_StartDeathCam (objtype *ob);
#endif


void	T_Path (objtype *ob);
void	T_Shoot (objtype *ob);
void	T_Bite (objtype *ob);
void	T_DogChase (objtype *ob);
void	T_Chase (objtype *ob);
void	T_Projectile (objtype *ob);
void	T_Stand (objtype *ob);

void A_DeathScream (objtype *ob);

// *** PRE-V1.4 APOGEE RESTORATION *** - Relocated code to a separate file for v1.2
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
#include "wl_srock.c"
#endif

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void	T_Schabb (objtype *ob);
void	T_SchabbThrow (objtype *ob);
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
void	T_Fake (objtype *ob);
void	T_FakeFire (objtype *ob);
void	T_Ghosts (objtype *ob);
#endif

void A_Slurpie (objtype *ob);
void A_HitlerMorph (objtype *ob);
void A_MechaSound (objtype *ob);
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


// *** PRE-V1.4 APOGEE + S3DNA RESTORATION *** - Relocated code to separate
// files for for v1.2, but even here, WL_FSMOK.C is not included for S3DNA.
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
#ifndef GAMEVER_NOAH3D
#include "wl_fsmok.c"
#endif
#include "wl_fproj.c"
#endif



/*
=============================================================================

							GUARD

=============================================================================
*/

//
// guards
//

extern	statetype s_grdstand;

extern	statetype s_grdpath1;
extern	statetype s_grdpath1s;
extern	statetype s_grdpath2;
extern	statetype s_grdpath3;
extern	statetype s_grdpath3s;
extern	statetype s_grdpath4;

extern	statetype s_grdpain;
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
extern	statetype s_grdpain1;
#endif

extern	statetype s_grdgiveup;

extern	statetype s_grdshoot1;
extern	statetype s_grdshoot2;
extern	statetype s_grdshoot3;
extern	statetype s_grdshoot4;

extern	statetype s_grdchase1;
extern	statetype s_grdchase1s;
extern	statetype s_grdchase2;
extern	statetype s_grdchase3;
extern	statetype s_grdchase3s;
extern	statetype s_grdchase4;

extern	statetype s_grddie1;
extern	statetype s_grddie1d;
extern	statetype s_grddie2;
extern	statetype s_grddie3;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_grddead;
extern	statetype s_grddead2;
#else
extern	statetype s_grddie4;
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_grdstand	= {true,SPR_GRD_W1_1,0,T_Stand,NULL,&s_grdstand};
#else
statetype s_grdstand	= {true,SPR_GRD_S_1,0,T_Stand,NULL,&s_grdstand};
#endif

statetype s_grdpath1 	= {true,SPR_GRD_W1_1,20,T_Path,NULL,&s_grdpath1s};
statetype s_grdpath1s 	= {true,SPR_GRD_W1_1,5,NULL,NULL,&s_grdpath2};
statetype s_grdpath2 	= {true,SPR_GRD_W2_1,15,T_Path,NULL,&s_grdpath3};
statetype s_grdpath3 	= {true,SPR_GRD_W3_1,20,T_Path,NULL,&s_grdpath3s};
statetype s_grdpath3s 	= {true,SPR_GRD_W3_1,5,NULL,NULL,&s_grdpath4};
statetype s_grdpath4 	= {true,SPR_GRD_W4_1,15,T_Path,NULL,&s_grdpath1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_grdpain 	= {2,SPR_GRD_PAIN,10,NULL,NULL,&s_grdchase1};
statetype s_grdpain1 	= {2,SPR_GRD_PAIN,10,NULL,NULL,&s_grdchase1};
#else
statetype s_grdpain 	= {2,SPR_GRD_PAIN_1,10,NULL,NULL,&s_grdchase1};
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
statetype s_grdpain1 	= {2,SPR_GRD_PAIN_2,10,NULL,NULL,&s_grdchase1};
#endif
#endif

statetype s_grdshoot1 	= {false,SPR_GRD_SHOOT1,20,NULL,NULL,&s_grdshoot2};
statetype s_grdshoot2 	= {false,SPR_GRD_SHOOT2,20,NULL,T_Shoot,&s_grdshoot3};
statetype s_grdshoot3 	= {false,SPR_GRD_SHOOT3,20,NULL,NULL,&s_grdchase1};

statetype s_grdchase1 	= {true,SPR_GRD_W1_1,10,T_Chase,NULL,&s_grdchase1s};
statetype s_grdchase1s 	= {true,SPR_GRD_W1_1,3,NULL,NULL,&s_grdchase2};
statetype s_grdchase2 	= {true,SPR_GRD_W2_1,8,T_Chase,NULL,&s_grdchase3};
statetype s_grdchase3 	= {true,SPR_GRD_W3_1,10,T_Chase,NULL,&s_grdchase3s};
statetype s_grdchase3s 	= {true,SPR_GRD_W3_1,3,NULL,NULL,&s_grdchase4};
statetype s_grdchase4 	= {true,SPR_GRD_W4_1,8,T_Chase,NULL,&s_grdchase1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_grddie1		= {false,SPR_GRD_PAIN,15,NULL,A_DeathScream,&s_grddie2};
statetype s_grddie2		= {false,SPR_GRD_DIE_1,15,NULL,NULL,&s_grddie3};
statetype s_grddie3		= {false,SPR_GRD_DIE_2,15,NULL,NULL,&s_grddead};
statetype s_grddead		= {false,SPR_GRD_DEAD,55,NULL,NULL,&s_grddead2};
statetype s_grddead2		= {false,SPR_GRD_DEAD2,55,NULL,NULL,&s_grddead};
#else
statetype s_grddie1		= {false,SPR_GRD_DIE_1,15,NULL,A_DeathScream,&s_grddie2};
statetype s_grddie2		= {false,SPR_GRD_DIE_2,15,NULL,NULL,&s_grddie3};
statetype s_grddie3		= {false,SPR_GRD_DIE_3,15,NULL,NULL,&s_grddie4};
statetype s_grddie4		= {false,SPR_GRD_DEAD,0,NULL,NULL,&s_grddie4};
#endif


// *** S3DNA + ALPHA RESTORATION ***
#if (!defined SPEAR) && (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
//
// ghosts
//
extern	statetype s_blinkychase1;
extern	statetype s_blinkychase2;
extern	statetype s_inkychase1;
extern	statetype s_inkychase2;
extern	statetype s_pinkychase1;
extern	statetype s_pinkychase2;
extern	statetype s_clydechase1;
extern	statetype s_clydechase2;

statetype s_blinkychase1 	= {false,SPR_BLINKY_W1,10,T_Ghosts,NULL,&s_blinkychase2};
statetype s_blinkychase2 	= {false,SPR_BLINKY_W2,10,T_Ghosts,NULL,&s_blinkychase1};

statetype s_inkychase1 		= {false,SPR_INKY_W1,10,T_Ghosts,NULL,&s_inkychase2};
statetype s_inkychase2 		= {false,SPR_INKY_W2,10,T_Ghosts,NULL,&s_inkychase1};

statetype s_pinkychase1 	= {false,SPR_PINKY_W1,10,T_Ghosts,NULL,&s_pinkychase2};
statetype s_pinkychase2 	= {false,SPR_PINKY_W2,10,T_Ghosts,NULL,&s_pinkychase1};

statetype s_clydechase1 	= {false,SPR_CLYDE_W1,10,T_Ghosts,NULL,&s_clydechase2};
statetype s_clydechase2 	= {false,SPR_CLYDE_W2,10,T_Ghosts,NULL,&s_clydechase1};
#endif

//
// dogs
//

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_dogstand;
#endif
extern	statetype s_dogpath1;
extern	statetype s_dogpath1s;
extern	statetype s_dogpath2;
extern	statetype s_dogpath3;
extern	statetype s_dogpath3s;
extern	statetype s_dogpath4;

extern	statetype s_dogjump1;
extern	statetype s_dogjump2;
extern	statetype s_dogjump3;
extern	statetype s_dogjump4;
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_dogjump5;
#endif

extern	statetype s_dogchase1;
extern	statetype s_dogchase1s;
extern	statetype s_dogchase2;
extern	statetype s_dogchase3;
extern	statetype s_dogchase3s;
extern	statetype s_dogchase4;

extern	statetype s_dogdie1;
extern	statetype s_dogdie1d;
extern	statetype s_dogdie2;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_dogdead;
extern	statetype s_dogdead2;
#else
extern	statetype s_dogdie3;
extern	statetype s_dogdead;
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_dogstand	= {true,SPR_DOG_W1_1,0,T_Stand,NULL,&s_dogstand};
#endif

statetype s_dogpath1 	= {true,SPR_DOG_W1_1,20,T_Path,NULL,&s_dogpath1s};
statetype s_dogpath1s 	= {true,SPR_DOG_W1_1,5,NULL,NULL,&s_dogpath2};
statetype s_dogpath2 	= {true,SPR_DOG_W2_1,15,T_Path,NULL,&s_dogpath3};
statetype s_dogpath3 	= {true,SPR_DOG_W3_1,20,T_Path,NULL,&s_dogpath3s};
statetype s_dogpath3s 	= {true,SPR_DOG_W3_1,5,NULL,NULL,&s_dogpath4};
statetype s_dogpath4 	= {true,SPR_DOG_W4_1,15,T_Path,NULL,&s_dogpath1};

statetype s_dogjump1 	= {false,SPR_DOG_JUMP1,10,NULL,NULL,&s_dogjump2};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_dogjump2 	= {false,SPR_DOG_JUMP2,10,NULL,NULL,&s_dogjump3};
statetype s_dogjump3 	= {false,SPR_DOG_JUMP3,10,NULL,T_Bite,&s_dogjump4};
statetype s_dogjump4 	= {false,SPR_DOG_JUMP4,20,NULL,T_Bite,&s_dogchase1};
#else
statetype s_dogjump2 	= {false,SPR_DOG_JUMP2,10,NULL,T_Bite,&s_dogjump3};
statetype s_dogjump3 	= {false,SPR_DOG_JUMP3,10,NULL,NULL,&s_dogjump4};
statetype s_dogjump4 	= {false,SPR_DOG_JUMP1,10,NULL,NULL,&s_dogjump5};
statetype s_dogjump5 	= {false,SPR_DOG_W1_1,10,NULL,NULL,&s_dogchase1};
#endif

statetype s_dogchase1 	= {true,SPR_DOG_W1_1,10,T_DogChase,NULL,&s_dogchase1s};
statetype s_dogchase1s 	= {true,SPR_DOG_W1_1,3,NULL,NULL,&s_dogchase2};
statetype s_dogchase2 	= {true,SPR_DOG_W2_1,8,T_DogChase,NULL,&s_dogchase3};
statetype s_dogchase3 	= {true,SPR_DOG_W3_1,10,T_DogChase,NULL,&s_dogchase3s};
statetype s_dogchase3s 	= {true,SPR_DOG_W3_1,3,NULL,NULL,&s_dogchase4};
statetype s_dogchase4 	= {true,SPR_DOG_W4_1,8,T_DogChase,NULL,&s_dogchase1};

statetype s_dogdie1		= {false,SPR_DOG_DIE_1,15,NULL,A_DeathScream,&s_dogdie2};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_dogdie2		= {false,SPR_DOG_DIE_2,15,NULL,NULL,&s_dogdead};
statetype s_dogdead		= {false,SPR_DOG_DEAD,55,NULL,NULL,&s_dogdead2};
statetype s_dogdead2		= {false,SPR_DOG_DEAD2,55,NULL,NULL,&s_dogdead};
#else
statetype s_dogdie2		= {false,SPR_DOG_DIE_2,15,NULL,NULL,&s_dogdie3};
statetype s_dogdie3		= {false,SPR_DOG_DIE_3,15,NULL,NULL,&s_dogdead};
statetype s_dogdead		= {false,SPR_DOG_DEAD,15,NULL,NULL,&s_dogdead};
#endif


//
// officers
//

extern	statetype s_ofcstand;

extern	statetype s_ofcpath1;
extern	statetype s_ofcpath1s;
extern	statetype s_ofcpath2;
extern	statetype s_ofcpath3;
extern	statetype s_ofcpath3s;
extern	statetype s_ofcpath4;

extern	statetype s_ofcpain;
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
extern	statetype s_ofcpain1;
#endif

extern	statetype s_ofcgiveup;

extern	statetype s_ofcshoot1;
extern	statetype s_ofcshoot2;
extern	statetype s_ofcshoot3;
extern	statetype s_ofcshoot4;

extern	statetype s_ofcchase1;
extern	statetype s_ofcchase1s;
extern	statetype s_ofcchase2;
extern	statetype s_ofcchase3;
extern	statetype s_ofcchase3s;
extern	statetype s_ofcchase4;

extern	statetype s_ofcdie1;
extern	statetype s_ofcdie2;
extern	statetype s_ofcdie3;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_ofcdead;
extern	statetype s_ofcdead2;
#else
extern	statetype s_ofcdie4;
extern	statetype s_ofcdie5;
#endif

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_ofcstand	= {true,SPR_OFC_W1_1,0,T_Stand,NULL,&s_ofcstand};
#else
statetype s_ofcstand	= {true,SPR_OFC_S_1,0,T_Stand,NULL,&s_ofcstand};
#endif

statetype s_ofcpath1 	= {true,SPR_OFC_W1_1,20,T_Path,NULL,&s_ofcpath1s};
statetype s_ofcpath1s 	= {true,SPR_OFC_W1_1,5,NULL,NULL,&s_ofcpath2};
statetype s_ofcpath2 	= {true,SPR_OFC_W2_1,15,T_Path,NULL,&s_ofcpath3};
statetype s_ofcpath3 	= {true,SPR_OFC_W3_1,20,T_Path,NULL,&s_ofcpath3s};
statetype s_ofcpath3s 	= {true,SPR_OFC_W3_1,5,NULL,NULL,&s_ofcpath4};
statetype s_ofcpath4 	= {true,SPR_OFC_W4_1,15,T_Path,NULL,&s_ofcpath1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_ofcpain 	= {2,SPR_OFC_PAIN,10,NULL,NULL,&s_ofcchase1};
statetype s_ofcpain1 	= {2,SPR_OFC_PAIN,10,NULL,NULL,&s_ofcchase1};
#else
statetype s_ofcpain 	= {2,SPR_OFC_PAIN_1,10,NULL,NULL,&s_ofcchase1};
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
statetype s_ofcpain1 	= {2,SPR_OFC_PAIN_2,10,NULL,NULL,&s_ofcchase1};
#endif
#endif

statetype s_ofcshoot1 	= {false,SPR_OFC_SHOOT1,6,NULL,NULL,&s_ofcshoot2};
statetype s_ofcshoot2 	= {false,SPR_OFC_SHOOT2,20,NULL,T_Shoot,&s_ofcshoot3};
statetype s_ofcshoot3 	= {false,SPR_OFC_SHOOT3,10,NULL,NULL,&s_ofcchase1};

statetype s_ofcchase1 	= {true,SPR_OFC_W1_1,10,T_Chase,NULL,&s_ofcchase1s};
statetype s_ofcchase1s 	= {true,SPR_OFC_W1_1,3,NULL,NULL,&s_ofcchase2};
statetype s_ofcchase2 	= {true,SPR_OFC_W2_1,8,T_Chase,NULL,&s_ofcchase3};
statetype s_ofcchase3 	= {true,SPR_OFC_W3_1,10,T_Chase,NULL,&s_ofcchase3s};
statetype s_ofcchase3s 	= {true,SPR_OFC_W3_1,3,NULL,NULL,&s_ofcchase4};
statetype s_ofcchase4 	= {true,SPR_OFC_W4_1,8,T_Chase,NULL,&s_ofcchase1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_ofcdie1		= {false,SPR_OFC_PAIN,11,NULL,A_DeathScream,&s_ofcdie2};
statetype s_ofcdie2		= {false,SPR_OFC_DIE_1,11,NULL,NULL,&s_ofcdie3};
statetype s_ofcdie3		= {false,SPR_OFC_DIE_2,11,NULL,NULL,&s_ofcdead};
statetype s_ofcdead		= {false,SPR_OFC_DEAD,55,NULL,NULL,&s_ofcdead2};
statetype s_ofcdead2		= {false,SPR_OFC_DEAD2,55,NULL,NULL,&s_ofcdead};
#else
statetype s_ofcdie1		= {false,SPR_OFC_DIE_1,11,NULL,A_DeathScream,&s_ofcdie2};
statetype s_ofcdie2		= {false,SPR_OFC_DIE_2,11,NULL,NULL,&s_ofcdie3};
statetype s_ofcdie3		= {false,SPR_OFC_DIE_3,11,NULL,NULL,&s_ofcdie4};
statetype s_ofcdie4		= {false,SPR_OFC_DIE_4,11,NULL,NULL,&s_ofcdie5};
statetype s_ofcdie5		= {false,SPR_OFC_DEAD,0,NULL,NULL,&s_ofcdie5};
#endif


//
// mutant
//

extern	statetype s_mutstand;

extern	statetype s_mutpath1;
extern	statetype s_mutpath1s;
extern	statetype s_mutpath2;
extern	statetype s_mutpath3;
extern	statetype s_mutpath3s;
extern	statetype s_mutpath4;

extern	statetype s_mutpain;
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
extern	statetype s_mutpain1;
#endif

extern	statetype s_mutgiveup;

extern	statetype s_mutshoot1;
extern	statetype s_mutshoot2;
extern	statetype s_mutshoot3;
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_mutshoot4;
#endif

extern	statetype s_mutchase1;
extern	statetype s_mutchase1s;
extern	statetype s_mutchase2;
extern	statetype s_mutchase3;
extern	statetype s_mutchase3s;
extern	statetype s_mutchase4;

extern	statetype s_mutdie1;
extern	statetype s_mutdie2;
extern	statetype s_mutdie3;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_mutdead;
extern	statetype s_mutdead2;
#else
extern	statetype s_mutdie4;
extern	statetype s_mutdie5;
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_mutstand	= {true,SPR_MUT_W1_1,0,T_Stand,NULL,&s_mutstand};
#else
statetype s_mutstand	= {true,SPR_MUT_S_1,0,T_Stand,NULL,&s_mutstand};
#endif

statetype s_mutpath1 	= {true,SPR_MUT_W1_1,20,T_Path,NULL,&s_mutpath1s};
statetype s_mutpath1s 	= {true,SPR_MUT_W1_1,5,NULL,NULL,&s_mutpath2};
statetype s_mutpath2 	= {true,SPR_MUT_W2_1,15,T_Path,NULL,&s_mutpath3};
statetype s_mutpath3 	= {true,SPR_MUT_W3_1,20,T_Path,NULL,&s_mutpath3s};
statetype s_mutpath3s 	= {true,SPR_MUT_W3_1,5,NULL,NULL,&s_mutpath4};
statetype s_mutpath4 	= {true,SPR_MUT_W4_1,15,T_Path,NULL,&s_mutpath1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_mutpain 	= {2,SPR_MUT_PAIN,10,NULL,NULL,&s_mutchase1};
statetype s_mutpain1 	= {2,SPR_MUT_PAIN,10,NULL,NULL,&s_mutchase1};
#else
statetype s_mutpain 	= {2,SPR_MUT_PAIN_1,10,NULL,NULL,&s_mutchase1};
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
statetype s_mutpain1 	= {2,SPR_MUT_PAIN_2,10,NULL,NULL,&s_mutchase1};
#endif
#endif

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
statetype s_mutshoot1 	= {false,SPR_MUT_SHOOT1,6,NULL,NULL,&s_mutshoot2};
statetype s_mutshoot2 	= {false,SPR_MUT_SHOOT2,20,NULL,T_Shoot,&s_mutshoot3};
statetype s_mutshoot3 	= {false,SPR_MUT_SHOOT3,10,NULL,NULL,&s_mutshoot4};
statetype s_mutshoot4 	= {false,SPR_MUT_SHOOT4,20,NULL,T_Shoot,&s_mutchase1};
// *** S3DNA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
statetype s_mutshoot1 	= {false,SPR_MUT_SHOOT1,6,NULL,T_Shoot,&s_mutshoot2};
statetype s_mutshoot2 	= {false,SPR_MUT_SHOOT2,30,NULL,T_Shoot,&s_mutshoot3};
statetype s_mutshoot3 	= {false,SPR_MUT_SHOOT3,20,NULL,NULL,&s_mutchase1};
#else
statetype s_mutshoot1 	= {false,SPR_MUT_SHOOT1,6,NULL,T_Shoot,&s_mutshoot2};
statetype s_mutshoot2 	= {false,SPR_MUT_SHOOT2,20,NULL,NULL,&s_mutshoot3};
statetype s_mutshoot3 	= {false,SPR_MUT_SHOOT3,10,NULL,T_Shoot,&s_mutshoot4};
statetype s_mutshoot4 	= {false,SPR_MUT_SHOOT4,20,NULL,NULL,&s_mutchase1};
#endif

statetype s_mutchase1 	= {true,SPR_MUT_W1_1,10,T_Chase,NULL,&s_mutchase1s};
statetype s_mutchase1s 	= {true,SPR_MUT_W1_1,3,NULL,NULL,&s_mutchase2};
statetype s_mutchase2 	= {true,SPR_MUT_W2_1,8,T_Chase,NULL,&s_mutchase3};
statetype s_mutchase3 	= {true,SPR_MUT_W3_1,10,T_Chase,NULL,&s_mutchase3s};
statetype s_mutchase3s 	= {true,SPR_MUT_W3_1,3,NULL,NULL,&s_mutchase4};
statetype s_mutchase4 	= {true,SPR_MUT_W4_1,8,T_Chase,NULL,&s_mutchase1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_mutdie1		= {false,SPR_MUT_PAIN,7,NULL,A_DeathScream,&s_mutdie2};
statetype s_mutdie2		= {false,SPR_MUT_DIE_1,7,NULL,NULL,&s_mutdie3};
statetype s_mutdie3		= {false,SPR_MUT_DIE_2,7,NULL,NULL,&s_mutdead};
statetype s_mutdead		= {false,SPR_MUT_DEAD,55,NULL,NULL,&s_mutdead2};
statetype s_mutdead2		= {false,SPR_MUT_DEAD2,55,NULL,NULL,&s_mutdead};
#else
statetype s_mutdie1		= {false,SPR_MUT_DIE_1,7,NULL,A_DeathScream,&s_mutdie2};
statetype s_mutdie2		= {false,SPR_MUT_DIE_2,7,NULL,NULL,&s_mutdie3};
statetype s_mutdie3		= {false,SPR_MUT_DIE_3,7,NULL,NULL,&s_mutdie4};
statetype s_mutdie4		= {false,SPR_MUT_DIE_4,7,NULL,NULL,&s_mutdie5};
statetype s_mutdie5		= {false,SPR_MUT_DEAD,0,NULL,NULL,&s_mutdie5};
#endif

#endif // GAMEVER_WOLFREV > GV_WR_WL920312

//
// SS
//

extern	statetype s_ssstand;

extern	statetype s_sspath1;
extern	statetype s_sspath1s;
extern	statetype s_sspath2;
extern	statetype s_sspath3;
extern	statetype s_sspath3s;
extern	statetype s_sspath4;

extern	statetype s_sspain;
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
extern	statetype s_sspain1;
#endif

extern	statetype s_ssshoot1;
extern	statetype s_ssshoot2;
extern	statetype s_ssshoot3;
extern	statetype s_ssshoot4;
extern	statetype s_ssshoot5;
extern	statetype s_ssshoot6;
extern	statetype s_ssshoot7;
extern	statetype s_ssshoot8;
extern	statetype s_ssshoot9;

extern	statetype s_sschase1;
extern	statetype s_sschase1s;
extern	statetype s_sschase2;
extern	statetype s_sschase3;
extern	statetype s_sschase3s;
extern	statetype s_sschase4;

extern	statetype s_ssdie1;
extern	statetype s_ssdie2;
extern	statetype s_ssdie3;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_ssdead;
extern	statetype s_ssdead2;
#else
extern	statetype s_ssdie4;
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_ssstand	= {true,SPR_SS_W1_1,0,T_Stand,NULL,&s_ssstand};
#else
statetype s_ssstand	= {true,SPR_SS_S_1,0,T_Stand,NULL,&s_ssstand};
#endif

statetype s_sspath1 	= {true,SPR_SS_W1_1,20,T_Path,NULL,&s_sspath1s};
statetype s_sspath1s 	= {true,SPR_SS_W1_1,5,NULL,NULL,&s_sspath2};
statetype s_sspath2 	= {true,SPR_SS_W2_1,15,T_Path,NULL,&s_sspath3};
statetype s_sspath3 	= {true,SPR_SS_W3_1,20,T_Path,NULL,&s_sspath3s};
statetype s_sspath3s 	= {true,SPR_SS_W3_1,5,NULL,NULL,&s_sspath4};
statetype s_sspath4 	= {true,SPR_SS_W4_1,15,T_Path,NULL,&s_sspath1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_sspain 		= {2,SPR_SS_PAIN,10,NULL,NULL,&s_sschase1};
statetype s_sspain1 	= {2,SPR_SS_PAIN,10,NULL,NULL,&s_sschase1};
#else
statetype s_sspain 		= {2,SPR_SS_PAIN_1,10,NULL,NULL,&s_sschase1};
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
statetype s_sspain1 	= {2,SPR_SS_PAIN_2,10,NULL,NULL,&s_sschase1};
#endif
#endif

statetype s_ssshoot1 	= {false,SPR_SS_SHOOT1,20,NULL,NULL,&s_ssshoot2};
statetype s_ssshoot2 	= {false,SPR_SS_SHOOT2,20,NULL,T_Shoot,&s_ssshoot3};
statetype s_ssshoot3 	= {false,SPR_SS_SHOOT3,10,NULL,NULL,&s_ssshoot4};
statetype s_ssshoot4 	= {false,SPR_SS_SHOOT2,10,NULL,T_Shoot,&s_ssshoot5};
statetype s_ssshoot5 	= {false,SPR_SS_SHOOT3,10,NULL,NULL,&s_ssshoot6};
statetype s_ssshoot6 	= {false,SPR_SS_SHOOT2,10,NULL,T_Shoot,&s_ssshoot7};
statetype s_ssshoot7  	= {false,SPR_SS_SHOOT3,10,NULL,NULL,&s_ssshoot8};
statetype s_ssshoot8  	= {false,SPR_SS_SHOOT2,10,NULL,T_Shoot,&s_ssshoot9};
statetype s_ssshoot9  	= {false,SPR_SS_SHOOT3,10,NULL,NULL,&s_sschase1};

statetype s_sschase1 	= {true,SPR_SS_W1_1,10,T_Chase,NULL,&s_sschase1s};
statetype s_sschase1s 	= {true,SPR_SS_W1_1,3,NULL,NULL,&s_sschase2};
statetype s_sschase2 	= {true,SPR_SS_W2_1,8,T_Chase,NULL,&s_sschase3};
statetype s_sschase3 	= {true,SPR_SS_W3_1,10,T_Chase,NULL,&s_sschase3s};
statetype s_sschase3s 	= {true,SPR_SS_W3_1,3,NULL,NULL,&s_sschase4};
statetype s_sschase4 	= {true,SPR_SS_W4_1,8,T_Chase,NULL,&s_sschase1};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_ssdie1		= {false,SPR_SS_PAIN,15,NULL,A_DeathScream,&s_ssdie2};
statetype s_ssdie2		= {false,SPR_SS_DIE_1,15,NULL,NULL,&s_ssdie3};
statetype s_ssdie3		= {false,SPR_SS_DIE_2,15,NULL,NULL,&s_ssdead};
statetype s_ssdead		= {false,SPR_SS_DEAD,55,NULL,NULL,&s_ssdead2};
statetype s_ssdead2		= {false,SPR_SS_DEAD2,55,NULL,NULL,&s_ssdead};
#else
statetype s_ssdie1		= {false,SPR_SS_DIE_1,15,NULL,A_DeathScream,&s_ssdie2};
statetype s_ssdie2		= {false,SPR_SS_DIE_2,15,NULL,NULL,&s_ssdie3};
statetype s_ssdie3		= {false,SPR_SS_DIE_3,15,NULL,NULL,&s_ssdie4};
statetype s_ssdie4		= {false,SPR_SS_DEAD,0,NULL,NULL,&s_ssdie4};
#endif


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
extern	statetype s_bossstand;
statetype s_bossstand	= {false,SPR_BOSS_W1,0,NULL,NULL,&s_bossstand};
//#ifndef SPEAR
#elif (!defined SPEAR)
//
// hans
//
extern	statetype s_bossstand;

extern	statetype s_bosschase1;
extern	statetype s_bosschase1s;
extern	statetype s_bosschase2;
extern	statetype s_bosschase3;
extern	statetype s_bosschase3s;
extern	statetype s_bosschase4;

extern	statetype s_bossdie1;
extern	statetype s_bossdie2;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_bossdead;
extern	statetype s_bossdead2;
#else
extern	statetype s_bossdie3;
extern	statetype s_bossdie4;
#endif

extern	statetype s_bossshoot1;
extern	statetype s_bossshoot2;
extern	statetype s_bossshoot3;
extern	statetype s_bossshoot4;
extern	statetype s_bossshoot5;
extern	statetype s_bossshoot6;
extern	statetype s_bossshoot7;
extern	statetype s_bossshoot8;


statetype s_bossstand	= {false,SPR_BOSS_W1,0,T_Stand,NULL,&s_bossstand};

statetype s_bosschase1 	= {false,SPR_BOSS_W1,10,T_Chase,NULL,&s_bosschase1s};
statetype s_bosschase1s	= {false,SPR_BOSS_W1,3,NULL,NULL,&s_bosschase2};
statetype s_bosschase2 	= {false,SPR_BOSS_W2,8,T_Chase,NULL,&s_bosschase3};
statetype s_bosschase3 	= {false,SPR_BOSS_W3,10,T_Chase,NULL,&s_bosschase3s};
statetype s_bosschase3s	= {false,SPR_BOSS_W3,3,NULL,NULL,&s_bosschase4};
statetype s_bosschase4 	= {false,SPR_BOSS_W4,8,T_Chase,NULL,&s_bosschase1};

statetype s_bossdie1	= {false,SPR_BOSS_DIE1,15,NULL,A_DeathScream,&s_bossdie2};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_bossdie2	= {false,SPR_BOSS_DIE2,15,NULL,NULL,&s_bossdead};
statetype s_bossdead	= {false,SPR_BOSS_DEAD,55,NULL,NULL,&s_bossdead2};
statetype s_bossdead2	= {false,SPR_BOSS_DEAD2,55,NULL,NULL,&s_bossdead};
#else
statetype s_bossdie2	= {false,SPR_BOSS_DIE2,15,NULL,NULL,&s_bossdie3};
statetype s_bossdie3	= {false,SPR_BOSS_DIE3,15,NULL,NULL,&s_bossdie4};
statetype s_bossdie4	= {false,SPR_BOSS_DEAD,0,NULL,NULL,&s_bossdie4};
#endif

statetype s_bossshoot1 	= {false,SPR_BOSS_SHOOT1,30,NULL,NULL,&s_bossshoot2};
statetype s_bossshoot2 	= {false,SPR_BOSS_SHOOT2,10,NULL,T_Shoot,&s_bossshoot3};
statetype s_bossshoot3 	= {false,SPR_BOSS_SHOOT3,10,NULL,T_Shoot,&s_bossshoot4};
statetype s_bossshoot4 	= {false,SPR_BOSS_SHOOT2,10,NULL,T_Shoot,&s_bossshoot5};
statetype s_bossshoot5 	= {false,SPR_BOSS_SHOOT3,10,NULL,T_Shoot,&s_bossshoot6};
statetype s_bossshoot6 	= {false,SPR_BOSS_SHOOT2,10,NULL,T_Shoot,&s_bossshoot7};
statetype s_bossshoot7 	= {false,SPR_BOSS_SHOOT3,10,NULL,T_Shoot,&s_bossshoot8};
statetype s_bossshoot8 	= {false,SPR_BOSS_SHOOT1,10,NULL,NULL,&s_bosschase1};


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
//
// gretel
//
extern	statetype s_gretelstand;

extern	statetype s_gretelchase1;
extern	statetype s_gretelchase1s;
extern	statetype s_gretelchase2;
extern	statetype s_gretelchase3;
extern	statetype s_gretelchase3s;
extern	statetype s_gretelchase4;

extern	statetype s_greteldie1;
extern	statetype s_greteldie2;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_greteldead;
extern	statetype s_greteldead2;
#else
extern	statetype s_greteldie3;
extern	statetype s_greteldie4;
#endif

extern	statetype s_gretelshoot1;
extern	statetype s_gretelshoot2;
extern	statetype s_gretelshoot3;
extern	statetype s_gretelshoot4;
extern	statetype s_gretelshoot5;
extern	statetype s_gretelshoot6;
extern	statetype s_gretelshoot7;
extern	statetype s_gretelshoot8;


statetype s_gretelstand	= {false,SPR_GRETEL_W1,0,T_Stand,NULL,&s_gretelstand};

statetype s_gretelchase1 	= {false,SPR_GRETEL_W1,10,T_Chase,NULL,&s_gretelchase1s};
statetype s_gretelchase1s	= {false,SPR_GRETEL_W1,3,NULL,NULL,&s_gretelchase2};
statetype s_gretelchase2 	= {false,SPR_GRETEL_W2,8,T_Chase,NULL,&s_gretelchase3};
statetype s_gretelchase3 	= {false,SPR_GRETEL_W3,10,T_Chase,NULL,&s_gretelchase3s};
statetype s_gretelchase3s	= {false,SPR_GRETEL_W3,3,NULL,NULL,&s_gretelchase4};
statetype s_gretelchase4 	= {false,SPR_GRETEL_W4,8,T_Chase,NULL,&s_gretelchase1};

statetype s_greteldie1	= {false,SPR_GRETEL_DIE1,15,NULL,A_DeathScream,&s_greteldie2};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_greteldie2	= {false,SPR_GRETEL_DIE2,15,NULL,NULL,&s_greteldead};
statetype s_greteldead	= {false,SPR_GRETEL_DEAD,55,NULL,NULL,&s_greteldead2};
statetype s_greteldead2	= {false,SPR_GRETEL_DEAD2,55,NULL,NULL,&s_greteldead};
#else
statetype s_greteldie2	= {false,SPR_GRETEL_DIE2,15,NULL,NULL,&s_greteldie3};
statetype s_greteldie3	= {false,SPR_GRETEL_DIE3,15,NULL,NULL,&s_greteldie4};
statetype s_greteldie4	= {false,SPR_GRETEL_DEAD,0,NULL,NULL,&s_greteldie4};
#endif

statetype s_gretelshoot1 	= {false,SPR_GRETEL_SHOOT1,30,NULL,NULL,&s_gretelshoot2};
statetype s_gretelshoot2 	= {false,SPR_GRETEL_SHOOT2,10,NULL,T_Shoot,&s_gretelshoot3};
statetype s_gretelshoot3 	= {false,SPR_GRETEL_SHOOT3,10,NULL,T_Shoot,&s_gretelshoot4};
statetype s_gretelshoot4 	= {false,SPR_GRETEL_SHOOT2,10,NULL,T_Shoot,&s_gretelshoot5};
statetype s_gretelshoot5 	= {false,SPR_GRETEL_SHOOT3,10,NULL,T_Shoot,&s_gretelshoot6};
statetype s_gretelshoot6 	= {false,SPR_GRETEL_SHOOT2,10,NULL,T_Shoot,&s_gretelshoot7};
statetype s_gretelshoot7 	= {false,SPR_GRETEL_SHOOT3,10,NULL,T_Shoot,&s_gretelshoot8};
statetype s_gretelshoot8 	= {false,SPR_GRETEL_SHOOT1,10,NULL,NULL,&s_gretelchase1};
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
#endif


/*
===============
=
= SpawnStand
=
===============
*/

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void SpawnStand (enemy_t which, id0_int_t tilex, id0_int_t tiley, id0_int_t dir, id0_boolean_t ambush)
#else
void SpawnStand (enemy_t which, id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

	switch (which)
	{
	case en_guard:
		SpawnNewObj (tilex,tiley,&s_grdstand);
		newobj->speed = SPDPATROL;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (!loadedgame)
		  gamestate.killtotal++;
#endif
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case en_officer:
		SpawnNewObj (tilex,tiley,&s_ofcstand);
		newobj->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_mutant:
		SpawnNewObj (tilex,tiley,&s_mutstand);
		newobj->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
#endif

	case en_ss:
		SpawnNewObj (tilex,tiley,&s_ssstand);
		newobj->speed = SPDPATROL;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (!loadedgame)
		  gamestate.killtotal++;
#endif
		break;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	case en_dog:
		SpawnNewObj (tilex,tiley,&s_dogstand);
		newobj->speed = SPDDOG;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
#endif
	}


	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (ambush)
		newobj->flags |= FL_AMBUSH;
#else
	map = mapsegs[0]+farmapylookup[tiley]+tilex;
	if (*map == AMBUSHTILE)
	{
		tilemap[tilex][tiley] = 0;

		if (*(map+1) >= AREATILE)
			tile = *(map+1);
		if (*(map-mapwidth) >= AREATILE)
			tile = *(map-mapwidth);
		if (*(map+mapwidth) >= AREATILE)
			tile = *(map+mapwidth);
		if ( *(map-1) >= AREATILE)
			tile = *(map-1);

		*map = tile;
		newobj->areanumber = tile-AREATILE;

		newobj->flags |= FL_AMBUSH;
	}
#endif

	newobj->obclass = (classtype)(guardobj+which);
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
	newobj->hitpoints = starthitpoints[which];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][which];
#endif
	newobj->dir = (dirtype)(dir*2);
	newobj->flags |= FL_SHOOTABLE;
}



// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
===============
=
= SpawnDeadGuard
=
===============
*/

void SpawnDeadGuard (id0_int_t tilex, id0_int_t tiley)
{
	SpawnNewObj (tilex,tiley,&s_grddie4);
	newobj->obclass = inertobj;
}
#endif


#ifndef SPEAR
/*
===============
=
= SpawnBoss
=
===============
*/

// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnBoss (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnBoss (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

	SpawnNewObj (tilex,tiley,&s_bossstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = bossobj;
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
	newobj->hitpoints = starthitpoints[en_boss];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_boss];
#endif
	// *** ALPHA RESTORATION
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	newobj->flags |= FL_SHOOTABLE;
#else
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = south;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
}

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
===============
=
= SpawnGretel
=
===============
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnGretel (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnGretel (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

	SpawnNewObj (tilex,tiley,&s_gretelstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = gretelobj;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	newobj->hitpoints = starthitpoints[en_gretel];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_gretel];
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = north;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
===============
=
= SpawnPatrol
=
===============
*/

void SpawnPatrol (enemy_t which, id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
{
	switch (which)
	{
	case en_guard:
		SpawnNewObj (tilex,tiley,&s_grdpath1);
		newobj->speed = SPDPATROL;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (!loadedgame)
		  gamestate.killtotal++;
#endif
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case en_officer:
		SpawnNewObj (tilex,tiley,&s_ofcpath1);
		newobj->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
#endif

	case en_ss:
		SpawnNewObj (tilex,tiley,&s_sspath1);
		newobj->speed = SPDPATROL;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (!loadedgame)
		  gamestate.killtotal++;
#endif
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case en_mutant:
		SpawnNewObj (tilex,tiley,&s_mutpath1);
		newobj->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
#endif

	case en_dog:
		SpawnNewObj (tilex,tiley,&s_dogpath1);
		newobj->speed = SPDDOG;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (!loadedgame)
		  gamestate.killtotal++;
#endif
		break;
	}

	newobj->obclass = (classtype)(guardobj+which);
	newobj->dir = (dirtype)(dir*2);
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	newobj->hitpoints = starthitpoints[which];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][which];
#endif
	newobj->distance = tileglobal;
	newobj->flags |= FL_SHOOTABLE;
	newobj->active = ac_yes/*true*/;

	actorat[newobj->tilex][newobj->tiley] = 0;		// don't use original spot

	switch (dir)
	{
	case 0:
		newobj->tilex++;
		break;
	case 1:
		newobj->tiley--;
		break;
	case 2:
		newobj->tilex--;
		break;
	case 3:
		newobj->tiley++;
		break;
	}

	actorat[newobj->tilex][newobj->tiley] = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(newobj);
}
#endif // GAMEVER_NOAH3D



/*
==================
=
= A_DeathScream
=
==================
*/

void A_DeathScream (objtype *ob)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	PlaySoundLocActor(D_EDIESND,ob);
#else
//#ifndef UPLOAD
// REFKEEN: UPLOAD is a var, but DEATHSCREAM6SND isn't always defined
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
if (!UPLOAD)
{
#ifndef SPEAR
	if (mapon==9 && !US_RndT())
#else
	if ((mapon==18 || mapon==19) && !US_RndT())
#endif
	{
	 switch(ob->obclass)
	 {
	  case mutantobj:
	  case guardobj:
	  case officerobj:
	  case ssobj:
	  case dogobj:
		PlaySoundLocActor(DEATHSCREAM6SND,ob);
		return;
	 }
	}
}
#endif

	switch (ob->obclass)
	{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	case guardobj:
	case officerobj:
	case ssobj:
		switch (US_RndT()/64)
		{
		case 0:
		case 1:
			SD_PlaySound (DEATHSCREAM1SND);
			break;
		case 2:
			SD_PlaySound (DEATHSCREAM2SND);
			break;
		case 3:
			SD_PlaySound (DEATHSCREAM3SND);
			break;
		}
		break;
#else // GAMEVER_WOLFREV > GV_WR_WL920312
	case mutantobj:
		PlaySoundLocActor(AHHHGSND,ob);
		break;

	case guardobj:
		{
		 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		 soundnames/*id0_int_t*/ sounds[4]={ DEATHSCREAM1SND,
				 DEATHSCREAM1SND,
				 DEATHSCREAM2SND,
				 DEATHSCREAM3SND
				 };

		 PlaySoundLocActor(sounds[US_RndT()/64],ob);
#else
		 soundnames/*id0_int_t*/ sounds[9]={ DEATHSCREAM1SND,
				 DEATHSCREAM2SND,
				 DEATHSCREAM3SND,
				 DEATHSCREAM4SND,
				 DEATHSCREAM5SND,
				 DEATHSCREAM7SND,
				 DEATHSCREAM8SND,
				 DEATHSCREAM9SND
				 };

//		 #ifndef UPLOAD
	if (!UPLOAD)
		 PlaySoundLocActor(sounds[US_RndT()%8],ob);
//		 #else
	else
		 PlaySoundLocActor(sounds[US_RndT()%2],ob);
//		 #endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
		}
		break;
	case officerobj:
		PlaySoundLocActor(NEINSOVASSND,ob);
		break;
	case ssobj:
		PlaySoundLocActor(LEBENSND,ob);	// JAB
		break;
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
	case dogobj:
		PlaySoundLocActor(DOGDEATHSND,ob);	// JAB
		break;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef SPEAR
	// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Originally PlaySoundLocActor was used
	case bossobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(MUTTISND,ob);
#else
		SD_PlaySound(MUTTISND);				// JAB
#endif
		break;
	case schabbobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(MEINGOTTSND,ob);
#else
		SD_PlaySound(MEINGOTTSND);
#endif
		break;
	case fakeobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(HITLERHASND,ob);
#else
		SD_PlaySound(HITLERHASND);
#endif
		break;
	case mechahitlerobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(SCHEISTSND,ob);
#else
		SD_PlaySound(SCHEISTSND);
#endif
		break;
	case realhitlerobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(EVASND,ob);
#else
		SD_PlaySound(EVASND);
#endif
		break;
	// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Looks like these were added after v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	case gretelobj:
		SD_PlaySound(MEINSND);
		break;
	case giftobj:
		SD_PlaySound(DONNERSND);
		break;
	case fatobj:
		SD_PlaySound(ROSESND);
		break;
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
#else
	case spectreobj:
		SD_PlaySound(GHOSTFADESND);
		break;
	case angelobj:
		SD_PlaySound(ANGELDEATHSND);
		break;
	case transobj:
		SD_PlaySound(TRANSDEATHSND);
		break;
	case uberobj:
		SD_PlaySound(UBERDEATHSND);
		break;
	case willobj:
		SD_PlaySound(WILHELMDEATHSND);
		break;
	case deathobj:
		SD_PlaySound(KNIGHTDEATHSND);
		break;
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	}
#endif // S3DNA RESTORATION
}


/*
=============================================================================

						 SPEAR ACTORS

=============================================================================
*/

#ifdef SPEAR

void T_Launch (objtype *ob);
void T_Will (objtype *ob);

extern	statetype s_angelshoot1;
extern	statetype s_deathshoot1;
extern	statetype s_spark1;

//
// trans
//
extern	statetype s_transstand;

extern	statetype s_transchase1;
extern	statetype s_transchase1s;
extern	statetype s_transchase2;
extern	statetype s_transchase3;
extern	statetype s_transchase3s;
extern	statetype s_transchase4;

extern	statetype s_transdie0;
extern	statetype s_transdie01;
extern	statetype s_transdie1;
extern	statetype s_transdie2;
extern	statetype s_transdie3;
extern	statetype s_transdie4;

extern	statetype s_transshoot1;
extern	statetype s_transshoot2;
extern	statetype s_transshoot3;
extern	statetype s_transshoot4;
extern	statetype s_transshoot5;
extern	statetype s_transshoot6;
extern	statetype s_transshoot7;
extern	statetype s_transshoot8;


statetype s_transstand	= {false,SPR_TRANS_W1,0,T_Stand,NULL,&s_transstand};

statetype s_transchase1 	= {false,SPR_TRANS_W1,10,T_Chase,NULL,&s_transchase1s};
statetype s_transchase1s	= {false,SPR_TRANS_W1,3,NULL,NULL,&s_transchase2};
statetype s_transchase2 	= {false,SPR_TRANS_W2,8,T_Chase,NULL,&s_transchase3};
statetype s_transchase3 	= {false,SPR_TRANS_W3,10,T_Chase,NULL,&s_transchase3s};
statetype s_transchase3s	= {false,SPR_TRANS_W3,3,NULL,NULL,&s_transchase4};
statetype s_transchase4 	= {false,SPR_TRANS_W4,8,T_Chase,NULL,&s_transchase1};

statetype s_transdie0	= {false,SPR_TRANS_W1,1,NULL,A_DeathScream,&s_transdie01};
statetype s_transdie01	= {false,SPR_TRANS_W1,1,NULL,NULL,&s_transdie1};
statetype s_transdie1	= {false,SPR_TRANS_DIE1,15,NULL,NULL,&s_transdie2};
statetype s_transdie2	= {false,SPR_TRANS_DIE2,15,NULL,NULL,&s_transdie3};
statetype s_transdie3	= {false,SPR_TRANS_DIE3,15,NULL,NULL,&s_transdie4};
statetype s_transdie4	= {false,SPR_TRANS_DEAD,0,NULL,NULL,&s_transdie4};

statetype s_transshoot1 	= {false,SPR_TRANS_SHOOT1,30,NULL,NULL,&s_transshoot2};
statetype s_transshoot2 	= {false,SPR_TRANS_SHOOT2,10,NULL,T_Shoot,&s_transshoot3};
statetype s_transshoot3 	= {false,SPR_TRANS_SHOOT3,10,NULL,T_Shoot,&s_transshoot4};
statetype s_transshoot4 	= {false,SPR_TRANS_SHOOT2,10,NULL,T_Shoot,&s_transshoot5};
statetype s_transshoot5 	= {false,SPR_TRANS_SHOOT3,10,NULL,T_Shoot,&s_transshoot6};
statetype s_transshoot6 	= {false,SPR_TRANS_SHOOT2,10,NULL,T_Shoot,&s_transshoot7};
statetype s_transshoot7 	= {false,SPR_TRANS_SHOOT3,10,NULL,T_Shoot,&s_transshoot8};
statetype s_transshoot8 	= {false,SPR_TRANS_SHOOT1,10,NULL,NULL,&s_transchase1};


/*
===============
=
= SpawnTrans
=
===============
*/

void SpawnTrans (id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;

	if (SoundBlasterPresent && DigiMode != sds_Off)
		s_transdie01.tictime = 105;

	SpawnNewObj (tilex,tiley,&s_transstand);
	newobj->obclass = transobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_trans];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


//
// uber
//
void T_UShoot (objtype *ob);

extern	statetype s_uberstand;

extern	statetype s_uberchase1;
extern	statetype s_uberchase1s;
extern	statetype s_uberchase2;
extern	statetype s_uberchase3;
extern	statetype s_uberchase3s;
extern	statetype s_uberchase4;

extern	statetype s_uberdie0;
extern	statetype s_uberdie01;
extern	statetype s_uberdie1;
extern	statetype s_uberdie2;
extern	statetype s_uberdie3;
extern	statetype s_uberdie4;
extern	statetype s_uberdie5;

extern	statetype s_ubershoot1;
extern	statetype s_ubershoot2;
extern	statetype s_ubershoot3;
extern	statetype s_ubershoot4;
extern	statetype s_ubershoot5;
extern	statetype s_ubershoot6;
extern	statetype s_ubershoot7;


statetype s_uberstand	= {false,SPR_UBER_W1,0,T_Stand,NULL,&s_uberstand};

statetype s_uberchase1 	= {false,SPR_UBER_W1,10,T_Chase,NULL,&s_uberchase1s};
statetype s_uberchase1s	= {false,SPR_UBER_W1,3,NULL,NULL,&s_uberchase2};
statetype s_uberchase2 	= {false,SPR_UBER_W2,8,T_Chase,NULL,&s_uberchase3};
statetype s_uberchase3 	= {false,SPR_UBER_W3,10,T_Chase,NULL,&s_uberchase3s};
statetype s_uberchase3s	= {false,SPR_UBER_W3,3,NULL,NULL,&s_uberchase4};
statetype s_uberchase4 	= {false,SPR_UBER_W4,8,T_Chase,NULL,&s_uberchase1};

statetype s_uberdie0	= {false,SPR_UBER_W1,1,NULL,A_DeathScream,&s_uberdie01};
statetype s_uberdie01	= {false,SPR_UBER_W1,1,NULL,NULL,&s_uberdie1};
statetype s_uberdie1	= {false,SPR_UBER_DIE1,15,NULL,NULL,&s_uberdie2};
statetype s_uberdie2	= {false,SPR_UBER_DIE2,15,NULL,NULL,&s_uberdie3};
statetype s_uberdie3	= {false,SPR_UBER_DIE3,15,NULL,NULL,&s_uberdie4};
statetype s_uberdie4	= {false,SPR_UBER_DIE4,15,NULL,NULL,&s_uberdie5};
statetype s_uberdie5	= {false,SPR_UBER_DEAD,0,NULL,NULL,&s_uberdie5};

statetype s_ubershoot1 	= {false,SPR_UBER_SHOOT1,30,NULL,NULL,&s_ubershoot2};
statetype s_ubershoot2 	= {false,SPR_UBER_SHOOT2,12,NULL,T_UShoot,&s_ubershoot3};
statetype s_ubershoot3 	= {false,SPR_UBER_SHOOT3,12,NULL,T_UShoot,&s_ubershoot4};
statetype s_ubershoot4 	= {false,SPR_UBER_SHOOT4,12,NULL,T_UShoot,&s_ubershoot5};
statetype s_ubershoot5 	= {false,SPR_UBER_SHOOT3,12,NULL,T_UShoot,&s_ubershoot6};
statetype s_ubershoot6 	= {false,SPR_UBER_SHOOT2,12,NULL,T_UShoot,&s_ubershoot7};
statetype s_ubershoot7 	= {false,SPR_UBER_SHOOT1,12,NULL,NULL,&s_uberchase1};


/*
===============
=
= SpawnUber
=
===============
*/

void SpawnUber (id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;

	if (SoundBlasterPresent && DigiMode != sds_Off)
		s_uberdie01.tictime = 70;

	SpawnNewObj (tilex,tiley,&s_uberstand);
	newobj->obclass = uberobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_uber];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= T_UShoot
=
===============
*/

void T_UShoot (objtype *ob)
{
	id0_int_t	dx,dy,dist;

	T_Shoot (ob);

	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;
	if (dist <= 1)
		TakeDamage (10,ob);
}


//
// will
//
extern	statetype s_willstand;

extern	statetype s_willchase1;
extern	statetype s_willchase1s;
extern	statetype s_willchase2;
extern	statetype s_willchase3;
extern	statetype s_willchase3s;
extern	statetype s_willchase4;

extern	statetype s_willdie1;
extern	statetype s_willdie2;
extern	statetype s_willdie3;
extern	statetype s_willdie4;
extern	statetype s_willdie5;
extern	statetype s_willdie6;

extern	statetype s_willshoot1;
extern	statetype s_willshoot2;
extern	statetype s_willshoot3;
extern	statetype s_willshoot4;
extern	statetype s_willshoot5;
extern	statetype s_willshoot6;


statetype s_willstand	= {false,SPR_WILL_W1,0,T_Stand,NULL,&s_willstand};

statetype s_willchase1 	= {false,SPR_WILL_W1,10,T_Will,NULL,&s_willchase1s};
statetype s_willchase1s	= {false,SPR_WILL_W1,3,NULL,NULL,&s_willchase2};
statetype s_willchase2 	= {false,SPR_WILL_W2,8,T_Will,NULL,&s_willchase3};
statetype s_willchase3 	= {false,SPR_WILL_W3,10,T_Will,NULL,&s_willchase3s};
statetype s_willchase3s	= {false,SPR_WILL_W3,3,NULL,NULL,&s_willchase4};
statetype s_willchase4 	= {false,SPR_WILL_W4,8,T_Will,NULL,&s_willchase1};

statetype s_willdeathcam	= {false,SPR_WILL_W1,1,NULL,NULL,&s_willdie1};

statetype s_willdie1	= {false,SPR_WILL_W1,1,NULL,A_DeathScream,&s_willdie2};
statetype s_willdie2	= {false,SPR_WILL_W1,10,NULL,NULL,&s_willdie3};
statetype s_willdie3	= {false,SPR_WILL_DIE1,10,NULL,NULL,&s_willdie4};
statetype s_willdie4	= {false,SPR_WILL_DIE2,10,NULL,NULL,&s_willdie5};
statetype s_willdie5	= {false,SPR_WILL_DIE3,10,NULL,NULL,&s_willdie6};
statetype s_willdie6	= {false,SPR_WILL_DEAD,20,NULL,NULL,&s_willdie6};

statetype s_willshoot1 	= {false,SPR_WILL_SHOOT1,30,NULL,NULL,&s_willshoot2};
statetype s_willshoot2 	= {false,SPR_WILL_SHOOT2,10,NULL,T_Launch,&s_willshoot3};
statetype s_willshoot3 	= {false,SPR_WILL_SHOOT3,10,NULL,T_Shoot,&s_willshoot4};
statetype s_willshoot4 	= {false,SPR_WILL_SHOOT4,10,NULL,T_Shoot,&s_willshoot5};
statetype s_willshoot5 	= {false,SPR_WILL_SHOOT3,10,NULL,T_Shoot,&s_willshoot6};
statetype s_willshoot6 	= {false,SPR_WILL_SHOOT4,10,NULL,T_Shoot,&s_willchase1};


/*
===============
=
= SpawnWill
=
===============
*/

void SpawnWill (id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;

	if (SoundBlasterPresent && DigiMode != sds_Off)
		s_willdie2.tictime = 70;

	SpawnNewObj (tilex,tiley,&s_willstand);
	newobj->obclass = willobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_will];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


/*
================
=
= T_Will
=
================
*/

void T_Will (objtype *ob)
{
	id0_long_t move;
	id0_int_t	dx,dy,dist;
	id0_boolean_t	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{
		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			if (ob->obclass == willobj)
				NewState (ob,&s_willshoot1);
			else if (ob->obclass == angelobj)
				NewState (ob,&s_angelshoot1);
			else
				NewState (ob,&s_deathshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}


//
// death
//
extern	statetype s_deathstand;

extern	statetype s_deathchase1;
extern	statetype s_deathchase1s;
extern	statetype s_deathchase2;
extern	statetype s_deathchase3;
extern	statetype s_deathchase3s;
extern	statetype s_deathchase4;

extern	statetype s_deathdie1;
extern	statetype s_deathdie2;
extern	statetype s_deathdie3;
extern	statetype s_deathdie4;
extern	statetype s_deathdie5;
extern	statetype s_deathdie6;
extern	statetype s_deathdie7;
extern	statetype s_deathdie8;
extern	statetype s_deathdie9;

extern	statetype s_deathshoot1;
extern	statetype s_deathshoot2;
extern	statetype s_deathshoot3;
extern	statetype s_deathshoot4;
extern	statetype s_deathshoot5;


statetype s_deathstand	= {false,SPR_DEATH_W1,0,T_Stand,NULL,&s_deathstand};

statetype s_deathchase1 	= {false,SPR_DEATH_W1,10,T_Will,NULL,&s_deathchase1s};
statetype s_deathchase1s	= {false,SPR_DEATH_W1,3,NULL,NULL,&s_deathchase2};
statetype s_deathchase2 	= {false,SPR_DEATH_W2,8,T_Will,NULL,&s_deathchase3};
statetype s_deathchase3 	= {false,SPR_DEATH_W3,10,T_Will,NULL,&s_deathchase3s};
statetype s_deathchase3s	= {false,SPR_DEATH_W3,3,NULL,NULL,&s_deathchase4};
statetype s_deathchase4 	= {false,SPR_DEATH_W4,8,T_Will,NULL,&s_deathchase1};

statetype s_deathdeathcam	= {false,SPR_DEATH_W1,1,NULL,NULL,&s_deathdie1};

statetype s_deathdie1	= {false,SPR_DEATH_W1,1,NULL,A_DeathScream,&s_deathdie2};
statetype s_deathdie2	= {false,SPR_DEATH_W1,10,NULL,NULL,&s_deathdie3};
statetype s_deathdie3	= {false,SPR_DEATH_DIE1,10,NULL,NULL,&s_deathdie4};
statetype s_deathdie4	= {false,SPR_DEATH_DIE2,10,NULL,NULL,&s_deathdie5};
statetype s_deathdie5	= {false,SPR_DEATH_DIE3,10,NULL,NULL,&s_deathdie6};
statetype s_deathdie6	= {false,SPR_DEATH_DIE4,10,NULL,NULL,&s_deathdie7};
statetype s_deathdie7	= {false,SPR_DEATH_DIE5,10,NULL,NULL,&s_deathdie8};
statetype s_deathdie8	= {false,SPR_DEATH_DIE6,10,NULL,NULL,&s_deathdie9};
statetype s_deathdie9	= {false,SPR_DEATH_DEAD,0,NULL,NULL,&s_deathdie9};

statetype s_deathshoot1 	= {false,SPR_DEATH_SHOOT1,30,NULL,NULL,&s_deathshoot2};
statetype s_deathshoot2 	= {false,SPR_DEATH_SHOOT2,10,NULL,T_Launch,&s_deathshoot3};
statetype s_deathshoot3 	= {false,SPR_DEATH_SHOOT4,10,NULL,T_Shoot,&s_deathshoot4};
statetype s_deathshoot4 	= {false,SPR_DEATH_SHOOT3,10,NULL,T_Launch,&s_deathshoot5};
statetype s_deathshoot5 	= {false,SPR_DEATH_SHOOT4,10,NULL,T_Shoot,&s_deathchase1};


/*
===============
=
= SpawnDeath
=
===============
*/

void SpawnDeath (id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;

	if (SoundBlasterPresent && DigiMode != sds_Off)
		s_deathdie2.tictime = 105;

	SpawnNewObj (tilex,tiley,&s_deathstand);
	newobj->obclass = deathobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_death];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}

/*
===============
=
= T_Launch
=
===============
*/

void T_Launch (objtype *ob)
{
	id0_long_t	deltax,deltay;
	float	angle;
	id0_int_t		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = ID0_M_PI*2+angle;
	iangle = angle/(ID0_M_PI*2)*ANGLES;
	if (ob->obclass == deathobj)
	{
		T_Shoot (ob);
		if (ob->state == &s_deathshoot2)
		{
			iangle-=4;
			if (iangle<0)
				iangle+=ANGLES;
		}
		else
		{
			iangle+=4;
			if (iangle>=ANGLES)
				iangle-=ANGLES;
		}
	}

	GetNewActor ();
	newobj->state = &s_rocket;
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = rocketobj;
	switch(ob->obclass)
	{
	case deathobj:
		newobj->state = &s_hrocket;
		newobj->obclass = hrocketobj;
		PlaySoundLocActor (KNIGHTMISSILESND,newobj);
		break;
	case angelobj:
		newobj->state = &s_spark1;
		newobj->obclass = sparkobj;
		PlaySoundLocActor (ANGELFIRESND,newobj);
		break;
	default:
		PlaySoundLocActor (MISSILEFIRESND,newobj);
	}

	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->speed = 0x2000l;
	newobj->flags = FL_NONMARK;
	newobj->active = ac_yes/*true*/;
}



//
// angel
//
void A_Relaunch (objtype *ob);
void A_Victory (objtype *ob);
void A_StartAttack (objtype *ob);
void A_Breathing (objtype *ob);

extern	statetype s_angelstand;

extern	statetype s_angelchase1;
extern	statetype s_angelchase1s;
extern	statetype s_angelchase2;
extern	statetype s_angelchase3;
extern	statetype s_angelchase3s;
extern	statetype s_angelchase4;

extern	statetype s_angeldie1;
extern	statetype s_angeldie11;
extern	statetype s_angeldie2;
extern	statetype s_angeldie3;
extern	statetype s_angeldie4;
extern	statetype s_angeldie5;
extern	statetype s_angeldie6;
extern	statetype s_angeldie7;
extern	statetype s_angeldie8;
extern	statetype s_angeldie9;

extern	statetype s_angelshoot1;
extern	statetype s_angelshoot2;
extern	statetype s_angelshoot3;
extern	statetype s_angelshoot4;
extern	statetype s_angelshoot5;
extern	statetype s_angelshoot6;

extern	statetype s_angeltired;
extern	statetype s_angeltired2;
extern	statetype s_angeltired3;
extern	statetype s_angeltired4;
extern	statetype s_angeltired5;
extern	statetype s_angeltired6;
extern	statetype s_angeltired7;

extern	statetype s_spark1;
extern	statetype s_spark2;
extern	statetype s_spark3;
extern	statetype s_spark4;


statetype s_angelstand	= {false,SPR_ANGEL_W1,0,T_Stand,NULL,&s_angelstand};

statetype s_angelchase1 	= {false,SPR_ANGEL_W1,10,T_Will,NULL,&s_angelchase1s};
statetype s_angelchase1s	= {false,SPR_ANGEL_W1,3,NULL,NULL,&s_angelchase2};
statetype s_angelchase2 	= {false,SPR_ANGEL_W2,8,T_Will,NULL,&s_angelchase3};
statetype s_angelchase3 	= {false,SPR_ANGEL_W3,10,T_Will,NULL,&s_angelchase3s};
statetype s_angelchase3s	= {false,SPR_ANGEL_W3,3,NULL,NULL,&s_angelchase4};
statetype s_angelchase4 	= {false,SPR_ANGEL_W4,8,T_Will,NULL,&s_angelchase1};

statetype s_angeldie1	= {false,SPR_ANGEL_W1,1,NULL,A_DeathScream,&s_angeldie11};
statetype s_angeldie11	= {false,SPR_ANGEL_W1,1,NULL,NULL,&s_angeldie2};
statetype s_angeldie2	= {false,SPR_ANGEL_DIE1,10,NULL,A_Slurpie,&s_angeldie3};
statetype s_angeldie3	= {false,SPR_ANGEL_DIE2,10,NULL,NULL,&s_angeldie4};
statetype s_angeldie4	= {false,SPR_ANGEL_DIE3,10,NULL,NULL,&s_angeldie5};
statetype s_angeldie5	= {false,SPR_ANGEL_DIE4,10,NULL,NULL,&s_angeldie6};
statetype s_angeldie6	= {false,SPR_ANGEL_DIE5,10,NULL,NULL,&s_angeldie7};
statetype s_angeldie7	= {false,SPR_ANGEL_DIE6,10,NULL,NULL,&s_angeldie8};
statetype s_angeldie8	= {false,SPR_ANGEL_DIE7,10,NULL,NULL,&s_angeldie9};
statetype s_angeldie9	= {false,SPR_ANGEL_DEAD,130,NULL,A_Victory,&s_angeldie9};

statetype s_angelshoot1 	= {false,SPR_ANGEL_SHOOT1,10,NULL,A_StartAttack,&s_angelshoot2};
statetype s_angelshoot2 	= {false,SPR_ANGEL_SHOOT2,20,NULL,T_Launch,&s_angelshoot3};
statetype s_angelshoot3 	= {false,SPR_ANGEL_SHOOT1,10,NULL,A_Relaunch,&s_angelshoot2};

statetype s_angeltired 	= {false,SPR_ANGEL_TIRED1,40,NULL,A_Breathing,&s_angeltired2};
statetype s_angeltired2	= {false,SPR_ANGEL_TIRED2,40,NULL,NULL,&s_angeltired3};
statetype s_angeltired3	= {false,SPR_ANGEL_TIRED1,40,NULL,A_Breathing,&s_angeltired4};
statetype s_angeltired4	= {false,SPR_ANGEL_TIRED2,40,NULL,NULL,&s_angeltired5};
statetype s_angeltired5	= {false,SPR_ANGEL_TIRED1,40,NULL,A_Breathing,&s_angeltired6};
statetype s_angeltired6	= {false,SPR_ANGEL_TIRED2,40,NULL,NULL,&s_angeltired7};
statetype s_angeltired7	= {false,SPR_ANGEL_TIRED1,40,NULL,A_Breathing,&s_angelchase1};

statetype s_spark1 	= {false,SPR_SPARK1,6,T_Projectile,NULL,&s_spark2};
statetype s_spark2 	= {false,SPR_SPARK2,6,T_Projectile,NULL,&s_spark3};
statetype s_spark3 	= {false,SPR_SPARK3,6,T_Projectile,NULL,&s_spark4};
statetype s_spark4 	= {false,SPR_SPARK4,6,T_Projectile,NULL,&s_spark1};


//#pragma argsused
void A_Slurpie (objtype *ob)
{
 SD_PlaySound(SLURPIESND);
}

//#pragma argsused
void A_Breathing (objtype *ob)
{
 SD_PlaySound(ANGELTIREDSND);
}

/*
===============
=
= SpawnAngel
=
===============
*/

void SpawnAngel (id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;


	if (SoundBlasterPresent && DigiMode != sds_Off)
		s_angeldie11.tictime = 105;

	SpawnNewObj (tilex,tiley,&s_angelstand);
	newobj->obclass = angelobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_angel];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


/*
=================
=
= A_Victory
=
=================
*/

//#pragma argsused
void A_Victory (objtype *ob)
{
	playstate = ex_victorious;
}


/*
=================
=
= A_StartAttack
=
=================
*/

void A_StartAttack (objtype *ob)
{
	ob->temp1 = 0;
}


/*
=================
=
= A_Relaunch
=
=================
*/

void A_Relaunch (objtype *ob)
{
	if (++ob->temp1 == 3)
	{
		NewState (ob,&s_angeltired);
		return;
	}

	if (US_RndT()&1)
	{
		NewState (ob,&s_angelchase1);
		return;
	}
}




//
// spectre
//
void T_SpectreWait (objtype *ob);
void A_Dormant (objtype *ob);

extern	statetype s_spectrewait1;
extern	statetype s_spectrewait2;
extern	statetype s_spectrewait3;
extern	statetype s_spectrewait4;

extern	statetype s_spectrechase1;
extern	statetype s_spectrechase2;
extern	statetype s_spectrechase3;
extern	statetype s_spectrechase4;

extern	statetype s_spectredie1;
extern	statetype s_spectredie2;
extern	statetype s_spectredie3;
extern	statetype s_spectredie4;

extern	statetype s_spectrewake;

statetype s_spectrewait1	= {false,SPR_SPECTRE_W1,10,T_Stand,NULL,&s_spectrewait2};
statetype s_spectrewait2	= {false,SPR_SPECTRE_W2,10,T_Stand,NULL,&s_spectrewait3};
statetype s_spectrewait3	= {false,SPR_SPECTRE_W3,10,T_Stand,NULL,&s_spectrewait4};
statetype s_spectrewait4	= {false,SPR_SPECTRE_W4,10,T_Stand,NULL,&s_spectrewait1};

statetype s_spectrechase1	= {false,SPR_SPECTRE_W1,10,T_Ghosts,NULL,&s_spectrechase2};
statetype s_spectrechase2	= {false,SPR_SPECTRE_W2,10,T_Ghosts,NULL,&s_spectrechase3};
statetype s_spectrechase3	= {false,SPR_SPECTRE_W3,10,T_Ghosts,NULL,&s_spectrechase4};
statetype s_spectrechase4	= {false,SPR_SPECTRE_W4,10,T_Ghosts,NULL,&s_spectrechase1};

statetype s_spectredie1	= {false,SPR_SPECTRE_F1,10,NULL,NULL,&s_spectredie2};
statetype s_spectredie2	= {false,SPR_SPECTRE_F2,10,NULL,NULL,&s_spectredie3};
statetype s_spectredie3	= {false,SPR_SPECTRE_F3,10,NULL,NULL,&s_spectredie4};
statetype s_spectredie4	= {false,SPR_SPECTRE_F4,300,NULL,NULL,&s_spectrewake};
statetype s_spectrewake	= {false,SPR_SPECTRE_F4,10,NULL,A_Dormant,&s_spectrewake};

/*
===============
=
= SpawnSpectre
=
===============
*/

void SpawnSpectre (id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;

	SpawnNewObj (tilex,tiley,&s_spectrewait1);
	newobj->obclass = spectreobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_spectre];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH; // |FL_NEVERMARK|FL_NONMARK;
	if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= A_Dormant
=
===============
*/

void A_Dormant (objtype *ob)
{
	id0_long_t	deltax,deltay;
	id0_int_t	xl,xh,yl,yh;
	id0_int_t	x,y;
	id0_unsigned_t	tile;

	deltax = ob->x - player->x;
	if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
		goto moveok;
	deltay = ob->y - player->y;
	if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
		goto moveok;

	return;
moveok:

	xl = (ob->x-MINDIST) >> TILESHIFT;
	xh = (ob->x+MINDIST) >> TILESHIFT;
	yl = (ob->y-MINDIST) >> TILESHIFT;
	yh = (ob->y+MINDIST) >> TILESHIFT;

	for (y=yl ; y<=yh ; y++)
		for (x=xl ; x<=xh ; x++)
		{
			tile = actorat[x][y];
			if (!tile)
				continue;
			if (tile<256)
				return;
			if ((COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(tile))->flags&FL_SHOOTABLE)
				return;
		}

	ob->flags |= FL_AMBUSH | FL_SHOOTABLE;
	ob->flags &= ~FL_ATTACKMODE;
	ob->dir = nodir;
	NewState (ob,&s_spectrewait1);
}


#endif

/*
=============================================================================

						 SCHABBS / GIFT / FAT

=============================================================================
*/

// *** ALPHA RESTORATION ***
#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
===============
=
= SpawnGhosts
=
===============
*/

void SpawnGhosts (id0_int_t which, id0_int_t tilex, id0_int_t tiley)
{
	id0_unsigned_t	id0_far *map,tile;

	switch(which)
	{
	 case en_blinky:
	   SpawnNewObj (tilex,tiley,&s_blinkychase1);
	   break;
	 case en_clyde:
	   SpawnNewObj (tilex,tiley,&s_clydechase1);
	   break;
	 case en_pinky:
	   SpawnNewObj (tilex,tiley,&s_pinkychase1);
	   break;
	 case en_inky:
	   SpawnNewObj (tilex,tiley,&s_inkychase1);
	   break;
	}

	newobj->obclass = ghostobj;
	newobj->speed = SPDDOG;

	newobj->dir = east;
	newobj->flags |= FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
#endif



// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
void	T_Gift (objtype *ob);
void	T_GiftThrow (objtype *ob);

void	T_Fat (objtype *ob);
void	T_FatThrow (objtype *ob);
#endif

//
// schabb
//
extern	statetype s_schabbstand;

extern	statetype s_schabbchase1;
extern	statetype s_schabbchase1s;
extern	statetype s_schabbchase2;
extern	statetype s_schabbchase3;
extern	statetype s_schabbchase3s;
extern	statetype s_schabbchase4;

extern	statetype s_schabbdie1;
extern	statetype s_schabbdie2;
extern	statetype s_schabbdie3;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_schabbdead;
extern	statetype s_schabbdead2;
#else
extern	statetype s_schabbdie4;
extern	statetype s_schabbdie5;
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
extern	statetype s_schabbdie6;
#endif
#endif

extern	statetype s_schabbshoot1;
extern	statetype s_schabbshoot2;

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_needle1;
extern	statetype s_needle2;
extern	statetype s_needle3;
extern	statetype s_needle4;
#endif

// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (!defined GAMEVER_NOAH3D)
extern	statetype s_schabbdeathcam;
#endif


statetype s_schabbstand	= {false,SPR_SCHABB_W1,0,T_Stand,NULL,&s_schabbstand};

statetype s_schabbchase1 	= {false,SPR_SCHABB_W1,10,T_Schabb,NULL,&s_schabbchase1s};
statetype s_schabbchase1s	= {false,SPR_SCHABB_W1,3,NULL,NULL,&s_schabbchase2};
statetype s_schabbchase2 	= {false,SPR_SCHABB_W2,8,T_Schabb,NULL,&s_schabbchase3};
statetype s_schabbchase3 	= {false,SPR_SCHABB_W3,10,T_Schabb,NULL,&s_schabbchase3s};
statetype s_schabbchase3s	= {false,SPR_SCHABB_W3,3,NULL,NULL,&s_schabbchase4};
statetype s_schabbchase4 	= {false,SPR_SCHABB_W4,8,T_Schabb,NULL,&s_schabbchase1};

// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (!defined GAMEVER_NOAH3D)
statetype s_schabbdeathcam	= {false,SPR_SCHABB_W1,1,NULL,NULL,&s_schabbdie1};
#endif

// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_schabbdie1	= {false,SPR_SCHABB_W1,10,NULL,A_DeathScream,&s_schabbdie2};
statetype s_schabbdie2	= {false,SPR_SCHABB_DIE1,10,NULL,NULL,&s_schabbdie3};
statetype s_schabbdie3	= {false,SPR_SCHABB_DIE2,10,NULL,NULL,&s_schabbdead};
statetype s_schabbdead	= {false,SPR_SCHABB_DEAD,55,NULL,NULL,&s_schabbdead2};
statetype s_schabbdead2	= {false,SPR_SCHABB_DEAD2,55,NULL,NULL,&s_schabbdead};
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
statetype s_schabbdie1	= {false,SPR_SCHABB_W1,10,NULL,NULL,&s_schabbdie2};
statetype s_schabbdie2	= {false,SPR_SCHABB_DIE1,10,NULL,NULL,&s_schabbdie3};
statetype s_schabbdie3	= {false,SPR_SCHABB_DIE2,10,NULL,NULL,&s_schabbdie4};
statetype s_schabbdie4	= {false,SPR_SCHABB_DIE3,10,NULL,NULL,&s_schabbdie5};
statetype s_schabbdie5	= {false,SPR_SCHABB_DEAD,0,NULL,NULL,&s_schabbdie5};
#else
statetype s_schabbdie1	= {false,SPR_SCHABB_W1,10,NULL,A_DeathScream,&s_schabbdie2};
statetype s_schabbdie2	= {false,SPR_SCHABB_W1,10,NULL,NULL,&s_schabbdie3};
statetype s_schabbdie3	= {false,SPR_SCHABB_DIE1,10,NULL,NULL,&s_schabbdie4};
statetype s_schabbdie4	= {false,SPR_SCHABB_DIE2,10,NULL,NULL,&s_schabbdie5};
statetype s_schabbdie5	= {false,SPR_SCHABB_DIE3,10,NULL,NULL,&s_schabbdie6};
statetype s_schabbdie6	= {false,SPR_SCHABB_DEAD,20,NULL,A_StartDeathCam,&s_schabbdie6};
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_schabbshoot1 	= {false,SPR_SCHABB_SHOOT1,20,NULL,NULL,&s_schabbshoot2};
statetype s_schabbshoot2 	= {false,SPR_SCHABB_SHOOT2,20,NULL,T_SchabbThrow,&s_schabbchase1};
#else
statetype s_schabbshoot1 	= {false,SPR_SCHABB_SHOOT1,30,NULL,NULL,&s_schabbshoot2};
statetype s_schabbshoot2 	= {false,SPR_SCHABB_SHOOT2,10,NULL,T_SchabbThrow,&s_schabbchase1};

statetype s_needle1 	= {false,SPR_HYPO1,6,T_Projectile,NULL,&s_needle2};
statetype s_needle2 	= {false,SPR_HYPO2,6,T_Projectile,NULL,&s_needle3};
statetype s_needle3 	= {false,SPR_HYPO3,6,T_Projectile,NULL,&s_needle4};
statetype s_needle4 	= {false,SPR_HYPO4,6,T_Projectile,NULL,&s_needle1};
#endif


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
//
// gift
//
extern	statetype s_giftstand;

extern	statetype s_giftchase1;
extern	statetype s_giftchase1s;
extern	statetype s_giftchase2;
extern	statetype s_giftchase3;
extern	statetype s_giftchase3s;
extern	statetype s_giftchase4;

extern	statetype s_giftdie1;
extern	statetype s_giftdie2;
extern	statetype s_giftdie3;
extern	statetype s_giftdie4;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_giftdead;
extern	statetype s_giftdead2;
#else
extern	statetype s_giftdie5;
extern	statetype s_giftdie6;
#endif

extern	statetype s_giftshoot1;
extern	statetype s_giftshoot2;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_giftshoot3;
extern	statetype s_giftshoot4;
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_needle1;
extern	statetype s_needle2;
extern	statetype s_needle3;
extern	statetype s_needle4;
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_giftdeathcam;
#endif

// *** SHAREWARE V1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
extern	statetype s_boom1;
extern	statetype s_boom2;
extern	statetype s_boom3;
#endif


statetype s_giftstand	= {false,SPR_GIFT_W1,0,T_Stand,NULL,&s_giftstand};

statetype s_giftchase1 	= {false,SPR_GIFT_W1,10,T_Gift,NULL,&s_giftchase1s};
statetype s_giftchase1s	= {false,SPR_GIFT_W1,3,NULL,NULL,&s_giftchase2};
statetype s_giftchase2 	= {false,SPR_GIFT_W2,8,T_Gift,NULL,&s_giftchase3};
statetype s_giftchase3 	= {false,SPR_GIFT_W3,10,T_Gift,NULL,&s_giftchase3s};
statetype s_giftchase3s	= {false,SPR_GIFT_W3,3,NULL,NULL,&s_giftchase4};
statetype s_giftchase4 	= {false,SPR_GIFT_W4,8,T_Gift,NULL,&s_giftchase1};

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
statetype s_giftdeathcam	= {false,SPR_GIFT_W1,1,NULL,NULL,&s_giftdie1};
#endif

statetype s_giftdie1	= {false,SPR_GIFT_W1,1,NULL,A_DeathScream,&s_giftdie2};
statetype s_giftdie2	= {false,SPR_GIFT_W1,10,NULL,NULL,&s_giftdie3};
statetype s_giftdie3	= {false,SPR_GIFT_DIE1,10,NULL,NULL,&s_giftdie4};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_giftdie4	= {false,SPR_GIFT_DIE2,10,NULL,NULL,&s_giftdead};
statetype s_giftdead	= {false,SPR_GIFT_DEAD,55,NULL,NULL,&s_giftdead2};
statetype s_giftdead2	= {false,SPR_GIFT_DEAD2,55,NULL,NULL,&s_giftdead};
#else
statetype s_giftdie4	= {false,SPR_GIFT_DIE2,10,NULL,NULL,&s_giftdie5};
statetype s_giftdie5	= {false,SPR_GIFT_DIE3,10,NULL,NULL,&s_giftdie6};
statetype s_giftdie6	= {false,SPR_GIFT_DEAD,20,NULL,A_StartDeathCam,&s_giftdie6};
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_giftshoot1 	= {false,SPR_GIFT_SHOOT1,10,NULL,NULL,&s_giftshoot2};
statetype s_giftshoot2 	= {false,SPR_GIFT_SHOOT2,10,NULL,NULL,&s_giftshoot3};
statetype s_giftshoot3 	= {false,SPR_GIFT_SHOOT3,10,NULL,NULL,&s_giftshoot4};
statetype s_giftshoot4 	= {false,SPR_GIFT_SHOOT4,10,NULL,T_GiftThrow,&s_giftchase1};
#else
statetype s_giftshoot1 	= {false,SPR_GIFT_SHOOT1,30,NULL,NULL,&s_giftshoot2};
statetype s_giftshoot2 	= {false,SPR_GIFT_SHOOT2,10,NULL,T_GiftThrow,&s_giftchase1};
#endif


// *** PRE-V1.4 APOGEE RESTORATION *** - Relocated code to a separate file for v1.2
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
#include "wl_srock.c"
#endif


//
// fat
//
extern	statetype s_fatstand;

extern	statetype s_fatchase1;
extern	statetype s_fatchase1s;
extern	statetype s_fatchase2;
extern	statetype s_fatchase3;
extern	statetype s_fatchase3s;
extern	statetype s_fatchase4;

extern	statetype s_fatdie1;
extern	statetype s_fatdie2;
extern	statetype s_fatdie3;
extern	statetype s_fatdie4;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_fatdead;
extern	statetype s_fatdead2;
#else
extern	statetype s_fatdie5;
extern	statetype s_fatdie6;
#endif

extern	statetype s_fatshoot1;
extern	statetype s_fatshoot2;
extern	statetype s_fatshoot3;
extern	statetype s_fatshoot4;
extern	statetype s_fatshoot5;
extern	statetype s_fatshoot6;

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_needle1;
extern	statetype s_needle2;
extern	statetype s_needle3;
extern	statetype s_needle4;
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	statetype s_fatdeathcam;
#endif


statetype s_fatstand	= {false,SPR_FAT_W1,0,T_Stand,NULL,&s_fatstand};

statetype s_fatchase1 	= {false,SPR_FAT_W1,10,T_Fat,NULL,&s_fatchase1s};
statetype s_fatchase1s	= {false,SPR_FAT_W1,3,NULL,NULL,&s_fatchase2};
statetype s_fatchase2 	= {false,SPR_FAT_W2,8,T_Fat,NULL,&s_fatchase3};
statetype s_fatchase3 	= {false,SPR_FAT_W3,10,T_Fat,NULL,&s_fatchase3s};
statetype s_fatchase3s	= {false,SPR_FAT_W3,3,NULL,NULL,&s_fatchase4};
statetype s_fatchase4 	= {false,SPR_FAT_W4,8,T_Fat,NULL,&s_fatchase1};

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
statetype s_fatdeathcam	= {false,SPR_FAT_W1,1,NULL,NULL,&s_fatdie1};
#endif

statetype s_fatdie1	= {false,SPR_FAT_W1,1,NULL,A_DeathScream,&s_fatdie2};
statetype s_fatdie2	= {false,SPR_FAT_W1,10,NULL,NULL,&s_fatdie3};
statetype s_fatdie3	= {false,SPR_FAT_DIE1,10,NULL,NULL,&s_fatdie4};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_fatdie4	= {false,SPR_FAT_DIE2,10,NULL,NULL,&s_fatdead};
statetype s_fatdead	= {false,SPR_FAT_DEAD,55,NULL,NULL,&s_fatdead2};
statetype s_fatdead2	= {false,SPR_FAT_DEAD2,55,NULL,NULL,&s_fatdead};
#else
statetype s_fatdie4	= {false,SPR_FAT_DIE2,10,NULL,NULL,&s_fatdie5};
statetype s_fatdie5	= {false,SPR_FAT_DIE3,10,NULL,NULL,&s_fatdie6};
statetype s_fatdie6	= {false,SPR_FAT_DEAD,20,NULL,A_StartDeathCam,&s_fatdie6};
#endif

statetype s_fatshoot1 	= {false,SPR_FAT_SHOOT1,30,NULL,NULL,&s_fatshoot2};
statetype s_fatshoot2 	= {false,SPR_FAT_SHOOT2,10,NULL,T_GiftThrow,&s_fatshoot3};
statetype s_fatshoot3 	= {false,SPR_FAT_SHOOT3,10,NULL,T_Shoot,&s_fatshoot4};
statetype s_fatshoot4 	= {false,SPR_FAT_SHOOT4,10,NULL,T_Shoot,&s_fatshoot5};
statetype s_fatshoot5 	= {false,SPR_FAT_SHOOT3,10,NULL,T_Shoot,&s_fatshoot6};
statetype s_fatshoot6 	= {false,SPR_FAT_SHOOT4,10,NULL,T_Shoot,&s_fatchase1};
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10


/*
===============
=
= SpawnSchabbs
=
===============
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnSchabbs (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnSchabbs (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (DigiMode != sds_Off)
		s_schabbdie1.tictime = 140;
	else
		s_schabbdie1.tictime = 15;
#else
	if (DigiMode != sds_Off)
		s_schabbdie2.tictime = 140;
	else
		s_schabbdie2.tictime = 5;
#endif

	SpawnNewObj (tilex,tiley,&s_schabbstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = schabbobj;
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
	newobj->hitpoints = starthitpoints[en_schabbs];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_schabbs];
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = south;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
===============
=
= SpawnGift
=
===============
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnGift (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnGift (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

	if (DigiMode != sds_Off)
	  s_giftdie2.tictime = 140;
	else
	  s_giftdie2.tictime = 5;

	SpawnNewObj (tilex,tiley,&s_giftstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = giftobj;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	newobj->hitpoints = starthitpoints[en_gift];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_gift];
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = north;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= SpawnFat
=
===============
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnFat (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnFat (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

	if (DigiMode != sds_Off)
	  s_fatdie2.tictime = 140;
	else
	  s_fatdie2.tictime = 5;

	SpawnNewObj (tilex,tiley,&s_fatstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = fatobj;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	newobj->hitpoints = starthitpoints[en_fat];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_fat];
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = south;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10


// *** PRE-V1.4 APOGEE RESTORATION *** - Relocated code to a separate file for v1.2
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
#include "wl_fproj.c"
#endif



/*
=================
=
= T_SchabbThrow
=
=================
*/

void T_SchabbThrow (objtype *ob)
{
	id0_long_t	deltax,deltay;
	float	angle;
	id0_int_t		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = ID0_M_PI*2+angle;
	iangle = angle/(ID0_M_PI*2)*ANGLES;

	GetNewActor ();
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	newobj->state = &s_coconut;
#else
	newobj->state = &s_needle1;
#endif
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = needleobj;
	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->speed = 0x2000l;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	actorat[newobj->tilex][newobj->tiley] = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(newobj);
	newobj->areanumber = *(mapsegs[0]+farmapylookup[newobj->tiley]+newobj->tilex)-AREATILE;
#else
	newobj->flags = FL_NONMARK;
	newobj->active = ac_yes/*true*/;
#endif

	PlaySoundLocActor (SCHABBSTHROWSND,newobj);
}

// *** PRE-V1.4 APOGEE RESTORATION *** - Relocated code to a separate file for v1.2; Not included in v1.0 at all.
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
#include "wl_fsmok.c"
#endif


// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
=================
=
= T_GiftThrow
=
=================
*/

void T_GiftThrow (objtype *ob)
{
	id0_long_t	deltax,deltay;
	float	angle;
	id0_int_t		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = ID0_M_PI*2+angle;
	iangle = angle/(ID0_M_PI*2)*ANGLES;

	GetNewActor ();
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	newobj->state = &s_coconut;
#else
	newobj->state = &s_rocket;
#endif
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = rocketobj;
	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->speed = 0x2000l;
	newobj->flags = FL_NONMARK;
	newobj->active = ac_yes/*true*/;

	PlaySoundLocActor (MISSILEFIRESND,newobj);
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10



/*
=================
=
= T_Schabb
=
=================
*/

void T_Schabb (objtype *ob)
{
	id0_long_t move;
	id0_int_t	dx,dy,dist;
	id0_boolean_t	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,&s_schabbshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}




// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
=================
=
= T_Gift
=
=================
*/

void T_Gift (objtype *ob)
{
	id0_long_t move;
	id0_int_t	dx,dy,dist;
	id0_boolean_t	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,&s_giftshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}




/*
=================
=
= T_Fat
=
=================
*/

void T_Fat (objtype *ob)
{
	id0_long_t move;
	id0_int_t	dx,dy,dist;
	id0_boolean_t	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,&s_fatshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10



/*
=============================================================================

							HITLERS

=============================================================================
*/


// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
//
// fake
//
extern	statetype s_fakestand;

extern	statetype s_fakechase1;
extern	statetype s_fakechase1s;
extern	statetype s_fakechase2;
extern	statetype s_fakechase3;
extern	statetype s_fakechase3s;
extern	statetype s_fakechase4;

extern	statetype s_fakedie1;
extern	statetype s_fakedie2;
extern	statetype s_fakedie3;
extern	statetype s_fakedie4;
extern	statetype s_fakedie5;
extern	statetype s_fakedie6;

extern	statetype s_fakeshoot1;
extern	statetype s_fakeshoot2;
extern	statetype s_fakeshoot3;
extern	statetype s_fakeshoot4;
extern	statetype s_fakeshoot5;
extern	statetype s_fakeshoot6;
extern	statetype s_fakeshoot7;
extern	statetype s_fakeshoot8;
extern	statetype s_fakeshoot9;

extern	statetype s_fire1;
extern	statetype s_fire2;

statetype s_fakestand	= {false,SPR_FAKE_W1,0,T_Stand,NULL,&s_fakestand};

statetype s_fakechase1 	= {false,SPR_FAKE_W1,10,T_Fake,NULL,&s_fakechase1s};
statetype s_fakechase1s	= {false,SPR_FAKE_W1,3,NULL,NULL,&s_fakechase2};
statetype s_fakechase2 	= {false,SPR_FAKE_W2,8,T_Fake,NULL,&s_fakechase3};
statetype s_fakechase3 	= {false,SPR_FAKE_W3,10,T_Fake,NULL,&s_fakechase3s};
statetype s_fakechase3s	= {false,SPR_FAKE_W3,3,NULL,NULL,&s_fakechase4};
statetype s_fakechase4 	= {false,SPR_FAKE_W4,8,T_Fake,NULL,&s_fakechase1};

statetype s_fakedie1	= {false,SPR_FAKE_DIE1,10,NULL,A_DeathScream,&s_fakedie2};
statetype s_fakedie2	= {false,SPR_FAKE_DIE2,10,NULL,NULL,&s_fakedie3};
statetype s_fakedie3	= {false,SPR_FAKE_DIE3,10,NULL,NULL,&s_fakedie4};
statetype s_fakedie4	= {false,SPR_FAKE_DIE4,10,NULL,NULL,&s_fakedie5};
statetype s_fakedie5	= {false,SPR_FAKE_DIE5,10,NULL,NULL,&s_fakedie6};
statetype s_fakedie6	= {false,SPR_FAKE_DEAD,0,NULL,NULL,&s_fakedie6};

statetype s_fakeshoot1 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot2};
statetype s_fakeshoot2 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot3};
statetype s_fakeshoot3 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot4};
statetype s_fakeshoot4 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot5};
statetype s_fakeshoot5 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot6};
statetype s_fakeshoot6 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot7};
statetype s_fakeshoot7 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot8};
statetype s_fakeshoot8 	= {false,SPR_FAKE_SHOOT,8,NULL,T_FakeFire,&s_fakeshoot9};
statetype s_fakeshoot9 	= {false,SPR_FAKE_SHOOT,8,NULL,NULL,&s_fakechase1};

statetype s_fire1 	= {false,SPR_FIRE1,6,NULL,T_Projectile,&s_fire2};
statetype s_fire2 	= {false,SPR_FIRE2,6,NULL,T_Projectile,&s_fire1};
#endif

//
// hitler
//
extern	statetype s_mechachase1;
extern	statetype s_mechachase1s;
extern	statetype s_mechachase2;
extern	statetype s_mechachase3;
extern	statetype s_mechachase3s;
extern	statetype s_mechachase4;

extern	statetype s_mechadie1;
extern	statetype s_mechadie2;
extern	statetype s_mechadie3;
extern	statetype s_mechadie4;

extern	statetype s_mechashoot1;
extern	statetype s_mechashoot2;
extern	statetype s_mechashoot3;
extern	statetype s_mechashoot4;
extern	statetype s_mechashoot5;
extern	statetype s_mechashoot6;


extern	statetype s_hitlerchase1;
extern	statetype s_hitlerchase1s;
extern	statetype s_hitlerchase2;
extern	statetype s_hitlerchase3;
extern	statetype s_hitlerchase3s;
extern	statetype s_hitlerchase4;

extern	statetype s_hitlerdie1;
extern	statetype s_hitlerdie2;
extern	statetype s_hitlerdie3;
extern	statetype s_hitlerdie4;
extern	statetype s_hitlerdie5;
extern	statetype s_hitlerdie6;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_hitlerdead;
extern	statetype s_hitlerdead2;
#else
extern	statetype s_hitlerdie7;
extern	statetype s_hitlerdie8;
extern	statetype s_hitlerdie9;
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
extern	statetype s_hitlerdie10;
#endif
#endif

extern	statetype s_hitlershoot1;
extern	statetype s_hitlershoot2;
extern	statetype s_hitlershoot3;
extern	statetype s_hitlershoot4;
extern	statetype s_hitlershoot5;
extern	statetype s_hitlershoot6;

// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (!defined GAMEVER_NOAH3D)
extern	statetype s_hitlerdeathcam;
#endif

statetype s_mechastand	= {false,SPR_MECHA_W1,0,T_Stand,NULL,&s_mechastand};

statetype s_mechachase1 	= {false,SPR_MECHA_W1,10,T_Chase,A_MechaSound,&s_mechachase1s};
statetype s_mechachase1s	= {false,SPR_MECHA_W1,6,NULL,NULL,&s_mechachase2};
statetype s_mechachase2 	= {false,SPR_MECHA_W2,8,T_Chase,NULL,&s_mechachase3};
statetype s_mechachase3 	= {false,SPR_MECHA_W3,10,T_Chase,A_MechaSound,&s_mechachase3s};
statetype s_mechachase3s	= {false,SPR_MECHA_W3,6,NULL,NULL,&s_mechachase4};
statetype s_mechachase4 	= {false,SPR_MECHA_W4,8,T_Chase,NULL,&s_mechachase1};

statetype s_mechadie1	= {false,SPR_MECHA_DIE1,10,NULL,A_DeathScream,&s_mechadie2};
statetype s_mechadie2	= {false,SPR_MECHA_DIE2,10,NULL,NULL,&s_mechadie3};
statetype s_mechadie3	= {false,SPR_MECHA_DIE3,10,NULL,A_HitlerMorph,&s_mechadie4};
statetype s_mechadie4	= {false,SPR_MECHA_DEAD,0,NULL,NULL,&s_mechadie4};

statetype s_mechashoot1 	= {false,SPR_MECHA_SHOOT1,30,NULL,NULL,&s_mechashoot2};
statetype s_mechashoot2 	= {false,SPR_MECHA_SHOOT2,10,NULL,T_Shoot,&s_mechashoot3};
statetype s_mechashoot3 	= {false,SPR_MECHA_SHOOT3,10,NULL,T_Shoot,&s_mechashoot4};
statetype s_mechashoot4 	= {false,SPR_MECHA_SHOOT2,10,NULL,T_Shoot,&s_mechashoot5};
statetype s_mechashoot5 	= {false,SPR_MECHA_SHOOT3,10,NULL,T_Shoot,&s_mechashoot6};
statetype s_mechashoot6 	= {false,SPR_MECHA_SHOOT2,10,NULL,T_Shoot,&s_mechachase1};


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
statetype s_hitlerchase1 	= {false,SPR_HITLER_W1,10,T_Chase,NULL,&s_hitlerchase1s};
statetype s_hitlerchase1s	= {false,SPR_HITLER_W1,6,NULL,NULL,&s_hitlerchase2};
statetype s_hitlerchase2 	= {false,SPR_HITLER_W2,8,T_Chase,NULL,&s_hitlerchase3};
statetype s_hitlerchase3 	= {false,SPR_HITLER_W3,10,T_Chase,NULL,&s_hitlerchase3s};
statetype s_hitlerchase3s	= {false,SPR_HITLER_W3,6,NULL,NULL,&s_hitlerchase4};
statetype s_hitlerchase4 	= {false,SPR_HITLER_W4,8,T_Chase,NULL,&s_hitlerchase1};
#else
statetype s_hitlerchase1 	= {false,SPR_HITLER_W1,6,T_Chase,NULL,&s_hitlerchase1s};
statetype s_hitlerchase1s	= {false,SPR_HITLER_W1,4,NULL,NULL,&s_hitlerchase2};
statetype s_hitlerchase2 	= {false,SPR_HITLER_W2,2,T_Chase,NULL,&s_hitlerchase3};
statetype s_hitlerchase3 	= {false,SPR_HITLER_W3,6,T_Chase,NULL,&s_hitlerchase3s};
statetype s_hitlerchase3s	= {false,SPR_HITLER_W3,4,NULL,NULL,&s_hitlerchase4};
statetype s_hitlerchase4 	= {false,SPR_HITLER_W4,2,T_Chase,NULL,&s_hitlerchase1};
#endif

// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (!defined GAMEVER_NOAH3D)
statetype s_hitlerdeathcam	= {false,SPR_HITLER_W1,10,NULL,NULL,&s_hitlerdie1};
#endif

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
statetype s_hitlerdie1	= {false,SPR_HITLER_W1,10,NULL,NULL,&s_hitlerdie2};
statetype s_hitlerdie2	= {false,SPR_HITLER_DIE1,10,NULL,A_Slurpie,&s_hitlerdie3};
statetype s_hitlerdie3	= {false,SPR_HITLER_DIE2,10,NULL,NULL,&s_hitlerdie4};
statetype s_hitlerdie4	= {false,SPR_HITLER_DIE3,10,NULL,NULL,&s_hitlerdie5};
statetype s_hitlerdie5	= {false,SPR_HITLER_DIE4,10,NULL,NULL,&s_hitlerdie6};
statetype s_hitlerdie6	= {false,SPR_HITLER_DIE5,10,NULL,NULL,&s_hitlerdie7};
statetype s_hitlerdie7	= {false,SPR_HITLER_DIE6,10,NULL,NULL,&s_hitlerdie8};
statetype s_hitlerdie8	= {false,SPR_HITLER_DIE7,10,NULL,NULL,&s_hitlerdie9};
statetype s_hitlerdie9	= {false,SPR_HITLER_DEAD,0,NULL,NULL,&s_hitlerdie9};
// *** S3DNA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
statetype s_hitlerdie1	= {false,SPR_HITLER_W1,1,NULL,A_DeathScream,&s_hitlerdie2};
statetype s_hitlerdie2	= {false,SPR_HITLER_W1,15,NULL,NULL,&s_hitlerdie3};
statetype s_hitlerdie3	= {false,SPR_HITLER_DIE1,15,NULL,A_Slurpie,&s_hitlerdie4};
statetype s_hitlerdie4	= {false,SPR_HITLER_DIE2,15,NULL,NULL,&s_hitlerdie5};
statetype s_hitlerdie5	= {false,SPR_HITLER_DEAD,55,NULL,NULL,&s_hitlerdie6};
statetype s_hitlerdie6	= {false,SPR_HITLER_DEAD2,55,NULL,A_StartDeathCam,&s_hitlerdead};
statetype s_hitlerdead	= {false,SPR_HITLER_DEAD,55,NULL,NULL,&s_hitlerdead2};
statetype s_hitlerdead2	= {false,SPR_HITLER_DEAD2,55,NULL,NULL,&s_hitlerdead};
#else
statetype s_hitlerdie1	= {false,SPR_HITLER_W1,1,NULL,A_DeathScream,&s_hitlerdie2};
statetype s_hitlerdie2	= {false,SPR_HITLER_W1,10,NULL,NULL,&s_hitlerdie3};
statetype s_hitlerdie3	= {false,SPR_HITLER_DIE1,10,NULL,A_Slurpie,&s_hitlerdie4};
statetype s_hitlerdie4	= {false,SPR_HITLER_DIE2,10,NULL,NULL,&s_hitlerdie5};
statetype s_hitlerdie5	= {false,SPR_HITLER_DIE3,10,NULL,NULL,&s_hitlerdie6};
statetype s_hitlerdie6	= {false,SPR_HITLER_DIE4,10,NULL,NULL,&s_hitlerdie7};
statetype s_hitlerdie7	= {false,SPR_HITLER_DIE5,10,NULL,NULL,&s_hitlerdie8};
statetype s_hitlerdie8	= {false,SPR_HITLER_DIE6,10,NULL,NULL,&s_hitlerdie9};
statetype s_hitlerdie9	= {false,SPR_HITLER_DIE7,10,NULL,NULL,&s_hitlerdie10};
statetype s_hitlerdie10	= {false,SPR_HITLER_DEAD,20,NULL,A_StartDeathCam,&s_hitlerdie10};
#endif

statetype s_hitlershoot1 	= {false,SPR_HITLER_SHOOT1,30,NULL,NULL,&s_hitlershoot2};
statetype s_hitlershoot2 	= {false,SPR_HITLER_SHOOT2,10,NULL,T_Shoot,&s_hitlershoot3};
statetype s_hitlershoot3 	= {false,SPR_HITLER_SHOOT3,10,NULL,T_Shoot,&s_hitlershoot4};
statetype s_hitlershoot4 	= {false,SPR_HITLER_SHOOT2,10,NULL,T_Shoot,&s_hitlershoot5};
statetype s_hitlershoot5 	= {false,SPR_HITLER_SHOOT3,10,NULL,T_Shoot,&s_hitlershoot6};
statetype s_hitlershoot6 	= {false,SPR_HITLER_SHOOT2,10,NULL,T_Shoot,&s_hitlerchase1};



// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
===============
=
= SpawnFakeHitler
=
===============
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnFakeHitler (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnFakeHitler (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (DigiMode != sds_Off)
	  s_hitlerdie1.tictime = 140;
	else
	  s_hitlerdie1.tictime = 15;
#else
	if (DigiMode != sds_Off)
	  s_hitlerdie2.tictime = 140;
	else
	  s_hitlerdie2.tictime = 5;
#endif

	SpawnNewObj (tilex,tiley,&s_fakestand);
	newobj->speed = SPDPATROL;

	newobj->obclass = fakeobj;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	newobj->hitpoints = starthitpoints[en_fake];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_fake];
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = north;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
#endif


/*
===============
=
= SpawnHitler
=
===============
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if 0 // REFKEEN: dir wasn't actually passed, assume a specific value for now
//#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
void SpawnHitler (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
#else
void SpawnHitler (id0_int_t tilex, id0_int_t tiley)
#endif
{
	id0_unsigned_t	id0_far *map,tile;

	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (DigiMode != sds_Off)
		s_hitlerdie2.tictime = 140;
	else
		s_hitlerdie2.tictime = 5;
#endif


	SpawnNewObj (tilex,tiley,&s_mechastand);
	newobj->speed = SPDPATROL;

	newobj->obclass = mechahitlerobj;
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
	newobj->hitpoints = starthitpoints[en_hitler];
#else
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_hitler];
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->dir = nodir; // REFKEEN: Assume uninitialized value is this for now
//	newobj->dir = dir*2;
#else
	newobj->dir = south;
#endif
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= A_HitlerMorph
=
===============
*/

void A_HitlerMorph (objtype *ob)
{
	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	id0_far *map,tile;
#else
	id0_unsigned_t	id0_far *map,tile,hitpoints[4]={500,700,800,900};
#endif


	SpawnNewObj (ob->tilex,ob->tiley,&s_hitlerchase1);
	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	newobj->speed = SPDPATROL;
#else
	newobj->speed = SPDPATROL*5;
#endif

	newobj->x = ob->x;
	newobj->y = ob->y;

	newobj->distance = ob->distance;
	newobj->dir = ob->dir;
	newobj->flags = ob->flags | FL_SHOOTABLE;

	newobj->obclass = realhitlerobj;
	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	newobj->hitpoints = 400;
#else
	newobj->hitpoints = hitpoints[gamestate.difficulty];
#endif
}


////////////////////////////////////////////////////////
//
// A_MechaSound
// A_Slurpie
//
////////////////////////////////////////////////////////
void A_MechaSound (objtype *ob)
{
	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (areabyplayer[ob->areanumber])
#endif
		PlaySoundLocActor (MECHSTEPSND,ob);
}


//#pragma argsused
void A_Slurpie (objtype *ob)
{
 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
 PlaySoundLocActor (SLURPIESND,ob);
 // *** S3DNA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
 SD_PlaySound(D_EDIESND);
#else
 SD_PlaySound(SLURPIESND);
#endif
}

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
=================
=
= T_FakeFire
=
=================
*/

void T_FakeFire (objtype *ob)
{
	id0_long_t	deltax,deltay;
	float	angle;
	id0_int_t		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = ID0_M_PI*2+angle;
	iangle = angle/(ID0_M_PI*2)*ANGLES;

	GetNewActor ();
	newobj->state = &s_fire1;
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->obclass = fireobj;
	newobj->speed = 0x1200l;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	actorat[newobj->tilex][newobj->tiley] = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(newobj);
	newobj->areanumber = *(mapsegs[0]+farmapylookup[newobj->tiley]+newobj->tilex)-AREATILE;
#else
	newobj->flags = FL_NEVERMARK;
	newobj->active = ac_yes/*true*/;
#endif

	PlaySoundLocActor (FLAMETHROWERSND,newobj);

}



/*
=================
=
= T_Fake
=
=================
*/

void T_Fake (objtype *ob)
{
	id0_long_t move;
	id0_int_t	dx,dy,dist;
	id0_boolean_t	dodge;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// A bit closer to Schabb/Gift/Fat behaviors in v1.0
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if ((dist <= 4) && CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,&s_fakeshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}
#else
	if (CheckLine(ob))			// got a shot at player?
	{
		if ( US_RndT() < (tics<<1) )
		{
		//
		// go into attack frame
		//
			NewState (ob,&s_fakeshoot1);
			return;
		}
	}

	if (ob->dir == nodir)
	{
		SelectDodgeDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}
#endif

	move = ob->speed*tics;

	while (move)
	{
		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
#else
		SelectDodgeDir (ob);
#endif

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}
#endif // GAMEVER_NOAH3D

#endif
/*
============================================================================

							STAND

============================================================================
*/


/*
===============
=
= T_Stand
=
===============
*/

void T_Stand (objtype *ob)
{
	SightPlayer (ob);
}


/*
============================================================================

								CHASE

============================================================================
*/

/*
=================
=
= T_Chase
=
=================
*/

void T_Chase (objtype *ob)
{
	id0_long_t move;
	id0_int_t	dx,dy,dist,chance;
	id0_boolean_t	dodge;

	// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
	if (gamestate.victoryflag)
		return;
#endif

	dodge = false;
	if (CheckLine(ob))	// got a shot at player?
	{
		dx = abs(ob->tilex - player->tilex);
		dy = abs(ob->tiley - player->tiley);
		dist = dx>dy ? dx : dy;
		if (!dist || (dist==1 && ob->distance<0x4000) )
			chance = 300;
		else
			chance = (tics<<4)/dist;

		if ( US_RndT()<chance)
		{
		//
		// go into attack frame
		//
			switch (ob->obclass)
			{
			case guardobj:
				NewState (ob,&s_grdshoot1);
				break;
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			case officerobj:
				NewState (ob,&s_ofcshoot1);
				break;
			case mutantobj:
				NewState (ob,&s_mutshoot1);
				break;
#endif
			case ssobj:
				NewState (ob,&s_ssshoot1);
				break;
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef SPEAR
			case bossobj:
				NewState (ob,&s_bossshoot1);
				break;
			// *** SHAREWARE V1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			case gretelobj:
				NewState (ob,&s_gretelshoot1);
				break;
#endif
			case mechahitlerobj:
				NewState (ob,&s_mechashoot1);
				break;
			case realhitlerobj:
				NewState (ob,&s_hitlershoot1);
				break;
#else
			case angelobj:
				NewState (ob,&s_angelshoot1);
				break;
			case transobj:
				NewState (ob,&s_transshoot1);
				break;
			case uberobj:
				NewState (ob,&s_ubershoot1);
				break;
			case willobj:
				NewState (ob,&s_willshoot1);
				break;
			case deathobj:
				NewState (ob,&s_deathshoot1);
				break;
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
			}
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}


// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
=================
=
= T_Ghosts
=
=================
*/

void T_Ghosts (objtype *ob)
{
	id0_long_t move;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// A bit closer to Schabb/Gift/Fat behaviors in v1.0
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_int_t	dx,dy,dist;

	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;
#endif

	if (ob->dir == nodir)
	{
		SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		if (ob->distance < 1)
			// REFKEEN: Assume NULL. Most known impact is on player death
			TakeDamage (5, NULL);
//			TakeDamage (5);
#endif
		SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}
#endif

/*
=================
=
= T_DogChase
=
=================
*/

void T_DogChase (objtype *ob)
{
	id0_long_t 	move;
	id0_int_t		dist,chance;
	id0_long_t	dx,dy;


	if (ob->dir == nodir)
	{
		SelectDodgeDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
	//
	// check for byte range
	//
		dx = player->x - ob->x;
		if (dx<0)
			dx = -dx;
		dx -= move;
		if (dx <= MINACTORDIST)
		{
			dy = player->y - ob->y;
			if (dy<0)
				dy = -dy;
			dy -= move;
			if (dy <= MINACTORDIST)
			{
				NewState (ob,&s_dogjump1);
				return;
			}
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		SelectDodgeDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}



/*
============================================================================

								PATH

============================================================================
*/


/*
===============
=
= SelectPathDir
=
===============
*/

void SelectPathDir (objtype *ob)
{
	id0_unsigned_t spot;

	spot = MAPSPOT(ob->tilex,ob->tiley,1)-ICONARROWS;

	if (spot<8)
	{
	// new direction
		ob->dir = (dirtype)spot;
	}

	ob->distance = TILEGLOBAL;

	if (!TryWalk (ob))
		ob->dir = nodir;
}


/*
===============
=
= T_Path
=
===============
*/

void T_Path (objtype *ob)
{
	id0_long_t 	move;
	id0_long_t 	deltax,deltay,size;

	if (SightPlayer (ob))
		return;

	if (ob->dir == nodir)
	{
		SelectPathDir (ob);
		if (ob->dir == nodir)
			return;					// all movement is blocked
	}


	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		if (ob->tilex>MAPSIZE || ob->tiley>MAPSIZE)
		{
			sprintf (str,"T_Path hit a wall at %u,%u, dir %u"
			,ob->tilex,ob->tiley,ob->dir);
			Quit (str);
		}



		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
		move -= ob->distance;

		SelectPathDir (ob);

		if (ob->dir == nodir)
			return;					// all movement is blocked
	}
}


/*
=============================================================================

								FIGHT

=============================================================================
*/


/*
===============
=
= T_Shoot
=
= Try to damage the player, based on skill level and player's speed
=
===============
*/

void T_Shoot (objtype *ob)
{
	id0_int_t	dx,dy,dist;
	id0_int_t	hitchance,damage;

	hitchance = 128;

	if (!areabyplayer[ob->areanumber])
		return;

	if (!CheckLine (ob))			// player is behind a wall
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	{
	  SD_PlaySound (FIRESND);
	  return;
	}
#else
	  return;
#endif

	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (ob->obclass == ssobj || ob->obclass == bossobj)
		dist = dist*2/3;					// ss are better shots
#endif

	if (thrustspeed >= RUNSPEED)
	{
		if (ob->flags&FL_VISABLE)
			hitchance = 160-dist*16;		// player can see to dodge
		else
			hitchance = 160-dist*8;
	}
	else
	{
		if (ob->flags&FL_VISABLE)
			hitchance = 256-dist*16;		// player can see to dodge
		else
			hitchance = 256-dist*8;
	}

// see if the shot was a hit

	if (US_RndT()<hitchance)
	{
		if (dist<2)
			damage = US_RndT()>>2;
		else if (dist<4)
			damage = US_RndT()>>3;
		else
			damage = US_RndT()>>4;

		TakeDamage (damage,ob);
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		SD_PlaySound (NAZIHITPLAYERSND);
		return;
#endif
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	SD_PlaySound (FIRESND);
#else
	switch(ob->obclass)
	{
	 case ssobj:
	   // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	   PlaySoundLocActor(SSFIRESND,obj);
#else
	   PlaySoundLocActor(SSFIRESND,ob);
#endif
	   break;
#ifndef SPEAR
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	 case giftobj:
	 case fatobj:
	   PlaySoundLocActor(MISSILEFIRESND,ob);
	   break;
#endif
	 case mechahitlerobj:
	 case realhitlerobj:
	 case bossobj:
	   PlaySoundLocActor(BOSSFIRESND,ob);
	   break;
	 case schabbobj:
	   PlaySoundLocActor(SCHABBSTHROWSND,ob);
	   break;
	 // *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	 case fakeobj:
	   PlaySoundLocActor(FLAMETHROWERSND,ob);
	   break;
#endif
#endif
	 default:
	   PlaySoundLocActor(NAZIFIRESND,ob);
	}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

}


/*
===============
=
= T_Bite
=
===============
*/

void T_Bite (objtype *ob)
{
	id0_long_t	dx,dy;
	id0_int_t	hitchance,damage;


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	PlaySoundLocActor(DOGATTACKSND,ob);	// JAB
#endif

	dx = player->x - ob->x;
	if (dx<0)
		dx = -dx;
	dx -= TILEGLOBAL;
	if (dx <= MINACTORDIST)
	{
		dy = player->y - ob->y;
		if (dy<0)
			dy = -dy;
		dy -= TILEGLOBAL;
		if (dy <= MINACTORDIST)
		{
		   if (US_RndT()<180)
		   {
			  // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			   // REFKEEN: Assume NULL. Most known impact is on player death
			   TakeDamage (US_RndT()>>4,NULL);
//			   TakeDamage (US_RndT()>>4);
			   // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			   SD_PlaySound (NAZIHITPLAYERSND);
#endif
#else
			   TakeDamage (US_RndT()>>4,ob);
#endif
			   return;
		   }
		}
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	SD_PlaySound (FIRESND);
#endif
	return;
}


// *** ALPHA RESTORATION ***
//#ifndef SPEAR
#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
============================================================================

							BJ VICTORY

============================================================================
*/


//
// BJ victory
//

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void T_Missile (objtype *ob);

extern	statetype s_flame;
extern	statetype s_fexplode1;
extern	statetype s_fexplode2;
extern	statetype s_fexplode3;

extern	statetype s_missile;
extern	statetype s_mexplode1;
extern	statetype s_mexplode2;
extern	statetype s_mexplode3;
#else
void T_BJRun (objtype *ob);
void T_BJJump (objtype *ob);
void T_BJDone (objtype *ob);
void T_BJYell (objtype *ob);

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
void T_DeathCam (objtype *ob);
#endif

extern	statetype s_bjrun1;
extern	statetype s_bjrun1s;
extern	statetype s_bjrun2;
extern	statetype s_bjrun3;
extern	statetype s_bjrun3s;
extern	statetype s_bjrun4;

extern	statetype s_bjjump1;
extern	statetype s_bjjump2;
extern	statetype s_bjjump3;
extern	statetype s_bjjump4;


statetype s_bjrun1 	= {false,SPR_BJ_W1,12,T_BJRun,NULL,&s_bjrun1s};
statetype s_bjrun1s	= {false,SPR_BJ_W1,3, NULL,NULL,&s_bjrun2};
statetype s_bjrun2 	= {false,SPR_BJ_W2,8,T_BJRun,NULL,&s_bjrun3};
statetype s_bjrun3 	= {false,SPR_BJ_W3,12,T_BJRun,NULL,&s_bjrun3s};
statetype s_bjrun3s	= {false,SPR_BJ_W3,3, NULL,NULL,&s_bjrun4};
statetype s_bjrun4 	= {false,SPR_BJ_W4,8,T_BJRun,NULL,&s_bjrun1};


statetype s_bjjump1	= {false,SPR_BJ_JUMP1,14,T_BJJump,NULL,&s_bjjump2};
statetype s_bjjump2	= {false,SPR_BJ_JUMP2,14,T_BJJump,T_BJYell,&s_bjjump3};
statetype s_bjjump3	= {false,SPR_BJ_JUMP3,14,T_BJJump,NULL,&s_bjjump4};
statetype s_bjjump4	= {false,SPR_BJ_JUMP4,300,NULL,T_BJDone,&s_bjjump4};
#endif


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
statetype s_deathcam = {false,0,0,NULL,NULL,NULL};
#endif
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_gameover = {false,0,0,NULL,NULL,NULL};

statetype s_flame = {true,SPR_CANTALOUPE,3,T_Missile,NULL,&s_flame};
statetype s_fexplode1 = {true,SPR_CANTALOUPEEXP,6,NULL,NULL,&s_fexplode2};
statetype s_fexplode2 = {true,SPR_CANTALOUPEEXP,6,NULL,NULL,&s_fexplode3};
statetype s_fexplode3 = {true,SPR_CANTALOUPEEXP,6,NULL,NULL,NULL};

statetype s_missile = {true,SPR_WATERMELON,3,T_Missile,NULL,&s_missile};
statetype s_mexplode1 = {true,SPR_WATERMELONEXP,6,NULL,NULL,&s_mexplode2};
statetype s_mexplode2 = {true,SPR_WATERMELONEXP,6,NULL,NULL,&s_mexplode3};
statetype s_mexplode3 = {true,SPR_WATERMELONEXP,6,NULL,NULL,NULL};
#endif


// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
===============
=
= SpawnBJVictory
=
===============
*/

void SpawnBJVictory (void)
{
	id0_unsigned_t	id0_far *map,tile;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	SpawnNewObj (player->tilex,player->tiley,&s_bjrun1);
#else
	SpawnNewObj (player->tilex,player->tiley+1,&s_bjrun1);
#endif
	newobj->x = player->x;
	newobj->y = player->y;
	newobj->obclass = bjobj;
	newobj->dir = north;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	newobj->temp1 = 5;			// tiles to run forward
#else
	newobj->temp1 = 6;			// tiles to run forward
#endif
}



/*
===============
=
= T_BJRun
=
===============
*/

void T_BJRun (objtype *ob)
{
	id0_long_t 	move;

	move = BJRUNSPEED*tics;

	while (move)
	{
		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}


		ob->x = ((id0_long_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((id0_long_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
		move -= ob->distance;

		SelectPathDir (ob);

		if ( !(--ob->temp1) )
		{
			NewState (ob,&s_bjjump1);
			return;
		}
	}
}


/*
===============
=
= T_BJJump
=
===============
*/

void T_BJJump (objtype *ob)
{
	id0_long_t 	move;

	move = BJJUMPSPEED*tics;
	MoveObj (ob,move);
}


/*
===============
=
= T_BJYell
=
===============
*/

void T_BJYell (objtype *ob)
{
	PlaySoundLocActor(YEAHSND,ob);	// JAB
}


/*
===============
=
= T_BJDone
=
===============
*/

//#pragma argsused
void T_BJDone (objtype *ob)
{
	playstate = ex_victorious;				// exit castle tile
}
#endif // GAMEVER_NOAH3D



//===========================================================================


// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D

#define MINMISSILEDIST 0x8000l
// S3DNA RESTORATION - Has code found in TryMove and ProjectileTryMove
id0_boolean_t MissileTryMove (objtype *ob)
{
	id0_int_t			xl,yl,xh,yh,x,y;
	objtype		*check;
	id0_long_t		deltax,deltay;

	xl = (ob->x-PROJSIZE) >>TILESHIFT;
	yl = (ob->y-PROJSIZE) >>TILESHIFT;

	xh = (ob->x+PROJSIZE) >>TILESHIFT;
	yh = (ob->y+PROJSIZE) >>TILESHIFT;

//
// check for solid walls
//
	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			if (actorat[x][y] && actorat[x][y]<refkeen_compat_wl_play_objoffset)
//			check = actorat[x][y];
//			if (check && check<objlist)
				return false;
		}

//
// check for actors
//
	if (yl>0)
		yl--;
	if (yh<MAPSIZE-1)
		yh++;
	if (xl>0)
		xl--;
	if (xh<MAPSIZE-1)
		xh++;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(actorat[x][y]);
			if (actorat[x][y] > refkeen_compat_wl_play_objoffset &&
			    (check->flags & FL_SHOOTABLE))
//			if (check > objlist && (check->flags & FL_SHOOTABLE) )
			{
				deltax = ob->x - check->x;
				if (deltax < -MINMISSILEDIST || deltax > MINMISSILEDIST)
					continue;
				deltay = ob->y - check->y;
				if (deltay < -MINMISSILEDIST || deltay > MINMISSILEDIST)
					continue;

				if (ob->obclass == flameobj)
				{
					DamageActor (check,(US_RndT()&0xF)+1);
					// check for pass through
					if (!(check->flags & FL_SHOOTABLE))
						return true;
				}
				else if (ob->obclass == missileobj)
				{
					DamageActor (check,((US_RndT()&3)+1)<<4);
					// check for pass through
					if (!(check->flags & FL_SHOOTABLE))
						return true;
				}
				return false;
			}
		}

	return true;
}

// S3DNA RESTORATION - Looks based on T_Projectile
void T_Missile (objtype *ob)
{
	id0_long_t	deltax,deltay;
	id0_int_t		damage;
	id0_long_t	speed;

	speed = (id0_long_t)ob->speed*tics;

	deltax = FixedByFrac(speed,costable[ob->angle]);
	deltay = -FixedByFrac(speed,sintable[ob->angle]);

	if (deltax>0x10000l)
		deltax = 0x10000l;
	if (deltay>0x10000l)
		deltay = 0x10000l;

	ob->x += deltax;
	ob->y += deltay;

	if (!MissileTryMove (ob))
	{
		if (ob->obclass == flameobj)
		{
			PlaySoundLocActor(D_COCHITSND,ob);
			ob->state = &s_fexplode1;
		}
		else if (ob->obclass == missileobj)
		{
			PlaySoundLocActor(D_WATHITSND,ob);
			ob->state = &s_mexplode1;
		}
		else
			ob->state = NULL;		// mark for removal

		return;
	}

	ob->tilex = ob->x >> TILESHIFT;
	ob->tiley = ob->y >> TILESHIFT;
}
#endif // GAMEVER_NOAH3D

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
===============
=
= CheckPosition
=
===============
*/

id0_boolean_t	CheckPosition (objtype *ob)
{
	id0_int_t	x,y,xl,yl,xh,yh;
//	objtype *check;

	xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
	yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

	xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
	yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

	//
	// check for solid walls
	//
	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			if (actorat[x][y] && actorat[x][y]<refkeen_compat_wl_play_objoffset)
//			check = actorat[x][y];
//			if (check && check<objlist)
				return false;
		}

	return true;
}


/*
===============
=
= A_StartDeathCam
=
===============
*/

void	A_StartDeathCam (objtype *ob)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	FinishPaletteShifts ();
	NewState (player,
		&s_deathcam);
	gamestate.victoryflag = true;
#else
	id0_long_t	dx,dy;
	float	fangle;
	id0_long_t    xmove,ymove;
	id0_long_t	dist;
	id0_int_t		temp,i;

	FinishPaletteShifts ();

	VW_WaitVBL (100);

	if (gamestate.victoryflag)
	{
		playstate = ex_victorious;				// exit castle tile
		return;
	}

	gamestate.victoryflag = true;
	VW_Bar (0,0,320,200-STATUSLINES,127);
	FizzleFade(bufferofs,displayofs,320,200-STATUSLINES,70,false);

	PM_UnlockMainMem ();
	CA_UpLevel ();
	CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	#ifdef JAPAN
	#ifndef JAPDEMO
	CA_CacheScreen(C_LETSSEEPIC);
	#endif
	#else
	Write(0,7,STR_SEEAGAIN);
	#endif
	CA_DownLevel ();
	PM_CheckMainMem ();

	VW_UpdateScreen ();

	IN_UserInput(300);

//
// line angle up exactly
//
	NewState (player,&s_deathcam);

	player->x = gamestate.killx;
	player->y = gamestate.killy;

	dx = ob->x - player->x;
	dy = player->y - ob->y;

	fangle = atan2(dy,dx);			// returns -pi to pi
	if (fangle<0)
		fangle = ID0_M_PI*2+fangle;

	player->angle = fangle/(ID0_M_PI*2)*ANGLES;

//
// try to position as close as possible without being in a wall
//
	dist = 0x14000l;
	do
	{
		xmove = FixedByFrac(dist,costable[player->angle]);
		ymove = -FixedByFrac(dist,sintable[player->angle]);

		player->x = ob->x - xmove;
		player->y = ob->y - ymove;
		dist += 0x1000;

	} while (!CheckPosition (player));
	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in unsigned
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

//
// go back to the game
//
	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;

	fizzlein = true;
	switch (ob->obclass)
	{
#ifndef SPEAR
	case schabbobj:
		NewState (ob,&s_schabbdeathcam);
		break;
	case realhitlerobj:
		NewState (ob,&s_hitlerdeathcam);
		break;
	case giftobj:
		NewState (ob,&s_giftdeathcam);
		break;
	case fatobj:
		NewState (ob,&s_fatdeathcam);
		break;
#endif
	}

#endif // GAMEVER_NOAH3D
}
#endif // GAMEVER_WOLFREV > GV_WR_WL1AP10

#endif

REFKEEN_NS_E
