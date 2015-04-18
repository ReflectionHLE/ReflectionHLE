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

With the right tools, this patched codebase can be used to reproduce any
of the executables coming from the following original releases, at least up to
a call to UNLZEXE 0.7/0.8, and with a micro-difference from the SOD v1.0 EXE:  
- Wolfenstein 3D Shareware v1.0+1.1+1.2+1.4, Apogee releases (with cheats).  
- Wolfenstein 3D Registered v1.1+1.2+1.4, Apogee releases (with cheats).  
- Wolfenstein 3D Registered v1.4, early GT Interactive release.  
- Wolfenstein 3D Registered v1.4, ID Software release (same EXE as in the early
GT Interactive release, except for a different logo in the signon screen).  
- Wolfenstein 3D Registered v1.4, late GT Interactive release
(no three-letter code is shown after completing an episode).  
- Wolfenstein 3D Registered v1.4, Activision release.  
- Spear of Destiny demo v1.0, FormGen release.  
- Spear of Destiny v1.0+1.4, FormGen releases (copy protected).  
- Spear of Destiny v1.4, Activision release (no copy protection).  

The originally released WOLF3D.PRJ file was used as a base for the
various project files included in this source tree. Each of them can
be used for building a specific game version out of the ones above.

How were the project files (and a bit more) modified from the original
----------------------------------------------------------------------

