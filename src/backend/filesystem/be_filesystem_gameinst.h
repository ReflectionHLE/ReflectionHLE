/* Copyright (C) 2014-2020 NY00123
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
#include "backend/filesystem/be_filesystem_path_len_bound.h"
#include "be_cross.h"
#include "be_filesystem_tchar.h"
#include "be_gamever.h"

typedef struct {
	const char *descStr;
	TCHAR instPath[BE_CROSS_PATH_LEN_BOUND];
	TCHAR writableFilesPath[BE_CROSS_PATH_LEN_BOUND];
	BE_GameVer_T verId;
} BE_GameInstallation_T;

#define BE_CROSS_MAX_GAME_INSTALLATIONS (4*BE_GAMEVER_LAST)
extern BE_GameInstallation_T g_be_gameinstallations[BE_CROSS_MAX_GAME_INSTALLATIONS];
extern int g_be_gameinstallations_num;

extern const BE_GameInstallation_T *g_be_gameinstallationsbyver[BE_GAMEVER_LAST];
extern const BE_GameInstallation_T *g_be_selectedGameInstallation;

// ***ASSUMPTION: descStr points to a C string literal which is never modified nor deleted!!!***
void BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg(
	const BE_GameVerDetails_T *details, const TCHAR *searchdir,
	const char *descStr, BE_TryAddGameInstallation_ErrorMsg_T *outErrMsg);

// ***ASSUMPTION: Again, descStr points to a C string literal which is never modified nor deleted!!!***
static inline void BEL_Cross_ConditionallyAddGameInstallation(
	const BE_GameVerDetails_T *details, const TCHAR *searchdir,
	const char *descStr)
{
	BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg(details, searchdir, descStr, NULL);
}

#endif
