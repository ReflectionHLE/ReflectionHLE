/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2014-2025 NY00123
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

extern statetype * (*RefKeen_GetObjStatePtrFromDOSPointer)(uint_fast32_t dosptr);

id0_long_t DoChecksum(id0_byte_t id0_far *source,id0_unsigned_t size,id0_long_t checksum);

static id0_long_t ChecksumInt8(id0_byte_t val, id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (xorLastByte)
		checksum += (*lastByte)^val;
	*lastByte = val;
	return checksum;
}

static id0_long_t ChecksumInt16(id0_word_t val, id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (xorLastByte)
		checksum += (*lastByte)^(val&255);
	*lastByte = val>>8;
	return DoChecksum((id0_byte_t *)&val, 2, checksum);
}

static id0_long_t ChecksumInt32(id0_longword_t val, id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (xorLastByte)
		checksum += (*lastByte)^(val&255);
	*lastByte = val>>24;
	return DoChecksum((id0_byte_t *)&val, 4, checksum);
}

id0_long_t ChecksumInt16Buffer(const id0_word_t *a, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	for (int i = 0; i < n; ++i)
		checksum = ChecksumInt16(a[i], checksum, &lastByte, i != 0);
	return checksum;
}

static id0_long_t WriteAndChecksumInt8LE(
	BE_FILE_T file, const id0_byte_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_writeInt8LE(file, ptr);
	return ChecksumInt8(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumInt8LE(
	BE_FILE_T file, id0_byte_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_readInt8LE(file, ptr) == 1)
		checksum = ChecksumInt8(*ptr, checksum, lastByte, xorLastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumInt16LE(
	BE_FILE_T file, const id0_int_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_writeInt16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumInt16LE(
	BE_FILE_T file, id0_int_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_readInt16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumIntU16LE(
	BE_FILE_T file, const id0_word_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_writeInt16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumIntU16LE(
	BE_FILE_T file, id0_word_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_readInt16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumInt32LE(
	BE_FILE_T file, const id0_long_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_writeInt32LE(file, ptr);
	return ChecksumInt32(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumInt32LE(
	BE_FILE_T file, id0_long_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_readInt32LE(file, ptr) == 4)
		checksum = ChecksumInt32(*ptr, checksum, lastByte, xorLastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumBooleanTo16LE(
	BE_FILE_T file, const id0_boolean_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_write_boolean_To16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumBooleanFrom16LE(
	BE_FILE_T file, id0_boolean_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_read_boolean_From16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumWeaponTypeTo16LE(
	BE_FILE_T file, const weapontype *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_write_weapontype_ToS16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumWeaponTypeFrom16LE(
	BE_FILE_T file, weapontype *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_read_weapontype_FromS16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
	return checksum;
}

static id0_long_t WriteAndChecksumDoorActionFrom16LE(
	BE_FILE_T file, const dooraction_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	BE_Cross_write_dooraction_t_ToU16LE(file, ptr);
	return ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
}

static id0_long_t ReadAndChecksumDoorActionFrom16LE(
	BE_FILE_T file, dooraction_t *ptr,
	id0_long_t checksum, id0_byte_t *lastByte, bool xorLastByte)
{
	if (BE_Cross_read_dooraction_t_FromU16LE(file, ptr) == 2)
		checksum = ChecksumInt16(*ptr, checksum, lastByte, xorLastByte);
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
	id0_word_t statedosoffset = o->state ? o->state->compatdospointer : 0;
	id0_word_t nextdosoffset = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(o->next);
	id0_word_t prevdosoffset = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(o->prev);
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

id0_long_t SaveAndChecksumGameState(BE_FILE_T file, const gametype *state, id0_long_t checksum)
{
	id0_byte_t lastByte;
	checksum = WriteAndChecksumInt16LE(file, &state->difficulty, checksum, &lastByte, false);
	checksum = WriteAndChecksumInt16LE(file, &state->mapon, checksum, &lastByte, true);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	checksum = WriteAndChecksumInt32LE(file, &state->oldscore, checksum, &lastByte, true);
#endif
	checksum = WriteAndChecksumInt32LE(file, &state->score, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt32LE(file, &state->nextextra, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->lives, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->health, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->ammo, checksum, &lastByte, true);
#ifdef GAMEVER_NOAH3D
	checksum = WriteAndChecksumInt16LE(file, &state->treasure, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->maxammo, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->gas, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->missiles, checksum, &lastByte, true);
	checksum = WriteAndChecksumBooleanTo16LE(file, &state->machinegun, checksum, &lastByte, true);
	checksum = WriteAndChecksumBooleanTo16LE(file, &state->chaingun, checksum, &lastByte, true);
	checksum = WriteAndChecksumBooleanTo16LE(file, &state->missile, checksum, &lastByte, true);
	checksum = WriteAndChecksumBooleanTo16LE(file, &state->flamethrower, checksum, &lastByte, true);
	checksum = WriteAndChecksumBooleanTo16LE(file, &state->automap, checksum, &lastByte, true);
#endif
	checksum = WriteAndChecksumInt16LE(file, &state->keys, checksum, &lastByte, true);
	checksum = WriteAndChecksumWeaponTypeTo16LE(file, &state->bestweapon, checksum, &lastByte, true);
	checksum = WriteAndChecksumWeaponTypeTo16LE(file, &state->weapon, checksum, &lastByte, true);
	checksum = WriteAndChecksumWeaponTypeTo16LE(file, &state->chosenweapon, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->faceframe, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->attackframe, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->attackcount, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->weaponframe, checksum, &lastByte, true);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef GAMEVER_NOAH3D
	checksum = WriteAndChecksumInt16LE(file, &state->episode, checksum, &lastByte, true);
#endif
	checksum = WriteAndChecksumInt16LE(file, &state->secretcount, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->treasurecount, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->killcount, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->secrettotal, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->treasuretotal, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt16LE(file, &state->killtotal, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt32LE(file, &state->TimeCount, checksum, &lastByte, true);
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = WriteAndChecksumInt32LE(file, &state->killx, checksum, &lastByte, true);
	checksum = WriteAndChecksumInt32LE(file, &state->killy, checksum, &lastByte, true);
#endif
	checksum = WriteAndChecksumBooleanTo16LE(file, &state->victoryflag, checksum, &lastByte, true);
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	return checksum;
}

id0_long_t LoadAndChecksumGameState(BE_FILE_T file, gametype *state, id0_long_t checksum)
{
	id0_byte_t lastByte;
	checksum = ReadAndChecksumInt16LE(file, &state->difficulty, checksum, &lastByte, false);
	checksum = ReadAndChecksumInt16LE(file, &state->mapon, checksum, &lastByte, true);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	checksum = ReadAndChecksumInt32LE(file, &state->oldscore, checksum, &lastByte, true);
#endif
	checksum = ReadAndChecksumInt32LE(file, &state->score, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt32LE(file, &state->nextextra, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->lives, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->health, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->ammo, checksum, &lastByte, true);
#ifdef GAMEVER_NOAH3D
	checksum = ReadAndChecksumInt16LE(file, &state->treasure, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->maxammo, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->gas, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->missiles, checksum, &lastByte, true);
	checksum = ReadAndChecksumBooleanFrom16LE(file, &state->machinegun, checksum, &lastByte, true);
	checksum = ReadAndChecksumBooleanFrom16LE(file, &state->chaingun, checksum, &lastByte, true);
	checksum = ReadAndChecksumBooleanFrom16LE(file, &state->missile, checksum, &lastByte, true);
	checksum = ReadAndChecksumBooleanFrom16LE(file, &state->flamethrower, checksum, &lastByte, true);
	checksum = ReadAndChecksumBooleanFrom16LE(file, &state->automap, checksum, &lastByte, true);
#endif
	checksum = ReadAndChecksumInt16LE(file, &state->keys, checksum, &lastByte, true);
	checksum = ReadAndChecksumWeaponTypeFrom16LE(file, &state->bestweapon, checksum, &lastByte, true);
	checksum = ReadAndChecksumWeaponTypeFrom16LE(file, &state->weapon, checksum, &lastByte, true);
	checksum = ReadAndChecksumWeaponTypeFrom16LE(file, &state->chosenweapon, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->faceframe, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->attackframe, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->attackcount, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->weaponframe, checksum, &lastByte, true);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef GAMEVER_NOAH3D
	checksum = ReadAndChecksumInt16LE(file, &state->episode, checksum, &lastByte, true);
#endif
	checksum = ReadAndChecksumInt16LE(file, &state->secretcount, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->treasurecount, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->killcount, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->secrettotal, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->treasuretotal, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt16LE(file, &state->killtotal, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt32LE(file, &state->TimeCount, checksum, &lastByte, true);
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = ReadAndChecksumInt32LE(file, &state->killx, checksum, &lastByte, true);
	checksum = ReadAndChecksumInt32LE(file, &state->killy, checksum, &lastByte, true);
#endif
	checksum = ReadAndChecksumBooleanFrom16LE(file, &state->victoryflag, checksum, &lastByte, true);
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	return checksum;
}

id0_long_t SaveAndChecksumLevelRatios(BE_FILE_T file, const LRstruct *ratios, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	for (int i = 0; i < n; ++i)
	{
		checksum = WriteAndChecksumInt16LE(file, &ratios[i].kill, checksum, &lastByte, i != 0);
		checksum = WriteAndChecksumInt16LE(file, &ratios[i].secret, checksum, &lastByte, true);
		checksum = WriteAndChecksumInt16LE(file, &ratios[i].treasure, checksum, &lastByte, true);
		checksum = WriteAndChecksumInt32LE(file, &ratios[i].time, checksum, &lastByte, true);
#ifdef GAMEVER_NOAH3D
		checksum = WriteAndChecksumInt32LE(file, &ratios[i].par, checksum, &lastByte, true);
#endif
	}
	return checksum;
}

id0_long_t LoadAndChecksumLevelRatios(BE_FILE_T file, LRstruct *ratios, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	for (int i = 0; i < n; ++i)
	{
		checksum = ReadAndChecksumInt16LE(file, &ratios[i].kill, checksum, &lastByte, i != 0);
		checksum = ReadAndChecksumInt16LE(file, &ratios[i].secret, checksum, &lastByte, true);
		checksum = ReadAndChecksumInt16LE(file, &ratios[i].treasure, checksum, &lastByte, true);
		checksum = ReadAndChecksumInt32LE(file, &ratios[i].time, checksum, &lastByte, true);
#ifdef GAMEVER_NOAH3D
		checksum = ReadAndChecksumInt32LE(file, &ratios[i].par, checksum, &lastByte, true);
#endif
	}
	return checksum;
}

id0_long_t SaveAndChecksumStatObjects(BE_FILE_T file, const statobj_t *stats, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	for (int i = 0; i < n; ++i)
	{
		// BACKWARDS COMPATIBILITY
		id0_word_t visspotdosoffset = COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(stats[i].visspot);
		// Now writing
		checksum = WriteAndChecksumInt8LE(file, &stats[i].tilex, checksum, &lastByte, i != 0);
		checksum = WriteAndChecksumInt8LE(file, &stats[i].tiley, checksum, &lastByte, true);
		checksum = WriteAndChecksumIntU16LE(file, &visspotdosoffset, checksum, &lastByte, true); // BACKWARDS COMPATIBILITY
		checksum = WriteAndChecksumInt16LE(file, &stats[i].shapenum, checksum, &lastByte, true);
		checksum = WriteAndChecksumInt8LE(file, &stats[i].flags, checksum, &lastByte, true);
		checksum = WriteAndChecksumInt8LE(file, &stats[i].itemnumber, checksum, &lastByte, true);
	}
	return checksum;
}

id0_long_t LoadAndChecksumStatObjects(BE_FILE_T file, statobj_t *stats, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	for (int i = 0; i < n; ++i)
	{
		// BACKWARDS COMPATIBILITY
		id0_word_t visspotdosoffset = COMPAT_SPOTVIS_CONVERT_SPOT_PTR_TO_DOS_PTR(stats[i].visspot);
		// Now reading
		checksum = ReadAndChecksumInt8LE(file, &stats[i].tilex, checksum, &lastByte, i != 0);
		checksum = ReadAndChecksumInt8LE(file, &stats[i].tiley, checksum, &lastByte, true);
		checksum = ReadAndChecksumIntU16LE(file, &visspotdosoffset, checksum, &lastByte, true); // BACKWARDS COMPATIBILITY
		checksum = ReadAndChecksumInt16LE(file, &stats[i].shapenum, checksum, &lastByte, true);
		checksum = ReadAndChecksumInt8LE(file, &stats[i].flags, checksum, &lastByte, true);
		checksum = ReadAndChecksumInt8LE(file, &stats[i].itemnumber, checksum, &lastByte, true);

		stats[i].visspot = COMPAT_SPOTVIS_CONVERT_DOS_PTR_TO_SPOT_PTR(visspotdosoffset);
	}
	return checksum;
}

id0_long_t SaveAndChecksumDoorObjects(BE_FILE_T file, const doorobj_t *doors, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	id0_byte_t padding = 0;
	for (int i = 0; i < n; ++i)
	{
		checksum = WriteAndChecksumInt8LE(file, &doors[i].tilex, checksum, &lastByte, i != 0);
		checksum = WriteAndChecksumInt8LE(file, &doors[i].tiley, checksum, &lastByte, true);
		checksum = WriteAndChecksumBooleanTo16LE(file, &doors[i].vertical, checksum, &lastByte, true);
#ifdef GAMEVER_NOAH3D
		checksum = WriteAndChecksumBooleanTo16LE(file, &doors[i].seen, checksum, &lastByte, true);
#endif
		checksum = WriteAndChecksumInt8LE(file, &doors[i].lock, checksum, &lastByte, true);
		checksum = WriteAndChecksumInt8LE(file, &padding, checksum, &lastByte, true);
		checksum = WriteAndChecksumDoorActionFrom16LE(file, &doors[i].action, checksum, &lastByte, true);
		checksum = WriteAndChecksumInt16LE(file, &doors[i].ticcount, checksum, &lastByte, true);
	}
	return checksum;
}

id0_long_t LoadAndChecksumDoorObjects(BE_FILE_T file, doorobj_t *doors, int n, id0_long_t checksum)
{
	id0_byte_t lastByte;
	id0_byte_t padding = 0;
	for (int i = 0; i < n; ++i)
	{
		checksum = ReadAndChecksumInt8LE(file, &doors[i].tilex, checksum, &lastByte, i != 0);
		checksum = ReadAndChecksumInt8LE(file, &doors[i].tiley, checksum, &lastByte, true);
		checksum = ReadAndChecksumBooleanFrom16LE(file, &doors[i].vertical, checksum, &lastByte, true);
#ifdef GAMEVER_NOAH3D
		checksum = ReadAndChecksumBooleanFrom16LE(file, &doors[i].seen, checksum, &lastByte, true);
#endif
		checksum = ReadAndChecksumInt8LE(file, &doors[i].lock, checksum, &lastByte, true);
		checksum = ReadAndChecksumInt8LE(file, &padding, checksum, &lastByte, true);
		checksum = ReadAndChecksumDoorActionFrom16LE(file, &doors[i].action, checksum, &lastByte, true);
		checksum = ReadAndChecksumInt16LE(file, &doors[i].ticcount, checksum, &lastByte, true);
	}
	return checksum;
}

REFKEEN_NS_E
