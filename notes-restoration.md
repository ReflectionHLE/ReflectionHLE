*** WARNING: DO NOT TRY TOO HARD TO BUILD ANY OF THE ORIGINAL EXECUTABLES! ***

Please remember that any little difference, not matter how small it is,
can lead to a vastly different EXE layout. This includes differences in:
- The development tools (or parts of such); For instance, a compiler, a linker,
an assembler, or even a support library or header. A version number is not
a promise for having the exact tool used to reproduce some executable.
- The order in which source code files are listed in a given project file.
- Project settings.
- Source file specific settings in such a project.
- The order in which source code files are passed to a linker.
- Any modification to a source code file (especially a header file).
- More than any of the above.

Following the warning, a description of the ways in which the executables were
reproduced is given.

With Borland C++ 3.1, this patched codebase can be used to reproduce any of the
executables coming from the following original releases, with a few caveats:
- Wolfenstein 3D Shareware 1.4 Apogee (with cheats).
- Wolfenstein 3D Registered 1.4 Activision.
- Spear of Destiny 1.4 Activision (no copy protection).

The macros that should be defined in VERSION.H for each build are:
- Wolf3D Shar. Apogee: ARTSEXTERN, DEMOSEXTERN, CARMACIZED, UPLOAD.
- Wolf3D Reg. Activision: GOODTIMES, ARTSEXTERN, DEMOSEXTERN, CARMACIZED.
- SOD Activision: SPEAR, GOODTIMES, ARTSEXTERN, DEMOSEXTERN, CARMACIZED.

Borland C++ 3.1 for DOS should be used for these builds; Not Borland C++ 2.0
nor any version of Turbo C/C++, but precisely Borland C++ 3.1. You should make
copies of the correct GAMEPAL.OBJ and SIGNON.OBJ object files within in the
"OBJ" subdirectory (multiple choices are found in "STATIC").
The WOLF3D.PRJ file has been updated to reflect this.

After opening the project with BC31, go to Options -> Compiler -> Advanced
code generation. Make sure that the chosen "Instruction Set" is 80286 for
the Apogee build or 80386 for any of the Activision builds. Chances are you
will need to set up the correct LIB and INCLUDE paths in the project.
Make sure that VERSION.H is properly edited as described above.

Furthermore, for the Apogee build, you should remove the files WOLFHACK.C
and WHACK_A.ASM from the project (using the IDE). A better alternative is
excluding the two files from link (via local "Options").

You should then be able to build the corresponding executable. Note that if
you want to add WOLFHACK.C and WHACK_A.ASM back for the Activision builds,
they should be added *exactly* at their original locations in the files list,
i.e., WOLFHACK.C is the third in the list (following H_LDIV.ASM)
and WHACK_A.ASM is the fourth.

With the help of LZEXE 0.91, you can get precisely the original Shareware
Apogee executable, byte-by-byte. There are a few issues with the Activision
builds, though:
- The Wolf3D Activision EXE (as available from Steam for a while) cannot be
unpacked with UNLZEXE 0.8. Maybe a few of its first bytes were manually
modified, and maybe not. Anyway, UNLZEXE 0.7 can be used instead.
- For a currently unknown reason, using Borland C++ 3.1 as instructed above,
any of the executables gets padded with a chunk of zeros representing the BSS
image. A way to tell the IDE to not do this has not been found, although
a manual call to TLINK (or a different revision of Borland C++) may help 
here. It looks like the /v argument recognized by TLINK has an impact.
While the original Apogee EXE is filled with such zeros after unpacking,
this is not the case with the Activision EXEs. Ignoring these, though, you
basically get the same data. To observe that, first pack any of the constructed
Activision EXEs with LZEXE 0.91 and the unpack with UNLZEXE 0.7 (effectively
removing unrelated trailing data). Next, unpack any of the Activision
executables originally released using UNLZEXE 0.7 as before.
Up to the BSS segment and a couple of length fields in the headers of all
involved EXEs, you should get the exact same EXEs as the originals.
