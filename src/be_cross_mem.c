/* Copyright (C) 2014-2024 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

#include "refkeen_config.h" // MUST precede other contents due to e.g., endianness-based ifdefs

#include "be_cross.h"
#include "be_cross_mem_internal.h"
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
//
// Previously, there were also 0 EMS bytes and 65520 XMS bytes ranges in use, but
// these were disabled in be_cross_mem.c (they don't have to be enabled, technically).
//
// Furthermore, after moving a few embedded chunks (mostly TEXTSCN.SCN) to the
// memory managed *here* (rather than using native malloc function separately),
// we had to increased available (far) memory.

// The very first "segment" in the emulated space
#define EMULATED_FIRST_SEG 0
// Different portions of the space being emulated - start points
#define EMULATED_NEAR_SEG (EMULATED_FIRST_SEG+EMULATED_FIRST_PARAGRAPHS)
#define EMULATED_FAR_SEG (EMULATED_NEAR_SEG+EMULATED_NEAR_PARAGRAPHS+EMULATED_GAP_BETWEEN_HEAPS_PARAGRAPHS)
// Used to obtain a pointer to some location in mmEmulatedMemSpace
#define EMULATED_SEG_TO_PTR(seg) (mmEmulatedMemSpace+(seg)*16)

#ifdef _MSC_VER
/*static*/ uint8_t __declspec(align(16)) g_be_emulatedMemSpace[EMULATED_CONVENTIONAL_SIZE + EMULATED_EMS_SIZE];
#else
/*static*/ uint8_t __attribute__ ((aligned (16))) g_be_emulatedMemSpace[EMULATED_CONVENTIONAL_SIZE + EMULATED_EMS_SIZE];
#endif

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

// FIXME - Maybe not the most efficient, but still working
static void BE_Cross_RefreshNearBytesLeft(void)
{
	const uint8_t *prevBlockEnd = g_be_emulatedMemSpace + 16*EMULATED_NEAR_SEG;
	BE_MemoryBlock_T *block = g_nearBlocks;
	g_nearBytesLeft = 0;
	for (int i = 0; i < g_numOfNearBlocks; ++i, ++block)
	{
		g_nearBytesLeft = BE_Cross_TypedMax(int, g_nearBytesLeft, block->ptr - prevBlockEnd);
		prevBlockEnd = block->ptr + block->len;
	}
	const uint8_t * const endOfNearMem = g_be_emulatedMemSpace + 16*(EMULATED_NEAR_SEG+EMULATED_NEAR_PARAGRAPHS);
	g_nearBytesLeft = BE_Cross_TypedMax(int, g_nearBytesLeft, endOfNearMem - prevBlockEnd);
}

static void BE_Cross_RefreshFarBytesLeft(void)
{
	const uint8_t *prevBlockEnd = g_be_emulatedMemSpace + 16*EMULATED_FAR_SEG;
	BE_MemoryBlock_T *block = g_farBlocks;
	g_farBytesLeft = 0;
	for (int i = 0; i < g_numOfFarBlocks; ++i, ++block)
	{
		g_farBytesLeft = BE_Cross_TypedMax(int, g_farBytesLeft, block->ptr - prevBlockEnd);
		prevBlockEnd = block->ptr + block->len;
	}
	const uint8_t * const endOfFarMem = g_be_emulatedMemSpace + 16*(EMULATED_FAR_SEG+EMULATED_FAR_PARAGRAPHS);
	g_farBytesLeft = BE_Cross_TypedMax(int, g_farBytesLeft, endOfFarMem - prevBlockEnd);
}

void *BE_Cross_Bmalloc(uint16_t size)
{
	// Define vars here so C++ compilation doesn't fail with gotos
	uint8_t *prevBlockEnd, *endOfNearMem;
	BE_MemoryBlock_T *block;

	if (g_numOfNearBlocks == MAX_NO_OF_BLOCKS_PER_CLASS)
		goto outofmemory;
	prevBlockEnd = g_be_emulatedMemSpace + 16*EMULATED_NEAR_SEG;
	block = g_nearBlocks;
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

	endOfNearMem = g_be_emulatedMemSpace + 16*(EMULATED_NEAR_SEG+EMULATED_NEAR_PARAGRAPHS);
	if (endOfNearMem - prevBlockEnd >= size) // Add a new block at the end
	{
addnewblock:
		++g_numOfNearBlocks;
		block->ptr = prevBlockEnd;
		block->len = size;
		BE_Cross_RefreshNearBytesLeft();
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
	// Define vars here so C++ compilation doesn't fail with gotos
	uint8_t *prevBlockEnd, *endOfFarMem;
	BE_MemoryBlock_T *block;

	if (g_numOfFarBlocks == MAX_NO_OF_BLOCKS_PER_CLASS)
		goto outofmemory;
	prevBlockEnd = g_be_emulatedMemSpace + 16*EMULATED_FAR_SEG;
	block = g_farBlocks;
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

	endOfFarMem = g_be_emulatedMemSpace + 16*(EMULATED_FAR_SEG+EMULATED_FAR_PARAGRAPHS);
	if (endOfFarMem - prevBlockEnd >= size) // Add a new block at the end
	{
addnewblock:
		++g_numOfFarBlocks;
		block->ptr = prevBlockEnd;
		block->len = size;
		BE_Cross_RefreshFarBytesLeft();
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
			--g_numOfNearBlocks;
			memmove(block, block+1, (g_numOfNearBlocks-(block-g_nearBlocks))*sizeof(*block));
			BE_Cross_RefreshNearBytesLeft();
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
			--g_numOfFarBlocks;
			memmove(block, block+1, (g_numOfFarBlocks-(block-g_farBlocks))*sizeof(*block));
			BE_Cross_RefreshFarBytesLeft();
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
