/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2014-2020 NY00123
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

// New file used for writing/reading saved games while preserving compatibility

#include "wl_def.h"

REFKEEN_NS_B

BE_CROSS_IMPLEMENT_FP_READWRITE_S16LE_FUNCS(activetype)
BE_CROSS_IMPLEMENT_FP_READWRITE_U16LE_FUNCS(classtype)
BE_CROSS_IMPLEMENT_FP_READWRITE_U16LE_FUNCS(dirtype)
BE_CROSS_IMPLEMENT_FP_READWRITE_S16LE_FUNCS(weapontype)
BE_CROSS_IMPLEMENT_FP_READWRITE_U16LE_FUNCS(dooraction_t)

extern statetype * (*RefKeen_GetObjStatePtrFromDOSPointer)(uint_fast32_t dosptr); // TODO move this?

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#define USE_CHECKSUMS 1
#endif

#if USE_CHECKSUMS
id0_long_t DoChecksum(id0_byte_t id0_far *source,id0_unsigned_t size,id0_long_t checksum);

static id0_long_t ChecksumInt8(id0_byte_t val, id0_long_t checksum, id0_byte_t *lastByte)
{
	checksum += (*lastByte)^val;
	*lastByte = val;
	return checksum;
}

static id0_long_t ChecksumInt16(id0_word_t val, id0_long_t checksum, id0_byte_t *lastByte)
{
	checksum += (*lastByte)^(val&255);
	*lastByte = val>>8;
	return DoChecksum((id0_byte_t *)&val, 2, checksum);
}

static id0_long_t ChecksumInt32(id0_longword_t val, id0_long_t checksum, id0_byte_t *lastByte)
{
	checksum += (*lastByte)^(val&255);
	*lastByte = val>>24;
	return DoChecksum((id0_byte_t *)&val, 4, checksum);
}

id0_long_t ChecksumInt16Buffer(id0_word_t a[], int n, id0_long_t checksum)
{
	id0_byte_t lastByte = a[0] & 0xFF; // Cancel out first byte
	for (int i = 0; i < n; ++i)
		checksum = ChecksumInt16(a[i], checksum, &lastByte);
	return checksum;
}
#endif // USE_CHECKSUMS

static id0_long_t WriteAndChecksumInt8LE(
	BE_FILE_T file, const id0_byte_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	BE_Cross_writeInt8LE(file, ptr);
	return ChecksumInt8(*ptr, checksum, lastByte);
}

static id0_long_t ReadAndChecksumInt8LE(
	BE_FILE_T file, id0_byte_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	if (BE_Cross_readInt8LE(file, ptr) == 1)
		checksum = ChecksumInt8(*ptr, checksum, lastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumInt16LE(
	BE_FILE_T file, const id0_int_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	BE_Cross_writeInt16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte);
}

static id0_long_t ReadAndChecksumInt16LE(
	BE_FILE_T file, id0_int_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	if (BE_Cross_readInt16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumBooleanFrom16LE(
	BE_FILE_T file, const id0_boolean_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	BE_Cross_write_boolean_To16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte);
}

static id0_long_t ReadAndChecksumBooleanFrom16LE(
	BE_FILE_T file, id0_boolean_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	if (BE_Cross_read_boolean_From16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumDoorActionFrom16LE(
	BE_FILE_T file, const dooraction_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	BE_Cross_write_dooraction_t_ToU16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte);
}

static id0_long_t ReadAndChecksumDoorActionFrom16LE(
	BE_FILE_T file, dooraction_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte)
{
	if (BE_Cross_read_dooraction_t_FromU16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte);
	return checksum;
}

id0_boolean_t SaveObject(BE_FILE_T file, const objtype *o)
{
	// BACKWARDS COMPATIBILITY
	id0_byte_t padding = 0;
	id0_word_t statedosoffset = o->state ? o->state->compatdospointer : 0;
	id0_word_t nextdosoffset = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(o->next);
	id0_word_t prevdosoffset = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(o->prev);
	// Now writing
	return ((BE_Cross_write_activetype_ToS16LE(file, &o->active) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->ticcount) == 2)
	        && (BE_Cross_write_classtype_ToU16LE(file, &o->obclass) == 2)
	        && (BE_Cross_writeInt16LE(file, &statedosoffset) == 2) // BACKWARDS COMPATIBILITY
	        && (BE_Cross_writeInt8LE(file, &o->flags) == 1)
	        && (BE_Cross_writeInt8LE(file, &padding) == 1)
	        && (BE_Cross_writeInt32LE(file, &o->distance) == 4)
	        && (BE_Cross_write_dirtype_ToU16LE(file, &o->dir) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->x) == 4)
	        && (BE_Cross_writeInt32LE(file, &o->y) == 4)
	        && (BE_Cross_writeInt16LE(file, &o->tilex) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tiley) == 2)
	        && (BE_Cross_writeInt8LE(file, &o->areanumber) == 1)
#ifdef GAMEVER_NOAH3D
	        && (BE_Cross_writeInt8LE(file, &o->snore) == 1)
#else
	        && (BE_Cross_writeInt8LE(file, &padding) == 1)
#endif
	        && (BE_Cross_writeInt16LE(file, &o->viewx) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->viewheight) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->transx) == 4)
	        && (BE_Cross_writeInt32LE(file, &o->transy) == 4)
	        && (BE_Cross_writeInt16LE(file, &o->angle) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->hitpoints) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->speed) == 4)
