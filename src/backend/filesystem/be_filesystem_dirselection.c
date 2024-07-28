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

#include "refkeen_config.h"
#include "../gamedefs/be_gamedefs_structs.h"
#include "be_filesystem_app_paths.h"
#include "be_filesystem_gameinst.h"
#include "be_filesystem_root_paths.h"
#include "be_filesystem_string_ops.h"
#include "be_filesystem_tchar.h"
#include "be_gamever.h"
#include "be_st.h"

int BE_Cross_DirSelection_GetNumOfRootPaths(void)
{
	return g_be_rootPathsNum;
}

const char **BE_Cross_DirSelection_GetRootPathsNames(void)
{
	return g_be_rootPathsNames;
}

static TCHAR g_be_dirSelection_currPath[BE_CROSS_PATH_LEN_BOUND];
static TCHAR * const g_be_dirSelection_currPathEnd = g_be_dirSelection_currPath + BE_CROSS_PATH_LEN_BOUND;
static TCHAR *g_be_dirSelection_separatorPtrs[BE_CROSS_PATH_LEN_BOUND];
static TCHAR ** const g_be_dirSelection_separatorPtrsEnd = g_be_dirSelection_separatorPtrs + BE_CROSS_PATH_LEN_BOUND;
static TCHAR **g_be_dirSelection_currSeparatorPtrPtr;
static int g_be_dirSelection_rootPathIndex;
// Dynamically allocated memory
static char *g_be_dirSelection_dirnamesBuffer = NULL;
static char **g_be_dirSelection_dirnamesBufferPtrPtrs = NULL;

void BE_Cross_DirSelection_FillLastSelectedPath(char *buffer, size_t len)
{
#ifdef REFKEEN_PLATFORM_WINDOWS
	if (WideCharToMultiByte(CP_UTF8, 0, g_be_dirSelection_currPath, -1,
	                        buffer, len, NULL, NULL))
		buffer[len - 1] = '\0';
	else
		buffer[0] = '\0';
#else
	BE_Cross_safeandfastcstringcopy(buffer, buffer + len,
	                                g_be_dirSelection_currPath);
#endif
}

static void BEL_Cross_DirSelection_ClearResources(void)
{
	free(g_be_dirSelection_dirnamesBuffer);
	free(g_be_dirSelection_dirnamesBufferPtrPtrs);
	g_be_dirSelection_dirnamesBuffer = NULL;
	g_be_dirSelection_dirnamesBufferPtrPtrs = NULL;
}

