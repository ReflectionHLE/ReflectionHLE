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

#include "refkeen.h"
#include "be_launcher.h"
#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE
#include "be_title_and_version.h"
#endif

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

#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE
static void show_command_line_help()
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
#ifdef REFKEEN_HAS_VER_CATADVENTURES
	BE_ST_puts("-skipintro: Skip what is found in the original intro EXE and start game.");
#ifdef REFKEEN_HAS_VER_CATABYSS
	BE_ST_puts("-showslides: Show the electronic catalog / hint book.");
#else
	BE_ST_puts("-showslides: Show the hint book.");
#endif
#endif
	BE_ST_puts("-passorigargs <...>: Pass all following arguments to the game.");
	BE_ST_puts("-datadir <...>: Specify an alternative location for modifiable");
	BE_ST_puts("game files like saved games, separated by game version.");
	BE_ST_puts("Miscellaneous txt files are also covered.");
	BE_ST_puts("-cfgdir <...>: Specify an alternative path for Reflection Keen's");
	BE_ST_puts("new cfg files, along with gamecontrollerdb.txt.");
#ifdef REFKEEN_ENABLE_LAUNCHER
	BE_ST_puts("-fulllauncher: Show a fullscreen launcher window.");
	BE_ST_puts("-softlauncher: Show a software-rendered launcher window (not fullscreen).");
#endif
	BE_ST_puts("");
	BE_ST_puts("Note: The path passed to -datadir or -cfgdir is assumed to exist.");
	BE_ST_puts("");
	BE_ST_puts("Supported game versions:");
	for (int gameVerVal = 0; gameVerVal < BE_GAMEVER_LAST; ++gameVerVal)
	{
		if (gameVerVal < BE_GAMEVER_LAST-1)
		{
			BE_ST_printf("%s, ", refkeen_gamever_strs[gameVerVal]);
			if (gameVerVal % 6 == 5) // HACK for line splitting
				BE_ST_puts("");
		}
		else
			BE_ST_printf("%s.\n", refkeen_gamever_strs[gameVerVal]);
	}

	BE_ST_HandleExit(0);
}
#endif // REFKEEN_CONFIG_ENABLE_CMDLINE

int main(int argc, char **argv)
{
#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE

	// Parse arguments
	bool showHelp = false;
#ifdef REFKEEN_HAS_VER_CATADVENTURES
	bool skipIntro = false;
	bool showSlides = false;
#endif
	int selectedGameVerVal = BE_GAMEVER_LAST;

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

			for (selectedGameVerVal = 0; selectedGameVerVal < BE_GAMEVER_LAST; ++selectedGameVerVal)
				if (!BE_Cross_strcasecmp(argv[2], refkeen_gamever_strs[selectedGameVerVal]))
					break;

			if (selectedGameVerVal == BE_GAMEVER_LAST)
			{
				showHelp = true;
				break;
			}

			argv += 2;
			argc -= 2;
		}
#ifdef REFKEEN_HAS_VER_CATADVENTURES
		else if (!BE_Cross_strcasecmp(1+argv[1], "skipintro"))
		{
			skipIntro = true;
			++argv;
			--argc;
		}
		else if (!BE_Cross_strcasecmp(1+argv[1], "showslides"))
		{
			showSlides = true;
			++argv;
			--argc;
		}
#endif
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
#ifdef REFKEEN_HAS_VER_CATADVENTURES
		extern void (*refkeen_game_exe_main_funcs[BE_GAMEVER_LAST])(void);
		extern void (*refkeen_slidecat_exe_main_funcs[BE_GAMEVER_LAST])(void);

		BE_Cross_StartGame(argc, argv, showSlides ? refkeen_slidecat_exe_main_funcs[refkeen_current_gamever] :
		                               (skipIntro ? refkeen_game_exe_main_funcs[refkeen_current_gamever] : NULL));
#else
		BE_Cross_StartGame(argc, argv, NULL);
#endif
	}

#else // REFKEEN_CONFIG_ENABLE_CMDLINE

	BE_Cross_PrepareGameInstallations();
	BE_Launcher_Start();

#endif // REFKEEN_CONFIG_ENABLE_CMDLINE

	BE_ST_ShutdownAll();
	return 0;
}
