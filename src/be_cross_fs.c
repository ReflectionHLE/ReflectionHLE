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
#include <setjmp.h>
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
#include "be_cross.h"
#include "be_features.h"
#include "be_sound_device_flags.h"

#include "crc32/crc32.h"
#include "unlzexe/unlzexe.h"
#ifdef ENABLE_PKLITE
#include "depklite/depklite.h"
#endif

// Use this in case x is a macro defined to be a narrow string literal
#define CSTR_TO_TCSTR(x) _T(x)

#define REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS 1

BE_GameVer_T refkeen_current_gamever;

static const BE_EXEFileDetails_T *g_be_current_exeFileDetails;

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

static unsigned char *g_be_current_exeImage;

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


// gameVer should be BE_GAMEVER_LAST if no specific version is desired
// FIXME: See BE_Cross_InitGame for explanation about setVerOnly
static void BEL_Cross_SelectGameInstallation(int gameVerVal, bool setVerOnly)
{
	if (gameVerVal == BE_GAMEVER_LAST)
	{
		if (!g_be_gameinstallations_num)
		{
			if (setVerOnly)
				return;
			BE_ST_ExitWithErrorMsg("BEL_Cross_SelectGameInstallation: No compatible game installation found!");
		}
		g_be_selectedGameInstallation = &g_be_gameinstallations[0];
	}
	else
	{
		g_be_selectedGameInstallation = g_be_gameinstallationsbyver[gameVerVal];
		if (!g_be_selectedGameInstallation)
		{
			char errorBuffer[80];
			if (setVerOnly)
				return;
			BE_Cross_safeandfastcstringcopy_2strs(errorBuffer, errorBuffer+sizeof(errorBuffer), "BEL_Cross_SelectGameInstallation: Can't find game installation: ",  refkeen_gamever_strs[gameVerVal]);
			BE_ST_ExitWithErrorMsg(errorBuffer);
		}
	}

	if (setVerOnly)
	{
		refkeen_current_gamever = g_be_selectedGameInstallation->verId;
		return;
	}
	g_refKeenCfg.lastSelectedGameVer = refkeen_current_gamever = g_be_selectedGameInstallation->verId;

	for (void (**patcherFuncPtr)(void) = g_be_gamever_ptrs[refkeen_current_gamever]->patcherFuncPtrs;
             *patcherFuncPtr; ++patcherFuncPtr)
		(*patcherFuncPtr)();
}

// Here the magic happens - used to clear a portion of the stack before
// changing to another "main" function (in case we get a loop).
// In a way, this should be similar to C++ exception handling,
// just C-compatible.
/*static*/ jmp_buf g_be_mainFuncJumpBuffer;

static void BEL_Cross_LoadEXEImageToMem(void)
{
	char errorMsg[256];
	FILE *exeFp = BEL_Cross_apply_file_action_in_dir(g_be_current_exeFileDetails->exeName, BE_FILE_REQUEST_READ, g_be_selectedGameInstallation->instPath, NULL);
	if (!exeFp)
	{
		snprintf(errorMsg, sizeof(errorMsg), "BEL_Cross_LoadEXEImageToMem: Can't open EXE after checking it!\nFilename: %s", g_be_current_exeFileDetails->exeName);
		BE_ST_ExitWithErrorMsg(errorMsg);
	}

	// FIXME - Use BE_Cross_Bmalloc/farmalloc and shuffle things around
	g_be_current_exeImage = (unsigned char *)malloc(g_be_current_exeFileDetails->decompExeImageSize);
	if (!g_be_current_exeImage)
	{
		fclose(exeFp);
		snprintf(errorMsg, sizeof(errorMsg), "BEL_Cross_LoadEXEImageToMem: Can't allocate memory for unpacked EXE copy!\nFilename: %s", g_be_current_exeFileDetails->exeName);
		BE_ST_ExitWithErrorMsg(errorMsg);
	}

	bool success;
	switch (g_be_current_exeFileDetails->compressionType)
	{
	case BE_EXECOMPRESSION_NONE:
	{
		uint16_t offsetInPages;
		fseek(exeFp, 8, SEEK_SET);
		BE_Cross_readInt16LE(exeFp, &offsetInPages);
		fseek(exeFp, 16*offsetInPages, SEEK_SET);
		success = (fread(g_be_current_exeImage, g_be_current_exeFileDetails->decompExeImageSize, 1, exeFp) == 1);
		break;
	}
	case BE_EXECOMPRESSION_LZEXE9X:
		success = Unlzexe_unpack(exeFp, g_be_current_exeImage, g_be_current_exeFileDetails->decompExeImageSize);
		break;
#ifdef ENABLE_PKLITE
	case BE_EXECOMPRESSION_PKLITE105:
		success = depklite_unpack(exeFp, g_be_current_exeImage, g_be_current_exeFileDetails->decompExeImageSize);
		break;
#endif
	}

	fclose(exeFp);
	if (!success)
	{
		free(g_be_current_exeImage);
		snprintf(errorMsg, sizeof(errorMsg), "decompExeImage: Failed to copy EXE in unpacked form!\nFilename: %s", g_be_current_exeFileDetails->exeName);
		BE_ST_ExitWithErrorMsg(errorMsg);
	}
}

static void BEL_Cross_FreeEXEImageFromMem(void)
{
	free(g_be_current_exeImage);
}

