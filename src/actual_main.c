/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "refkeen.h"
#include "SDL_main.h"

#if (!defined REFKEEN_ENABLE_LAUNCHER) && (!defined REFKEEN_CONFIG_ENABLE_CMDLINE)
#error "At least one of REFKEEN_ENABLE_LAUNCHER and REFKEEN_CONFIG_ENABLE_CMDLINE must be defined!"
#endif

int g_be_argc;
const char **g_be_argv;

void (*be_lastSetMainFuncPtr)(void);

const char *be_main_arg_datadir = NULL;
const char *be_main_arg_newcfgdir = NULL;

#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE
static void show_command_line_help()
{
	// HACK - For text mode emulation (and exit handler)
	BE_ST_PrepareForGameStartupWithoutAudio();

	BE_ST_puts("*** " REFKEEN_TITLE_AND_VER_STRING " - Command line arguments ***");
	BE_ST_puts("");
#ifdef REFKEEN_ENABLE_LAUNCHER
	BE_ST_puts("Launcher is started unless any command line argument is passed.");
	BE_ST_puts("");
#endif
	BE_ST_puts("List of possible command line arguments:");
	BE_ST_puts("-gamever <VER>: Select game version supported by this executable.");
#ifdef REFKEEN_VER_CATADVENTURES
	BE_ST_puts("-skipintro: Skip what is found in the original intro EXE and start game.");
#ifdef REFKEEN_VER_CATABYSS
	BE_ST_puts("-showslides: Show the electronic catalog / hint book.");
#else
	BE_ST_puts("-showslides: Show the hint book.");
#endif
#endif
	BE_ST_puts("-passorigargs <...>: Pass all following arguments to the original game port.");
	BE_ST_puts("-datadir <...>: Specify an alternative path for game data (separated by ver.).");
	BE_ST_puts("-cfgdir <...>: Specify an alternative path for new cfg files (not old CONFIG).");
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
			if (gameVerVal % 4 == 3) // HACK for line splitting
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
	BE_ST_InitCommon();

#ifdef REFKEEN_CONFIG_ENABLE_CMDLINE

	// Parse arguments
	bool showHelp = false;
#ifdef REFKEEN_VER_CATADVENTURES
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
		{
			showHelp = true;
		}
		else if (!BE_Cross_strcasecmp(1+argv[1], "gamever"))
		{
			if ((argc == 2) || !(*argv[2]))
			{
				showHelp = true;
				break;
			}
			for (selectedGameVerVal = 0; selectedGameVerVal < BE_GAMEVER_LAST; ++selectedGameVerVal)
			{
				if (!BE_Cross_strcasecmp(argv[2], refkeen_gamever_strs[selectedGameVerVal]))
					break;
			}
			if (selectedGameVerVal == BE_GAMEVER_LAST)
			{
				showHelp = true;
				break;
			}

			argv += 2;
			argc -= 2;
		}
#ifdef REFKEEN_VER_CATADVENTURES
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

	if (showHelp)
	{
		show_command_line_help();
	}
	else
	{
// HACK
#ifdef REFKEEN_VER_CATABYSS
#define catacombs_exe_main abysgame_exe_main
#elif (defined REFKEEN_VER_CATARM)
#define catacombs_exe_main armgame_exe_main
#elif (defined REFKEEN_VER_CATAPOC)
#define catacombs_exe_main apocgame_exe_main
#endif
		// Main functions prototypes
		void abysgame_exe_main(void);
		void armgame_exe_main(void);
		void apocgame_exe_main(void);

		BE_Cross_PrepareGameInstallations();
#ifdef REFKEEN_ENABLE_LAUNCHER
		if (startLauncher)
		{
			BE_Launcher_Start();
		}
		else
#endif
		{
			BE_Cross_InitGame(selectedGameVerVal);
#ifdef REFKEEN_VER_CATADVENTURES
			// Main functions prototypes
			void catacombs_exe_main(void); // See HACK above
			void slidecat_exe_main(void);
			BE_Cross_StartGame(argc, argv, showSlides ? &slidecat_exe_main : (skipIntro ? &catacombs_exe_main : NULL));
#else
			BE_Cross_StartGame(argc, argv, NULL);
#endif
		}
	}

#else // REFKEEN_CONFIG_ENABLE_CMDLINE

	BE_Cross_PrepareGameInstallations();
	BE_Launcher_Start();

#endif // REFKEEN_CONFIG_ENABLE_CMDLINE

	BE_ST_ShutdownAll();
	return 0;
}
