/* Copyright (C) 2014-2025 NY00123
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

#include "be_cross.h"
#include "be_filesystem_app_paths.h"
#include "be_filesystem_string_ops.h"
#include "be_filesystem_root_paths.h"

#ifdef REFKEEN_PLATFORM_ANDROID
#include "be_filesystem_mkdir.h"
#include <jni.h>
// HACK - Adding a dependency on SDL2 for Android! (Used for external storage path, and for calling Java function)
#include "SDL_system.h"
#endif


TCHAR g_be_appDataPath[BE_CROSS_PATH_LEN_BOUND];
TCHAR g_be_appNewCfgPath[BE_CROSS_PATH_LEN_BOUND];

#ifdef REFKEEN_PLATFORM_WINDOWS
static const TCHAR *g_be_rootDrivePaths[] = {_T("a:\\"),_T("b:\\"),_T("c:\\"),_T("d:\\"),_T("e:\\"),_T("f:\\"),_T("g:\\"),_T("h:\\"),_T("i:\\"),_T("j:\\"),_T("k:\\"),_T("l:\\"),_T("m:\\"),_T("n:\\"),_T("o:\\"),_T("p:\\"),_T("q:\\"),_T("r:\\"),_T("s:\\"),_T("t:\\"),_T("u:\\"),_T("v:\\"),_T("w:\\"),_T("x:\\"),_T("y:\\"),_T("z:\\")};
static const char *g_be_rootDrivePathsNames[] = {"a:\\","b:\\","c:\\","d:\\","e:\\","f:\\","g:\\","h:\\","i:\\","j:\\","k:\\","l:\\","m:\\","n:\\","o:\\","p:\\","q:\\","r:\\","s:\\","t:\\","u:\\","v:\\","w:\\","x:\\","y:\\","z:\\"};
#endif

extern const char *be_main_arg_datadir;
extern const char *be_main_arg_newcfgdir;

void BE_Cross_PrepareAppPaths(void)
{
#ifdef REFKEEN_PLATFORM_WINDOWS
	const TCHAR *homeVar = _tgetenv(_T("HOMEPATH"));
	const TCHAR *envVar = _tgetenv(_T("APPDATA"));

	// HACK - Ignore be_main_arg_datadir for now
	if (envVar && *envVar)
	{
		BEL_Cross_safeandfastctstringcopy_2strs(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), envVar, _T("\\reflectionhle"));
	}
	else
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "APPDATA environment variable is not properly defined.\n");
		if (homeVar && *homeVar)
		{
			BEL_Cross_safeandfastctstringcopy_2strs(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), homeVar, _T("\\AppData\\Roaming\\reflectionhle"));
		}
		else
		{
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "HOMEPATH environment variable is not properly defined.\n");
			BEL_Cross_safeandfastctstringcopy(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), _T("."));
		}
	}

	if (be_main_arg_newcfgdir)
	{
		BEL_Cross_safeandfastcstringcopytoctstring(g_be_appNewCfgPath, g_be_appNewCfgPath+BE_Cross_ArrayLen(g_be_appNewCfgPath), be_main_arg_newcfgdir);
	}
	else // This is why be_main_arg_datadir has been ignored (using g_be_appDataPath as a temporary buffer)
	{
		memcpy(g_be_appNewCfgPath, g_be_appDataPath, sizeof(g_be_appDataPath));
	}

	if (be_main_arg_datadir) // Now checking be_main_arg_datadir
	{
		BEL_Cross_safeandfastcstringcopytoctstring(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), be_main_arg_datadir);
	}

	/*** Root paths ***/

	// List of drives
	DWORD drivesBitMasks = GetLogicalDrives();
	for (int driveNum = 0; driveNum < 26; ++driveNum)
		if (drivesBitMasks & (1 << driveNum))
			BEL_Cross_AddRootPath(g_be_rootDrivePaths[driveNum], g_be_rootDrivePathsNames[driveNum], g_be_rootDrivePathsNames[driveNum]);

	// Home dir
	if (homeVar && *homeVar)
		BEL_Cross_AddRootPathIfDir(homeVar, "home", "Home dir");

	TCHAR path[BE_CROSS_PATH_LEN_BOUND];
	DWORD dwType;
	DWORD dwSize;
	LSTATUS status;

	// Steam installation dir
	dwType = 0;
	dwSize = sizeof(path);
