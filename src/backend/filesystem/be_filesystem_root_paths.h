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

#ifndef BE_FILESYSTEM_ROOT_PATHS_H
#define BE_FILESYSTEM_ROOT_PATHS_H

#include "be_cross.h"
#include "be_filesystem_dir.h"
#include "be_filesystem_len_bounds.h"
#include "be_filesystem_tchar.h"

// A list of "root paths" from which one can choose a game dir (using just ASCII characters)
extern TCHAR g_be_rootPaths[BE_CROSS_MAX_ROOT_PATHS][BE_CROSS_PATH_LEN_BOUND];
extern const char *g_be_rootPathsKeys[BE_CROSS_MAX_ROOT_PATHS];
extern const char *g_be_rootPathsNames[BE_CROSS_MAX_ROOT_PATHS];
extern int g_be_rootPathsNum;

/*** WARNING: The key and name are assumed to be C STRING LITERALS, and so are *NOT* copied! ***/
void BEL_Cross_AddRootPath(
	const TCHAR *rootPath, const char *rootPathKey, const char *rootPathName);
	
/*** WARNING: Same as above ***/
static inline void BEL_Cross_AddRootPathIfDir(const TCHAR *rootPath, const char *rootPathKey, const char *rootPathName)
{
	if (BEL_Cross_IsDir(rootPath))
		BEL_Cross_AddRootPath(rootPath, rootPathKey, rootPathName);
}

#endif
