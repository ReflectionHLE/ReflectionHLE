/* Copyright (C) 2014-2017 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "refkeen_config.h" // MUST precede other contents due to e.g., endianness-based ifdefs

#include "be_st.h" // For BE_ST_ExitWithErrorMsg

// A static memory buffer used for our allocations, made out of 16-bytes
// long paragraphs (each of them beginning with some emulated "segment")
//
// NOTE: Main mem (near+far) should consist of 335*1024 bytes
// (for Keen Dreams with EGA graphics)
//
// Based on tests with Catacomb Abyss (even if not precise), targetting...
// - 3408 bytes returned by coreleft (before manually removing/ignoring a gap between near and far memory in the game's memory manager).
// - 448592 returned by farcoreleft.
// - 0 EMS bytes.
// - 65520 XMS bytes.

// The very first "segment" in the emulated space
#define EMULATED_FIRST_SEG 0
// A gap between the near and far heaps
#define EMULATED_GAP_BETWEEN_HEAPS_PARAGRAPHS 103
// Different portions of the space being emulated - start points
#define EMULATED_NEAR_SEG (EMULATED_FIRST_SEG+EMULATED_FIRST_PARAGRAPHS)
#define EMULATED_FAR_SEG (EMULATED_NEAR_SEG+EMULATED_NEAR_PARAGRAPHS-+EMULATED_GAP_BETWEEN_HEAPS_PARAGRAPHS)
#define EMULATED_EMS_SEG (EMULATED_FAR_SEG+EMULATED_FAR_PARAGRAPHS)
#define EMULATED_XMS_SEG (EMULATED_EMS_SEG+EMULATED_EMS_PARAGRAPHS)
// Lengths in paragraphs of the different sections
#define EMULATED_FIRST_PARAGRAPHS 4096
#define EMULATED_NEAR_PARAGRAPHS 213
#define EMULATED_FAR_PARAGRAPHS 28037
#define EMULATED_EMS_PARAGRAPHS 0 // Yes!
#define EMULATED_XMS_PARAGRAPHS 4095
// Used to obtain a pointer to some location in mmEmulatedMemSpace
#define EMULATED_SEG_TO_PTR(seg) (mmEmulatedMemSpace+(seg)*16)

/*static*/ uint8_t __attribute__ ((aligned (16))) g_be_emulatedMemSpace[16*(EMULATED_FIRST_PARAGRAPHS+EMULATED_NEAR_PARAGRAPHS+EMULATED_GAP_BETWEEN_HEAPS_PARAGRAPHS+EMULATED_FAR_PARAGRAPHS+EMULATED_EMS_PARAGRAPHS+EMULATED_XMS_PARAGRAPHS)];

/*** Memory blocks definitions ***/

#define MAX_NO_OF_BLOCKS_PER_CLASS 64

typedef struct {
	uint8_t *ptr;
	uint32_t len; // Can use uint16_t for near memory, but usually it doesn't reduce memory use, due to struct padding
} BE_MemoryBlock_T;

static int g_numOfNearBlocks, g_numOfFarBlocks;
/*static*/ uint16_t g_nearBytesLeft = 16*EMULATED_NEAR_PARAGRAPHS;
/*static*/ uint32_t g_farBytesLeft = 16*EMULATED_FAR_PARAGRAPHS;
static BE_MemoryBlock_T g_nearBlocks[MAX_NO_OF_BLOCKS_PER_CLASS], g_farBlocks[MAX_NO_OF_BLOCKS_PER_CLASS];

// C99
uint16_t BE_Cross_Bcoreleft(void);
uint32_t BE_Cross_Bfarcoreleft(void);
uint16_t BE_Cross_GetPtrNormalizedSeg(void *ptr);
uint16_t BE_Cross_GetPtrNormalizedOff(void *ptr);
void *BE_Cross_BMK_FP(uint16_t seg, uint16_t off);

