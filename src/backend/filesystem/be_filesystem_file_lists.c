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

#include "refkeen_config.h"
#include "be_features.h"
#include "be_filesystem_dir.h"
#include "be_filesystem_gameinst.h"
#include "be_filesystem_string_ops.h"
#include "be_filesystem_tchar.h"

// MICRO-OPTIMIZATION: Not needed for all games
#ifdef BE_CROSS_ENABLE_SORTED_FILENAMES_FUNC
int BE_Cross_GetSortedRewritableFilenames_AsUpperCase(char *outFilenames, int maxNum, int strLenBound, const char *suffix)
{
	TCHAR *d_name;
	size_t sufLen = strlen(suffix);
	char *nextFilename = outFilenames, *outFilenamesEnd = outFilenames + maxNum*strLenBound, *outFilenamesLast = outFilenamesEnd - strLenBound;
	char *checkFilename, *checkCh, *dnameCStr;
	// For the sake of consistency we look for files just in this path
	BE_DIR_T dir = BEL_Cross_OpenDir(g_be_selectedGameInstallation->writableFilesPath);
	if (!dir)
	{
		return 0;
	}
	for (d_name = BEL_Cross_ReadDir(dir); d_name; d_name = BEL_Cross_ReadDir(dir))
	{
		size_t len = _tcslen(d_name);
		TCHAR *tchPtr;
		/*** Ignore non-ASCII filenames ***/
		if (*BEL_Cross_tstr_find_nonascii_ptr(d_name))
		{
			continue;
		}
		if ((len < sufLen) || BEL_Cross_tstr_to_cstr_ascii_casecmp(d_name+len-sufLen, suffix))
		{
			continue;
		}
		len -= sufLen;
		/*** Possibly a HACK - Modify d_name itself ***/
		len = (len >= (size_t)strLenBound) ? (strLenBound-1) : len;
		d_name[len] = _T('\0');
		/*** Another HACK - Further convert d_name from wide string on Windows (and watch out due to strict aliasing rules) ***/
		tchPtr = d_name;
		dnameCStr = (char *)tchPtr;
		for (checkCh = dnameCStr; *tchPtr; ++checkCh, ++tchPtr)
		{
			*checkCh = BE_Cross_toupper(*tchPtr); // Even if *tchPtr is a wide char, we know it's an ASCII char at this point
		}
#ifdef REFKEEN_PLATFORM_WINDOWS
		*checkCh = '\0'; // Required if converted from wide string
#endif
		// This is basically insertion-sort, but we store
		// the *last* entries if there isn't enough room.
		for (checkFilename = outFilenames; checkFilename < nextFilename; checkFilename += strLenBound)
		{
			if (strcmp(checkFilename, dnameCStr) > 0)
			{
				break;
			}
		}
		// Gone over all inserted entries
		if (checkFilename == nextFilename)
		{
			if (nextFilename < outFilenamesEnd)
			{
				memcpy(nextFilename, dnameCStr, 1+len);
				nextFilename += strLenBound;
			}
			else
			{
				memmove(outFilenames, outFilenames+strLenBound, strLenBound*(maxNum-1));
				memcpy(outFilenamesLast, dnameCStr, 1+len);
			}
		}
		// Shift existing entries and insert new one
		else
		{
			// If there's room for another entry, shift "forward"
			if (nextFilename < outFilenamesEnd)
			{
				memmove(checkFilename + strLenBound, checkFilename, outFilenamesEnd-checkFilename-strLenBound);
				memcpy(checkFilename, dnameCStr, 1+len);
				nextFilename += strLenBound;
			}
			// Otherwise shift "backwards", but only if there's already an entry "smaller" than current one
			else if (checkFilename != outFilenames)
			{
				memmove(outFilenames, outFilenames+strLenBound, (checkFilename-strLenBound)-outFilenames);
				memcpy(checkFilename-strLenBound, dnameCStr, 1+len);
			}
		};
	}
	BEL_Cross_CloseDir(dir);
	return (nextFilename-outFilenames)/strLenBound;
}
#endif


