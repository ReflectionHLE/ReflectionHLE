#include "id_heads.h"

void id0_main(void);

int main(int argc, char **argv)
{
	id0_argc = argc;
	// HACK: In Keen Dreams CGA v1.05, even if argc == 1, argv[1] is accessed...
	// Furthermore, in Keen Dreams Shareware v1.13, argc, argv[1], argv[2] and argv[3] are all modified...
	// And then in Catacomb Abyss, argv[3] is compared to "1".
	char *our_workaround_argv[] = { argv[0], "", "", "", NULL };
	if (argc < 5)
	{
		for (int currarg = 1; currarg < argc; ++currarg)
		{
			our_workaround_argv[currarg] = argv[currarg];
		}
		id0_argv = our_workaround_argv;
	}
	else
	{
		id0_argv = argv;
	}
	BE_SDL_InitAll();
	BE_Cross_Compat_FillObjStatesWithDOSOffsets(); // Saved games compatibility
	id0_main();
	BE_SDL_ShutdownAll();
	return 0;
}
