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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>

#include "refkeen_config.h" // MUST precede other contents due to e.g., endianness-based ifdefs

#include "be_gamever.h"
#include "be_st.h" // For BE_ST_ExitWithErrorMsg; TODO: Also for g_refKeenCfg

#ifdef REFKEEN_PLATFORM_WINDOWS
#include <shlwapi.h> // SHGetValue
#endif

#include "backend/filesystem/be_filesystem_app_paths.h"
#include "backend/filesystem/be_filesystem_dir.h"
#include "backend/filesystem/be_filesystem_file_ops.h"
#include "backend/filesystem/be_filesystem_gameinst.h"
#include "backend/filesystem/be_filesystem_mkdir.h"
#include "backend/filesystem/be_filesystem_path_len_bound.h"
#include "backend/filesystem/be_filesystem_root_paths.h"
#include "backend/filesystem/be_filesystem_string_ops.h"
#include "backend/filesystem/be_filesystem_tchar.h"
#include "backend/gamedefs/be_gamedefs_structs.h"
#include "backend/startup/be_startup.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_sound_device_flags.h"

#include "crc32/crc32.h"

// Use this in case x is a macro defined to be a narrow string literal
#define CSTR_TO_TCSTR(x) _T(x)

#define REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS 1

BE_GameVer_T refkeen_current_gamever;

const char *BE_Cross_GetGameInstallationDescription(int num)
{
	return g_be_gameinstallations[num].descStr;
}

int BE_Cross_GetGameVerFromInstallation(int num)
{
	return g_be_gameinstallations[num].verId;
}

#include "backend/gamedefs/be_gamedefs.h"


// Loads a file originally embedded into the EXE (for DOS) to a newly allocated
// chunk of memory. Should be freed with BE_Cross_free_mem_loaded_embedded_rsrc.
// Returns chunk size if successful, or a negative number in case of failure.
int BE_Cross_load_embedded_rsrc_to_mem(const char *filename, void **ptr)
{
	BE_FILE_T fp = BE_Cross_open_readonly_for_reading(filename);
	if (!fp)
		return -1;

	int filesize = BE_Cross_FileLengthFromHandle(fp);
	*ptr = malloc(filesize);
	bool success = (*ptr) && (fread(*ptr, filesize, 1, fp) == 1);
	if (!success)
		free(*ptr); // This is OK with NULL

	fclose(fp);
	return success ? filesize : -1;
}

// Frees file loaded using BE_Cross_load_embedded_rsrc_to_mem. Accepts a NULL pointer.
void BE_Cross_free_mem_loaded_embedded_rsrc(void *ptr)
{
	free(ptr);
}

void *BE_Cross_BmallocFromEmbeddedData(const char *name, uint16_t *pSize)
{
	const BE_EmbeddedGameFileDetails_T *embeddedFile = g_be_current_exeFileDetails->embeddedFiles;
	if (embeddedFile)
		for (; embeddedFile->fileDetails.filename && BE_Cross_strcasecmp(name, embeddedFile->fileDetails.filename); ++embeddedFile)
			;

	if (!embeddedFile || !(embeddedFile->fileDetails.filename))
		BE_ST_ExitWithErrorMsg("BE_Cross_BmallocFromEmbeddedData: Unrecognized embedded data name!");

#ifdef REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS
	if (Crc32_ComputeBuf(0, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize) != embeddedFile->fileDetails.crc32)
		BE_ST_ExitWithErrorMsg("BE_Cross_BmallocFromEmbeddedData: Unexpectedly got the wrong CRC32!");
#endif

	void *ptr = BE_Cross_Bmalloc(embeddedFile->fileDetails.filesize);
	if (ptr)
	{
		memcpy(ptr, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize);
		if (pSize)
			*pSize = embeddedFile->fileDetails.filesize;
	}
	return ptr;
}

void *BE_Cross_BfarmallocFromEmbeddedData(const char *name, uint32_t *pSize)
{
	const BE_EmbeddedGameFileDetails_T *embeddedFile = g_be_current_exeFileDetails->embeddedFiles;
	if (embeddedFile)
		for (; embeddedFile->fileDetails.filename && BE_Cross_strcasecmp(name, embeddedFile->fileDetails.filename); ++embeddedFile)
			;

	if (!embeddedFile || !(embeddedFile->fileDetails.filename))
		BE_ST_ExitWithErrorMsg("BE_Cross_BfarmallocFromEmbeddedData: Unrecognized embedded data name!");

#ifdef REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS
	if (Crc32_ComputeBuf(0, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize) != embeddedFile->fileDetails.crc32)
		BE_ST_ExitWithErrorMsg("BE_Cross_BfarmallocFromEmbeddedData: Unexpectedly got the wrong CRC32!");
#endif

	void *ptr = BE_Cross_Bfarmalloc(embeddedFile->fileDetails.filesize);
	if (ptr)
	{
		memcpy(ptr, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize);
		if (pSize)
			*pSize = embeddedFile->fileDetails.filesize;
	}
	return ptr;
}

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

