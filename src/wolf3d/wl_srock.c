/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020 NY00123
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

// *** PRE-V1.4 APOGEE RESTORATION *** - #include this file
// at a location in WL_ACT2.C depending on version

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_coconut;
extern	statetype s_cocohit;
#else
extern	statetype s_rocket;
extern	statetype s_smoke1;
extern	statetype s_smoke2;
extern	statetype s_smoke3;
extern	statetype s_smoke4;
// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
extern	statetype s_boom2;
extern	statetype s_boom3;
#endif
#endif

void A_Smoke (objtype *ob);

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
statetype s_coconut	 	= {false,SPR_ANIMALPROJ,16,T_Projectile,NULL,&s_coconut};
statetype s_cocohit	 	= {false,SPR_ANIMALPROJEXP,18,NULL,NULL,NULL};
#else
statetype s_rocket	 	= {true,SPR_ROCKET_1,3,T_Projectile,A_Smoke,&s_rocket};
statetype s_smoke1	 	= {false,SPR_SMOKE_1,3,NULL,NULL,&s_smoke2};
statetype s_smoke2	 	= {false,SPR_SMOKE_2,3,NULL,NULL,&s_smoke3};
statetype s_smoke3	 	= {false,SPR_SMOKE_3,3,NULL,NULL,&s_smoke4};
statetype s_smoke4	 	= {false,SPR_SMOKE_4,3,NULL,NULL,NULL};

// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
statetype s_boom1	 	= {false,SPR_BOOM_1,6,NULL,NULL,&s_boom2};
statetype s_boom2	 	= {false,SPR_BOOM_2,6,NULL,NULL,&s_boom3};
statetype s_boom3	 	= {false,SPR_BOOM_3,6,NULL,NULL,NULL};
#endif
#endif // GAMEVER_NOAH3D

#ifdef SPEAR

extern	statetype s_hrocket;
extern	statetype s_hsmoke1;
extern	statetype s_hsmoke2;
extern	statetype s_hsmoke3;
extern	statetype s_hsmoke4;
extern	statetype s_hboom2;
extern	statetype s_hboom3;

void A_Smoke (objtype *ob);

statetype s_hrocket	 	= {true,SPR_HROCKET_1,3,T_Projectile,A_Smoke,&s_hrocket};
statetype s_hsmoke1	 	= {false,SPR_HSMOKE_1,3,NULL,NULL,&s_hsmoke2};
statetype s_hsmoke2	 	= {false,SPR_HSMOKE_2,3,NULL,NULL,&s_hsmoke3};
statetype s_hsmoke3	 	= {false,SPR_HSMOKE_3,3,NULL,NULL,&s_hsmoke4};
statetype s_hsmoke4	 	= {false,SPR_HSMOKE_4,3,NULL,NULL,NULL};

statetype s_hboom1	 	= {false,SPR_HBOOM_1,6,NULL,NULL,&s_hboom2};
statetype s_hboom2	 	= {false,SPR_HBOOM_2,6,NULL,NULL,&s_hboom3};
statetype s_hboom3	 	= {false,SPR_HBOOM_3,6,NULL,NULL,NULL};

#endif
