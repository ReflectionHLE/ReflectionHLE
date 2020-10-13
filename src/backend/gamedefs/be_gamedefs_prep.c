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

#include "refkeen_config.h"

#include "be_gamever.h"
#include "be_st.h" // For g_refKeenCfg

#include "backend/filesystem/be_filesystem_app_paths.h"
#include "backend/filesystem/be_filesystem_gameinst.h"
#include "backend/filesystem/be_filesystem_root_paths.h"
#include "backend/filesystem/be_filesystem_string_ops.h"
#include "be_features.h"
#include "be_sound_device_flags.h"

// Use this in case x is a macro defined to be a narrow string literal
#define CSTR_TO_TCSTR(x) _T(x)

#include "backend/gamedefs/be_gamedefs.h"

#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) && ((defined REFKEEN_PLATFORM_WINDOWS) || (defined REFKEEN_PLATFORM_MACOS))
#define BE_CHECK_GOG_INSTALLATIONS

#ifdef REFKEEN_PLATFORM_WINDOWS
static const TCHAR *g_be_catacombs_gog_subdirnames_withdirsep[] = {_T("\\Cat3D"), _T("\\Abyss"), _T("\\Armageddon"), _T("\\Apocalypse")};
#endif
#ifdef REFKEEN_PLATFORM_MACOS
static const TCHAR *g_be_catacombs_gog_subdirnames_withdirsep[] = {_T("/2CAT3D"), _T("/3CABYSS"), _T("/4CATARM"), _T("/5APOC")};
#endif

#endif

