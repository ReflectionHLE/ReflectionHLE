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
 
#ifndef BE_FILESYSTEM_GAMEINST_H
#define BE_FILESYSTEM_GAMEINST_H

#include "../gamedefs/be_gamedefs_structs.h"
#include "be_filesystem_len_bounds.h"
#include "be_cross.h"
#include "be_filesystem_tchar.h"
#include "be_gamever.h"

#define BE_GAMEINST_DESCRIPTION_LEN_BOUND 56

typedef struct {
	char descStr[BE_GAMEINST_DESCRIPTION_LEN_BOUND];
	TCHAR instPath[BE_CROSS_PATH_LEN_BOUND];
	TCHAR writableFilesPath[BE_CROSS_PATH_LEN_BOUND];
	BE_GameVer_T verId;
} BE_GameInstallation_T;

#define BE_CROSS_MAX_GAME_INSTALLATIONS (4*BE_GAMEVER_LAST)
extern BE_GameInstallation_T g_be_gameinstallations[BE_CROSS_MAX_GAME_INSTALLATIONS];
extern int g_be_gameinstallations_num;

extern const BE_GameInstallation_T *g_be_gameinstallationsbyver[BE_GAMEVER_LAST];
extern const BE_GameInstallation_T *g_be_selectedGameInstallation;

void BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg(
	const BE_GameVerDetails_T *details, const TCHAR *searchdir,
	const char *descStr, BE_TryAddGameInstallation_ErrorMsg_T *outErrMsg);

static inline void BEL_Cross_ConditionallyAddGameInstallation(
	const BE_GameVerDetails_T *details, const TCHAR *searchdir,
	const char *descStr)
{
	BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg(details, searchdir, descStr, NULL);
}

// Given a string representing a list of filenames separated
// by an internally used delimiter, fills in *outFilename the
// first one pointed by *filenames, if outFilename is non-NULL.
// *filenames is updated to point to the following file
// if any is left. It's otherwise set to NULL.
void BEL_ST_GetNextGameFileName(
	const char **filenames,
	char (*outFilename)[BE_CROSS_DOS_FILENAME_LEN_BOUND]);

// Attempts to open any file matching any of the given filenames in
// the given path. Assumes refkeen_current_gamever was appropriately set.
BE_FILE_T BEL_Cross_OpenMatchingGameFileForReading(const char *filenames, const TCHAR *path);

#endif
