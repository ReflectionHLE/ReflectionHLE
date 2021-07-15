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
 
#include "refkeen_config.h" // MUST precede other contents due to macros

#include "backend/filesystem/be_filesystem_app_paths.h"
#include "backend/filesystem/be_filesystem_dir.h"
#include "backend/filesystem/be_filesystem_file_ops.h"
#include "backend/filesystem/be_filesystem_gameinst.h"
#include "backend/filesystem/be_filesystem_mkdir.h"
#include "backend/filesystem/be_filesystem_string_ops.h"

BE_FILE_T BEL_Cross_apply_file_action_in_dir(
	const char *filename, BE_FileRequest_T request,
	const TCHAR *searchdir, TCHAR (*outfullpath)[BE_CROSS_PATH_LEN_BOUND])
{
	TCHAR *d_name;
	BE_DIR_T dir = BEL_Cross_OpenDir(searchdir);
	if (!dir)
		return NULL;

	TCHAR fullpath[BE_CROSS_PATH_LEN_BOUND];

	for (d_name = BEL_Cross_ReadDir(dir); d_name; d_name = BEL_Cross_ReadDir(dir))
	{
		/*** Ignore non-ASCII filenames ***/
		if (*BEL_Cross_tstr_find_nonascii_ptr(d_name))
			continue;
		if (!BEL_Cross_tstr_to_cstr_ascii_casecmp(d_name, filename))
		{
			// Just a little sanity check
			if (_tcslen(searchdir) + 1 + _tcslen(d_name) >= BE_CROSS_PATH_LEN_BOUND)
			{
				BEL_Cross_CloseDir(dir);
				return NULL;
			}
			TCHAR *fullpathEnd = fullpath + BE_Cross_ArrayLen(fullpath);
			BEL_Cross_safeandfastctstringcopy_3strs(fullpath, fullpathEnd, searchdir, _T("/"), d_name);

			BEL_Cross_CloseDir(dir);
			if (outfullpath)
				memcpy(*outfullpath, fullpath, sizeof(fullpath));
			if (request != BE_FILE_REQUEST_DELETE)
				return _tfopen(
				    fullpath,
				    (request == BE_FILE_REQUEST_OVERWRITE)
				      ? _T("wb") : _T("rb"));
			else
			{
				_tremove(fullpath);
				return NULL;
			}
		}
	}
	BEL_Cross_CloseDir(dir);

	if (request != BE_FILE_REQUEST_OVERWRITE)
		return NULL;
	TCHAR *fullpathEnd = fullpath + BE_Cross_ArrayLen(fullpath);
	TCHAR *fullpathPtr = BEL_Cross_safeandfastctstringcopy_2strs(fullpath, fullpathEnd, searchdir, _T("/"));
	// Create actual new files with a lower case, just because that's a common pattern in Unix-like setups
	// (basically a modified BE_Cross_safeandfastcstringcopy, also copying a narrow string to a wide string).
	//
	// Note: fullpathPtr should initially point to an instance of '\0', so fullpathPtr < fullpathEnd.
	char ch;
	do
	{
		ch = *filename++;
		*fullpathPtr++ = BE_Cross_tolower(ch); // This includes the null terminator if there's the room
	} while ((fullpathPtr < fullpathEnd) && ch);
	// These work in case fullpathPtr == fullpathEnd, and also if not
	--fullpathPtr;
	*fullpathPtr = _T('\0');

	if (outfullpath)
		memcpy(*outfullpath, fullpath, sizeof(fullpath));
	return _tfopen(fullpath, _T("wb"));
}

static void BEL_Cross_CreateTrimmedFilename(const char *inFilename, char (*outFileName)[BE_CROSS_DOS_FILENAME_LEN_BOUND])
{
	// Remove trailing spaces (required for filenames stored in SCRIPT.HNT The Catacomb Armageddon v1.02 (used by HINTCAT.EXE))
	char *fixedFilenamePtr = BE_Cross_safeandfastcstringcopy((*outFileName), (*outFileName) + sizeof(*outFileName), inFilename);
	if (fixedFilenamePtr != *outFileName) // Copied string isn't empty (which would actually be bad, anyway...)
	{
		--fixedFilenamePtr;
		do
		{
			if (*fixedFilenamePtr != ' ')
				break;
			*fixedFilenamePtr = '\0';
			// Technically, checking *(fixedFilenamePtr-1) leads to undefined behaviors, and even
			// the pointer itself doesn't have a clear manner, so do the following check (of the *original ptr)
			if (fixedFilenamePtr-- == *outFileName)
				break;
		}
		while (true);
	}
}

