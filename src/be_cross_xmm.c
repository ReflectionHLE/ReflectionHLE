/* Copyright (C) 2020-2021 Braden "Blzut3" Obrzut
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

#include "be_cross_mem_internal.h"
#include "be_cross_xmm.h"

#include <string.h>

// We could set this to 65536 but the games we support only allocate one block
// so we only need 2 handles (handle 0 is conventional memory)
#define XMM_MAX_HANDLES 2
#define XMM_MAX_LOCKS 255

static struct {
	struct {
		uint8_t *ptr;
		// XMM usually manages KB but we need to know the length of conventional
		// memory down to the byte in order to detect overrun correctly
		uint32_t sizeBytes;
		uint8_t lock;
		bool allocated;
	} handles[XMM_MAX_HANDLES];

	uint16_t freeKb;
} g_xmmState = {
	/* .handles = */ {
		{
			/* .ptr = */ g_be_emulatedMemSpace,
			/* .sizeBytes = */ sizeof(g_be_emulatedMemSpace),
			/* .lock = */ 1,
			/* .allocated = */ true
		},
	},
	/* .freeKb = */ 0xFFFF,
};

static BE_XMM_Version BEL_Cross_XMM_GetXmsVersionNumber(void)
{
	static BE_XMM_Version xmmVersion = {
		/* .xmsVersion = */ 0x0200,
		/* .driverVersion = */ 0x0100,
		/* .hmaExists = */ 0,
	};
	return xmmVersion;
}

static BE_XMM_Error BEL_Cross_XMM_RequestHma(uint16_t hmaBytesRequired)
{
	// Right now we don't have HMA in our emulated address space
	return BE_XMM_HMA_DOES_NOT_EXIST;
}

static BE_XMM_Error BEL_Cross_XMM_ReleaseHma(void)
{
	return BE_XMM_HMA_DOES_NOT_EXIST;
}

// Used for A20 gate functions since we don't currently have a need to emulate
static BE_XMM_Error BEL_Cross_XMM_NotImplemented(void)
{
	return BE_XMM_NOT_IMPLEMENTED;
}

