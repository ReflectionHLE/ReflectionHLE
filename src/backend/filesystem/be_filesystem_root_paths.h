/* Copyright (C) 2014-2022 NY00123
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
