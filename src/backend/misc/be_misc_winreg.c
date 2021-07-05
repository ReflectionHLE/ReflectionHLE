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
#include "be_misc_winreg.h"
#include <windows.h>
#include <shlwapi.h>

void BEL_Cross_Registry_TryGetPaths(
	const TCHAR *key, const TCHAR *value,
	TCHAR (*paths)[3][BE_CROSS_PATH_LEN_BOUND], int *count)
{
	DWORD dwType, dwSize;
	TCHAR data[BE_CROSS_PATH_LEN_BOUND];
	HKEY hkResult;

	*count = 0;
	dwSize = sizeof(data);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ|KEY_WOW64_64KEY, &hkResult) == ERROR_SUCCESS)
	{
		if ((RegQueryValueEx(hkResult, value, NULL, &dwType, (LPBYTE)data, &dwSize) == ERROR_SUCCESS) &&
		    (dwType == REG_SZ))
			memcpy((*paths)[(*count)++], data, dwSize);
		RegCloseKey(hkResult);
	}
	
	dwSize = sizeof(data);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ|KEY_WOW64_32KEY, &hkResult) == ERROR_SUCCESS)
	{
		if ((RegQueryValueEx(hkResult, value, NULL, &dwType, (LPBYTE)data, &dwSize) == ERROR_SUCCESS) &&
		    (dwType == REG_SZ))
			memcpy((*paths)[(*count)++], data, dwSize);
		RegCloseKey(hkResult);
	}

	dwSize = sizeof(data);
	if ((SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\GOG.COM\\GOGCATACOMBSPACK"), _T("PATH"), &dwType, data, &dwSize)
	     == ERROR_SUCCESS) && (dwType == REG_SZ))
		memcpy((*paths)[(*count)++], data, dwSize);
}

