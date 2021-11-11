/* Copyright (C) 2014-2021 NY00123
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

#ifndef BE_GAMEDEFS_STRUCTS_H
#define BE_GAMEDEFS_STRUCTS_H

#include "refkeen_config.h"
#include "../filesystem/be_filesystem_tchar.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h"

// Describes a file originally embedded somewhere (in an EXE file's image)
typedef struct {
	BE_GameFileDetails_T fileDetails;
	int offset; // Location of file/chunk in the EXE image
} BE_EmbeddedGameFileDetails_T;

typedef enum {
	BE_EXECOMPRESSION_NONE, BE_EXECOMPRESSION_LZEXE9X,
#ifdef ENABLE_PKLITE
	BE_EXECOMPRESSION_PKLITE105,
#endif
} BE_ExeCompression_T;

typedef struct {
	const BE_EmbeddedGameFileDetails_T *embeddedFiles;
	const char *subDescription; // For launcher, if non-null
	const char *subGameVerStr; // Like subDescription, but for command-line
	const char *exeNames; // NULL if all we want is a main function pointer
	void (*mainFuncPtr)(void);
	void (*embeddedFilesLoaderFuncPtr)(void);
	int decompExeImageSize;
	BE_ExeCompression_T compressionType;
	bool passArgsToMainFunc;
} BE_EXEFileDetails_T;

typedef struct {
	const BE_GameFileDetails_T *reqFiles;
	const BE_EXEFileDetails_T *exeFiles;
	const TCHAR *writableFilesDir;
	const char *description;
	void (**patcherFuncPtrs)(void);
	int audioDeviceFlags;
	BE_GameVer_T verId;
} BE_GameVerDetails_T;

extern const BE_GameVerDetails_T *g_be_gamever_ptrs[];

#endif