static void BEL_Cross_DoCallMainFunc(void)
{
	setjmp(g_be_mainFuncJumpBuffer); // Ignore returned value, always doing the same thing

	// Do start!
	BE_ST_AltControlScheme_Reset();

	if (g_be_current_exeFileDetails->embeddedFiles)
	{
		BEL_Cross_LoadEXEImageToMem();
		g_be_current_exeFileDetails->embeddedFilesLoaderFuncPtr();
		BEL_Cross_FreeEXEImageFromMem();
	}

	be_lastSetMainFuncPtr = g_be_current_exeFileDetails->mainFuncPtr;
	if (g_be_current_exeFileDetails->passArgsToMainFunc)
		((void (*)(int, const char **))be_lastSetMainFuncPtr)(g_be_argc, g_be_argv); // HACK
	else
		be_lastSetMainFuncPtr();
}

// When a new main function is called in the middle (BE_Cross_Bexecv),
// by default the current (non-new) main function is stored as
// a "one time" function that shall *never* be called again.
//
// To prevent this, BE_Cross_Bexecv should get a finalizer function pointer,
// used to reset the sub-program as represented by current main function
// to its original state. (Emphasis on global and static variables.)
#define MAX_NUM_OF_ONE_TIME_MAIN_FUNCS 4

static int g_be_numOfOneTimeMainFuncs = 0;
static void (*g_be_oneTimeMainFuncs[MAX_NUM_OF_ONE_TIME_MAIN_FUNCS])(void);

void BE_Cross_Bexecv(void (*mainFunc)(void), const char **argv, void (*finalizer)(void), bool passArgsToMainFunc)
{
	for (int i = 0; i < g_be_numOfOneTimeMainFuncs; ++i)
		if (g_be_oneTimeMainFuncs[i] == mainFunc)
			BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - One-time function unexpectedly called again!");

	if (finalizer)
		finalizer();
	else
	{
		if (g_be_numOfOneTimeMainFuncs == MAX_NUM_OF_ONE_TIME_MAIN_FUNCS)
			BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - Too many one-time functions called!");
		g_be_oneTimeMainFuncs[g_be_numOfOneTimeMainFuncs++] = be_lastSetMainFuncPtr;
	}

	// Note this does NOT work for memory not managed by us (e.g., simple calls to malloc)
	void BEL_Cross_ClearMemory(void);
	BEL_Cross_ClearMemory();

	g_be_argv = argv;
	for (g_be_argc = 0; *argv; ++g_be_argc, ++argv)
		;

	// Locate the right EXE
	for (g_be_current_exeFileDetails = g_be_gamever_ptrs[refkeen_current_gamever]->exeFiles; g_be_current_exeFileDetails->mainFuncPtr && (g_be_current_exeFileDetails->mainFuncPtr != mainFunc); ++g_be_current_exeFileDetails)
		;
	if (!g_be_current_exeFileDetails->mainFuncPtr)
		BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - Unrecognized main function!");

	longjmp(g_be_mainFuncJumpBuffer, 0); // A little bit of magic
}

void BE_Cross_InitGame(int gameVerVal)
{
	// FIXME: We have a problem:
	// 1. BE_ST_PrepareForGameStartupWithoutAudio should theoretically be
	// called before BEL_Cross_SelectGameInstallation, so an error message
	// can be displayed if no matching game version is found.
	// 2. On the other hand, as of writing this, we first need to set
	// refkeen_current_gamever via BEL_Cross_SelectGameInstallation.
	// Reason is that before the call to BE_ST_PrepareForGameStartupWithoutAudio
	// is completed, this version may currently determine if VSync is enabled
	// when AUTO is chosen (it's disabled for Keen Dreams with CGA graphics).
	BEL_Cross_SelectGameInstallation(gameVerVal, true);
	BE_ST_PrepareForGameStartupWithoutAudio(); // Some additional preparation required
	BEL_Cross_SelectGameInstallation(gameVerVal, false);
	BE_ST_InitAudio(); // Do this now, since we can tell if we want digi audio out or not
}

void BE_Cross_StartGame(int argc, char **argv, void (*mainFuncPtr)(void))
{
	// Prepare arguments for ported game code
	g_be_argc = argc;
	// HACK: In Keen Dreams CGA v1.05, even if argc == 1, argv[1] is accessed...
	// Furthermore, in Keen Dreams Shareware v1.13, argc, argv[1], argv[2] and argv[3] are all modified...
	// And then in Catacomb Abyss, argv[3] is compared to "1". In its INTROSCN.EXE argv[4] is compared...

	// REFKEEN - As long as argv[0] isn't used, use a placeholder
	const char *our_workaround_argv[] = { "PROG.EXE", "", "", "", "", "", "", "", "", NULL };
	if (argc < 10)
	{
		for (int currarg = 1; currarg < argc; ++currarg)
		{
			our_workaround_argv[currarg] = argv[currarg];
		}
		g_be_argv = our_workaround_argv;
	}
	else
	{
		// REFKEEN - Hack, but we don't access argv directly anyway...
		g_be_argv = (const char **)argv;
	}

	// Locate the right EXE
	if (!mainFuncPtr)
		mainFuncPtr = g_be_gamever_ptrs[refkeen_current_gamever]->exeFiles->mainFuncPtr;

	for (g_be_current_exeFileDetails = g_be_gamever_ptrs[refkeen_current_gamever]->exeFiles; g_be_current_exeFileDetails->mainFuncPtr && (g_be_current_exeFileDetails->mainFuncPtr != mainFuncPtr); ++g_be_current_exeFileDetails)
		;
	if (!g_be_current_exeFileDetails->mainFuncPtr)
		BE_ST_ExitWithErrorMsg("BE_Cross_StartGame - Unrecognized main function!");

	snprintf(g_refKeenCfg.lastSelectedGameExe, sizeof(g_refKeenCfg.lastSelectedGameExe), "%s", g_be_current_exeFileDetails->exeName ? g_be_current_exeFileDetails->exeName : "");

	BEL_Cross_DoCallMainFunc(); // Do a bit more preparation and then begin
}
