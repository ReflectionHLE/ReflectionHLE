// *** PRE-V1.4 APOGEE RESTORATION *** - #include this file
// at a location in WL_ACT2.C depending on version

/*
=================
=
= A_Smoke
=
=================
*/

void A_Smoke (objtype *ob)
{
	GetNewActor ();
#ifdef SPEAR
	if (ob->obclass == hrocketobj)
		new->state = &s_hsmoke1;
	else
#endif
		new->state = &s_smoke1;
	new->ticcount = 6;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = inertobj;
	new->active = true;

	new->flags = FL_NEVERMARK;
}

