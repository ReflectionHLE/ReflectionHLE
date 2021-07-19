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

#include "refkeen_config.h"

#include "be_gamever.h"
#include "be_st.h" // For g_refKeenCfg

#include "../filesystem/be_filesystem_app_paths.h"
#include "../filesystem/be_filesystem_gameinst.h"
#include "../filesystem/be_filesystem_root_paths.h"
#include "../filesystem/be_filesystem_string_ops.h"
#include "be_features.h"
#include "be_sound_device_flags.h"

#ifdef REFKEEN_PLATFORM_WINDOWS
#include "../misc/be_misc_winreg.h"
#endif

// Use this in case x is a macro defined to be a narrow string literal
#define CSTR_TO_TCSTR(x) _T(x)

#include "be_gamedefs.h"

#if ((defined REFKEEN_HAS_VER_CATACOMB_ALL) && \
     ((defined REFKEEN_PLATFORM_WINDOWS) || (defined REFKEEN_PLATFORM_MACOS))) || \
    (((defined REFKEEN_HAS_VER_WL6AC14) || \
      (defined REFKEEN_HAS_VER_SODAC14) || \
      (defined REFKEEN_HAS_VER_N3DWT10)) && (defined REFKEEN_PLATFORM_WINDOWS))
#define BE_CHECK_GOG_INSTALLATIONS
#endif

static void BEL_Cross_TryAddInst_Common(
	TCHAR (*path)[BE_CROSS_PATH_LEN_BOUND], const BE_GameVerDetails_T **gameVers,
	const TCHAR **gameVerSubPaths, const char *descStr)
{
	const BE_GameVerDetails_T **gameVer;
	const TCHAR **gameVerSubPath;
	size_t len = _tcslen(*path);
	for (gameVer = gameVers, gameVerSubPath = gameVerSubPaths; *gameVer; ++gameVer)
	{
		if (gameVerSubPath)
			BEL_Cross_safeandfastctstringcopy((*path)+len, (*path)+BE_Cross_ArrayLen(*path)-len, *gameVerSubPath++);
		BEL_Cross_ConditionallyAddGameInstallation(*gameVer, *path, descStr);
	}
}

#ifdef REFKEEN_PLATFORM_WINDOWS
static void BEL_Cross_TryAddRegistryInst(
	const TCHAR *registryKey, const TCHAR *registryValue,
	const TCHAR *subPath, const BE_GameVerDetails_T **gameVers,
	const TCHAR **gameVerSubPaths, const char *descStr)
{
	int numOfCheckedPaths = 0, i;
	TCHAR checkedPaths[3][BE_CROSS_PATH_LEN_BOUND], **gameVerSubPath;
	const BE_GameVerDetails_T **gameVer;

	BEL_Cross_Registry_TryGetPaths(
		registryKey, registryValue, &checkedPaths, &numOfCheckedPaths);
	for (i = 0; i < numOfCheckedPaths; ++i)
	{
		BEL_Cross_safeandfastctstringcopy(checkedPaths[i]+_tcslen(checkedPaths[i]), checkedPaths[i]+BE_Cross_ArrayLen(checkedPaths[i])-_tcslen(checkedPaths[i]), subPath);
		BEL_Cross_TryAddInst_Common(&checkedPaths[i], gameVers, gameVerSubPaths, descStr);
	}
}
#endif