#ifdef _WIN64
	status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\WOW6432NODE\\VALVE\\STEAM"), _T("INSTALLPATH"), &dwType, path, &dwSize);
#else
	status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\VALVE\\STEAM"), _T("INSTALLPATH"), &dwType, path, &dwSize);
#endif
	if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
		BEL_Cross_AddRootPathIfDir(path, "steam", "Steam (installation)");
	
	// GOG.com installation dir
	dwType = 0;
	dwSize = sizeof(path);
#ifdef _WIN64
	status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\WOW6432NODE\\GOG.COM"), _T("DEFAULTPACKPATH"), &dwType, path, &dwSize);
#else
	status = SHGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\GOG.COM"), _T("DEFAULTPACKPATH"), &dwType, path, &dwSize);
#endif
	if ((status == ERROR_SUCCESS) && (dwType == REG_SZ))
		BEL_Cross_AddRootPathIfDir(path, "gog", "GOG Games (default)");
#endif

#ifdef REFKEEN_PLATFORM_ANDROID
	// HACK - Adding a dependency on SDL2 for Android!

	// FIXME - These environment variables don't seem to be shown in any
	// official documentation for Android, but at least EXTERNAL_STORAGE
	// appears to do the job, and they're simple to use from C/C++.

	// We still need to check/ask for permission to access the storage, on Android 6.0 and later

	// Prepare to call Javaj function
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz = env->GetObjectClass(activity);
	jmethodID method_id = env->GetMethodID(clazz, "requestReadExternalStoragePermission", "()I");
	// Do call it
	bool haveReadPermission = (bool)(env->CallIntMethod(activity, method_id));
	// Clean up references
	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	if (haveReadPermission)
	{
		const char *primaryStorage = getenv("EXTERNAL_STORAGE");
		if (primaryStorage && *primaryStorage)
			BEL_Cross_AddRootPathIfDir(primaryStorage, "externalstorage", "Primary storage");
		else
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "EXTERNAL_STORAGE environment variable is not properly defined.\n");
		// Let's ignore SECONDARY_STORAGE for now, since we may get a colon-delimited list of paths
		const char *externalSDCardStorage = getenv("EXTERNAL_SDCARD_STORAGE");
		if (externalSDCardStorage && *externalSDCardStorage)
			BEL_Cross_AddRootPathIfDir(externalSDCardStorage, "externalsdcardstorage", "External SD Card storage");
	}

	char path[BE_CROSS_PATH_LEN_BOUND];

	const char *externalStoragePath = SDL_AndroidGetExternalStoragePath();
	if (externalStoragePath && *externalStoragePath)
	{
		BEL_Cross_safeandfastctstringcopy_2strs(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), externalStoragePath, "/appdata");
		memcpy(g_be_appNewCfgPath, g_be_appDataPath, sizeof(g_be_appDataPath));
		// Let's add this just in case (sdcard directory cannot be naively opened on Android 7.0)
		BE_Cross_safeandfastcstringcopy_2strs(path, path+BE_Cross_ArrayLen(path), externalStoragePath, "/user_gameinsts");
		// In contrary to other root paths, we should create this one on first launch
		BEL_Cross_mkdir(externalStoragePath); // Non-recursive
		BEL_Cross_mkdir(path);
		BEL_Cross_AddRootPathIfDir(path, "appgameinsts", "App-local game installations dir");
		// HACK - We don't look at arguments set by the user (for modifying e.g., g_be_appDataPath), but then these are never sent on Android...
	}
#elif (defined REFKEEN_PLATFORM_UNIX)
	const char *homeVar = getenv("HOME");

#ifndef REFKEEN_PLATFORM_MACOS
	const char *envVar;
