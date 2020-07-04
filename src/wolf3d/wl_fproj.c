// *** PRE-V1.4 APOGEE RESTORATION *** - #include this file
// at a location in WL_ACT2.C depending on version

 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
===================
=
= ProjectileTryMove
=
= returns true if move ok
===================
*/

#define PROJSIZE	0x2000

id0_boolean_t ProjectileTryMove (objtype *ob)
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
			check = actorat[x][y];
			if (check && check<objlist)
				return false;
		}

	return true;
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10



/*
=================
=
= T_Projectile
=
=================
*/

void T_Projectile (objtype *ob)
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

	deltax = LABS(ob->x - player->x);
	deltay = LABS(ob->y - player->y);

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (!TryMove (ob))
#else
	if (!ProjectileTryMove (ob))
#endif
	{
		// *** SHAREWARE V1.0+1.1 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		PlaySoundLocActor(MISSILEHITSND,ob);
		ob->state = &s_cocohit;
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		RemoveObj(ob);
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
		ob->state = NULL;
		PlaySoundLocActor(MISSILEHITSND,ob);
#else
		if (ob->obclass == rocketobj)
		{
			PlaySoundLocActor(MISSILEHITSND,ob);
			ob->state = &s_boom1;
		}
#ifdef SPEAR
		else if (ob->obclass == hrocketobj)
		{
			PlaySoundLocActor(MISSILEHITSND,ob);
			ob->state = &s_hboom1;
		}
#endif
		else
			ob->state = NULL;		// mark for removal
#endif // RESTORATION

		return;
	}

	if (deltax < PROJECTILESIZE && deltay < PROJECTILESIZE)
	{	// hit the player
		switch (ob->obclass)
		{
		case needleobj:
			damage = (US_RndT() >>3) + 20;
			break;
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		case rocketobj:
		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
		case hrocketobj:
		case sparkobj:
#endif
			damage = (US_RndT() >>3) + 30;
			break;
#endif
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		case fireobj:
			damage = (US_RndT() >>3);
			break;
#endif
		}

		TakeDamage (damage,ob);
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		RemoveObj(ob);
#else
		ob->state = NULL;		// mark for removal
#endif
		return;
	}

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	ob->tilex = ob->x >> TILESHIFT;
	ob->tiley = ob->y >> TILESHIFT;
#endif

}

