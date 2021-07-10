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

#include "be_cross_emm.h"
#include "be_cross_mem_internal.h"

#include <string.h>

#define EMS_PAGE_SIZE 0x4000
// EMS requires at least 64 handles but no more than 255 even though the handle
// is a word value and not a byte. This seems to be because some of the state
// functions are artificially limited to returning no more than 255 handles.
// Although 64 is the official minimum since we only need to support games we
// really only need 1.
#define EMM_MAX_HANDLES 1
// EMS 4.0 specifies max EMS memory as 32MB (2048 pages). I see no technical
// reason this couldn't be 65535 pages, but also see no reason for that many.
#define EMM_MAX_PAGES 2048

// We could only possibly request 65535 pages per handle, so logical pages can
// only range from 0-65534.
#define EMM_PAGE_NOT_MAPPED 65535

static uint8_t * const g_be_emulatedEmsSpace = g_be_emulatedMemSpace + EMULATED_CONVENTIONAL_SIZE;

typedef struct {
	BE_EMM_Handle handle;
	uint16_t page;
} BE_EMM_PageMapping;

static struct {
	struct {
		uint8_t* ptr;
		uint16_t pages;
		bool allocated;
	} handles[EMM_MAX_HANDLES];

	BE_EMM_AllocationInfo allocInfo;
	BE_EMM_PageMapping mapping[4];

	BE_EMM_Handle savedPageMapHandle;
	BE_EMM_PageMapping savedPageMap[4];
} g_emmState = {
	{ { 0 }, },

	/* allocInfo = */ {
		/* .availablePages = */ EMM_MAX_PAGES,
		/* .totalPages = */ EMM_MAX_PAGES
	},

	/* mapping = */ {
		{ 0, EMM_PAGE_NOT_MAPPED },
		{ 0, EMM_PAGE_NOT_MAPPED },
		{ 0, EMM_PAGE_NOT_MAPPED },
		{ 0, EMM_PAGE_NOT_MAPPED },
	},

	/* savedPageMapHandle = */ EMM_MAX_HANDLES,

	/* savedPageMap = */ {
		{ 0, EMM_PAGE_NOT_MAPPED },
		{ 0, EMM_PAGE_NOT_MAPPED },
		{ 0, EMM_PAGE_NOT_MAPPED },
		{ 0, EMM_PAGE_NOT_MAPPED },
	},
};

static inline uint8_t* BEL_Cross_EMM_LogicalPageAddress(BE_EMM_Handle handle, uint16_t page)
{
	return g_emmState.handles[handle].ptr + page*EMS_PAGE_SIZE;
}

static inline uint8_t* BEL_Cross_EMM_PhysicalPageAddress(uint8_t page)
{
	return g_be_emulatedEmsSpace + page*EMS_PAGE_SIZE;
}

static inline bool BEL_Cross_EMM_PageMapIsEmpty(const BE_EMM_PageMapping* pageMap)
{
	for (int i = 0; i < 4; ++i)
	{
		if (pageMap[i].page != EMM_PAGE_NOT_MAPPED)
			return false;
	}
	return true;
}

BE_EMM_Error BE_Cross_EMM_GetStatus(void)
{
	return BE_EMM_NO_ERROR;
}

uint16_t BE_Cross_EMM_GetPageFrame(void)
{
	return BE_Cross_GetPtrNormalizedSeg(g_be_emulatedEmsSpace);
}

BE_EMM_AllocationInfo BE_Cross_EMM_GetUnallocedPageCount(void)
{
	return g_emmState.allocInfo;
}

BE_EMM_Error BE_Cross_EMM_AllocatePages(uint16_t request, BE_EMM_Handle *handle)
{
	if (request == 0)
		return BE_EMM_REQUESTED_ZERO_PAGES;

	if (request > g_emmState.allocInfo.pagesAvailable)
	{
		if (request > g_emmState.allocInfo.pagesTotal)
			return BE_EMM_NOT_ENOUGH_TOTAL_PAGES;
		return BE_EMM_NOT_ENOUGH_AVAILABLE_PAGES;
	}

	// Could use a more clever algorithm for speed, but games usually only do
	// one allocation.
	for (BE_EMM_Handle block = 0; block < EMM_MAX_HANDLES; ++block)
	{
		if (g_emmState.handles[block].allocated)
			continue;

		g_emmState.handles[block].ptr = (uint8_t*)malloc(request*EMS_PAGE_SIZE);
		if (g_emmState.handles[block].ptr == NULL)
		{
			*handle = 0;
			return BE_EMM_HARDWARE_MALFUNCTION;
		}
		g_emmState.handles[block].pages = request;
		g_emmState.handles[block].allocated = true;
		g_emmState.allocInfo.pagesAvailable -= request;

		*handle = block;
		return BE_EMM_NO_ERROR;
	}

	return BE_EMM_ALL_HANDLES_IN_USE;
}

