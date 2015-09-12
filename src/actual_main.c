/* Copyright (C) 2014-2015 NY00123
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

int id0_argc;
const char **id0_argv;

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

#ifdef REFKEEN_VER_KDREAMS
	BE_ST_puts("Reflection Keen - Command line arguments:");
#elif (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATADVENTURES)
	BE_ST_puts("Reflection Catacomb 3-D - Command line arguments:");
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif
	BE_ST_puts("");
	BE_ST_puts("Launcher is started unless any command line argument is passed.");
	BE_ST_puts("");
	BE_ST_puts("List of possible command line arguments:");
	BE_ST_puts("-gamever <VER>: Selects game version supported by this executable.");
#ifdef REFKEEN_VER_CATADVENTURES
	BE_ST_puts("-skipintro: Skips what is found in the original intro EXE and starts game.");
#endif
	BE_ST_puts("-passorigargs <...>: Passes all following arguments to the original game port.");
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
	bool startLauncher = (argc == 1);
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
		if (startLauncher)
		{
			BE_Launcher_Start();
		}
		else
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
