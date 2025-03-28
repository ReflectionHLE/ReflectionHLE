/* Copyright (C) 2014-2025 NY00123
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