// SPECIAL - Attempts to open config/config.vdf file from Steam
// installation, used for importing controller mappings.
//
// Do *NOT* assume any specific format for the newline chars!
// (May be LF or CR-LF)
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
BE_FILE_T BE_Cross_open_steamcfg_for_reading(void)
{
	TCHAR steam_config_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS

	DWORD dwType = 0;
	DWORD dwSize = sizeof(steam_config_path);
	LSTATUS status = SHGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\VALVE\\STEAM", L"INSTALLPATH", &dwType, steam_config_path, &dwSize);
	if ((status != ERROR_SUCCESS) || (dwType != REG_SZ))
		return NULL;
	BEL_Cross_safeandfastctstringcopy(steam_config_path + _tcslen(steam_config_path), steam_config_path + sizeof(steam_config_path)/sizeof(TCHAR), L"\\config\\config.vdf");

#elif (defined REFKEEN_PLATFORM_UNIX)

	const char *homeVar = getenv("HOME");
	if (!homeVar || !(*homeVar))
		return NULL;

#ifdef REFKEEN_PLATFORM_MACOS
	BE_Cross_safeandfastcstringcopy_2strs(steam_config_path, steam_config_path+sizeof(steam_config_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/config/config.vdf");
#else
	BE_Cross_safeandfastcstringcopy_2strs(steam_config_path, steam_config_path+sizeof(steam_config_path)/sizeof(TCHAR), homeVar, "/.steam/steam/config/config.vdf");
#endif

#endif // UNIX
	return _tfopen(steam_config_path, _T("rb"));
}
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

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
		status = SHGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\GOG.COM\\GOGCATACOMBSPACK", L"PATH", &dwType, gog_catacombs_paths[0], &dwSize);
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
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreamse100, _T("."), "Keen Dreams EGA v1.00 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreamsc100, _T("."), "Keen Dreams CGA v1.00 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreamse113, _T("."), "Keen Dreams EGA v1.13 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreamsc105, _T("."), "Keen Dreams CGA v1.05 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreamse193, _T("."), "Keen Dreams EGA v1.93 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreamse120, _T("."), "Keen Dreams EGA v1.20 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, _T("."), "Keen Dreams 2015 (Local)");
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_kdreams_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_kdreams_path);
		status = SHGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 356200", L"INSTALLLOCATION", &dwType, steam_kdreams_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Keen Dreams 2015 (Steam)");
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_kdreams_path, steam_kdreams_path+sizeof(steam_kdreams_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Keen Dreams/KDreams.app/Contents/Resources");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Keen Dreams 2015 (Steam)");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_kdreams_path, steam_kdreams_path+sizeof(steam_kdreams_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Keen Dreams");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Keen Dreams 2015 (Steam)");
			BE_Cross_safeandfastcstringcopy_2strs(steam_kdreams_path, steam_kdreams_path+sizeof(steam_kdreams_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Keen Dreams");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_kdreams2015, steam_kdreams_path, "Keen Dreams 2015 (Steam)");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

#endif // REFKEEN_HAS_VER_KDREAMS

#ifdef REFKEEN_HAS_VER_CAT3D
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_cat3d100, _T("."), "Catacomb 3-D v1.00 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_cat3d122, _T("."), "Catacomb 3-D v1.22 (Local)");
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[0]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_cat3d122, path, "Catacomb 3-D v1.22 (GOG.com)");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_CATABYSS
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catabyss113, _T("."), "Catacomb Abyss v1.13 (Local)");
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catabyss124, _T("."), "Catacomb Abyss v1.24 (Local)");
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[1]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catabyss124, path, "Catacomb Abyss v1.24 (GOG.com)");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_CATARM
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catarm102, _T("."), "Catacomb Armageddon v1.02 (Local)");
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[2]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catarm102, path, "Catacomb Armageddon v1.02 (GOG.com)");
		}
#endif
#endif

#ifdef REFKEEN_HAS_VER_CATAPOC
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catapoc101, _T("."), "Catacomb Apocalypse v1.01 (Local)");
#ifdef BE_CHECK_GOG_INSTALLATIONS
		for (int i = 0; i < numOfGogPathsToCheck; ++i)
		{
			memcpy(path, gog_catacombs_paths[i], sizeof(path));
			BEL_Cross_safeandfastctstringcopy(path_gog_catacombs_prefix_ends[i], pathEnd, g_be_catacombs_gog_subdirnames_withdirsep[3]);
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_catapoc101, path, "Catacomb Apocalypse v1.01 (GOG.com)");
		}
#endif
#endif
#ifdef REFKEEN_HAS_VER_WL1AP10
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl1ap10, _T("."), "Wolfenstein 3D Shareware v1.0 (Local)");
#endif
#ifdef REFKEEN_HAS_VER_WL1AP11
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl1ap11, _T("."), "Wolfenstein 3D Shareware v1.1 (Local)");
#endif
#ifdef REFKEEN_HAS_VER_WL1AP12
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl1ap12, _T("."), "Wolfenstein 3D Shareware v1.2 (Local)");
#endif
#ifdef REFKEEN_HAS_VER_WL1AP14
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl1ap14, _T("."), "Wolfenstein 3D Shareware v1.2 (Local)");
#endif