BE_EMM_Error BE_Cross_EMM_MapPage(BE_EMM_Handle handle, uint16_t logicalPage, uint8_t physicalPage)
{
	if (handle >= EMM_MAX_HANDLES)
		return BE_EMM_INVALID_HANDLE;

	if (!g_emmState.handles[handle].allocated)
		return BE_EMM_INVALID_HANDLE;

	if (g_emmState.handles[handle].pages < logicalPage)
		return BE_EMM_LOGICAL_PAGE_OUT_OF_RANGE;

	if (physicalPage >= 4)
		return BE_EMM_PHYSICAL_PAGE_OUT_OF_RANGE;

	BE_EMM_PageMapping * const mapping = &g_emmState.mapping[physicalPage];

	// Check for no-op
	if (mapping->handle == handle && mapping->page == logicalPage)
		return BE_EMM_NO_ERROR;

#ifndef NDEBUG
	// We don't support aliasing (which isn't required per EMS 4.0 spec).
	// There's no reason to check for this in release builds, but it may be
	// useful to produce a warning in debug builds.
	for (int i = 0; i < 4; ++i)
	{
		if (g_emmState.mapping[i].handle == handle && g_emmState.mapping[i].page == logicalPage)
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "EMS aliasing detected with physical page %d and %d", i, physicalPage);
	}
#endif

	// Unmap existing page
	if (mapping->page != EMM_PAGE_NOT_MAPPED)
	{
		memcpy(
			BEL_Cross_EMM_LogicalPageAddress(mapping->handle, mapping->page),
			BEL_Cross_EMM_PhysicalPageAddress(physicalPage),
			EMS_PAGE_SIZE
		);
	}

	mapping->handle = handle;
	mapping->page = logicalPage;

	memcpy(
		BEL_Cross_EMM_PhysicalPageAddress(physicalPage),
		BEL_Cross_EMM_LogicalPageAddress(handle, logicalPage),
		EMS_PAGE_SIZE
	);
	return BE_EMM_NO_ERROR;
}

BE_EMM_Error BE_Cross_EMM_DeallocatePages(BE_EMM_Handle handle)
{
	if (handle >= EMM_MAX_HANDLES)
		return BE_EMM_INVALID_HANDLE;

	if (!g_emmState.handles[handle].allocated)
		return BE_EMM_INVALID_HANDLE;

	g_emmState.allocInfo.pagesAvailable += g_emmState.handles[handle].pages;
	free(g_emmState.handles[handle].ptr);
	g_emmState.handles[handle].ptr = NULL;
	g_emmState.handles[handle].allocated = false;

	// Mark any mapped pages as unmapped
	for (int i = 0; i < 4; ++i)
	{
		if (g_emmState.mapping[i].handle == handle)
			g_emmState.mapping[i].page = EMM_PAGE_NOT_MAPPED;
	}

	return BE_EMM_NO_ERROR;
}

uint8_t BE_Cross_EMM_GetVersion(void)
{
	return 0x32;
}

BE_EMM_Error BE_Cross_EMM_SavePageMap(BE_EMM_Handle handle)
{
	if (handle >= EMM_MAX_HANDLES)
		return BE_EMM_INVALID_HANDLE;

	if (!g_emmState.handles[handle].allocated)
		return BE_EMM_INVALID_HANDLE;

	if (!BEL_Cross_EMM_PageMapIsEmpty(g_emmState.savedPageMap))
	{
		if (g_emmState.savedPageMapHandle == handle)
			return BE_EMM_STATE_SAVE_ALREADY_ASSOCIATED_WITH_HANDLE;
		else
			return BE_EMM_STATE_SAVE_AREA_FULL;
	}

	g_emmState.savedPageMapHandle = handle;
	BE_Cross_EMM_GetPageMap(g_emmState.savedPageMap);
	return BE_EMM_NO_ERROR;
}