#ifdef REFKEEN_PLATFORM_UNIX
static void BEL_Cross_TryAddSteamInst(
	const TCHAR *homeVar,
	const TCHAR *subPath, const BE_GameVerDetails_T **gameVers,
	const TCHAR **gameVerSubPaths, const char *descStr)
{
	const BE_GameVerDetails_T **gameVer;
	TCHAR checkedPath[BE_CROSS_PATH_LEN_BOUND];

	if (homeVar && *homeVar)
	{
#ifdef REFKEEN_PLATFORM_MACOS
		BE_Cross_safeandfastcstringcopy_3strs(checkedPath, checkedPath+BE_Cross_ArrayLen(checkedPath), homeVar, "/Library/Application Support/Steam/SteamApps/common", subPath);
		BEL_Cross_TryAddInst_Common(&checkedPath, gameVers, gameVerSubPaths, descStr);
#else
		// They changed from SteamApps to steamapps at some point, so check both
		BE_Cross_safeandfastcstringcopy_3strs(checkedPath, checkedPath+BE_Cross_ArrayLen(checkedPath), homeVar, "/.steam/steam/SteamApps/common", subPath);
		BEL_Cross_TryAddInst_Common(&checkedPath, gameVers, gameVerSubPaths, descStr);
		BE_Cross_safeandfastcstringcopy_3strs(checkedPath, checkedPath+BE_Cross_ArrayLen(checkedPath), homeVar, "/.steam/steam/steamapps/common", subPath);
		BEL_Cross_TryAddInst_Common(&checkedPath, gameVers, gameVerSubPaths, descStr);
#endif
	}
}
#endif

static void BEL_Cross_CheckForKnownInstallations(void)
{
#ifdef REFKEEN_PLATFORM_UNIX // Including MACOS
	const char *homeVar = getenv("HOME");
#endif


#ifdef REFKEEN_HAS_VER_KDREAMS
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
	const BE_GameVerDetails_T *kdreamsVers[] = {&g_be_gamever_kdreams2015, 0};

#ifdef REFKEEN_PLATFORM_WINDOWS
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 356200"),
		_T("INSTALLLOCATION"), _T(""), kdreamsVers, NULL, "Steam");
#else
	BEL_Cross_TryAddSteamInst(
#if (defined REFKEEN_PLATFORM_MACOS)
		homeVar, _T("/Keen Dreams/KDreams.app/Contents/Resources"),
#else
		homeVar, _T("/Keen Dreams"),
#endif
		kdreamsVers, NULL, "Steam");
#endif // PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_KDREAMS


#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) && (defined BE_CHECK_GOG_INSTALLATIONS)
#if (defined REFKEEN_PLATFORM_WINDOWS) || (defined REFKEEN_PLATFORM_MACOS)

	const BE_GameVerDetails_T *catacombVers[] = {
#ifdef REFKEEN_HAS_VER_CAT3D
		&g_be_gamever_cat3d122,
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
		&g_be_gamever_catabyss124,
#endif
#ifdef REFKEEN_HAS_VER_CATARM
		&g_be_gamever_catarm102,
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
		&g_be_gamever_catapoc101,
#endif
		0,
	};

#ifdef REFKEEN_PLATFORM_WINDOWS
	const TCHAR *catacombSubdirs[] = {
#ifdef REFKEEN_HAS_VER_CAT3D
		_T("\\Cat3D"),
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
		_T("\\Abyss"),
#endif
#ifdef REFKEEN_HAS_VER_CATARM
		_T("\\Armageddon"),
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
		_T("\\Apocalypse"),
#endif
		0,
	};

	// Old location
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GOGCATACOMBSPACK"), _T("PATH"), _T(""),
		catacombVers, catacombSubdirs, "GOG.com");
	// New location
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GAMES\\1207659189"), _T("PATH"), _T(""),
		catacombVers, catacombSubdirs, "GOG.com");
#endif

#ifdef REFKEEN_PLATFORM_MACOS
	const TCHAR *catacombSubdirs[] = {
#ifdef REFKEEN_HAS_VER_CAT3D
		_T("/2CAT3D"),
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
		_T("/3CABYSS"),
#endif
#ifdef REFKEEN_HAS_VER_CATARM
		_T("/4CATARM"),
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
		_T("/5APOC"),
#endif
	};

	TCHAR catacombsMacPath[BE_CROSS_PATH_LEN_BOUND];
	static const TCHAR *catacombsMacInst = _T("/Applications/Catacombs Pack/Catacomb Pack.app/Contents/Resources/Catacomb Pack.boxer/C 1 CATACOMB.harddisk");
	BEL_Cross_safeandfastctstringcopy(
		catacombsMacPath,
		catacombsMacPath+BE_Cross_ArrayLen(catacombsMacPath),
		catacombsMacInst);
	BEL_Cross_TryAddInst_Common(&catacombsMacPath, catacombVers, catacombSubdirs, "GOG.com");
	if (homeVar && *homeVar)
	{
		BEL_Cross_safeandfastctstringcopy_2strs(
			catacombsMacPath,
			catacombsMacPath+BE_Cross_ArrayLen(catacombsMacPath),
			homeVar, catacombsMacInst);
		BEL_Cross_TryAddInst_Common(&catacombsMacPath, catacombVers, catacombSubdirs, "GOG.com");
	}