#ifdef REFKEEN_HAS_VER_WL6AC14
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, _T("."), "Wolfenstein 3D Activision v1.4 (Local)");
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_wolf3d_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_wolf3d_path);
		status = SHGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 2270", L"INSTALLLOCATION", &dwType, steam_wolf3d_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
		{
			BEL_Cross_safeandfastctstringcopy(steam_wolf3d_path+_tcslen(steam_wolf3d_path), steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR)-_tcslen(steam_wolf3d_path), L"\\base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Wolfenstein 3D Activision v1.4 (Steam)");
		}
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_wolf3d_path, steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Wolfenstein 3D/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Wolfenstein 3D Activision v1.4 (Steam)");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_wolf3d_path, steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Wolfenstein 3D/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Wolfenstein 3D Activision v1.4 (Steam)");
			BE_Cross_safeandfastcstringcopy_2strs(steam_wolf3d_path, steam_wolf3d_path+sizeof(steam_wolf3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Wolfenstein 3D/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_wl6ac14, steam_wolf3d_path, "Wolfenstein 3D Activision v1.4 (Steam)");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_WL6AC14

#ifdef REFKEEN_HAS_VER_SDMFG10
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sdmfg10, _T("."), "Spear of Destiny Demo v1.0 (Local)");
#endif

#ifdef REFKEEN_HAS_VER_SODAC14
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, _T("."), "Spear of Destiny Activision v1.4 (Local)");
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_sod_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_sod_path);
		status = SHGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 9000", L"INSTALLLOCATION", &dwType, steam_sod_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
		{
			BEL_Cross_safeandfastctstringcopy(steam_sod_path+_tcslen(steam_sod_path), steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR)-_tcslen(steam_sod_path), L"\\base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Spear of Destiny Activision v1.4 (Steam)");
		}
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_sod_path, steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Spear of Destiny/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Spear of Destiny Activision v1.4 (Steam)");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_sod_path, steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Spear of Destiny/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Spear of Destiny Activision v1.4 (Steam)");
			BE_Cross_safeandfastcstringcopy_2strs(steam_sod_path, steam_sod_path+sizeof(steam_sod_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Spear of Destiny/base");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_sodac14, steam_sod_path, "Spear of Destiny Activision v1.4 (Steam)");
#endif
		}
#endif // REFKEEN_PLATFORM

#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
#endif // REFKEEN_HAS_VER_SODAC14

#ifdef REFKEEN_HAS_VER_N3DWT10
		BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, _T("."), "Super 3-D Noah's Ark (Local)");
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

		TCHAR steam_n3d_path[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
		dwType = 0;
		dwSize = sizeof(steam_n3d_path);
		status = SHGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\UNINSTALL\\STEAM APP 371180", L"INSTALLLOCATION", &dwType, steam_n3d_path, &dwSize);
		if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Super 3-D Noah's Ark (Steam)");
#elif (defined REFKEEN_PLATFORM_UNIX)
		if (homeVar && *homeVar)
		{
#ifdef REFKEEN_PLATFORM_MACOS
			BE_Cross_safeandfastcstringcopy_2strs(steam_n3d_path, steam_n3d_path+sizeof(steam_n3d_path)/sizeof(TCHAR), homeVar, "/Library/Application Support/Steam/SteamApps/common/Super 3-D Noah's Ark");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Super 3-D Noah's Ark");
#else
			// They changed from SteamApps to steamapps at some point, so check both two
			BE_Cross_safeandfastcstringcopy_2strs(steam_n3d_path, steam_n3d_path+sizeof(steam_n3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/SteamApps/common/Super 3-D Noah's Ark");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Super 3-D Noah's Ark (Steam)");
			BE_Cross_safeandfastcstringcopy_2strs(steam_n3d_path, steam_n3d_path+sizeof(steam_n3d_path)/sizeof(TCHAR), homeVar, "/.steam/steam/steamapps/common/Super 3-D Noah's Ark");
			BEL_Cross_ConditionallyAddGameInstallation(&g_be_gamever_n3dwt10, steam_n3d_path, "Super 3-D Noah's Ark (Steam)");
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
		BEL_Cross_ConditionallyAddGameInstallation(details, path, details->customInstDescription);
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


const char *BE_Cross_GetEXEFileDescriptionStrForGameVer(const char *exeFileName, int verId)
{
	const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[verId]->exeFiles;
	for (; exeFile->mainFuncPtr && (!exeFile->exeName || strcmp(exeFile->exeName, exeFileName)); ++exeFile)
		;
	return (exeFile->mainFuncPtr ? exeFile->subDescription : NULL); // subDescription may also be NULL
}

void (*BE_Cross_GetAccessibleMainFuncPtrForGameVer(const char *exeFileName, int verId))(void)
{
	const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[verId]->exeFiles;
	for (; exeFile->mainFuncPtr && (!exeFile->exeName || strcmp(exeFile->exeName, exeFileName)); ++exeFile)
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
