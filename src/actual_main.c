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

#include "refkeen.h"
#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE
#include "be_title_and_version.h"
#include "backend/video/be_video_emu.h" // TXT_COLS_NUM
#include "backend/gamedefs/be_gamedefs_structs.h"
#endif

#include <string.h>
#include "SDL_main.h"

#if (!defined REFKEEN_ENABLE_LAUNCHER) && (!defined REFKEEN_CONFIG_ENABLE_CMDLINE)
#error "At least one of REFKEEN_ENABLE_LAUNCHER and REFKEEN_CONFIG_ENABLE_CMDLINE must be defined!"
#endif

int g_be_argc;
const char **g_be_argv;

void (*be_lastSetMainFuncPtr)(void);

const char *be_main_arg_datadir = NULL;
const char *be_main_arg_newcfgdir = NULL;

BE_GameVer_T refkeen_current_gamever;

void BE_Launcher_Start(void);

#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE
static void show_command_line_help(void)
{
	// HACK - For text mode emulation (and exit handler)
	BE_ST_PrepareForGameStartupWithoutAudio();

	BE_ST_puts("*** " REFKEEN_TITLE_AND_VER_STRING " - Command line arguments ***");
	BE_ST_puts("");
#ifdef REFKEEN_ENABLE_LAUNCHER
	BE_ST_puts("To skip the launcher you can use -gamever <VER>,");
	BE_ST_puts("or just -passorigargs with or without additional arguments.");
	BE_ST_puts("");
#endif
	BE_ST_puts("List of possible command line arguments:");
	BE_ST_puts("-gamever <VER>: Select game version supported by this executable.");
	BE_ST_puts("-listgamevers: Show a list of all game versions supported by this executable.");
	BE_ST_puts("-passorigargs <...>: Pass all following arguments to the game.");
	BE_ST_puts("-datadir <...>: Specify an alternative location for modifiable");
	BE_ST_puts("game files like saved games, separated by game version.");
	BE_ST_puts("Miscellaneous txt files are also covered.");
	BE_ST_puts("-cfgdir <...>: Specify an alternative path for ReflectionHLE's");
	BE_ST_puts("new cfg files, along with gamecontrollerdb.txt.");
#ifdef REFKEEN_ENABLE_LAUNCHER
	BE_ST_puts("-fulllauncher: Show a fullscreen launcher window.");
	BE_ST_puts("-softlauncher: Show a software-rendered launcher window (not fullscreen).");
#endif
	BE_ST_puts("");
	BE_ST_puts("Note: The path passed to -datadir or -cfgdir is assumed to exist.");

	BE_ST_HandleExit(0);
}

static void game_vers_cached_print(
	char (*buffer)[TXT_COLS_NUM], char **ptr,
	const char *verStr, const char *subVerStr, const char *endMark)
{
	int attempt = 0, len;
	while (1)
	{
		len = snprintf(*ptr, sizeof(*buffer) - (*ptr - *buffer),
		               "%s%s%s%s", verStr,
		               subVerStr ? ":" : "",
		               subVerStr ? subVerStr : "", endMark);
		if (len + (*ptr - *buffer) >= sizeof(*buffer))
		{
			if (++attempt == 2)
				BE_ST_ExitWithErrorMsg("game_vers_cached_print: Unexpectedly long string!");
			**ptr = '\0';
			BE_ST_puts(*buffer);
			*ptr = *buffer;
			continue;
		}
		break;
	}
	*ptr += len;
}

static void show_game_vers(void)
{
	char buffer[TXT_COLS_NUM] = "", *ptr = buffer;
	// HACK - For text mode emulation (and exit handler)
	BE_ST_PrepareForGameStartupWithoutAudio();

	BE_ST_puts("*** " REFKEEN_TITLE_AND_VER_STRING " - Supported game versions ***");
	BE_ST_puts("");
	for (int gameVerVal = 0; gameVerVal < BE_GAMEVER_LAST; ++gameVerVal)
	{
		bool wasPrinted = false;
		const char *lastSubGameVerStr = NULL;

		// Printing is technically postponed by one line, due to the
		// need to decide if to end a string with a comma or a period
		const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[gameVerVal]->exeFiles;
		for (; exeFile->mainFuncPtr; ++exeFile)
			if (exeFile->subGameVerStr)
			{
				game_vers_cached_print(
					&buffer, &ptr,
					refkeen_gamever_strs[gameVerVal],
					lastSubGameVerStr, ", ");

				lastSubGameVerStr = exeFile->subGameVerStr;
			}

		game_vers_cached_print(
			&buffer, &ptr,
			refkeen_gamever_strs[gameVerVal],
			lastSubGameVerStr,
			(gameVerVal < BE_GAMEVER_LAST - 1) ? ", " : ".");
	}
	BE_ST_puts(buffer);
	BE_ST_HandleExit(0);
}