static BE_XMM_Error BEL_Cross_XMM_QueryFreeExtendedMemory(uint16_t *largestFreeBlockKb, uint16_t *totalFreeKb)
{
	if (largestFreeBlockKb)
		*largestFreeBlockKb = g_xmmState.freeKb;
	if (totalFreeKb)
		*totalFreeKb = g_xmmState.freeKb;

	if (g_xmmState.freeKb == 0)
		return BE_XMM_ALL_ALLOCATED;

	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_AllocateExtendedMemoryBlock(uint16_t requestKb, BE_XMM_Handle *handle)
{
	BE_XMM_Handle block = 0;

	if (requestKb > g_xmmState.freeKb)
		return BE_XMM_ALL_ALLOCATED;

	// Given that we're not actually managing the memory space this could fairly
	// easily be optimized with LRU on the handles, but games usually just
	// allocate one large handle so not sure if there's a need to do anything
	// fancy.
	while (block++ < XMM_MAX_HANDLES)
	{
		if (g_xmmState.handles[block].allocated)
			continue;

		g_xmmState.handles[block].ptr = (uint8_t*)malloc(requestKb*1024);
		if (g_xmmState.handles[block].ptr == NULL)
		{
			// XMS specification isn't super clear about what a NULL handle is
			// nor what BL should be in this case. However, this condition
			// shouldn't occur on any modern OS due to virtual memory and
			// deferred allocations so not super concerned.
			*handle = 0;
			return BE_XMM_ALL_ALLOCATED;
		}
		g_xmmState.handles[block].sizeBytes = requestKb*1024;
		g_xmmState.handles[block].allocated = true;
		g_xmmState.freeKb -= requestKb;

		*handle = block;
		return BE_XMM_SUCCESS;
	}

	return BE_XMM_ALL_HANDLES_IN_USE;
}

static BE_XMM_Error BEL_Cross_XMM_FreeExtendedMemoryBlock(BE_XMM_Handle handle)
{
	if (handle > XMM_MAX_HANDLES)
		return BE_XMM_INVALID_HANDLE;

	if (g_xmmState.handles[handle].lock)
		return BE_XMM_HANDLE_LOCKED;

	g_xmmState.freeKb += g_xmmState.handles[handle].sizeBytes/1024;
	free(g_xmmState.handles[handle].ptr);
	g_xmmState.handles[handle].ptr = NULL;
	g_xmmState.handles[handle].allocated = false;

	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_MoveExtendedMemoryBlock(BE_XMM_ExtendedMemoryMove copy)
{
	if (copy.sourceHandle > XMM_MAX_HANDLES)
		return BE_XMM_SOURCE_HANDLE_INVALID;
	if (copy.destHandle > XMM_MAX_HANDLES)
		return BE_XMM_DEST_HANDLE_INVALID;
	if (g_xmmState.handles[copy.sourceHandle].sizeBytes < copy.sourceOffset)
		return BE_XMM_SOURCE_OFFSET_INVALID;
	if (g_xmmState.handles[copy.destHandle].sizeBytes < copy.destOffset)
		return BE_XMM_DEST_OFFSET_INVALID;

	// This check is not part of XMS standard but since we're dealing with
	// protected operating systems it's either return an error or crash anyway.
	if (g_xmmState.handles[copy.sourceHandle].sizeBytes - copy.sourceOffset < copy.length ||
	    g_xmmState.handles[copy.destHandle].sizeBytes - copy.destOffset < copy.length)
		return BE_XMM_LENGTH_INVALID;

	// Must be even per XMS spec
	if ((copy.length & 1) == 1)
		return BE_XMM_LENGTH_INVALID;

	// Overlap must be a forward move
	if (copy.sourceHandle == copy.destHandle && copy.sourceOffset >= copy.destOffset && copy.sourceOffset - copy.destOffset < copy.length)
		return BE_XMM_MOVE_INVALID_OVERLAP;

	memmove(g_xmmState.handles[copy.destHandle].ptr + copy.destOffset, g_xmmState.handles[copy.sourceHandle].ptr + copy.sourceOffset, copy.length);
	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_LockExtendedMemoryBlock(BE_XMM_Handle handle, void *lockedPtr)
{
	if (handle > XMM_MAX_HANDLES)
		return BE_XMM_INVALID_HANDLE;

	if (g_xmmState.handles[handle].lock == XMM_MAX_LOCKS)
		return BE_XMM_LOCK_COUNT_OVERFLOW;

	++g_xmmState.handles[handle].lock;
	if (lockedPtr)
		lockedPtr = g_xmmState.handles[handle].ptr;
	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_UnlockExtendedMemoryBlock(BE_XMM_Handle handle)
{
	if (handle > XMM_MAX_HANDLES)
		return BE_XMM_INVALID_HANDLE;

	if (!g_xmmState.handles[handle].lock)
		return BE_XMM_NOT_LOCKED;

	--g_xmmState.handles[handle].lock;
	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_GetHandleInformation(BE_XMM_Handle handle, BE_XMM_HandleInfo *hInfo)
{
	if (handle > XMM_MAX_HANDLES)
		return BE_XMM_INVALID_HANDLE;

	hInfo->lockCount = g_xmmState.handles[handle].lock;
	hInfo->blockLengthKb = g_xmmState.handles[handle].sizeBytes/1024;

	hInfo->freeHandles = 0;
	for (int i = 1; i < XMM_MAX_HANDLES; ++i)
	{
		if (!g_xmmState.handles[i].allocated)
			++hInfo->freeHandles;
	}

	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_ReallocateExtendedMemoryBlock(BE_XMM_Handle handle, uint16_t requestKb)
{
	if (handle > XMM_MAX_HANDLES)
		return BE_XMM_INVALID_HANDLE;

	if (g_xmmState.handles[handle].lock)
		return BE_XMM_HANDLE_LOCKED;

	if (g_xmmState.freeKb + g_xmmState.handles[handle].sizeBytes/1024 < requestKb)
		return BE_XMM_ALL_ALLOCATED;

	uint8_t* newptr = (uint8_t*)realloc(g_xmmState.handles[handle].ptr, requestKb);
	if (newptr == NULL) // Intended error code for this condition not very clear
		return BE_XMM_ALL_ALLOCATED;
	g_xmmState.handles[handle].ptr = newptr;

	g_xmmState.freeKb += g_xmmState.handles[handle].sizeBytes/1024;
	g_xmmState.freeKb -= requestKb;
	g_xmmState.handles[handle].sizeBytes = requestKb*1024;

	return BE_XMM_SUCCESS;
}

static BE_XMM_Error BEL_Cross_XMM_RequestUpperMemoryBlock(uint16_t requestKb, uint16_t *segment, uint16_t *sizeKb)
{
	// Like with HMA, our emulated address space does not include UMA
	return BE_XMM_NO_UMB_AVAILABLE;
}

static BE_XMM_Error BEL_Cross_XMM_ReleaseUpperMemoryBlock(uint16_t segment)
{
	return BE_XMM_UMB_SEGMENT_INVALID;
}

bool BE_Cross_GetXmsDriverInstalled(void)
{
	return true;
}

BE_XMM_Control *BE_Cross_GetXmsControl(void)
{
	static BE_XMM_Control xmm = {
		BEL_Cross_XMM_GetXmsVersionNumber,
		BEL_Cross_XMM_RequestHma,
		BEL_Cross_XMM_ReleaseHma,
		BEL_Cross_XMM_NotImplemented,
		BEL_Cross_XMM_NotImplemented,
		BEL_Cross_XMM_NotImplemented,
		BEL_Cross_XMM_NotImplemented,
		BEL_Cross_XMM_NotImplemented,
		BEL_Cross_XMM_QueryFreeExtendedMemory,
		BEL_Cross_XMM_AllocateExtendedMemoryBlock,
		BEL_Cross_XMM_FreeExtendedMemoryBlock,
		BEL_Cross_XMM_MoveExtendedMemoryBlock,
		BEL_Cross_XMM_LockExtendedMemoryBlock,
		BEL_Cross_XMM_UnlockExtendedMemoryBlock,
		BEL_Cross_XMM_GetHandleInformation,
		BEL_Cross_XMM_ReallocateExtendedMemoryBlock,
		BEL_Cross_XMM_RequestUpperMemoryBlock,
		BEL_Cross_XMM_ReleaseUpperMemoryBlock,
	};

	return &xmm;
}
