/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2021-2025 NY00123
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

#include "wl_def.h"
#include "wl_dobjl.h"

REFKEEN_NS_B

/*
REFKEEN: doorobjlist struct accessors used in case of buffer
overruns/overflows. This is an attempt to emulate vanilla bugs.
in all original DOS exes inspected as of writing this, the doorobjlist
array is followed by laststatobj, and then by statobjlist.

ASSUMPTION: MAXDOORS <= door < 128, and offset >= 0
*/

#if REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS

#ifdef GAMEVER_NOAH3D
#define DOOROBJ_SIZE 12
#define DOOROBJ_EXTRA_OFFSET 2
#else
#define DOOROBJ_SIZE 10
#define DOOROBJ_EXTRA_OFFSET 0
#endif

#define STATOBJ_SIZE 8

static id0_byte_t DoorObjList_GetByte(id0_int_t door, int offset)
{
	if (door == MAXDOORS)
	{
		if (offset == 0)
			return COMPAT_STATOBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(laststatobj)&0xFF;
		else if (offset == 1)
			return COMPAT_STATOBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(laststatobj)>>8;
	}

	int statobjlist_offset = (door - MAXDOORS) * DOOROBJ_SIZE + offset - 2;
	int statobj_num = statobjlist_offset / STATOBJ_SIZE;
	if (statobj_num >= MAXSTATS)
		Quit("DoorObjList_GetByte: Unimplemented due to overflow across statobjlist");

	switch (statobjlist_offset % STATOBJ_SIZE)
	{
	case 0: return statobjlist[statobj_num].tilex;
	case 1: return statobjlist[statobj_num].tiley;
	case 2: return COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(statobjlist[statobj_num].visspot)&0xFF;
	case 3: return COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(statobjlist[statobj_num].visspot)>>8;
	case 4: return statobjlist[statobj_num].shapenum & 0xFF;
	case 5: return (id0_word_t)statobjlist[statobj_num].shapenum >> 8;
	case 6: return statobjlist[statobj_num].flags;
	case 7: return statobjlist[statobj_num].itemnumber;
	}
	return 0;
}

static void DoorObjList_SetByte(id0_int_t door, int offset, id0_byte_t val)
{

	if ((door == MAXDOORS) && (offset < 2))
	{
		id0_word_t dos_ptr = COMPAT_STATOBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(laststatobj);
		id0_word_t shift = 8 * offset;
		dos_ptr &= ~(0xFF << shift);
		dos_ptr |= (val << shift);
		laststatobj = COMPAT_STATOBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(dos_ptr);
		return;
	}

	int statobjlist_offset = (door - MAXDOORS) * DOOROBJ_SIZE + offset - 2;
	int statobj_num = statobjlist_offset / STATOBJ_SIZE;
	if (statobj_num >= MAXSTATS)
		Quit("DoorObjList_SetByte: Unimplemented due to overflow across statobjlist");
	switch (statobjlist_offset % STATOBJ_SIZE)
	{
	case 0: statobjlist[statobj_num].tilex = val; break;
	case 1: statobjlist[statobj_num].tiley = val; break;

	case 2:
	case 3:
	{
		id0_word_t dos_ptr = COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(statobjlist[statobj_num].visspot);
		id0_word_t shift = 8*(statobjlist_offset % STATOBJ_SIZE - 2);
		dos_ptr &= ~(0xFF << shift);
		dos_ptr |= (val << shift);
		statobjlist[statobj_num].visspot = COMPAT_SPOTVIS_CONVERT_DOS_PTR_TO_SPOT_PTR(dos_ptr);
		break;
	}

	case 4: statobjlist[statobj_num].shapenum = (statobjlist[statobj_num].shapenum & 0xFF00) | val; break;
	case 5: statobjlist[statobj_num].shapenum = (statobjlist[statobj_num].shapenum & 0xFF) | (val << 8); break;
	case 6: statobjlist[statobj_num].flags = val; break;
	case 7: statobjlist[statobj_num].itemnumber = val; break;
	}
}