The released sources, including the project file, appear to be in a state close
to what was used to make the late Activision EXEs, so it's not a surprise that
the various versions were generally made in a kind-of reverse order
(although it's not exactly a linear order).
- The two projects for the Activision builds target the 386 architecture,
while the ones for all earlier builds target the 286.  
- The locations of SIGNUP.OBJ and GAMEPAL.OBJ are updated. Different files may
be used for different game versions. In fact, while the palette is the same for
the various SOD builds, different OBJs are used, nevertheless.
This is because the palette is a whole segment in the FormGen SOD EXEs,
but just a part of the data segment (aka the dseg) in the Activision SOD EXE.
Similarly, the Wolf3D palette has its own segment in the Apogee releases
preceding v1.4, while it's a part of the dseg in the rest.  
- Paths to "OBJ" directories are replaced, so each project generates objects in
its own subdirectory within "OBJ".  
- Paths to development environment (INCLUDE and LIB directories) are modified.
As expected there are chances you will still want to edit these, depending on
your setup of development tools.  
- VERSION.H is edited. Each project defines a version-identifier macro,
which is used to let the preprocessor define a few macros used in the
original codebase (like UPLOAD for the Apogee shareware release).  
- This macro is separately added via a command line argument to the assembler
for ID_VL_A.ASM and ID_VH_A.ASM, though, since the global project setting
doesn't seem to apply to ASM files. These two ASM files do include a new
VERSION.EQU file now. No such change was done for any of the other ASM files
because there's no need.  
- A few chunks of WL_ACT2.C are relocated into the following three files, which
are "#included" from WL_ACT2.C: WL_FPROJ.C, WL_FSMOK.C, WL_SROCK.C.
This is done since the exact location of each such chunk in WL_ACT2.C
depends on the version (basically pre-v1.4 Apogee releases vs all the rest),
and because this could otherwise lead to a lot of code duplication, with
possibly unexpected consequences for anybody tinkering with the code.
In fact, it happened while preparing the projects for Wolf3D v1.1+1.2!  
- WOLFHACK.C and WHACK_A.ASM are removed from all non-Activision projects
(they seem to be used just in projects targeting the 386). Furthermore,
WL_TEXT.C is removed from the SOD FormGen projects. It is not removed from the
SOD Activision project, though (which seems to be based on the Wolf3D one).  
- In general, the reconstructed pre-v1.4 Apogee projects files are based on the
(reconstructed) SOD demo project. A few more details are given later, but
WL_TEXT.C is back in these.  
- The list of objects in the SOD demo and pre-v1.4 Apogee projects
(for linking purposes) has modified orderings. In all v1.4 releases
(Wolf3D and SOD), the following objects are linked in the given order:
SIGNON, GAMEPAL, ID_CA, ID_IN. In the SOD demo, though, the order of linkage
is: ID_CA, ID_IN, SIGNON, GAMEPAL. In addition, in the pre-v1.4 releases the
order is given by: ID_CA, GAMEPAL, SIGNON, ID_IN.  
- Almost all optimizations are removed in the pre-v1.4 projects (Wolf3D/SOD).
In the list of optimizations as shown in the BC++ 3.0 IDE,
only "Suppress redundant loads", "Jump optimization" and "Standard stack frame"
are toggled on. Furthermore, the remaining selected choices are:
"Register Variable" - "Automatic", "Common Subexpressions" - "No Optimization"
and "Optimize For" - "Speed".  
- There may be at least one other difference at the least. Obviously, source
code files other than VERSION.H are edited as required. This includes the
addition of the new header file GFXV_APO.H for the Apogee builds,
with resource definitions taken off the Wolf4SDL source port.  

Building each EXE
=================

Required tools:
- Borland C++ 3.0 (and no other version), for all pre-v1.4 releases of
Wolf3D and SOD.  
- Borland C++ 3.1 (exactly this one) for the v1.4 releases of Wolf3D and SOD.  
- LZEXE 0.91.  
- STRIPBSS for certain EXEs (should be included in the repository originally
containing these modified Wolf3D/SOD sources).  
- UNLZEXE 0.7/0.8 for certain EXEs (one requires 0.8, another one needs 0.7).  

Notes before trying to build anything:
- This may depend on luck. Maybe you'll get a bit different EXE.  
- A project file should be opened with Borland C++ using a command similar to
the following, if not the same: "bc WL1APO10.PRJ".  
- LZEXE 0.91 and UNLZEXE 0.7/0.8 should be used in a similar manner with the
corresponding EXE as the input, e.g., "lzexe WL1APO10.EXE". LZEXE may ask you
to confirm if the EXE has trailing data (e.g., debugging symbols).  
- STRIPBSS looks for a MAP file generated by Borland C++'s linker, so make
sure you're in the directory that has the generated MAP file (and EXE) before
using STRIPBSS. It should be used like this: "stripbss WL1APO10.EXE".  
- It is assumed that you have a working DOS-compatible environment with a ready
installation of the correct version of Borland C++, including a properly set
PATH environment variable.  
- There's no separate WL6APO11 project. You should look for WL6APO12, since
versions 1.1 and 1.2 of Registered Wolf3D share the same EXE.  
- There's also no separate project for any cheats-disabled Apogee EXE.
Originally, the cheats were removed by opening each EXE in a hex editor and
replacing the "goobers" command-line code with seven spaces. If this is rather
done by modifying the sources, the generated EXE layout is expected to be
different, given that the "Duplicate strings merged" option is toggled on
(and possibly also if it weren't originally toggled).  
- Due to apparent Borland C++ 3.0 bugs, when it's used to build a project for
the first time (no DSK or SYM file), there are chances the build process will
stop with unexpected compilation error/warning messages. If things are working
as expected, you should select the error in the Message window to see what is
its "cause" in the sources and jump to that location in the code
(you should be able to press on the "Enter" key in order to do this),
and then retry to compile from there. Repeat until an EXE is built.  
- If there are other weird problems, try removing the SYM file and then
rebuilding. Remember, though, that luck is important here, and again you
may fail to get an EXE which is clearly close to an original one.  

Building any of the pre-v1.4 Apogee EXEs
----------------------------------------

1. Open project with Borland C++ 3.0.  
2. Press on F9 to build. Skip any compilation error as described above.  
3. Quit Borland C++ and use LZEXE 0.91 to pack the generated EXE.  
4. Hopefully you should get exactly the original EXE.  

Building the SOD demo v1.0 FormGen EXE
--------------------------------------

1. Open project with Borland C++ 3.0.  
2. Press on F9 to build. Skip any compilation error as described above.  
3. Quit Borland C++ and run STRIPBSS with the generated EXE as the input.  
4. Use LZEXE 0.91 to pack the generated EXE.  
5. Hopefully you should get exactly the original EXE.  

Building the SOD v1.0 FormGen EXE (up to calls to UNLZEXE8 and BSS size)
------------------------------------------------------------------------

1. Open project with Borland C++ 3.0.  
2. Press on F9 to build. Skip any compilation error as described above.  
3. Quit Borland C++ and use LZEXE 0.91 to pack the generated EXE.  
4. Hopefully you should get an EXE close to the original, although the latter
has the "LZ91" string removed, along with a difference in a couple of bytes
inside the EXE header, specifying an amount of additional memory the program
needs (measured in paragraphs i.e., 16-bytes groups).  

You can unpack both EXEs with UNLZEXE8 (but better make a backup first) so the
difference regarding the "LZ91" string is gone (UNLZEXE7 is expected to refuse
unpacking the original EXE). However, there'll still be a difference with these
other two bytes. Usually, they're used to specify the size of the so-called
"BSS section", and STRIPBSS should be used on the EXE created by Borland C++
to get similar results. In this case, though, even without STRIPBSS, the two
SOD v1.0 FormGen EXEs (original and recreated) should have the exact same size
(before, and after LZEXE decompression), so it is not clear why is there any
"BSS section" in the original. To further add to the confusion, it was common
to use a modified EXE that skips the Copy Protection screen, possibly more
often than the original EXE, and this file may have its own "BSS section" size.

Building any of the (cheats-enabled) v1.4 Apogee EXEs
-----------------------------------------------------

1. Open project with Borland C++ 3.1.  
2. Press on F9 to build.  
3. Quit Borland C++ and use LZEXE 0.91 to pack the generated EXE.  
4. Hopefully you should get exactly the original EXE.  

Building the SOD v1.4 FormGen EXE
---------------------------------
1. Open project with Borland C++ 3.1.  
2. Press on F9 to build.  
3. Quit Borland C++ and delete the OBJ file OBJ\SODFOR14\WL_GAME.OBJ.  
4. Repeat step 1 and then repeat step 2.  
5. Quit Borland C++ and run STRIPBSS with the generated EXE as the input.  
6. Use LZEXE 0.91 to pack the generated EXE.  
7. Hopefully you should get exactly the original EXE.  

Building the Wolf3D v1.4 early GT or ID EXE (same EXE up to SIGNON.OBJ)
-----------------------------------------------------------------------
1. Open project with Borland C++ 3.1.  
2. Press on F9 to build.  
3. Quit Borland C++ and run STRIPBSS with the generated EXE as the input.  
4. Hopefully you should get exactly the original EXE.  

Building the Wolf3D v1.4 late GT EXE
------------------------------------
1. Open project with Borland C++ 3.1.  
2. Press on F9 to build.  
3. Quit Borland C++ and run STRIPBSS with the generated EXE as the input.  
4. Use LZEXE 0.91 to pack the generated EXE.  
5. Hopefully you should get exactly the original EXE.  

Building any of the v1.4 Activision EXEs (up to calls to UNLZEXE7)
------------------------------------------------------------------
1. Open project with Borland C++ 3.1.  
2. Press on F9 to build.  
3. Quit Borland C++ and run STRIPBSS with the generated EXE as the input.  
4. Use LZEXE 0.91 to pack the generated EXE.  
5. Hopefully you should get an EXE close to the original. They should basically
be more-or-less the same, although the LZEXE signature (not the "LZ91" string)
of one EXE is a bit different from the other's (seem shifted for most).  

You can unpack both EXEs with UNLZEXE7 (but better make a backup first) so you
can hopefully get identical EXEs afterwards.

A few final notes
-----------------

The source code as originally released by ID Software seems to be in some
intermediate state in-between the late GT Interactive release of Wolf3D and
the Activision release. As in the former, the GT logo is shown in the signon
screen and some text is shown on quit (after changing to text mode). On the
other hand, the project file is closer to the one used here for the Activision
release of Wolf3D, having the source files WOLFHACK.C and WHACK_A.ASM added to
the project, as well as targeting 80386 rather than 80286. If a guess can be
made, maybe the changes were done while preparing the Japanese version of
Wolfenstein 3D for the PC-98, to be published by Imagineer. There were
at least a few more localized versions (apparently for DOS),
though, so this could apply to any of these.