static const char **BEL_Cross_DirSelection_PrepareDirsAndGetNames(int *outNumOfSubDirs)
{
	TCHAR *d_name;
	BE_DIR_T dir = BEL_Cross_OpenDir(g_be_dirSelection_currPath);
	if (!dir)
		return NULL;

	TCHAR fullpath[BE_CROSS_PATH_LEN_BOUND];
	TCHAR *fullpathEnd = fullpath + BE_Cross_ArrayLen(fullpath);

	int numOfSubDirs = 0;
	int charsToAllocateForNames = 0;
	for (d_name = BEL_Cross_ReadDir(dir); d_name; d_name = BEL_Cross_ReadDir(dir))
	{
		/*** Ignore non-ASCII dirnames or any of a few special entries ***/
		if (*BEL_Cross_tstr_find_nonascii_ptr(d_name) || !_tcscmp(d_name, _T(".")) || !_tcscmp(d_name, _T("..")))
			continue;

		BEL_Cross_safeandfastctstringcopy_3strs(fullpath, fullpathEnd, g_be_dirSelection_currPath, _T("/"), d_name);
		if (!BEL_Cross_IsDir(fullpath))
			continue;

		++numOfSubDirs;

		charsToAllocateForNames += 1 + _tcslen(d_name);
	}

	g_be_dirSelection_dirnamesBuffer = (char *)malloc(charsToAllocateForNames);
	g_be_dirSelection_dirnamesBufferPtrPtrs = (char **)malloc(numOfSubDirs*sizeof(char *));
	if (!(g_be_dirSelection_dirnamesBuffer && g_be_dirSelection_dirnamesBufferPtrPtrs))
	{
		BEL_Cross_DirSelection_ClearResources();
		BEL_Cross_CloseDir(dir);
		BE_ST_ExitWithErrorMsg("BEL_Cross_DirSelection_PrepareDirsAndGetNames: Out of memory!");
	}

	// Re-scan, and be ready for the case directory contents have changed
	int repeatedNumOfSubDirs = 0;
	int repeatedCharsToAllocateForNames = 0;
	BEL_Cross_RewindDir(dir);
	char *dirnameBufferPtr = g_be_dirSelection_dirnamesBuffer;
	for (d_name = BEL_Cross_ReadDir(dir); d_name; d_name = BEL_Cross_ReadDir(dir))
	{
		/*** Ignore non-ASCII dirnames or any of a few special entries ***/
		if (*BEL_Cross_tstr_find_nonascii_ptr(d_name) || !_tcscmp(d_name, _T(".")) || !_tcscmp(d_name, _T("..")))
			continue;

		BEL_Cross_safeandfastctstringcopy_3strs(fullpath, fullpathEnd, g_be_dirSelection_currPath, _T("/"), d_name);
		if (!BEL_Cross_IsDir(fullpath))
			continue;

		size_t len = _tcslen(d_name);
		if ((++repeatedNumOfSubDirs > numOfSubDirs) || ((repeatedCharsToAllocateForNames += 1 + len) > charsToAllocateForNames))
		{
			--repeatedNumOfSubDirs;
			break;
		}

		char *currDirnameBufferPtr = dirnameBufferPtr;

		TCHAR *tchPtr = d_name;
		for (size_t i = 0; i <= len; ++i)
			*currDirnameBufferPtr++ = *tchPtr++; // Possibly convert (ASCII only) wchar_t to char here

		// Basically insertion-sort
		char **dirnameBufferPtrPtr = g_be_dirSelection_dirnamesBufferPtrPtrs;
		int j;
		for (j = 0; j < repeatedNumOfSubDirs - 1; ++j, ++dirnameBufferPtrPtr)
			if (BE_Cross_strcasecmp(*dirnameBufferPtrPtr, dirnameBufferPtr) > 0)
			{
				memmove(dirnameBufferPtrPtr+1, dirnameBufferPtrPtr, (repeatedNumOfSubDirs - 1 - j)*sizeof(char*));
				break;
			}
		*dirnameBufferPtrPtr = dirnameBufferPtr;

		dirnameBufferPtr = currDirnameBufferPtr;
	}

	BEL_Cross_CloseDir(dir);
	*outNumOfSubDirs = repeatedNumOfSubDirs;
	return (const char **)g_be_dirSelection_dirnamesBufferPtrPtrs;
}

const char **BE_Cross_DirSelection_Start(int rootPathIndex, int *outNumOfSubDirs) // Start dir selection
{
	g_be_dirSelection_separatorPtrs[0] = BEL_Cross_safeandfastctstringcopy(g_be_dirSelection_currPath, g_be_dirSelection_currPathEnd, g_be_rootPaths[rootPathIndex]);
	g_be_dirSelection_rootPathIndex = rootPathIndex;
	g_be_dirSelection_currSeparatorPtrPtr = &g_be_dirSelection_separatorPtrs[0];
	return BEL_Cross_DirSelection_PrepareDirsAndGetNames(outNumOfSubDirs);
}

void BE_Cross_DirSelection_Finish(void) // Finish dir selection
{
	BEL_Cross_DirSelection_ClearResources();
}