#endif

	if (!homeVar || !(*homeVar))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "HOME environment variable is not properly defined.\n");
	}

	if (be_main_arg_datadir)
	{
		BE_Cross_safeandfastcstringcopy(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), be_main_arg_datadir);
	}
	else
	{
#ifdef REFKEEN_PLATFORM_MACOS
		// FIXME - Handle sandboxing?
		BE_Cross_safeandfastcstringcopy_2strs(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), homeVar, "/Library/Application Support/reflectionhle");
#else
		envVar = getenv("XDG_DATA_HOME");
		if (envVar && *envVar)
		{
			BE_Cross_safeandfastcstringcopy_2strs(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), envVar, "/reflectionhle");
		}
		else if (homeVar && *homeVar)
		{
			BE_Cross_safeandfastcstringcopy_2strs(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), homeVar, "/.local/share/reflectionhle");
		}
		else
		{
			BE_Cross_safeandfastcstringcopy(g_be_appDataPath, g_be_appDataPath+BE_Cross_ArrayLen(g_be_appDataPath), ".");
		}
#endif
	}

	if (be_main_arg_newcfgdir)
	{
		BE_Cross_safeandfastcstringcopy(g_be_appNewCfgPath, g_be_appNewCfgPath+BE_Cross_ArrayLen(g_be_appNewCfgPath), be_main_arg_newcfgdir);
	}
	else
	{
#ifdef REFKEEN_PLATFORM_MACOS
		// FIXME - Handle sandboxing?
		BE_Cross_safeandfastcstringcopy_2strs(g_be_appNewCfgPath, g_be_appNewCfgPath+BE_Cross_ArrayLen(g_be_appNewCfgPath), homeVar, "/Library/Application Support/reflectionhle");
#else
		envVar = getenv("XDG_CONFIG_HOME");
		if (envVar && *envVar)
		{
			BE_Cross_safeandfastcstringcopy_2strs(g_be_appNewCfgPath, g_be_appNewCfgPath+BE_Cross_ArrayLen(g_be_appNewCfgPath), envVar, "/reflectionhle");
		}
		else if (homeVar && *homeVar)
		{
			BE_Cross_safeandfastcstringcopy_2strs(g_be_appNewCfgPath, g_be_appNewCfgPath+BE_Cross_ArrayLen(g_be_appNewCfgPath), homeVar, "/.config/reflectionhle");
		}
		else
		{
			BE_Cross_safeandfastcstringcopy(g_be_appNewCfgPath, g_be_appNewCfgPath+BE_Cross_ArrayLen(g_be_appNewCfgPath), ".");
		}
#endif
	}

	/*** Root paths ***/

	char path[BE_CROSS_PATH_LEN_BOUND];

	if (homeVar && *homeVar) // Should be set, otherwise there's a big problem (but warning is printed)
	{
		// Home dir
		BEL_Cross_AddRootPathIfDir(homeVar, "home", "Home dir");
#ifdef REFKEEN_PLATFORM_MACOS
		// Steam installation dir
		BE_Cross_safeandfastcstringcopy_2strs(path, path+BE_Cross_ArrayLen(path), homeVar, "/Library/Application Support/Steam");
		BEL_Cross_AddRootPathIfDir(path, "steam", "Steam (installation)");
#else
		// Steam installation dir
		BE_Cross_safeandfastcstringcopy_2strs(path, path+BE_Cross_ArrayLen(path), homeVar, "/.steam/steam");
		BEL_Cross_AddRootPathIfDir(path, "steam", "Steam (installation)");
		// GOG.com installation dir
		BE_Cross_safeandfastcstringcopy_2strs(path, path+BE_Cross_ArrayLen(path), homeVar, "/GOG Games");
		BEL_Cross_AddRootPathIfDir(path, "gog", "GOG Games (default)");
#endif
	}
	// Finally the root itself (better keep it at the bottom of the list)
	BEL_Cross_AddRootPathIfDir("/", "/", "/");
#endif
}