void *BE_Cross_Bmalloc(uint16_t size)
{
	if (g_numOfNearBlocks == MAX_NO_OF_BLOCKS_PER_CLASS)
		goto outofmemory;

	uint8_t *prevBlockEnd = g_be_emulatedMemSpace + 16*EMULATED_NEAR_SEG;
	BE_MemoryBlock_T *block = g_nearBlocks;
	for (int i = 0; i < g_numOfNearBlocks; ++i, ++block)
	{
		if (block->ptr - prevBlockEnd >= size) // Sufficiently large gap found
		{
			memmove(block+1, block, (g_numOfNearBlocks-(block-g_nearBlocks))*sizeof(*block));
			// "block" points to a new block here
			goto addnewblock;
		}
		prevBlockEnd = block->ptr + block->len;
	}

	uint8_t *endOfNearMem = g_be_emulatedMemSpace + 16*(EMULATED_NEAR_SEG+EMULATED_NEAR_PARAGRAPHS);
	if (endOfNearMem - prevBlockEnd >= size) // Add a new block at the end
	{
addnewblock:
		g_nearBytesLeft -= size;
		++g_numOfNearBlocks;
		block->ptr = prevBlockEnd;
		block->len = size;
		return block->ptr;
	}

	// REFKEEN NOTE - Plain malloc should return NULL,
	// but we rather do the following for debugging
outofmemory:
	BE_ST_ExitWithErrorMsg("BE_Cross_Bmalloc: Out of memory!");
	return NULL; // Mute compiler warning
}

void *BE_Cross_Bfarmalloc(uint32_t size)
{
	if (g_numOfFarBlocks == MAX_NO_OF_BLOCKS_PER_CLASS)
		goto outofmemory;

	uint8_t *prevBlockEnd = g_be_emulatedMemSpace + 16*EMULATED_FAR_SEG;
	BE_MemoryBlock_T *block = g_farBlocks;
	for (int i = 0; i < g_numOfFarBlocks; ++i, ++block)
	{
		if (block->ptr - prevBlockEnd >= size) // Sufficiently large gap found
		{
			memmove(block+1, block, (g_numOfFarBlocks-(block-g_farBlocks))*sizeof(*block));
			// "block" points to a new block here
			goto addnewblock;
		}
		prevBlockEnd = block->ptr + block->len;
	}

	uint8_t *endOfFarMem = g_be_emulatedMemSpace + 16*(EMULATED_FAR_SEG+EMULATED_FAR_PARAGRAPHS);
	if (endOfFarMem - prevBlockEnd >= size) // Add a new block at the end
	{
addnewblock:
		g_farBytesLeft -= size;
		++g_numOfFarBlocks;
		block->ptr = prevBlockEnd;
		block->len = size;
		return block->ptr;
	}

	// REFKEEN NOTE - Plain malloc should return NULL,
	// but we rather do the following for debugging
outofmemory:
	BE_ST_ExitWithErrorMsg("BE_Cross_Bfarmalloc: Out of memory!");
	return NULL; // Mute compiler warning
}

void BE_Cross_Bfree(void *ptr)
{
	if (ptr == NULL)
		return;

	BE_MemoryBlock_T *block = g_nearBlocks;
	for (int i = 0; i < g_numOfNearBlocks; ++i, ++block)
		if (block->ptr == ptr)
		{
			g_nearBytesLeft += block->len;
			--g_numOfNearBlocks;
			memmove(block, block+1, (g_numOfNearBlocks-(block-g_nearBlocks)*sizeof(*block)));
			return;
		}

	BE_ST_ExitWithErrorMsg("BE_Cross_Bfree: Got an invalid pointer!");
}

void BE_Cross_Bfarfree(void *ptr)
{
	if (ptr == NULL)
		return;

	BE_MemoryBlock_T *block = g_farBlocks;
	for (int i = 0; i < g_numOfFarBlocks; ++i, ++block)
		if (block->ptr == ptr)
		{
			g_farBytesLeft += block->len;
			--g_numOfFarBlocks;
			memmove(block, block+1, (g_numOfFarBlocks-(block-g_farBlocks)*sizeof(*block)));
			return;
		}

	BE_ST_ExitWithErrorMsg("BE_Cross_Bfarfree: Got an invalid pointer!");
}

void BEL_Cross_ClearMemory(void)
{
	g_numOfNearBlocks = g_numOfFarBlocks = 0;
	g_nearBytesLeft = 16*EMULATED_NEAR_PARAGRAPHS;
	g_farBytesLeft = 16*EMULATED_FAR_PARAGRAPHS;
}