#endif

#endif // (defined REFKEEN_PLATFORM_WINDOWS) || (defined REFKEEN_PLATFORM_MACOS)
#endif // (defined REFKEEN_HAS_VER_CATACOMB_ALL) && (defined BE_CHECK_GOG_INSTALLATIONS)

#ifdef REFKEEN_HAS_VER_WL1AP14
#ifdef REFKEEN_PLATFORM_WINDOWS
	const BE_GameVerDetails_T *wolf3dapoVers[] = {&g_be_gamever_wl6ap14, 0};
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\3DREALMS\\WOLFENSTEIN 3D"), NULL, _T("\\Wolfenstein 3D"),
		wolf3dapoVers, NULL, "3drealms.com");
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\3DREALMS\\ANTHOLOGY"), NULL, _T("\\Wolfenstein 3D"),
		wolf3dapoVers, NULL, "3drealms.com");
#endif
#endif

#ifdef REFKEEN_HAS_VER_WL6AC14
	const BE_GameVerDetails_T *wolf3dactVers[] = {&g_be_gamever_wl6ac14, 0};

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#ifdef REFKEEN_PLATFORM_WINDOWS
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 2270"),
		_T("INSTALLLOCATION"), _T("\\base"), wolf3dactVers, NULL, "Steam");
#else
	BEL_Cross_TryAddSteamInst(
		homeVar, _T("/Wolfenstein 3D/base"), wolf3dactVers, NULL, "Steam");
#endif // PLATFORM
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

#if (defined REFKEEN_PLATFORM_WINDOWS) && (defined BE_CHECK_GOG_INSTALLATIONS)
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GAMES\\1441705046"),
		_T("PATH"), _T(""),
		wolf3dactVers, NULL, "GOG.com");
#endif // (defined REFKEEN_PLATFORM_WINDOWS) && (defined BE_CHECK_GOG_INSTALLATIONS)

#endif // REFKEEN_HAS_VER_WL6AC14

#ifdef REFKEEN_HAS_VER_SODAC14
	const BE_GameVerDetails_T *sodVers[] = {
		&g_be_gamever_sodac14, &g_be_gamever_sd2ac14,
		&g_be_gamever_sd3ac14, 0
	};

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#ifdef REFKEEN_PLATFORM_WINDOWS
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 9000"),
		_T("INSTALLLOCATION"), _T("\\base"), sodVers, NULL, "Steam");
#else
	BEL_Cross_TryAddSteamInst(
		homeVar, _T("/Spear of Destiny/base"), sodVers, NULL, "Steam");
#endif // PLATFORM
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

#if (defined REFKEEN_PLATFORM_WINDOWS) && (defined BE_CHECK_GOG_INSTALLATIONS)
	// Technically, just one mission should reside in each subdir, but since
	// we already have all vers in an array, simply look for all of them
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GAMES\\1441705126"),
		_T("PATH"), _T("\\M1"),
		sodVers, NULL, "GOG.com");
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GAMES\\1441705126"),
		_T("PATH"), _T("\\M2"),
		sodVers, NULL, "GOG.com");
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GAMES\\1441705126"),
		_T("PATH"), _T("\\M3"),
		sodVers, NULL, "GOG.com");
#endif // (defined REFKEEN_PLATFORM_WINDOWS) && (defined BE_CHECK_GOG_INSTALLATIONS)

#endif // REFKEEN_HAS_VER_SODAC14

#ifdef REFKEEN_HAS_VER_N3DWT10
	const BE_GameVerDetails_T *noah3dVers[] = {&g_be_gamever_n3dwt10, 0};

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#ifdef REFKEEN_PLATFORM_WINDOWS
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 371180"),
		_T("INSTALLLOCATION"), _T(""), noah3dVers, NULL, "Steam");