#ifdef GAMEVER_NOAH3D
	        && (BE_Cross_writeInt16LE(file, &o->temp2) == 2)
#else
	        && (BE_Cross_writeInt16LE(file, &o->temp1) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp2) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp3) == 2)
#endif
	        // BACKWARDS COMPATIBILITY
	        && (BE_Cross_writeInt16LE(file, &nextdosoffset) == 2)
	        && (BE_Cross_writeInt16LE(file, &prevdosoffset) == 2)
	);
}

id0_boolean_t LoadObject(BE_FILE_T file, objtype *o)
{
	// BACKWARDS COMPATIBILITY
	id0_byte_t padding;
	id0_word_t statedosoffset, nextdosoffset, prevdosoffset;
	// Now reading
	if ((BE_Cross_read_activetype_FromS16LE(file, &o->active) != 2)
	    || (BE_Cross_readInt16LE(file, &o->ticcount) != 2)
	    || (BE_Cross_read_classtype_FromU16LE(file, &o->obclass) != 2)
	    || (BE_Cross_readInt16LE(file, &statedosoffset) != 2) // BACKWARDS COMPATIBILITY
	    || (BE_Cross_readInt8LE(file, &o->flags) != 1)
	    || (BE_Cross_readInt8LE(file, &padding) != 1)
	    || (BE_Cross_readInt32LE(file, &o->distance) != 4)
	    || (BE_Cross_read_dirtype_FromU16LE(file, &o->dir) != 2)
	    || (BE_Cross_readInt32LE(file, &o->x) != 4)
	    || (BE_Cross_readInt32LE(file, &o->y) != 4)
	    || (BE_Cross_readInt16LE(file, &o->tilex) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tiley) != 2)
	    || (BE_Cross_readInt8LE(file, &o->areanumber) != 1)
#ifdef GAMEVER_NOAH3D
	    || (BE_Cross_readInt8LE(file, &o->snore) != 1)
#else
	    || (BE_Cross_readInt8LE(file, &padding) != 1)
#endif
	    || (BE_Cross_readInt16LE(file, &o->viewx) != 2)
	    || (BE_Cross_readInt16LE(file, &o->viewheight) != 2)
	    || (BE_Cross_readInt32LE(file, &o->transx) != 4)
	    || (BE_Cross_readInt32LE(file, &o->transy) != 4)
	    || (BE_Cross_readInt16LE(file, &o->angle) != 2)
	    || (BE_Cross_readInt16LE(file, &o->hitpoints) != 2)
	    || (BE_Cross_readInt32LE(file, &o->speed) != 4)
#ifdef GAMEVER_NOAH3D
	    || (BE_Cross_readInt16LE(file, &o->temp2) != 2)
#else
	    || (BE_Cross_readInt16LE(file, &o->temp1) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp2) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp3) != 2)
