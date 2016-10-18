/* Copyright (C) 2014-2016 NY00123
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

#include "id_heads.h"
#include "SDL_main.h"

int id0_argc;
const char **id0_argv;

const char *be_main_arg_datadir = NULL;
const char *be_main_arg_newcfgdir = NULL;

static void show_command_line_help()
{
	char gameverstrbuffer[80] = "";
	char *gameverstrptr = gameverstrbuffer, *gameverstrend = gameverstrbuffer + sizeof(gameverstrbuffer);
	for (int gameVerVal = 0; gameVerVal < BE_GAMEVER_LAST; ++gameVerVal)
	{
		if (gameVerVal < BE_GAMEVER_LAST-1)
			gameverstrptr = BE_Cross_safeandfastcstringcopy_2strs(gameverstrptr, gameverstrend, refkeen_gamever_strs[gameVerVal], ", ");
		else
			gameverstrptr = BE_Cross_safeandfastcstringcopy(gameverstrptr, gameverstrend, refkeen_gamever_strs[gameVerVal]);
	}

	// HACK - For text mode emulation (and exit handler)
	BE_ST_PrepareForGameStartup();

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
	BE_ST_printf("%s\n", gameverstrbuffer);
	BE_ST_HandleExit(0);
}

int main(int argc, char **argv)
{
	BE_ST_InitCommon();
	// Parse arguments
	bool showHelp = false;
#ifdef REFKEEN_VER_CATADVENTURES
	bool skipIntro = false;
#endif
	int selectedGameVerVal = BE_GAMEVER_LAST;

#ifdef REFKEEN_PLATFORM_OSX
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
		BE_Cross_PrepareGameInstallations();
#ifdef REFKEEN_ENABLE_LAUNCHER
		if (startLauncher)
		{
			BE_Launcher_Start();
		}
		else
#endif
		{
#ifdef REFKEEN_VER_CATADVENTURES
			BE_Cross_StartGame(selectedGameVerVal, argc, argv, skipIntro);
#else
			BE_Cross_StartGame(selectedGameVerVal, argc, argv, 0);
#endif
		}
	}
	BE_ST_ShutdownAll();
	return 0;
}
