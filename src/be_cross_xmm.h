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

#ifndef BE_CROSS_XMM_H
#define BE_CROSS_XMM_H

#include "be_cross.h"

// High level eXtended Memory Manager

// Use **ONLY* with memory allocated by BE_Cross_Bmalloc/BE_Cross_Bfarmalloc:
// Converts far pointer to linear space for XMM
static inline uint32_t BE_Cross_GetPtrXmsOff(void *ptr)
{
	extern uint8_t g_be_emulatedMemSpace[];
	return ((uint8_t *)ptr-g_be_emulatedMemSpace);
}

typedef enum {
	BE_XMM_FALSE = 0,
	BE_XMM_SUCCESS = 1,

	BE_XMM_NOT_IMPLEMENTED = 0x80,
	BE_XMM_VDISK_DETECTED = 0x81,
	BE_XMM_A20_ERROR = 0x82,

	BE_XMM_HMA_DOES_NOT_EXIST = 0x90,
	BE_XMM_HMA_IN_USE = 0x91,
	BE_XMM_HMA_REQUEST_TOO_SMALL = 0x92,
	BE_XMM_HMA_NOT_ALLOCATED = 0x93,
	BE_XMM_A20_STILL_ENABLED = 0x94,

	BE_XMM_ALL_ALLOCATED = 0xA0,
	BE_XMM_ALL_HANDLES_IN_USE = 0xA1,
	BE_XMM_INVALID_HANDLE = 0xA2,
	BE_XMM_SOURCE_HANDLE_INVALID = 0xA3,
	BE_XMM_SOURCE_OFFSET_INVALID = 0xA4,
	BE_XMM_DEST_HANDLE_INVALID = 0xA5,
	BE_XMM_DEST_OFFSET_INVALID = 0xA6,
	BE_XMM_LENGTH_INVALID = 0xA7,
	BE_XMM_MOVE_INVALID_OVERLAP = 0xA8,
	BE_XMM_PARITY_ERROR = 0xA9,
	BE_XMM_NOT_LOCKED = 0xAA,
	BE_XMM_HANDLE_LOCKED = 0xAB,
	BE_XMM_LOCK_COUNT_OVERFLOW = 0xAC,
	BE_XMM_LOCK_FAILED = 0xAD,

	BE_XMM_SMALLER_UMB_AVAILABLE = 0xB0,
	BE_XMM_NO_UMB_AVAILABLE = 0xB1,
	BE_XMM_UMB_SEGMENT_INVALID = 0xB2,
} BE_XMM_Error;

// When converting XMS code this function essentially determines if the return
// code is BL instead of AX. Since AX = 0 when BL is set there's no particular
// need to return two values. The queryA20 function is the only time when AX
// and BL can be 0 which is indicated here with BE_XMM_FALSE.
static inline bool BE_Cross_XMM_IsError(BE_XMM_Error code)
{
	return code >= BE_XMM_NOT_IMPLEMENTED;
}

typedef uint16_t BE_XMM_Handle;

// The order of members is important since games will have their own version of
// this struct with different names. Since they're otherwise identical we can
// type cast between them.
typedef struct {
	uint32_t length;
	BE_XMM_Handle sourceHandle;
	uint32_t sourceOffset;
	BE_XMM_Handle destHandle;
	uint32_t destOffset;
} BE_XMM_ExtendedMemoryMove;

typedef struct {
	uint8_t lockCount; // BH
	uint8_t freeHandles; // BL
	uint16_t blockLengthKb; // DX
} BE_XMM_HandleInfo;

typedef struct {
	uint16_t xmsVersion; // AX
	uint16_t driverVersion; // BX
	bool hmaExists; // DX
} BE_XMM_Version;

// XMS spec would normally require that the driver be queried for a singular
// function pointer that would be called with varying registers set for
// different functions. We take a little bit of extra overhead here via another
// layer of indirection to translate this to high level C while minimizing the
// logic changes in the calling code.
typedef struct {
	BE_XMM_Version (*getXmsVersionNumber)(void);
	BE_XMM_Error (*requestHma)(uint16_t hmaBytesRequired);
	BE_XMM_Error (*releaseHma)(void);
	BE_XMM_Error (*globalEnableA20)(void);
	BE_XMM_Error (*globalDisableA20)(void);
	BE_XMM_Error (*localEnableA20)(void);
	BE_XMM_Error (*localDisableA20)(void);
	BE_XMM_Error (*queryA20)(void);
	BE_XMM_Error (*queryFreeExtendedMemory)(uint16_t *largestFreeBlockKb, uint16_t *totalFreeKb);
	BE_XMM_Error (*allocateExtendedMemoryBlock)(uint16_t requestKb, BE_XMM_Handle *handle);
	BE_XMM_Error (*freeExtendedMemoryBlock)(BE_XMM_Handle handle);
	BE_XMM_Error (*moveExtendedMemoryBlock)(BE_XMM_ExtendedMemoryMove copy);
	BE_XMM_Error (*lockExtendedMemoryBlock)(BE_XMM_Handle handle, void *lockedPtr);
	BE_XMM_Error (*unlockExtendedMemoryBlock)(BE_XMM_Handle handle);
	BE_XMM_Error (*getHandleInformation)(BE_XMM_Handle handle, BE_XMM_HandleInfo *hInfo);
	BE_XMM_Error (*reallocateExtendedMemoryBlock)(BE_XMM_Handle handle, uint16_t requestKb);
	BE_XMM_Error (*requestUpperMemoryBlock)(uint16_t requestKb, uint16_t *segment, uint16_t *sizeKb);
	BE_XMM_Error (*releaseUpperMemoryBlock)(uint16_t segment);
} BE_XMM_Control;

// int 2fh, ax = 4300h
bool BE_Cross_GetXmsDriverInstalled(void);
// int 2fh, ax = 4310h
BE_XMM_Control *BE_Cross_GetXmsControl(void);

#endif // BE_CROSS_XMM_H