const char **BE_Cross_DirSelection_GetNext(int dirIndex, int *outNumOfSubDirs) // Enter dir by index into last array
{
	TCHAR *nextSeparatorPtr =
	// HACK for taking care of root path on Linux (/); No check for Windows-specific backslash, though!
	((*g_be_dirSelection_currSeparatorPtrPtr != g_be_dirSelection_currPath) && (*((*g_be_dirSelection_currSeparatorPtrPtr)-1) == _T('/')))
	? BEL_Cross_safeandfastcstringcopytoctstring(*g_be_dirSelection_currSeparatorPtrPtr, g_be_dirSelection_currPathEnd, g_be_dirSelection_dirnamesBufferPtrPtrs[dirIndex])
	: BEL_Cross_safeandfastcstringcopytoctstring(BEL_Cross_safeandfastctstringcopy(*g_be_dirSelection_currSeparatorPtrPtr, g_be_dirSelection_currPathEnd, _T("/")), g_be_dirSelection_currPathEnd, g_be_dirSelection_dirnamesBufferPtrPtrs[dirIndex]);

	BEL_Cross_DirSelection_ClearResources(); // Safe to do so now since copy has already been done
	if ((nextSeparatorPtr == g_be_dirSelection_currPathEnd) || (++g_be_dirSelection_currSeparatorPtrPtr == g_be_dirSelection_separatorPtrsEnd))
	{
		BE_ST_ExitWithErrorMsg("BEL_Cross_DirSelection_GetNext: Buffer overflow, or too deep hierarchy!");
	}
	*g_be_dirSelection_currSeparatorPtrPtr = nextSeparatorPtr;
	return BEL_Cross_DirSelection_PrepareDirsAndGetNames(outNumOfSubDirs);
}

const char **BE_Cross_DirSelection_GetPrev(int *outNumOfSubDirs) // Go up in the filesystem hierarchy
{
	BEL_Cross_DirSelection_ClearResources();
	if (g_be_dirSelection_currSeparatorPtrPtr == &g_be_dirSelection_separatorPtrs[0])
		return NULL;

	**(--g_be_dirSelection_currSeparatorPtrPtr) = _T('\0'); // Replace separator with this
	return BEL_Cross_DirSelection_PrepareDirsAndGetNames(outNumOfSubDirs);
}


// Attempt to add a game installation from currently selected dir;
// Returns BE_GAMEVER_LAST if no new supported game version is found; Otherwise game version id is returned.
// The given array is used in order to report an error for each checked version, in case of failure.
//
// Array MUST have at least BE_GAMEVER_LAST elements.
int BE_Cross_DirSelection_TryAddGameInstallation(BE_TryAddGameInstallation_ErrorMsg_T errorMsgsArray[])
{
	int verId;
	for (verId = 0; verId < BE_GAMEVER_LAST; ++verId)
	{
		if (g_be_gameinstallationsbyver[verId])
		{
			BE_Cross_safeandfastcstringcopy(errorMsgsArray[verId], errorMsgsArray[verId] + sizeof(errorMsgsArray[verId]), "Already available");
			continue;
		}

		const BE_GameVerDetails_T *details = g_be_gamever_ptrs[verId];
		BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg(details, g_be_dirSelection_currPath, NULL, &errorMsgsArray[verId]);
		if (g_be_gameinstallationsbyver[verId]) // Match found and added
		{
			TCHAR path[BE_CROSS_PATH_LEN_BOUND];
			TCHAR * const pathEnd = path + BE_CROSS_PATH_LEN_BOUND;
			BEL_Cross_safeandfastctstringcopy_4strs(path, pathEnd, g_be_appDataPath, _T("/"), details->writableFilesDir, _T(".txt"));
			FILE *fp = _tfopen(path, _T("w"));
			if (fp)
			{
				// HACK
#ifdef REFKEEN_PLATFORM_WINDOWS
				fprintf(fp, "%s %ls\n", g_be_rootPathsKeys[g_be_dirSelection_rootPathIndex], 1+g_be_dirSelection_separatorPtrs[0]);
#else
				fprintf(fp, "%s %s\n", g_be_rootPathsKeys[g_be_dirSelection_rootPathIndex], 1+g_be_dirSelection_separatorPtrs[0]);
#endif
				fclose(fp);
			}
			else
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_DirSelection_TryAddGameInstallation: Can't add directory to txt file.\n");


			BEL_Cross_SortGameInstallations_ByVerId();
			break; // Finish
		}
	}
	return verId;
}