#endif
	        // BACKWARDS COMPATIBILITY
	    || (BE_Cross_readInt16LE(file, &nextdosoffset) != 2)
	    || (BE_Cross_readInt16LE(file, &prevdosoffset) != 2)
	)
		return false;

	o->state = RefKeen_GetObjStatePtrFromDOSPointer(statedosoffset);
	o->next = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(nextdosoffset);
	o->prev = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(prevdosoffset);
	return true;
}

id0_boolean_t SaveGameState(BE_FILE_T file, const gametype *state)
{
	return ((BE_Cross_writeInt16LE(file, &state->difficulty) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->mapon) == 2)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	        && (BE_Cross_writeInt32LE(file, &state->oldscore) == 4)
#endif
	        && (BE_Cross_writeInt32LE(file, &state->score) == 4)
	        && (BE_Cross_writeInt16LE(file, &state->nextextra) == 4)
	        && (BE_Cross_writeInt32LE(file, &state->lives) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->health) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->ammo) == 2)
#ifdef GAMEVER_NOAH3D
	        && (BE_Cross_writeInt16LE(file, &state->treasure) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->maxammo) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->gas) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->missiles) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->machinegun) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->chaingun) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->missile) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->flamethrower) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &state->automap) == 2)
#endif
	        && (BE_Cross_writeInt16LE(file, &state->keys) == 2)
	        && (BE_Cross_write_weapontype_ToS16LE(file, &state->bestweapon) == 2)
	        && (BE_Cross_write_weapontype_ToS16LE(file, &state->weapon) == 2)
	        && (BE_Cross_write_weapontype_ToS16LE(file, &state->chosenweapon) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->faceframe) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->attackframe) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->attackcount) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->weaponframe) == 2)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef GAMEVER_NOAH3D
	        && (BE_Cross_writeInt16LE(file, &state->episode) == 2)
#endif
	        && (BE_Cross_writeInt16LE(file, &state->secretcount) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->treasurecount) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->killcount) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->secrettotal) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->treasuretotal) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->killtotal) == 2)
	        && (BE_Cross_writeInt32LE(file, &state->TimeCount) == 4)
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	        && (BE_Cross_writeInt32LE(file, &state->killx) == 4)
	        && (BE_Cross_writeInt32LE(file, &state->killy) == 4)
#endif
	        && (BE_Cross_write_boolean_To16LE(file, &state->victoryflag) == 2)
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	);
}

id0_boolean_t LoadGameState(BE_FILE_T file, gametype *state)
{
	return ((BE_Cross_readInt16LE(file, &state->difficulty) == 2)
	        && (BE_Cross_readInt16LE(file, &state->mapon) == 2)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	        && (BE_Cross_readInt32LE(file, &state->oldscore) == 4)
#endif
	        && (BE_Cross_readInt32LE(file, &state->score) == 4)
	        && (BE_Cross_readInt32LE(file, &state->nextextra) == 4)
	        && (BE_Cross_readInt16LE(file, &state->lives) == 2)
	        && (BE_Cross_readInt16LE(file, &state->health) == 2)
	        && (BE_Cross_readInt16LE(file, &state->ammo) == 2)
#ifdef GAMEVER_NOAH3D
	        && (BE_Cross_readInt16LE(file, &state->treasure) == 2)
	        && (BE_Cross_readInt16LE(file, &state->maxammo) == 2)
	        && (BE_Cross_readInt16LE(file, &state->gas) == 2)
	        && (BE_Cross_readInt16LE(file, &state->missiles) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->machinegun) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->chaingun) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->missile) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->flamethrower) == 2)
	        && (BE_Cross_read_boolean_From16LE(file, &state->automap) == 2)
