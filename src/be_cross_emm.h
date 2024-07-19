/* Copyright (C) 2020-2024 Braden "Blzut3" Obrzut
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

#ifndef BE_CROSS_EMM_H
#define BE_CROSS_EMM_H

#include "be_cross.h"

// High level Expanded Memory Manager

typedef uint16_t BE_EMM_Handle;

typedef enum {
	BE_EMM_NO_ERROR,

	BE_EMM_SOFTWARE_MALFUNCTION = 0x80,
	BE_EMM_HARDWARE_MALFUNCTION = 0x81,
	BE_EMM_INVALID_HANDLE = 0x83,
	BE_EMM_FUNCTION_UNDEFINED = 0x84,
	BE_EMM_ALL_HANDLES_IN_USE = 0x85,
	BE_EMM_PAGE_MAPPING_CONTEXT_ERROR = 0x86,
	BE_EMM_NOT_ENOUGH_TOTAL_PAGES = 0x87,
	BE_EMM_NOT_ENOUGH_AVAILABLE_PAGES = 0x88,
	BE_EMM_REQUESTED_ZERO_PAGES = 0x89,
	BE_EMM_LOGICAL_PAGE_OUT_OF_RANGE = 0x8A,
	BE_EMM_PHYSICAL_PAGE_OUT_OF_RANGE = 0x8B,
	BE_EMM_STATE_SAVE_AREA_FULL = 0x8C,
	BE_EMM_STATE_SAVE_ALREADY_ASSOCIATED_WITH_HANDLE = 0x8D,
	BE_EMM_STATE_SAVE_NOT_ASSOCIATED_WITH_HANDLE = 0x8E,
	BE_EMM_SUBFUNCTION_UNDEFINED = 0x8F,

	BE_EMM_SOURCE_CORRUPTED = 0xA3,
} BE_EMM_Error;

typedef struct {
	uint16_t pagesAvailable; // BX
	uint16_t pagesTotal; // DX 
} BE_EMM_AllocationInfo;

typedef struct {
	BE_EMM_Handle handle;
	uint16_t pages;
} BE_EMM_HandleAllocation;

// Function 1: int 67h, ah = 40h.
BE_EMM_Error BE_Cross_EMM_GetStatus(void);

// Function 2: int 67h, ah = 41h.
uint16_t BE_Cross_EMM_GetPageFrame(void);

// Function 3: int 67h, ah = 42h.
BE_EMM_AllocationInfo BE_Cross_EMM_GetUnallocedPageCount(void);

// Function 4: int 67h, ah = 43h.
BE_EMM_Error BE_Cross_EMM_AllocatePages(uint16_t request, BE_EMM_Handle *handle);

// Function 5: int 67h, ah = 44h.
BE_EMM_Error BE_Cross_EMM_MapPage(BE_EMM_Handle handle, uint16_t logicalPage, uint8_t physicalPage);

// Function 6: int 67h, ah = 45h.
BE_EMM_Error BE_Cross_EMM_DeallocatePages(BE_EMM_Handle handle);

// Function 7: int 67h, ah = 46h. Returns version that would be in al.
uint8_t BE_Cross_EMM_GetVersion(void);

// Function 8: int 67h, ah = 47h.
BE_EMM_Error BE_Cross_EMM_SavePageMap(BE_EMM_Handle handle);

// Function 9: int 67h, ah = 48h.
BE_EMM_Error BE_Cross_EMM_RestorePageMap(BE_EMM_Handle handle);

// Functions 10&11 are reserved.

// Function 12: int 67h, ah = 4Bh.
uint16_t BE_Cross_EMM_GetHandleCount(void);

// Function 13: int 67h, ah = 4Ch.
BE_EMM_Error BE_Cross_EMM_GetHandlePages(BE_EMM_Handle handle, uint16_t *pages);

// Function 14: int 67h, ah = 4Dh.
uint16_t BE_Cross_EMM_GetAllHandlePages(BE_EMM_HandleAllocation *allocations);

// Function 15: int 67h, ah = 4Eh, al = 0h.
void BE_Cross_EMM_GetPageMap(void* dest);

// Function 15: int 67h, ah = 4Eh, al = 1h.
BE_EMM_Error BE_Cross_EMM_SetPageMap(const void* src);

// Function 15: int 67h, ah = 4Eh, al = 2h.
BE_EMM_Error BE_Cross_EMM_GetSetPageMap(void* dest, const void* src);

// Function 15: int 67h, ah = 4Eh, al = 3h.
uint8_t BE_Cross_EMM_GetPageMapSize(void);

// BEGIN: Driver presence detection

typedef uint16_t BE_EMM_File;

// Substitute for int 21h, ah = 43h handler. It is expected that devname be
// "EMMXXXX0". This does not actually implement file IO so the API is largely
// a formality to keep calling code close to the original.
BE_EMM_File BE_Cross_EMM_Open(const char* devname);

// Substitute for int 21h, ah = 44h, al = 0h. Returns false if EMS is not
// available, otherwise returns true and populates data with bit 7 set
// indicating that this is not a file.
bool BE_Cross_EMM_Ioctl_GetDeviceData(BE_EMM_File handle, uint16_t *data);

// Substitute for int 21h, ah = 44h, al = 7h. Returns false if EMS is not
// available (al = 00h), otherwise returns true (al = ffh).
bool BE_Cross_EMM_Ioctl_GetStatus(BE_EMM_File handle);

// Substitute for int 21h, ah = 3eh. Only exists to take place of original API
void BE_Cross_EMM_Close(BE_EMM_File handle);

// END

#endif
