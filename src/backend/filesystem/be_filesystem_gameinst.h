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

#include "backend/filesystem/be_filesystem_path_len_bound.h"
#include "be_filesystem_tchar.h"
#include "be_gamever.h"

typedef struct {
	const char *descStr;
	TCHAR instPath[BE_CROSS_PATH_LEN_BOUND];
	TCHAR writableFilesPath[BE_CROSS_PATH_LEN_BOUND];
	BE_GameVer_T verId;
} BE_GameInstallation_T;

extern const BE_GameInstallation_T *g_be_selectedGameInstallation;

#endif