#endif
	        && (BE_Cross_readInt16LE(file, &state->keys) == 2)
	        && (BE_Cross_read_weapontype_FromS16LE(file, &state->bestweapon) == 2)
	        && (BE_Cross_read_weapontype_FromS16LE(file, &state->weapon) == 2)
	        && (BE_Cross_read_weapontype_FromS16LE(file, &state->chosenweapon) == 2)
	        && (BE_Cross_readInt16LE(file, &state->faceframe) == 2)
	        && (BE_Cross_readInt16LE(file, &state->attackframe) == 2)
	        && (BE_Cross_readInt16LE(file, &state->attackcount) == 2)
	        && (BE_Cross_readInt16LE(file, &state->weaponframe) == 2)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef GAMEVER_NOAH3D
	        && (BE_Cross_readInt16LE(file, &state->episode) == 2)
#endif
	        && (BE_Cross_readInt16LE(file, &state->secretcount) == 2)
	        && (BE_Cross_readInt16LE(file, &state->treasurecount) == 2)
	        && (BE_Cross_readInt16LE(file, &state->killcount) == 2)
	        && (BE_Cross_readInt16LE(file, &state->secrettotal) == 2)
	        && (BE_Cross_readInt16LE(file, &state->treasuretotal) == 2)
	        && (BE_Cross_readInt16LE(file, &state->killtotal) == 2)
	        && (BE_Cross_readInt32LE(file, &state->TimeCount) == 4)
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	        && (BE_Cross_readInt32LE(file, &state->killx) == 4)
	        && (BE_Cross_readInt32LE(file, &state->killy) == 4)
#endif
	        && (BE_Cross_read_boolean_From16LE(file, &state->victoryflag) == 2)
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	);
}

#if USE_CHECKSUMS
id0_long_t ChecksumGameState(const gametype *state, id0_long_t checksum)
{
	id0_byte_t b = state->difficulty & 0xFF; // Cancel out first byte
	checksum = ChecksumInt16(state->difficulty, checksum, &b);
	checksum = ChecksumInt16(state->mapon, checksum, &b);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	checksum = ChecksumInt32(state->oldscore, checksum, &b);
#endif
	checksum = ChecksumInt32(state->score, checksum, &b);
	checksum = ChecksumInt32(state->nextextra, checksum, &b);
	checksum = ChecksumInt16(state->lives, checksum, &b);
	checksum = ChecksumInt16(state->health, checksum, &b);
	checksum = ChecksumInt16(state->ammo, checksum, &b);
#ifdef GAMEVER_NOAH3D
	checksum = ChecksumInt16(state->treasure, checksum, &b);
	checksum = ChecksumInt16(state->ammo, checksum, &b);
	checksum = ChecksumInt16(state->gas, checksum, &b);
	checksum = ChecksumInt16(state->missiles, checksum, &b);
	checksum = ChecksumInt16(state->machinegun, checksum, &b);
	checksum = ChecksumInt16(state->chaingun, checksum, &b);
	checksum = ChecksumInt16(state->missile, checksum, &b);
	checksum = ChecksumInt16(state->flamethrower, checksum, &b);
	checksum = ChecksumInt16(state->automap, checksum, &b);
#endif
	checksum = ChecksumInt16(state->keys, checksum, &b);
	checksum = ChecksumInt16(state->bestweapon, checksum, &b);
	checksum = ChecksumInt16(state->weapon, checksum, &b);
	checksum = ChecksumInt16(state->chosenweapon, checksum, &b);
	checksum = ChecksumInt16(state->faceframe, checksum, &b);
	checksum = ChecksumInt16(state->attackframe, checksum, &b);
	checksum = ChecksumInt16(state->attackcount, checksum, &b);
	checksum = ChecksumInt16(state->weaponframe, checksum, &b);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef GAMEVER_NOAH3D
	checksum = ChecksumInt16(state->episode, checksum, &b);
#endif
	checksum = ChecksumInt16(state->secretcount, checksum, &b);
	checksum = ChecksumInt16(state->treasurecount, checksum, &b);
	checksum = ChecksumInt16(state->killcount, checksum, &b);
	checksum = ChecksumInt16(state->secrettotal, checksum, &b);
	checksum = ChecksumInt16(state->treasuretotal, checksum, &b);
	checksum = ChecksumInt16(state->killtotal, checksum, &b);
	checksum = ChecksumInt32(state->TimeCount, checksum, &b);
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = ChecksumInt32(state->killx, checksum, &b);
	checksum = ChecksumInt32(state->killy, checksum, &b);
#endif
	checksum = ChecksumInt16(state->victoryflag, checksum, &b);
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	return checksum;
}
#endif // USE_CHECKSUMS

