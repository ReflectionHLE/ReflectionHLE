// *** PRE-V1.4 APOGEE RESTORATION *** - #include this file
// at a location in WL_ACT2.C depending on version

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_EXEDEF_N3DWT10
extern	statetype s_coconut;
extern	statetype s_cocohit;
#else
extern	statetype s_rocket;
extern	statetype s_smoke1;
extern	statetype s_smoke2;
extern	statetype s_smoke3;
extern	statetype s_smoke4;
// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (!defined GAMEVER_EXEDEF_WL1AP10) && (!defined GAMEVER_EXEDEF_WL1AP11)
extern	statetype s_boom2;
extern	statetype s_boom3;
#endif
#endif

void A_Smoke (objtype *ob);

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_EXEDEF_N3DWT10
statetype s_coconut	 	= {false,SPR_ANIMALPROJ,16,T_Projectile,NULL,&s_coconut};
statetype s_cocohit	 	= {false,SPR_ANIMALPROJEXP,18,NULL,NULL,NULL};
#else
statetype s_rocket	 	= {true,SPR_ROCKET_1,3,T_Projectile,A_Smoke,&s_rocket};
statetype s_smoke1	 	= {false,SPR_SMOKE_1,3,NULL,NULL,&s_smoke2};
statetype s_smoke2	 	= {false,SPR_SMOKE_2,3,NULL,NULL,&s_smoke3};
statetype s_smoke3	 	= {false,SPR_SMOKE_3,3,NULL,NULL,&s_smoke4};
statetype s_smoke4	 	= {false,SPR_SMOKE_4,3,NULL,NULL,NULL};

// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (!defined GAMEVER_EXEDEF_WL1AP10) && (!defined GAMEVER_EXEDEF_WL1AP11)
statetype s_boom1	 	= {false,SPR_BOOM_1,6,NULL,NULL,&s_boom2};
statetype s_boom2	 	= {false,SPR_BOOM_2,6,NULL,NULL,&s_boom3};
statetype s_boom3	 	= {false,SPR_BOOM_3,6,NULL,NULL,NULL};
#endif
#endif // GAMEVER_EXEDEF_N3DWT10

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
