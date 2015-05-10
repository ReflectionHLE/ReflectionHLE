#include "id_heads.h"

#ifdef REFKEEN_VER_CATADVENTURES
#define second_main intro_exe_main
#elif defined REFKEEN_VER_CAT3D
#define second_main cat3d_exe_main
#elif defined REFKEEN_VER_KDREAMS
#define second_main kdreams_exe_main
#endif

int id0_argc;
const char **id0_argv;

void second_main (void); // MACRO is here

int main(int argc, char **argv)
{
	id0_argc = argc;
	// HACK: In Keen Dreams CGA v1.05, even if argc == 1, argv[1] is accessed...
	// Furthermore, in Keen Dreams Shareware v1.13, argc, argv[1], argv[2] and argv[3] are all modified...
	// And then in Catacomb Abyss, argv[3] is compared to "1". In its INTROSCN.EXE argv[4] is compared...

	// FIXME FIXME FIXME Using correct argv[0] for "compatibility" (see catabyss, ext_gelib.c)
	const char *our_workaround_argv[] = { "INTRO.EXE", "", "", "", "", NULL };
	if (argc < 6)
	{
		for (int currarg = 1; currarg < argc; ++currarg)
		{
			our_workaround_argv[currarg] = argv[currarg];
		}
		id0_argv = our_workaround_argv;
	}
	else
	{
		// REFKEEN - Hack, but we don't access argv directly anyway...
		id0_argv = (const char **)argv;
	}
	BE_ST_InitAll();
	BE_Cross_PrepareGameInstallations();
	BE_Cross_PrepareSearchPaths();
	BE_Cross_Compat_FillObjStatesWithDOSPointers(); // Saved games compatibility
	second_main();
	BE_ST_ShutdownAll();
	return 0;
}