void BE_Cross_PrepareGameInstallations(void)
{
	/*** Reset these ***/
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
		g_be_gameinstallationsbyver[i] = NULL;

	/*** Some misc. preparation ***/

	TCHAR path[BE_CROSS_PATH_LEN_BOUND];
	TCHAR *pathEnd = path + sizeof(path)/sizeof(TCHAR);

	// Try recognizing each supported game version in the current dir
	for (int i = 0; i < BE_GAMEVER_LAST; ++i)
		BEL_Cross_ConditionallyAddGameInstallation(g_be_gamever_ptrs[i], _T("."), "Local");

	if (!g_refKeenCfg.manualGameVerMode)
	{
		// A few common definitions
#ifdef REFKEEN_PLATFORM_WINDOWS
		DWORD dwType;
		DWORD dwSize;
		LSTATUS status;
#elif (defined REFKEEN_PLATFORM_UNIX) // Including MACOS
		const char *homeVar = getenv("HOME");
#endif


#ifdef REFKEEN_HAS_VER_CATACOMB_ALL

#ifdef REFKEEN_PLATFORM_WINDOWS
		TCHAR gog_catacombs_paths[1][BE_CROSS_PATH_LEN_BOUND];
		dwType = 0;
		dwSize = sizeof(gog_catacombs_paths[0]);
		status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\GOG.COM\\GOGCATACOMBSPACK"), _T("PATH"), &dwType, gog_catacombs_paths[0], &dwSize);
		int numOfGogPathsToCheck = ((status == ERROR_SUCCESS) && (dwType == REG_SZ)) ? 1 : 0;
		TCHAR *path_gog_catacombs_prefix_ends[1];
		if (numOfGogPathsToCheck)
		{
			path_gog_catacombs_prefix_ends[0] = path/*NOT gog_catacombs_paths[0]*/ + _tcslen(gog_catacombs_paths[0]);
		}
#endif
#ifdef REFKEEN_PLATFORM_MACOS
		int numOfGogPathsToCheck = 1;
		char gog_catacombs_paths[2][BE_CROSS_PATH_LEN_BOUND] = {
			"/Applications/Catacombs Pack/Catacomb Pack.app/Contents/Resources/Catacomb Pack.boxer/C 1 CATACOMB.harddisk",
			"" // Fill this very soon
		};
		char *path_gog_catacombs_prefix_ends[2] = {path/*NOT gog_catacombs_paths[0]*/ + strlen(gog_catacombs_paths[0]), NULL};
		if (homeVar && *homeVar)
		{
			BE_Cross_safeandfastcstringcopy_2strs(gog_catacombs_paths[1], gog_catacombs_paths[1]+sizeof(gog_catacombs_paths[1])/sizeof(TCHAR), homeVar, gog_catacombs_paths[0]);
			path_gog_catacombs_prefix_ends[1] = path/*NOT gog_catacombs_paths[1]*/ + strlen(gog_catacombs_paths[1]);
			++numOfGogPathsToCheck;
		}
#endif

#endif // REFKEEN_HAS_VER_CATACOMB_ALL

		/*** Now handling each version separately ***/

#ifdef REFKEEN_HAS_VER_KDREAMS
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_kdreams_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_kdreams_path);
		status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 356200"), _T("INSTALLLOCATION"), &dwType, steam_kdreams_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Steam");
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_kdreams_path, steam_kdreams_path+sizeof(steam_kdreams_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Keen Dreams/KDreams.app/Contents/Resources");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Steam");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_kdreams_path, steam_kdreams_path+sizeof(steam_kdreams_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Keen Dreams");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Steam");
			BE_Cross_safeandfastcstringcopy_2strs(steam_kdreams_path, steam_kdreams_path+sizeof(steam_kdreams_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Keen Dreams");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Steam");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_KDREAMS

#ifdef REFKEEN_HAS_VER_CAT3D
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[0]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_cat3d122, path, "GOG.com");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_CATABYSS
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[1]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catabyss124, path, "GOG.com");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_CATARM
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[2]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catarm102, path, "GOG.com");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_CATAPOC
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[3]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catapoc101, path, "GOG.com");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_WL6AC14
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_wolf3d_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_wolf3d_path);
		status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 2270"), _T("INSTALLLOCATION"), &dwType, steam_wolf3d_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
		{
			BEL_Cross_safeandfastctstringcopy(steam_wolf3d_path+_tcslen(steam_wolf3d_path), steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR)-_tcslen(steam_wolf3d_path), _T("\\base"));
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Steam");
		}
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_wolf3d_path, steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Wolfenstein 3D/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Steam");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_wolf3d_path, steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Wolfenstein 3D/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Steam");
			BE_Cross_safeandfastcstringcopy_2strs(steam_wolf3d_path, steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Wolfenstein 3D/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Steam");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_WL6AC14

#ifdef REFKEEN_HAS_VER_SODAC14
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_sod_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_sod_path);
		status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 9000"), _T("INSTALLLOCATION"), &dwType, steam_sod_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
		{
			BEL_Cross_safeandfastctstringcopy(steam_sod_path+_tcslen(steam_sod_path), steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR)-_tcslen(steam_sod_path), _T("\\base"));
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd2ac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd3ac14, steam_sod_path, "Steam");
		}
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_sod_path, steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Spear of Destiny/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd2ac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd3ac14, steam_sod_path, "Steam");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_sod_path, steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Spear of Destiny/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd2ac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd3ac14, steam_sod_path, "Steam");
			BE_Cross_safeandfastcstringcopy_2strs(steam_sod_path, steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Spear of Destiny/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd2ac14, steam_sod_path, "Steam");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sd3ac14, steam_sod_path, "Steam");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_SODAC14

#ifdef REFKEEN_HAS_VER_N3DWT10
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_n3d_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_n3d_path);
		status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 371180"), _T("INSTALLLOCATION"), &dwType, steam_n3d_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Steam");
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_n3d_path, steam_n3d_path+sizeof(steam_n3d_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Super 3-D Noah's Ark");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Steam");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_n3d_path, steam_n3d_path+sizeof(steam_n3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Super 3-D Noah's Ark");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Steam");
			BE_Cross_safeandfastcstringcopy_2strs(steam_n3d_path, steam_n3d_path+sizeof(steam_n3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Super 3-D Noah's Ark");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Steam");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_N3DWT10
	}
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