// Opens a read-only file for reading from a "search path" in a case-insensitive manner
BE_FILE_T BE_Cross_open_readonly_for_reading(const char *filename)
{
	char trimmedFilename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	BEL_Cross_CreateTrimmedFilename(filename, &trimmedFilename);
	// Trying writableFilesPath first, and then instPath in case of failure
	BE_FILE_T fp = BEL_Cross_apply_file_action_in_dir(trimmedFilename, BE_FILE_REQUEST_READ, g_be_selectedGameInstallation->writableFilesPath, NULL);
	if (fp)
		return fp;
	return BEL_Cross_apply_file_action_in_dir(trimmedFilename, BE_FILE_REQUEST_READ, g_be_selectedGameInstallation->instPath, NULL);
}

BE_FILE_T BE_Cross_open_matching_readonly_for_reading(const char *filename)
{
	char trimmedFilename[BE_CROSS_DOS_FILENAME_LEN_BOUND],
	     checkedFilename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	const char *filenames = NULL;
	const BE_GameVerDetails_T *details = g_be_gamever_ptrs[refkeen_current_gamever];
	const BE_GameFileDetails_T *fileDetailsBuffer;
	BEL_Cross_CreateTrimmedFilename(filename, &trimmedFilename);
	// Get matching filenames list
	// FIXME: BEL_Cross_OpenMatchingGameFileForReading will repeat going
	// through details->reqFiles as of writing this, but for now,
	// this should work.
	for (fileDetailsBuffer = details->reqFiles; fileDetailsBuffer->filenames; ++fileDetailsBuffer)
	{
		bool found = false;
		for (filenames = fileDetailsBuffer->filenames; filenames;)
		{
			BEL_ST_GetNextGameFileName(&filenames, &checkedFilename);
			if (!BE_Cross_strcasecmp(trimmedFilename, checkedFilename))
			{
				found = true;
				break;
			}
		}
		if (found)
			break;
	}
	if (!fileDetailsBuffer->filenames)
		return BE_CROSS_NIL_FILE;
	// Trying writableFilesPath first, and then instPath in case of failure
	BE_FILE_T fp = BEL_Cross_OpenMatchingGameFileForReading(fileDetailsBuffer->filenames, g_be_selectedGameInstallation->writableFilesPath);
	if (fp)
		return fp;
	return BEL_Cross_OpenMatchingGameFileForReading(fileDetailsBuffer->filenames, g_be_selectedGameInstallation->instPath);
}

// Opens a rewritable file for reading in a case-insensitive manner, checking just a single path
BE_FILE_T BE_Cross_open_rewritable_for_reading(const char *filename)
{
	char trimmedFilename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	BEL_Cross_CreateTrimmedFilename(filename, &trimmedFilename);
	return BEL_Cross_apply_file_action_in_dir(trimmedFilename, BE_FILE_REQUEST_READ, g_be_selectedGameInstallation->writableFilesPath, NULL);
}

// Opens a rewritable file for overwriting in a case-insensitive manner, checking just a single path
BE_FILE_T BE_Cross_open_rewritable_for_overwriting(const char *filename)
{
	char trimmedFilename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	BEL_Cross_CreateTrimmedFilename(filename, &trimmedFilename);
	return BEL_Cross_apply_file_action_in_dir(trimmedFilename, BE_FILE_REQUEST_OVERWRITE, g_be_selectedGameInstallation->writableFilesPath, NULL);
}

// Deletes a rewritable file if found, scanning just like BE_Cross_open_rewritable_for_overwriting
void BE_Cross_unlink_rewritable(const char *filename)
{
	char trimmedFilename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	BEL_Cross_CreateTrimmedFilename(filename, &trimmedFilename);
	BEL_Cross_apply_file_action_in_dir(trimmedFilename, BE_FILE_REQUEST_DELETE, g_be_selectedGameInstallation->writableFilesPath, NULL);
}

// Used for e.g., the RefKeen cfg file
BE_FILE_T BE_Cross_open_additionalfile_for_reading(const char *filename)
{
	return BEL_Cross_apply_file_action_in_dir(filename, BE_FILE_REQUEST_READ, g_be_appNewCfgPath, NULL);
}

BE_FILE_T BE_Cross_open_additionalfile_for_overwriting(const char *filename)
{
	// Do this just in case, but note that this isn't recursive
	BEL_Cross_mkdir(g_be_appNewCfgPath); // Non-recursive

	return BEL_Cross_apply_file_action_in_dir(filename, BE_FILE_REQUEST_OVERWRITE, g_be_appNewCfgPath, NULL);
}
