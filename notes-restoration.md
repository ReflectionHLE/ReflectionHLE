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
of the executables coming from the following original releases, at least
up to a call to UNLZEXE 0.7:  
- Wolfenstein 3D Shareware v1.4, Apogee release (with cheats).  
- Wolfenstein 3D Registered v1.4, Apogee release (with cheats).  
- Wolfenstein 3D Registered v1.4, ID Software release.  
- Wolfenstein 3D Registered v1.4, early GT Interactive release (same EXE as
in the ID Software release, except for a different logo in the signon screen).  
- Wolfenstein 3D Registered v1.4, late GT Interactive release
(no three-letter code is shown after completing an episode).  
- Wolfenstein 3D Registered v1.4, Activision release.  
- Spear of Destiny demo v1.0, FormGen release.  
- Spear of Destiny v1.4, Activision release (no copy protection).  

The originally released WOLF3D.PRJ file was used as a base for a total of
eight project files included in this source tree. Each of them can be
used for building a specific game version out of the ones above.

Borland C++ 3.1 should be used in all cases, except for the SOD demo project,
for which Borland C++ 3.0 should be used.

How were the project files (and a bit more) modified from the original:
- The two projects for the Activision builds target the 386 architecture,
while the ones for the Apogee, GT and SOD demo builds target 286.
- The locations of SIGNUP.OBJ and GAMEPAL.OBJ are updated. Different files may
be used for different game versions. In fact, while the palette is the same for
the SOD demo and Activision SOD builds, different OBJs are used nevertheless.
This is because the palette is a whole segment in the demo EXE, but just
a part of the data segment in the Activision SOD EXE.
- Paths to "OBJ" directories are replaced, so each project generates objects in
its own subdirectory within "OBJ".
- Paths to development environment (INCLUDE and LIB directories) are modified.
As expected there are chances you will want to edit these, depending on your
setup of development tools.
- VERSION.H is edited. Each project defines a version-identifier macro,
which is used to let the preprocessor define a few macros used in the
original codebase (like UPLOAD for the Apogee shareware release).
- WOLFHACK.C and WHACK_A.ASM are removed from all non-Activision projects.
In addition, WL_TEXT.C is removed from the SOD demo project.
- The list of objects in the SOD demo project (for linking purposes)
is reordered: ID_IN precedes SIGNON.
- Almost all optimizations are removed in the SOD demo project. In the list of
optimizations as shown in the BC++ 3.0 IDE, only "Suppress redundant loads",
"Jump optimization" and "Standard stack frame" are toggled on. Furthermore,
the remaining selected choices are: "Register Variable" - "Automatic",
"Common Subexpressions" - "No Optimization" and "Optimize For" - "Speed".
- There may be at least one other difference at the least. Obviously, source
code files other than VERSION.H are edited as required. This include the
addition of the new header file GFXV_APO.H for the Apogee builds,
with resource definitions taken off the Wolf4SDL source port.

With the exception of the SOD demo, Borland C++ 3.1 for DOS should be used for
these builds; Not Borland C++ 2.0 nor any version of Turbo C/C++, but precisely
Borland C++ 3.1.
For the SOD demo Borland C++ 3.0 should be used, and only that.

These projects should let you build all of the executables. At least with
Borland C++ 3.0, there are chances you will get weird macro-related error
messages. If everything is going as expected, you should select the error in
the Message window to see what is its "cause" in the sources and jump to that
location in the code, and then retry to compile from there.
Repeat until an EXE is built.

More generally, if there are problems then you should probably remove the
corresponding SYM file (e.g., WL6ACT14.SYM) and retry from scratch.

With each of the Apogee EXEs, things should be straightforward. Simply pack
the generated EXE with LZEXE 0.91. If anything has been done as expected then
you should get precisely an original Shareware or Registered Apogee 1.4 EXE,
byte-by-byte.

Some other bit of preprocessing is required for the rest of the EXEs, though.
Basically, the BSS section should be stripped out of the EXE. You can use
the tool STRIPBSS (supplied in the repository from which this modified Wolf3D
source tree originally comes). For a specific EXE that is going to be modified,
look in the MAP file generated with it for the location of the BSS section in
the EXE image. You can use the "GREP" tool supplied as a part of the collection
of development tools from BC++ for convenience. An example usage:

GREP " _BSS " WL6ACT14.MAP

The leftmost value outputted (a hexadecimal number) should be the one you are
interested in. This should be the second argument to STRIPBSS, just without
the mention of the 'H' letter. For WL6ACT14 this should be 3E03EH, so the
command involving STRIPBSS is:

STRIPBSS WL6ACT14.EXE 3E03E

This can similarly be applied to the other EXEs.

In the cases of the early GT (WL6GT114.EXE) and ID EXEs, after using STRIPBSS
you should get exactly the original EXE.

For each of the remaining EXEs, you can use LZEXE91. This does not mean
you already get the original EXE, though.

In the cases of the SOD demo and the late GT EXEs, you should be done and
get exactly the originals. For The Activision EXEs, though, a bit more work
is required. Looks like the LZEXE signature at the end of the file
(a bunch of bytes, not the string "LZ91" at the beginning) is a bit different
(seem shifted for most) from the output of LZEXE91. So, you should unpack
the original Activision EXE (but better make a backup first), using
UNLZEXE 0.7 (0.8 is expected to fail). For the sake of comparison,
the new EXE you have just packed using LZEXE91 should be unpacked in a
similar fashion. Hopefully, two compared (unpacked) EXEs should be identical.

One final note: The source code as originally released by ID Software seems
to be in some intermediate state in-between the later GT Interactive release
and the Activision release. As in the former, the GT logo is shown in the
signon screen and some text is shown on quit (after changing to text mode).
On the other hand, the project file is closer to the one used here for
the Activision release, having the source files WOLFHACK.C and WHACK_A.ASM
added to the project, as well as targeting 80386 rather than 80286.
