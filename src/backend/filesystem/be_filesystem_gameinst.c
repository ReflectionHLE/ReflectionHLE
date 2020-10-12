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

#include "be_filesystem_app_paths.h"
#include "be_filesystem_file_ops.h"
#include "be_filesystem_gameinst.h"
#include "be_filesystem_mkdir.h"
#include "be_filesystem_string_ops.h"
#include "be_st.h"
#include "crc32/crc32.h"

BE_GameInstallation_T g_be_gameinstallations[BE_CROSS_MAX_GAME_INSTALLATIONS];
int g_be_gameinstallations_num;

const BE_GameInstallation_T *g_be_gameinstallationsbyver[BE_GAMEVER_LAST];
const BE_GameInstallation_T *g_be_selectedGameInstallation;

const char *BE_Cross_GetGameInstallationDescription(int num)
{
	return g_be_gameinstallations[num].descStr;
}

int BE_Cross_GetGameVerFromInstallation(int num)
{
	return g_be_gameinstallations[num].verId;
}

static bool BEL_IsFileMatching(BE_FILE_T fp, const BE_GameFileDetails_T *details)
{
	uint32_t crc32;
	if (details->filesize != BE_Cross_FileLengthFromHandle(fp))
		return false;
	long int origOffset = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	bool ret = (!Crc32_ComputeFile(fp, &crc32) && (crc32 == details->crc32));
	fseek(fp, origOffset, SEEK_SET);
	return ret;
}

void BEL_ST_GetNextGameFileName(
	const char **filenames,
	char (*outFilename)[BE_CROSS_DOS_FILENAME_LEN_BOUND])
{
	const char *next = strchr(*filenames, '|');
	if (outFilename)
	{
		unsigned len = next ? (next - *filenames + 1) : 0;
		if (!len || (len > sizeof(*outFilename)))
			len = sizeof(*outFilename);
		BE_Cross_safeandfastcstringcopy(*outFilename, *outFilename + len, *filenames);
	}
	*filenames = next ? next + 1 : NULL;
}

BE_FILE_T BEL_Cross_OpenMatchingGameFileForReading(const char *filenames, const TCHAR *path)
{
	const BE_GameVerDetails_T *details = g_be_gamever_ptrs[refkeen_current_gamever];
	const BE_GameFileDetails_T *fileDetailsBuffer;
	char filename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	const char *ptr;
	for (fileDetailsBuffer = details->reqFiles; fileDetailsBuffer->filenames; ++fileDetailsBuffer)
		if (!strcmp(fileDetailsBuffer->filenames, filenames))
			break;
	if (!fileDetailsBuffer->filenames)
		return BE_CROSS_NIL_FILE;

	for (ptr = filenames; ptr;)
	{
		BEL_ST_GetNextGameFileName(&ptr, &filename);
		BE_FILE_T fp = BEL_Cross_apply_file_action_in_dir(filename, BE_FILE_REQUEST_READ, path, NULL);
		if (!fp)
			continue;

		if (BEL_IsFileMatching(fp, fileDetailsBuffer))
			return fp;
		BE_Cross_close(fp);
	}
	return BE_CROSS_NIL_FILE;
}

// Returns 0 if not found, 1 if found with some data mismatch, or 2 otherwise
//
// OPTIONAL ARGUMENT: outfullpath, if not NULL, is filled with
// the file's full path if found, even if its data isn't matching.
static int BEL_Cross_CheckGameFileDetails(
	const BE_GameFileDetails_T *details, const TCHAR *searchdir,
	TCHAR (*outfullpath)[BE_CROSS_PATH_LEN_BOUND])
{
	int ret = 0;
	char filename[BE_CROSS_DOS_FILENAME_LEN_BOUND];
	const char *ptr;
	for (ptr = details->filenames; ptr;)
	{
		BEL_ST_GetNextGameFileName(&ptr, &filename);
		BE_FILE_T fp = BEL_Cross_apply_file_action_in_dir(filename, BE_FILE_REQUEST_READ, searchdir, outfullpath);
		if (!fp)
			continue;

		if (BEL_IsFileMatching(fp, details))
		{
			BE_Cross_close(fp);
			return 2;
		}
		BE_Cross_close(fp);
		ret = 1;
	}
	return ret;
}

void BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg(
	const BE_GameVerDetails_T *details, const TCHAR *searchdir,
	const char *descStr, BE_TryAddGameInstallation_ErrorMsg_T *outErrMsg)
{
	char errorMsg[256];

	if (g_be_gameinstallationsbyver[details->verId])
		return;

	if (g_be_gameinstallations_num >= BE_CROSS_MAX_GAME_INSTALLATIONS)
		BE_ST_ExitWithErrorMsg("BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg: Too many game installations!");

	BE_GameInstallation_T *gameInstallation = &g_be_gameinstallations[g_be_gameinstallations_num];
	// If used correctly then these SHOULD have enough space
	BEL_Cross_safeandfastctstringcopy(gameInstallation->instPath, gameInstallation->instPath+sizeof(gameInstallation->instPath)/sizeof(TCHAR), searchdir);
	gameInstallation->verId = details->verId;
	snprintf(gameInstallation->descStr, sizeof(gameInstallation->descStr),
	         "%s (%s)", details->description, descStr ? descStr : "Custom");

	TCHAR tempFullPath[BE_CROSS_PATH_LEN_BOUND];

	TCHAR *endPtr = gameInstallation->writableFilesPath + sizeof(gameInstallation->writableFilesPath)/sizeof(TCHAR);
	BEL_Cross_safeandfastctstringcopy_3strs(gameInstallation->writableFilesPath, endPtr, g_be_appDataPath, _T("/"), details->writableFilesDir);

	for (const BE_GameFileDetails_T *fileDetailsBuffer = details->reqFiles; fileDetailsBuffer->filenames; ++fileDetailsBuffer)
	{
		// Check in writableFilesPath first. If WRONG file is found, REMOVE(!)
		switch (BEL_Cross_CheckGameFileDetails(fileDetailsBuffer, gameInstallation->writableFilesPath, &tempFullPath))
		{
		case 2: // Match found
			continue;
		case 1: // Wrong file found in writableFilesPath: DELETE, then verify it's actually deleted (if there are multiple files differing just by case, this is an error, too.)
		{
			// Actually, there's a special case in which we don't delete and even accept the different file...
			// Each EXE must be unmodified, though (used for version identification, and possibly also the extraction of embedded resources)
			// But it's also possible to have no EXE (e.g., Keen Dreams, 2015 release) so check this.
			//
			// KNOWN LIMITATION: Resources embedded in an EXE may *not* be modified.
			if (g_refKeenCfg.manualGameVerMode)
			{
				const BE_EXEFileDetails_T *exeFileDetailsBuffer;
				for (exeFileDetailsBuffer = details->exeFiles; exeFileDetailsBuffer->mainFuncPtr; ++exeFileDetailsBuffer)
					if (!BE_Cross_strcasecmp(fileDetailsBuffer->filenames, exeFileDetailsBuffer->exeNames))
						break;

				if (!exeFileDetailsBuffer->mainFuncPtr) // fileDetailsBuffer does not refer to an EXE file, so it may be skipped
					continue;
			}

			_tremove(tempFullPath);
			// FIXME: Rather modify BEL_Cross_CheckGameFileDetails to
			// optionally fill the actual file name itself, and then
			// try to re-open it, possibly with a different letter case.
			// This whole seytup may generally be better, either way.
			if (BEL_Cross_CheckGameFileDetails(fileDetailsBuffer, gameInstallation->writableFilesPath, NULL) >= 1)
			{
				snprintf(
					errorMsg, sizeof(errorMsg),
					"BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg: Cannot remove file with\n"
					"unexpected contents! Alternatively, one such file has been removed, but there's\n"
					"another one differing just by case, or with another matching name.\n"
					"Possible filenames: %s", fileDetailsBuffer->filenames);
				BE_ST_ExitWithErrorMsg(errorMsg);
			}
			break;
		}
		}
		// No match found (and possibly deleted wrong file from writableFilesPath), recheck in installation path
		switch (BEL_Cross_CheckGameFileDetails(fileDetailsBuffer, searchdir, NULL))
		{
		case 2: // Match found
			continue;
		case 1: // Wrong file found
			if (g_refKeenCfg.manualGameVerMode)
				continue; // A special case again (where a wrong file is acceptable)
			// Fall-through
		default:
			// (Matching, and in manual mode, also wrong) file not found, we cannot add a new game installation
			if (outErrMsg)
				BE_Cross_safeandfastcstringcopy_2strs(*outErrMsg, (*outErrMsg) + sizeof(*outErrMsg), "Wrong or missing file: ", fileDetailsBuffer->filenames);
			return;
		}
	}

	++g_be_gameinstallations_num; // We KNOW we have the required data at this point

	// Create dirs in case we need just writableFilesPath (creation isn't recursive)
	BEL_Cross_mkdir(g_be_appDataPath); // Non-recursive
	BEL_Cross_mkdir(gameInstallation->writableFilesPath);

	// Finish with this
	g_be_gameinstallationsbyver[details->verId] = gameInstallation;
}