#else
	BEL_Cross_TryAddSteamInst(
		homeVar, _T("/Super 3-D Noah's Ark"), noah3dVers, NULL, "Steam");
#endif // PLATFORM
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

#if (defined REFKEEN_PLATFORM_WINDOWS) && (defined BE_CHECK_GOG_INSTALLATIONS)
	BEL_Cross_TryAddRegistryInst(
		_T("SOFTWARE\\GOG.COM\\GAMES\\1672565562"),
		_T("PATH"), _T(""),
		noah3dVers, NULL, "GOG.com");
#endif // (defined REFKEEN_PLATFORM_WINDOWS) && (defined BE_CHECK_GOG_INSTALLATIONS)

#endif // REFKEEN_HAS_VER_N3DWT10
}

void BE_Cross_PrepareGameInstallations(void)
{
	/*** Reset these ***/
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
		g_be_gameinstallationsbyver[i] = NULL;

	/*** Some misc. preparation ***/

	TCHAR path[BE_CROSS_PATH_LEN_BOUND];
	TCHAR *pathEnd = path + BE_Cross_ArrayLen(path);

	// Try recognizing each supported game version in the current dir
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
		BEL_Cross_ConditionallyAddGameInstallation(g_be_gamever_ptrs[i], _T("."), "Local");

	// Go over possible existing installations
	if (!g_refKeenCfg.manualGameVerMode)
		BEL_Cross_CheckForKnownInstallations();

	/*** Finally check any custom dir ***/
	char buffer[2*BE_CROSS_PATH_LEN_BOUND];
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
	{
		const BE_GameVerDetails_T *details = g_be_gamever_ptrs[i];
		if (g_be_gameinstallationsbyver[details->verId])
			continue;

		BEL_Cross_safeandfastctstringcopy_4strs(path, pathEnd, g_be_appDataPath, _T("/"), details->writableFilesDir, _T(".txt"));
		FILE *fp = _tfopen(path, _T("r"));
		if (!fp)
			continue;

		bool lineRead = fgets(buffer, sizeof(buffer), fp);
		fclose(fp);

		if (!lineRead)
			continue;

		for (char *ptr = buffer; *ptr; ++ptr)
		{
			if (*ptr == '\n')
			{
				*ptr = '\0';
				break;
			}
			if ((*ptr < 32) || (*ptr >= 127)) // Non-ASCII?
				continue;
		}

		int j;
		for (j = 0; j < g_be_rootPathsNum; ++j)
			if (!strncmp(buffer, g_be_rootPathsKeys[j], strlen(g_be_rootPathsKeys[j])))
				break;
		if (j == g_be_rootPathsNum)
			continue;

		const char *dirWithoutRoot = buffer + strlen(g_be_rootPathsKeys[j]);
		if (*dirWithoutRoot == '\0')
			continue;
		++dirWithoutRoot;
		// Finally fill the path and conditionally add it
		BEL_Cross_safeandfastcstringcopytoctstring(BEL_Cross_safeandfastctstringcopy_2strs(path, pathEnd, g_be_rootPaths[j], _T("/")), pathEnd, dirWithoutRoot);
		BEL_Cross_ConditionallyAddGameInstallation(details, path, NULL);
	}
	/*** To finish, apply a simple bubble sort on the game installations, by game version (done for visual consistency in the launcher) ***/
	for (int count = g_be_gameinstallations_num - 1; count > 0; --count)
		for (int i = 0; i < count; ++i)
			if (g_be_gameinstallations[i].verId > g_be_gameinstallations[i+1].verId)
			{
				// FIXME - Copies are just a little bit expensive, but not a lot...
				BE_GameInstallation_T tempInst = g_be_gameinstallations[i];
				g_be_gameinstallations[i] = g_be_gameinstallations[i+1];
				g_be_gameinstallations[i+1] = tempInst;

				g_be_gameinstallationsbyver[g_be_gameinstallations[i].verId] = &g_be_gameinstallations[i];
				g_be_gameinstallationsbyver[g_be_gameinstallations[i+1].verId] = &g_be_gameinstallations[i+1];
			}
}
