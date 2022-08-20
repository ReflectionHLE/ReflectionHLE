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

#include "refkeen_config.h"
#include "../filesystem/be_filesystem_len_bounds.h"
#include "../filesystem/be_filesystem_string_ops.h"
#include "../filesystem/be_filesystem_tchar.h"
#include "be_cross.h"

#ifdef REFKEEN_PLATFORM_WINDOWS
#include <shlwapi.h>
#endif

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
BE_FILE_T BE_Cross_open_steamcfg_for_reading(void)
{
	TCHAR steam_config_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS

	DWORD dwType = 0;
	DWORD dwSize = sizeof(steam_config_path);
	LSTATUS status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\VALVE\\STEAM"), _T("INSTALLPATH"), &dwType, steam_config_path, &dwSize);
	if ((status != ERROR_SUCCESS) || (dwType != REG_SZ))
		return NULL;
	BEL_Cross_safeandfastctstringcopy(steam_config_path + _tcslen(steam_config_path), steam_config_path + BE_Cross_ArrayLen(steam_config_path), _T("\\config\\config.vdf"));

#elif (defined REFKEEN_PLATFORM_UNIX)

	const char *homeVar = getenv("HOME");
	if (!homeVar || !(*homeVar))
		return NULL;

#ifdef REFKEEN_PLATFORM_MACOS
	BE_Cross_safeandfastcstringcopy_2strs(steam_config_path, steam_config_path+BE_Cross_ArrayLen(steam_config_path), homeVar, "/Library/Application Support/Steam/config/config.vdf");
#else
	BE_Cross_safeandfastcstringcopy_2strs(steam_config_path, steam_config_path+BE_Cross_ArrayLen(steam_config_path), homeVar, "/.steam/steam/config/config.vdf");
#endif

#endif // UNIX
	return _tfopen(steam_config_path, _T("rb"));
}
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
