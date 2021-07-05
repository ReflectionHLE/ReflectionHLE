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

#include "be_gamedefs_structs.h"
#include <string.h>

const char *BE_Cross_GetEXEFileDescriptionStrForGameVer(const char *exeFileName, int verId)
{
	const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[verId]->exeFiles;
	for (; exeFile->mainFuncPtr && (!exeFile->exeNames || strcmp(exeFile->exeNames, exeFileName)); ++exeFile)
		;
	return (exeFile->mainFuncPtr ? exeFile->subDescription : NULL); // subDescription may also be NULL
}

void (*BE_Cross_GetAccessibleMainFuncPtrForGameVer(const char *exeFileName, int verId))(void)
{
	const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[verId]->exeFiles;
	for (; exeFile->mainFuncPtr && (!exeFile->exeNames || strcmp(exeFile->exeNames, exeFileName)); ++exeFile)
		;
	return ((exeFile->mainFuncPtr && exeFile->subDescription) ? exeFile->mainFuncPtr : g_be_gamever_ptrs[verId]->exeFiles->mainFuncPtr);
}

int BE_Cross_GetAccessibleEXEsCountForGameVer(int verId)
{
	int nOfEXEs = 1;
	for (const BE_EXEFileDetails_T *exeFile = 1 + g_be_gamever_ptrs[verId]->exeFiles; exeFile->mainFuncPtr; ++exeFile) // Skip first entry, which is the "default" accessible main function
		if (exeFile->subDescription)
			++nOfEXEs;
	return nOfEXEs;
}

void BE_Cross_FillAccessibleEXEFileNamesForGameVer(int verId, const char **outStrs)
{
	*outStrs++ = "Just launch the game normally";
	for (const BE_EXEFileDetails_T *exeFile = 1 + g_be_gamever_ptrs[verId]->exeFiles; exeFile->mainFuncPtr; ++exeFile) // Skip first entry
		if (exeFile->subDescription)
			*outStrs++ = exeFile->subDescription;
}

void (*BE_Cross_GetAccessibleEXEFuncPtrForGameVerByIndex(int index, int verId))(void)
{
	if (!index)
		return g_be_gamever_ptrs[verId]->exeFiles->mainFuncPtr;

	const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[verId]->exeFiles + 1;
	for (int i = 0; true; ++exeFile)
		if (exeFile->subDescription)
			if (++i == index)
				break;

	return exeFile->mainFuncPtr;
}


int BE_Cross_GetSelectedGameVerAudioDeviceFlags(void)
{
	return g_be_gamever_ptrs[refkeen_current_gamever]->audioDeviceFlags;
}