id0_boolean_t SaveLevelRatios(BE_FILE_T file, const LRstruct *ratios, int n)
{
	for (int i = 0; i < n; ++i)
		if ((BE_Cross_writeInt16LE(file, &ratios[i].kill) != 2)
		    || (BE_Cross_writeInt16LE(file, &ratios[i].secret) != 2)
		    || (BE_Cross_writeInt16LE(file, &ratios[i].treasure) != 2)
		    || (BE_Cross_writeInt32LE(file, &ratios[i].time) != 4)
#ifdef GAMEVER_NOAH3D
		    || (BE_Cross_writeInt32LE(file, &ratios[i].par) != 4)
#endif
		)
			return false;
	return true;
}

id0_boolean_t LoadLevelRatios(BE_FILE_T file, LRstruct *ratios, int n)
{
	for (int i = 0; i < n; ++i)
		if ((BE_Cross_readInt16LE(file, &ratios[i].kill) != 2)
		    || (BE_Cross_readInt16LE(file, &ratios[i].secret) != 2)
		    || (BE_Cross_readInt16LE(file, &ratios[i].treasure) != 2)
		    || (BE_Cross_readInt32LE(file, &ratios[i].time) != 4)
#ifdef GAMEVER_NOAH3D
		    || (BE_Cross_readInt32LE(file, &ratios[i].par) != 4)
#endif
		)
			return false;
	return true;
}

#if USE_CHECKSUMS
id0_long_t ChecksumLevelRatios(const LRstruct *ratios, int n, id0_long_t checksum)
{
	id0_byte_t b = ratios[0].kill & 0xFF; // Cancel out first byte
	for (int i = 0; i < n; ++i)
	{
		checksum = ChecksumInt16(ratios[i].kill, checksum, &b);
		checksum = ChecksumInt16(ratios[i].secret, checksum, &b);
		checksum = ChecksumInt16(ratios[i].treasure, checksum, &b);
		checksum = ChecksumInt32(ratios[i].time, checksum, &b);
#ifdef GAMEVER_NOAH3D
		checksum = ChecksumInt32(ratios[i].par, checksum, &b);
#endif
	}
	return checksum;
}
#endif

id0_boolean_t SaveStatObjects(BE_FILE_T file, const statobj_t *stats, int n)
{
	for (int i = 0; i < n; ++i)
	{
		// BACKWARDS COMPATIBILITY
		id0_word_t visspotdosoffset = COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(stats[i].visspot);
		// Now writing
		if ((BE_Cross_writeInt8LE(file, &stats[i].tilex) != 1)
		    || (BE_Cross_writeInt8LE(file, &stats[i].tiley) != 1)
		    || (BE_Cross_writeInt16LE(file, &visspotdosoffset) != 2) // BACKWARDS COMPATIBILITY
		    || (BE_Cross_writeInt16LE(file, &stats[i].shapenum) != 2)
		    || (BE_Cross_writeInt8LE(file, &stats[i].flags) != 1)
		    || (BE_Cross_writeInt8LE(file, &stats[i].itemnumber) != 1)
		)
			return false;
	}
	return true;
}

