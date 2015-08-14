/*** S3DNA RESTORATION - Recreated automap source file ***/

#include "WL_DEF.H"
#pragma hdrstop

void DrawAutomapTiles (int tilex, int tiley)
{
	int	x,y,tile;

	for (y=tiley;y<tiley+20;y++)
		for (x=tilex;x<tilex+40;x++)
		{
			tile = tilemap[x][y];
			if (gamestate.fullmap || (tile&0x20) || ((tile&0x80) && doorobjlist[tile&0x1F].field_4) || ((gamestate.difficulty == gd_baby) && (tile == 15)))
			{
				tile &= 0xFFDF;
				if (tile&0x80)
				{
					if (doorobjlist[tile&0x1F].vertical)
						tile = 65;
					else
						tile = 66;
				}
				else if (tile==31)
					tile = 75;
				else
					tile = (tile&0x1F)+35;
			}
			else
				tile = 35;
			VWB_DrawTile8 (8*(x-tilex),8*(y-tiley),tile);
		}
}

void DrawAutomapStatObjs (int tilex, int tiley)
{
	statobj_t	*statptr;
	int	maptx,mapty,tile;

	for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if (statptr->shapenum == -1)
			continue;
		if (statptr->shapenum == -2)
			continue;

		maptx = statptr->tilex - tilex;
		if ((maptx < 0) || (maptx > 39))
			continue;

		mapty = statptr->tiley - tiley;
		if ((mapty < 0) || (mapty > 19))
			continue;

		switch (statptr->itemnumber)
		{
		case bo_quiz:
			tile = 82;
			break;
		case bo_firstaid:
		case bo_food:
		case bo_fullheal:
			tile = 79;
			break;
		case bo_key1:
			tile = 76;
			break;
		case bo_key2:
			tile = 77;
			break;
		case bo_clip:
		case bo_machinegun:
		case bo_chaingun:
		case bo_25clip:
		case bo_bag:
		case bo_nutsling:
		case bo_nuts:
		case bo_melonsling:
		case bo_melons:
			tile = 80;
			break;
		case bo_banana:
		case bo_apple:
		case bo_peach:
		case bo_grapes:
			tile = 81;
			break;
		case block:
			tile = 78;
			break;
		default:
			tile = 0;
		}

		if (tile)
			VWB_DrawTile8 (8*maptx,8*mapty,tile);
	}
}

void DrawAutomap (int tilex, int tiley)
{
	// TODO (RESTORATION) - Are the direction names correct?
	int	maptx,mapty;
	dirtype	dir;

	DrawAutomapTiles (tilex,tiley);
	if (gamestate.fullmap)
		DrawAutomapStatObjs (tilex,tiley);
	// draw at player's location
	maptx = player->tilex - tilex;
	if ((maptx < 0) || (maptx > 39))
		return;

	mapty = player->tiley - tiley;
	if ((mapty < 0) || (mapty > 19))
		return;

	if (player->angle <= ANGLES/16)
		dir = east;
	else if (player->angle <= 3*ANGLES/16)
		dir = northeast;
	else if (player->angle <= 5*ANGLES/16)
		dir = north;
	else if (player->angle <= 7*ANGLES/16)
		dir = northwest;
	else if (player->angle <= 9*ANGLES/16)
		dir = west;
	else if (player->angle <= 11*ANGLES/16)
		dir = southwest;
	else if (player->angle <= 13*ANGLES/16)
		dir = south;
	else if (player->angle <= 15*ANGLES/16)
		dir = southeast;
	else
		dir = east;

	VWB_DrawTile8 (8*maptx,8*mapty,dir+67);
}

void OpenAutomap ()
{
	int	tilex,tiley;
	boolean		quit;

	tilex = player->tilex - 20;
	if (tilex < 0)
		tilex = 0;
	else if (tilex > 24)
		tilex = 24;

	tiley = player->tiley - 10;
	if (tiley < 0)
		tiley = 0;
	else if (tiley > 44)
		tiley = 44;

	ClearMemory ();
	VW_FadeOut ();
	DrawAutomap (tilex,tiley);
	VW_UpdateScreen ();
	VW_FadeIn ();

	quit = false;
	do
	{
		PollControls ();

		if ((controlx < 0) && (tilex > 0))
			tilex--;
		else if ((controlx > 0) && (tilex+39 < MAPSIZE-1))
			tilex++;

		if ((controly < 0) && (tiley > 0))
			tiley--;
		else if ((controly > 0) && (tiley+19 < MAPSIZE-1))
			tiley++;

		if (Keyboard[sc_Escape] || Keyboard[sc_Tab] || buttonstate[bt_attack] || buttonstate[bt_use])
			quit = true;

		if (!quit)
		{
			DrawAutomap (tilex,tiley);
			VW_UpdateScreen ();
		}
	} while (!quit);

	IN_ClearKeysDown ();
	if ((playstate == ex_stillplaying) && !loadedgame && !startgame)
	{
		DrawPlayScreen ();
		VW_FadeIn ();
	}

	PM_CheckMainMem ();
	lasttimecount = TimeCount;

	if (MousePresent)
		Mouse(MDelta);	// Clear accumulated mouse movement
}
