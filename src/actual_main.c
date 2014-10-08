/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "id_heads.h"

void id0_main(void);

int main(int argc, char **argv)
{
	id0_argc = argc;
	// HACK: In Keen Dreams CGA v1.05, even if argc == 1, argv[1] is accessed...
	// Furthermore, in Keen Dreams Shareware v1.13, argc, argv[1], argv[2] and argv[3] are all modified...
	char *our_workaround_argv[] = { argv[0], "", "", NULL };
	switch (argc)
	{
	case 1:
		id0_argv = our_workaround_argv;
		break;
	case 2:
		our_workaround_argv[1] = argv[1];
		id0_argv = our_workaround_argv;
		break;
	case 3:
		our_workaround_argv[1] = argv[1];
		our_workaround_argv[2] = argv[2];
		id0_argv = our_workaround_argv;
		break;
	default:
		id0_argv = argv;
		break;
	}
	BE_SDL_InitAll();
	BE_Cross_Compat_FillObjStatesWithDOSOffsets(); // Saved games compatibility
	id0_main();
	BE_SDL_ShutdownAll();
	return 0;
}