static id0_int_t DoorObjList_GetInt(id0_int_t door, int offset)
{
	if ((door == MAXDOORS) && (offset == 0))
		return COMPAT_STATOBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(laststatobj);

	int statobjlist_offset = (door - MAXDOORS) * DOOROBJ_SIZE + offset - 2;
	int statobj_num = statobjlist_offset / STATOBJ_SIZE;
	if (statobj_num >= MAXSTATS)
		Quit("DoorObjList_GetInt: Unimplemented due to overflow across statobjlist");

	switch (statobjlist_offset % STATOBJ_SIZE)
	{
	case 0: return statobjlist[statobj_num].tilex | (statobjlist[statobj_num].tiley << 8);
	case 2: return COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(statobjlist[statobj_num].visspot);
	case 4: return statobjlist[statobj_num].shapenum;
	case 6: return statobjlist[statobj_num].flags | (statobjlist[statobj_num].itemnumber << 8);
	}
	return 0;
}

static void DoorObjList_SetInt(id0_int_t door, int offset, id0_int_t val)
{
	if ((door == MAXDOORS) && (offset == 0))
	{
		laststatobj = COMPAT_STATOBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(val);
		return;
	}

	int statobjlist_offset = (door - MAXDOORS) * DOOROBJ_SIZE + offset - 2;
	int statobj_num = statobjlist_offset / STATOBJ_SIZE;
	if (statobj_num >= MAXSTATS)
		Quit("DoorObjList_SetInt: Unimplemented due to overflow across statobjlist");

	switch (statobjlist_offset % STATOBJ_SIZE)
	{
	case 0: statobjlist[statobj_num].tilex = val & 0xFF; statobjlist[statobj_num].tiley = (id0_word_t)val >> 8; break;
	case 2: statobjlist[statobj_num].visspot = COMPAT_SPOTVIS_CONVERT_DOS_PTR_TO_SPOT_PTR(val); break;
	case 4: statobjlist[statobj_num].shapenum = val; break;
	case 6: statobjlist[statobj_num].flags = val & 0xFF; statobjlist[statobj_num].itemnumber = (id0_word_t)val >> 8; break;
	}
}

id0_byte_t DoorObjList_GetTileX(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].tilex;
	return DoorObjList_GetByte(door, 0);
}

#if 0
void DoorObjList_SetTileX(id0_int_t door, id0_byte_t tilex)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].tilex = tilex;
	else
		DoorObjList_SetByte(door, 0, tilex);
}
#endif

id0_byte_t DoorObjList_GetTileY(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].tiley;
	return DoorObjList_GetByte(door, 1);
}

#if 0
void DoorObjList_SetTileY(id0_int_t door, id0_byte_t tiley)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].tiley = tiley;
	else
		DoorObjList_SetByte(door, 1, tiley);
}
#endif

id0_boolean_t DoorObjList_GetVertical(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].vertical;
	return DoorObjList_GetInt(door, 2);
}

#if 0
void DoorObjList_SetVertical(id0_int_t door, id0_boolean_t vertical)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].vertical = vertical;
	else
		DoorObjList_SetInt(door, 2, vertical);
}
#endif

#ifdef GAMEVER_NOAH3D
id0_boolean_t DoorObjList_GetSeen(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].seen;
	return DoorObjList_GetInt(door, 4);
}

void DoorObjList_SetSeen(id0_int_t door, id0_boolean_t seen)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].seen = seen;
	else
		DoorObjList_SetInt(door, 4, seen);
}
#endif

id0_byte_t DoorObjList_GetLock(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].lock;
	return DoorObjList_GetByte(door, 4 + DOOROBJ_EXTRA_OFFSET);
}

void DoorObjList_SetLock(id0_int_t door, id0_byte_t lock)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].lock = lock;
	else
		DoorObjList_SetByte(door, 4 + DOOROBJ_EXTRA_OFFSET, lock);
}

id0_int_t DoorObjList_GetAction(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].action;
	return DoorObjList_GetInt(door, 6 + DOOROBJ_EXTRA_OFFSET);
}

void DoorObjList_SetAction(id0_int_t door, dooraction_t action)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].action = action;
	else
		DoorObjList_SetInt(door, 6 + DOOROBJ_EXTRA_OFFSET, action);
}

#if 0
id0_int_t DoorObjList_GetTicCount(id0_int_t door)
{
	if ((door >= 0) && (door < MAXDOORS))
		return doorobjlist[door].ticcount;
	return DoorObjList_GetInt(door, 8 + DOOROBJ_EXTRA_OFFSET);

}
#endif

void DoorObjList_SetTicCount(id0_int_t door, id0_int_t ticcount)
{
	if ((door >= 0) && (door < MAXDOORS))
		doorobjlist[door].ticcount = ticcount;
	else
		DoorObjList_SetInt(door, 8 + DOOROBJ_EXTRA_OFFSET, ticcount);
}

#endif // REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS

REFKEEN_NS_E