static bool parse_game_ver(char *arg, int *gameVer, void (**mainFuncPtr)(void))
{
	char *sep = strchr(arg, ':');
	bool ret = false;
	if (sep)
		*sep = '\0';

	int selectedGameVer;
	for (selectedGameVer = 0; selectedGameVer < BE_GAMEVER_LAST; ++selectedGameVer)
		if (!BE_Cross_strcasecmp(arg, refkeen_gamever_strs[selectedGameVer]))
			break;

	if (selectedGameVer < BE_GAMEVER_LAST)
	{
		if (sep)
		{
			const BE_EXEFileDetails_T *exeFile = g_be_gamever_ptrs[selectedGameVer]->exeFiles;
			for (; exeFile->mainFuncPtr; ++exeFile)
				if (exeFile->subGameVerStr &&
				    !BE_Cross_strcasecmp(sep + 1, exeFile->subGameVerStr))
				{
					*gameVer = selectedGameVer;
					*mainFuncPtr = exeFile->mainFuncPtr;
					ret = true;
					break;
				}
		}
		else
		{
			*gameVer = selectedGameVer;
			*mainFuncPtr = 0;
			ret = true;
		}
	}

	if (sep)
		*sep = ':';
	return ret;
}
#endif // REFKEEN_CONFIG_ENABLE_CMDLINE

int main(int argc, char **argv)
{
#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE

	// Parse arguments
	bool showHelp = false, showGameVers = false;
	int selectedGameVerVal = BE_GAMEVER_LAST;
	void (*mainFuncPtr)(void);

#ifdef REFKEEN_PLATFORM_MACOS
	// A weird OS X hack, ignoring an argument possibly passed
	// if the app is launched from Finder (or with "open" command)
	if ((argc >= 2) && !strncmp(argv[1], "-psn_", 5))
	{
		++argv;
		--argc;
	}
#endif

#ifdef REFKEEN_ENABLE_LAUNCHER
	bool startLauncher = (argc == 1);
#endif
	while ((argc >= 2) && !showHelp)
	{
		if (!(*argv[1]))
			showHelp = true;
		else if (!BE_Cross_strcasecmp(1+argv[1], "gamever"))
		{
			if ((argc == 2) || !(*argv[2]))
			{
				showHelp = true;
				break;
			}

			if (!parse_game_ver(argv[2], &selectedGameVerVal, &mainFuncPtr))
			{
				showHelp = true;
				break;
			}

			argv += 2;
			argc -= 2;
		}
		else if (!BE_Cross_strcasecmp(1+argv[1], "listgamevers"))
		{
			showGameVers = true;
			break;
		}
		else if (!BE_Cross_strcasecmp(1+argv[1], "passorigargs"))
		{
			// The remaining args will be passed to the
			// original game code as-is
			++argv;
			--argc;
			break;
		}
		else if (!BE_Cross_strcasecmp(1+argv[1], "datadir") || !BE_Cross_strcasecmp(1+argv[1], "cfgdir"))
		{
			const char **be_main_arg_ptr = !BE_Cross_strcasecmp(1+argv[1], "datadir") ? &be_main_arg_datadir : &be_main_arg_newcfgdir;
			if ((argc == 2) || !(*argv[2]))
			{
				showHelp = true;
				break;
			}
			*be_main_arg_ptr = argv[2];

			argv += 2;
			argc -= 2;
		}
#ifdef REFKEEN_ENABLE_LAUNCHER
		else if (!BE_Cross_strcasecmp(1+argv[1], "fulllauncher"))
		{
			g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_FULL;
			startLauncher = true;
			++argv;
			--argc;
		}
		else if (!BE_Cross_strcasecmp(1+argv[1], "softlauncher"))
		{
			g_refKeenCfg.launcherWinType = LAUNCHER_WINDOW_SOFTWARE;
			startLauncher = true;
			++argv;
			--argc;
		}
#endif
		else
			showHelp = true;
	}

	// Call this here, because be_main_arg_datadir/be_main_arg_newcfgdir
	// may be referenced from an internally called function
	BE_ST_InitCommon();

	if (showHelp)
		show_command_line_help();
	else if (showGameVers)
		show_game_vers();
	else
	{
		BE_Cross_PrepareGameInstallations();
#ifdef REFKEEN_ENABLE_LAUNCHER
		if (startLauncher)
		{
			BE_Launcher_Start();
			BE_ST_ShutdownAll();
			return 0;
		}
#endif
		BE_Cross_InitGame(selectedGameVerVal);
		BE_Cross_StartGame(argc, argv, mainFuncPtr);
	}

#else // REFKEEN_CONFIG_ENABLE_CMDLINE

	BE_ST_InitCommon(); // We still need this, at least for reading cfg
	BE_Cross_PrepareGameInstallations();
	BE_Launcher_Start();

#endif // REFKEEN_CONFIG_ENABLE_CMDLINE

	BE_ST_ShutdownAll();
	return 0;
}