BE_EMM_Error BE_Cross_EMM_RestorePageMap(BE_EMM_Handle handle)
{
	if (handle >= EMM_MAX_HANDLES)
		return BE_EMM_INVALID_HANDLE;

	if (!g_emmState.handles[handle].allocated)
		return BE_EMM_INVALID_HANDLE;

	if (g_emmState.savedPageMapHandle != handle)
		return BE_EMM_STATE_SAVE_NOT_ASSOCIATED_WITH_HANDLE;

	if (BE_Cross_EMM_SetPageMap(g_emmState.savedPageMap) != BE_EMM_NO_ERROR)
		return BE_EMM_SOFTWARE_MALFUNCTION;

	// Empty saved state
	g_emmState.savedPageMapHandle = EMM_MAX_HANDLES;
	for (int i = 0; i < 4; ++i)
		g_emmState.savedPageMap[i].page = EMM_PAGE_NOT_MAPPED;

	return BE_EMM_NO_ERROR;
}

uint16_t BE_Cross_EMM_GetHandleCount(void)
{
	uint16_t count = 0;

	for (int i = 0; i < EMM_MAX_HANDLES; ++i)
	{
		if (g_emmState.handles[i].allocated)
			++count;
	}

	return count;
}

BE_EMM_Error BE_Cross_EMM_GetHandlePages(BE_EMM_Handle handle, uint16_t *pages)
{
	if (handle >= EMM_MAX_HANDLES || !g_emmState.handles[handle].allocated)
		return BE_EMM_INVALID_HANDLE;

	*pages = g_emmState.handles[handle].pages;
	return BE_EMM_NO_ERROR;
}

uint16_t BE_Cross_EMM_GetAllHandlePages(BE_EMM_HandleAllocation *allocations)
{
	uint16_t count = 0;

	for (int i = 0; i < EMM_MAX_HANDLES; ++i)
	{
		if (g_emmState.handles[i].allocated)
		{
			allocations->handle = i;
			allocations->pages = g_emmState.handles[i].pages;
			++allocations;
			++count;
		}
	}

	return count;
}

void BE_Cross_EMM_GetPageMap(void* dest)
{
	memcpy(dest, g_emmState.mapping, sizeof(g_emmState.mapping));
}

BE_EMM_Error BE_Cross_EMM_SetPageMap(const void* src)
{
	BE_EMM_PageMapping newMapping[4];
	memcpy(newMapping, src, sizeof(newMapping));

	for (int i = 0; i < 4; ++i)
	{
		BE_EMM_PageMapping * const mapping = &g_emmState.mapping[i];

		if (newMapping[i].page == EMM_PAGE_NOT_MAPPED)
		{
			if (mapping->page != EMM_PAGE_NOT_MAPPED)
			{
				// Page out existing mapping
				memcpy(
					BEL_Cross_EMM_LogicalPageAddress(mapping->handle, mapping->page),
					BEL_Cross_EMM_PhysicalPageAddress(i),
					EMS_PAGE_SIZE
				);
			}

			mapping->handle = 0;
			mapping->page = EMM_PAGE_NOT_MAPPED;
			continue;
		}

		if (BE_Cross_EMM_MapPage(newMapping[i].handle, newMapping[i].page, i) != BE_EMM_NO_ERROR)
			return BE_EMM_SOURCE_CORRUPTED;
	}

	return BE_EMM_NO_ERROR;
}

BE_EMM_Error BE_Cross_EMM_GetSetPageMap(void* dest, const void* src)
{
	BE_Cross_EMM_GetPageMap(dest);
	return BE_Cross_EMM_SetPageMap(src);
}

uint8_t BE_Cross_EMM_GetPageMapSize(void)
{
	return sizeof(g_emmState.mapping);
}

// BEGIN: EMM presence test - Open handle technique

#define EMM_FILE_INVALID 0
#define EMM_FILE_VALID 1

BE_EMM_File BE_Cross_EMM_Open(const char* devname)
{
	if (strcmp(devname, "EMMXXXX0") == 0)
		return EMM_FILE_VALID;
	return EMM_FILE_INVALID;
}

bool BE_Cross_EMM_Ioctl_GetDeviceData(BE_EMM_File handle, uint16_t *data)
{
	if (handle != EMM_FILE_VALID)
		return false;

	*data = 0x80;
	return true;
}

bool BE_Cross_EMM_Ioctl_GetStatus(BE_EMM_File handle)
{
	return handle == EMM_FILE_VALID;
}

void BE_Cross_EMM_Close(BE_EMM_File handle)
{
	// Just satisfies API
}

// END
