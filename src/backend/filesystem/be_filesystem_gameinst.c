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

// Returns 0 if not found, 1 if found with some data mismatch, or 2 otherwise
//
// OPTIONAL ARGUMENT: outfullpath, if not NULL, is filled with
// the file's full path if found, even if its data isn't matching.
static int BEL_Cross_CheckGameFileDetails(
	const BE_GameFileDetails_T *details, const TCHAR *searchdir,
	TCHAR (*outfullpath)[BE_CROSS_PATH_LEN_BOUND])
{
	BE_FILE_T fp = BEL_Cross_apply_file_action_in_dir(details->filename, BE_FILE_REQUEST_READ, searchdir, outfullpath);
	if (!fp)
		return 0;

	if (details->filesize == BE_Cross_FileLengthFromHandle(fp))
	{
		uint32_t crc32;
		if (!Crc32_ComputeFile(fp, &crc32) && (crc32 == details->crc32))
		{
			BE_Cross_close(fp);
			return 2;
		}
	}
	BE_Cross_close(fp);
	return 1;
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
	gameInstallation->descStr = descStr; // ASSUMPTION: This is a C string literal!!!

	TCHAR tempFullPath[BE_CROSS_PATH_LEN_BOUND];

	TCHAR *endPtr = gameInstallation->writableFilesPath + sizeof(gameInstallation->writableFilesPath)/sizeof(TCHAR);
	BEL_Cross_safeandfastctstringcopy_3strs(gameInstallation->writableFilesPath, endPtr, g_be_appDataPath, _T("/"), details->writableFilesDir);

	for (const BE_GameFileDetails_T *fileDetailsBuffer = details->reqFiles; fileDetailsBuffer->filename; ++fileDetailsBuffer)
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
					if (!BE_Cross_strcasecmp(fileDetailsBuffer->filename, exeFileDetailsBuffer->exeName))
						break;

				if (!exeFileDetailsBuffer->mainFuncPtr) // fileDetailsBuffer does not refer to an EXE file, so it may be skipped
					continue;
			}

			_tremove(tempFullPath);
			BE_FILE_T fp = BEL_Cross_apply_file_action_in_dir(fileDetailsBuffer->filename, BE_FILE_REQUEST_READ, gameInstallation->writableFilesPath, NULL);
			if (fp)
			{
				fclose(fp);
				snprintf(errorMsg, sizeof(errorMsg), "BEL_Cross_ConditionallyAddGameInstallation_WithReturnedErrMsg: Cannot remove file with\nunexpected contents! Alternatively, one such file has been removed, but there's\nanother one differing just by case.\nFilename: %s", fileDetailsBuffer->filename);
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
				BE_Cross_safeandfastcstringcopy_2strs(*outErrMsg, (*outErrMsg) + sizeof(*outErrMsg), "Wrong or missing file ", fileDetailsBuffer->filename);
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
