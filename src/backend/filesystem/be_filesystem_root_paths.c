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

#include "be_filesystem_root_paths.h"
#include "be_filesystem_string_ops.h"
#include "be_st.h"

TCHAR g_be_rootPaths[BE_CROSS_MAX_ROOT_PATHS][BE_CROSS_PATH_LEN_BOUND];
const char *g_be_rootPathsKeys[BE_CROSS_MAX_ROOT_PATHS];
const char *g_be_rootPathsNames[BE_CROSS_MAX_ROOT_PATHS];
int g_be_rootPathsNum;


void BEL_Cross_AddRootPath(const TCHAR *rootPath, const char *rootPathKey, const char *rootPathName)
{
	if (g_be_rootPathsNum >= BE_CROSS_MAX_ROOT_PATHS)
		BE_ST_ExitWithErrorMsg("BEL_Cross_AddRootPath: Too many root paths!");

	BEL_Cross_safeandfastctstringcopy(g_be_rootPaths[g_be_rootPathsNum], g_be_rootPaths[g_be_rootPathsNum]+BE_Cross_ArrayLen(g_be_rootPaths[g_be_rootPathsNum]), rootPath);
	g_be_rootPathsKeys[g_be_rootPathsNum] = rootPathKey;
	g_be_rootPathsNames[g_be_rootPathsNum] = rootPathName;
	++g_be_rootPathsNum;
}
