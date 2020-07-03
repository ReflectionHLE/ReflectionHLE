/*** S3DNA RESTORATION - Recreated automap source file ***/

#include "wl_def.h"
#pragma hdrstop

static void DrawMapWalls (int tx, int ty)
{
	int	x,y,tile;

	for (y=ty;y<ty+20;y++)
		for (x=tx;x<tx+40;x++)
		{
			tile = tilemap[x][y];
			if (gamestate.automap ||
			    (tile&0x20) || ((tile&0x80) && doorobjlist[tile&0x1F].seen) || ((gamestate.difficulty == gd_baby) && (tile == 15)))
			{
				tile &= 0xFFDF;
				if (tile&0x80)
				{
					if (doorobjlist[tile&0x1F].vertical)
						tile = 65;
					else
						tile = 66;
				}
				else if (tile == 31)
					tile = 75;
				else
					tile = (tile&0x1F)+35;
			}
			else
				tile = 35;
			VWB_DrawTile8 (8*(x-tx),8*(y-ty),tile);
		}
}

static void DrawMapPrizes (int tx, int ty)
{
	int	x,y,tile;
	statobj_t	*statptr;

	for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if (statptr->shapenum == -1)
			continue;
		if (statptr->shapenum == -2)
			continue;

		x = statptr->tilex - tx;
		if ((x < 0) || (x > 39))
			continue;

		y = statptr->tiley - ty;
		if ((y < 0) || (y > 19))
			continue;

		switch (statptr->itemnumber)
		{
		case bo_alpo:
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
		case bo_bandolier:
		case bo_flamethrower:
		case bo_gascan:
		case bo_launcher:
		case bo_missiles:
			tile = 80;
			break;
		case bo_cross:
		case bo_chalice:
		case bo_crown:
		case bo_chest:
			tile = 81;
			break;
		case block:
			tile = 78;
			break;
		default:
			tile = 0;
			break;
		}

		if (tile)
			VWB_DrawTile8 (8*x,8*y,tile);
	}
}

static void DrawAutoMap (int tx, int ty)
{
	int	px,py;
	int	dir;

	DrawMapWalls (tx,ty);
	if (gamestate.automap)
		DrawMapPrizes (tx,ty);
	// draw at player's location
	px = player->tilex - tx;
	if ((px < 0) || (px > 39))
		return;

	py = player->tiley - ty;
	if ((py < 0) || (py > 19))
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

	VWB_DrawTile8 (8*px,8*py,dir+67);
}

void AutoMap (void)
{
	int	x,y;
	boolean		done;

	x = player->tilex - 20;
	if (x < 0)
		x = 0;
	else if (x > 24)
		x = 24;

	y = player->tiley - 10;
	if (y < 0)
		y = 0;
	else if (y > 44)
		y = 44;

	ClearMemory ();
	VW_FadeOut ();
	DrawAutoMap (x,y);
	VW_UpdateScreen ();
	VW_FadeIn ();

	done = false;
	do
	{
		PollControls ();

		if ((controlx < 0) && (x > 0))
			x--;
		else if ((controlx > 0) && (x+39 < MAPSIZE-1))
			x++;

		if ((controly < 0) && (y > 0))
			y--;
		else if ((controly > 0) && (y+19 < MAPSIZE-1))
			y++;

		if (Keyboard[sc_Escape] || Keyboard[sc_Tab] ||
		    buttonstate[bt_attack] || buttonstate[bt_use])
			done = true;

		if (!done)
		{
			DrawAutoMap (x,y);
			VW_UpdateScreen ();
		}
	} while (!done);

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