id0_boolean_t LoadStatObjects(BE_FILE_T file, statobj_t *stats, int n)
{
	for (int i = 0; i < n; ++i)
	{
		// BACKWARDS COMPATIBILITY
		id0_word_t visspotdosoffset;
		// Now reading
		if ((BE_Cross_readInt8LE(file, &stats[i].tilex) != 1)
		    || (BE_Cross_readInt8LE(file, &stats[i].tiley) != 1)
		    || (BE_Cross_readInt16LE(file, &visspotdosoffset) != 2) // BACKWARDS COMPATIBILITY
		    || (BE_Cross_readInt16LE(file, &stats[i].shapenum) != 2)
		    || (BE_Cross_readInt8LE(file, &stats[i].flags) != 1)
		    || (BE_Cross_readInt8LE(file, &stats[i].itemnumber) != 1)
		)
			return false;

		stats[i].visspot = COMPAT_SPOTVIS_CONVERT_DOS_PTR_TO_SPOT_PTR(visspotdosoffset);
	}
	return true;
}

#if USE_CHECKSUMS
id0_long_t ChecksumStatObjects(const statobj_t *stats, int n, id0_long_t checksum)
{
	id0_byte_t b = stats[0].tilex & 0xFF; // Cancel out first byte
	for (int i = 0; i < n; ++i)
	{
		checksum = ChecksumInt8(stats[i].tilex, checksum, &b);
		checksum = ChecksumInt8(stats[i].tiley, checksum, &b);
		checksum = ChecksumInt16(COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(stats[i].visspot), checksum, &b); // BACKWARDS COMPATIBILITY
		checksum = ChecksumInt16(stats[i].shapenum, checksum, &b);
		checksum = ChecksumInt8(stats[i].flags, checksum, &b);
		checksum = ChecksumInt8(stats[i].itemnumber, checksum, &b);
	}
	return checksum;
}
#endif

id0_long_t SaveAndChecksumDoorObjects(BE_FILE_T file, const doorobj_t *doors, int n, id0_long_t checksum)
{
	id0_byte_t lastByte = doors[0].tilex; // Cancel out first byte
	id0_byte_t padding = 0;
	for (int i = 0; i < n; ++i)
	{
		checksum = WriteAndChecksumInt8LE(file, &doors[i].tilex, checksum, &lastByte);
		checksum = WriteAndChecksumInt8LE(file, &doors[i].tiley, checksum, &lastByte);
		checksum = WriteAndChecksumBooleanFrom16LE(file, &doors[i].vertical, checksum, &lastByte);
#ifdef GAMEVER_NOAH3D
		checksum = WriteAndChecksumBooleanFrom16LE(file, &doors[i].seen, checksum, &lastByte);
#endif
		checksum = WriteAndChecksumInt8LE(file, &doors[i].lock, checksum, &lastByte);
		checksum = WriteAndChecksumInt8LE(file, &padding, checksum, &lastByte);
		checksum = WriteAndChecksumDoorActionFrom16LE(file, &doors[i].action, checksum, &lastByte);
		checksum = WriteAndChecksumInt16LE(file, &doors[i].ticcount, checksum, &lastByte);
	}
	return checksum;
}

id0_long_t LoadAndChecksumDoorObjects(BE_FILE_T file, doorobj_t *doors, int n, id0_long_t checksum)
{
	id0_byte_t lastByte = doors[0].tilex; // Cancel out first byte
	id0_byte_t padding;
	for (int i = 0; i < n; ++i)
	{
		checksum = ReadAndChecksumInt8LE(file, &doors[i].tilex, checksum, &lastByte);
		checksum = ReadAndChecksumInt8LE(file, &doors[i].tiley, checksum, &lastByte);
		checksum = ReadAndChecksumBooleanFrom16LE(file, &doors[i].vertical, checksum, &lastByte);
#ifdef GAMEVER_NOAH3D
		checksum = ReadAndChecksumBooleanFrom16LE(file, &doors[i].seen, checksum, &lastByte);
#endif
		checksum = ReadAndChecksumInt8LE(file, &doors[i].lock, checksum, &lastByte);
		checksum = ReadAndChecksumInt8LE(file, &padding, checksum, &lastByte);
		checksum = ReadAndChecksumDoorActionFrom16LE(file, &doors[i].action, checksum, &lastByte);
		checksum = ReadAndChecksumInt16LE(file, &doors[i].ticcount, checksum, &lastByte);
	}
	return checksum;
}

REFKEEN_NS_E
