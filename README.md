# Reflection Keen

Reflection Keen is a project consisting of source ports of the
following titles, all being inspired by the Chocolate Doom port:
- Keen Dreams.
- Catacomb 3-D (The Descent) and The Catacomb Adventure Series.
- Wolfenstein 3D, Spear of Destiny and Super 3-D Noah's Ark (DOS version).

## Terms of use

All corresponding license documents should be found under LICENSES.

Since the covered ports are based on multiple codebases with
varying licenses, the terms are described for each source port
as a whole, as well as the differing components.

Note that these terms do not cover any original game data,
which should be obtained separately.

The terms applying to the source ports (i.e., the executables):

- Reflection Keen Dreams, Reflection Catacomb and Reflection Wolfenstein 3D
are all released under the GNU GPLv2+. See gpl-2.0.txt for more details.

The terms applying to each component separately:

- The ported Keen Dreams, Catacomb 3-D, The Catacomb Adventures Series and
Wolfenstein 3D codebases are released under the GNU GPLv2+. See gpl-2.0.txt.

- The ported Wolfenstein 3D code is also available under the original
terms for the Wolfenstein 3D sources from 1995. See id-wolf3d.txt.

- The Reflection Keen back-end and Nuked OPL3 are released
under the GNU LGPLv2.1+. See lgpl-2.1.txt.

- The included depklite implementation, a modification of ExeUnpacker,
is released under the MIT License. See mit-opentesarena.txt for details.

- The included CRC-32 implementation is in the public domain. The same
applies to the modified unlzexe source (based on UNLZEXE.DOC from v0.4).

## How to build from the source code

You will need CMake, a compatible C or C++ compiler and SDL2.
Additionally, SpeexDSP is required for audio resampling.

While it should be possible to build the code as C,
you'll have to build more separate executables,
due to the lack of C++ namespaces.

This was tested with GNU C and C++ for Linux and Windows builds,
the latter of which being done with Mingw-w64.

## Acknowledgements

See AUTHORS.md.
