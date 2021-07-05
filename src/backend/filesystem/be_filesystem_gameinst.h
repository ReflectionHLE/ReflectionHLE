/* Copyright (C) 2014-2021 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */
 
#ifndef BE_FILESYSTEM_GAMEINST_H
#define BE_FILESYSTEM_GAMEINST_H

#include "../gamedefs/be_gamedefs_structs.h"
#include "backend/filesystem/be_filesystem_len_bounds.h"
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
