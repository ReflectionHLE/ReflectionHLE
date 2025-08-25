# Acknowledgements

## Source codes on which this project is based on

- Original Keen Dreams source code releases (all supported versions).
- Catacomb 3-D v1.22 source code release.
- The Catacomb Abyss v1.24 source code release.
- The Catacomb Armageddon v1.02 source code release.
- The Catacomb Apocalypse v1.01 source code release.
- Wolfenstein 3D and Spear of Destiny source code release.
- The Nuked OPL3 emulator. (DOSBox' DBOPL was originally used.)
- A CRC-32 implementation.
- unlzexe v0.8.
- depklite, a modification (mostly a C port) of ExeUnpacker from OpenTESArena.

## Additional sources/data used for these ports

- A few fonts from fntcol16.zip for text mode emulation and the launcher,
after converting them.
- Minor source code modifications, for reproduction of Catacomb 3-D v1.00 and
The Catacomb Abyss v1.13.
- Reconstructed source codes for INTRO.EXE/CATABYSS.EXE/CATARM.EXE/CATAPOC.EXE
(Introduction Program), LOADSCN.EXE (screens shown on quit)
and DEMOCAT.EXE/HINTCAT.EXE (Electronic Catalog / Hint book),
a few DOS programs distributed with the Catacomb Adventure
Series (or at least specific versions of the episodes).
- Reconstructed source code for differing versions of Wolfenstein 3D and
Spear of Destiny, along with two versions of 3D Catacomb games
and the one DOS version of Super 3-D Noah's Ark.

As of Sep 26 2024, the additional sources in question may be found here:
https://bitbucket.org/gamesrc-ver-recreation/

## Keen Dreams open-source release

The original Keen Dreams source code release was made possible by
a crowdfunding effort. It was brought to you by Javier M. Chavez
and Chuck Naaden with additional support from:

* Dave Allen
* Kirill Illenseer
* Michael Jurich
* Tom Laermans
* Jeremy Newman
* Braden Obrzut
* Evan Ramos
* Sam Schultz
* Matt Stath
* Ian Williams
* Steven Zakulec
* et al

## Catacombs open-source release

The whole Catacombs series from the 1990s, as well as Hovertank 3D,
were open-sourced by Richard Mandel of Flat Rock Software.

## Wolfenstein 3D and Spear of Destiny open-source release

Wolfenstein 3D and Spear of Destiny were open-sourced
by John Carmack while working for id Software in the 1990s.

## ReflectionHLE programming

* Yoav N.

## ReflectionHLE contributors

* Braden Obrzut: Direct contributions to the ReflectionHLE code base,
plus other additions brought up in order to build macOS bundles.
* GoGoOtaku: Source code patches.
* dege-git: Source code patches.
* Maxim Genis: Application icon.

## Additional credits

* David Gow, for a quite useful hint about memory wrapping for Keen Dreams,
plus having earlier experience from working with him beforehand. The latter
was mainly (if not only) referring to the Omnispeak project, covering
a source port of Commander Keen 4-6 based on reverse-engineering.
Gow was doing related research works on at least a couple of
Keen games and was also working on the Handmade Penguin tutorial.
* adurdin, quantumg, keenrush, levellord, levellass and lemm for their
differing research works on Keen games. Among other things, that includes
patches for original DOS executables of Commander Keen games, as well as
contributions that helped making three projects based on
reverse-engineering possible. One is the aforementioned Omnispeak
source port of Keen 4-6, worked on with Gow and lemm and also making use
of earlier Keen 4 reverse engineering efforts of adurdin. The second one
is NetKeen, based on earlier works as mentioned here and started by lemm,
originally as a derivative of Keen 5. Aforementioned efforts involving
Keen 4 reversing and NetKeen also contributed to Omnispeak's development.
The third project to mention is Chocolate Keen, a source port of Keen 1-3
based on reverse-engineering. ReflectionHLE's author started it, based on
quantumg's works identifying various functions and lemm applying these works
to assembly sources. lemm eventually contributed to Chocolate Keen directly.
* Caitlin Shaw for CloneKeen, a reimplementation of Keen 1-3.
* Gerhard Stein for Commander Genius, covering a few engines
used as reimplementations of Keen 1-3 and Keen 4-6. Support
for Keen Dreams was added using Reflection Keen Dreams.
Commander Genius was originally started as a branch
of CloneKeen known as CloneKeenPlus. While less relevant to ReflectionHLE
itself, decompression code was reused for Chocolate Keen (Keen 1-3).

* Past and present members of the DOSBox team, or the following at the least:
harekiet, qbix79, finsterr, fanskapet, canadacow, c2woody, h-a-l-9000.
Not only the DOSBox project makes it possible to (quite faithfully) play a ton
of DOS games on more recent platforms, but its DBOPL emulator is used for AdLib
emulation; Other programs have had their uses of DBOPL over the years.
Originally, ReflectionHLE made use of DBOPL as well. Furthermore,
while not necessarily intended for assistance with porting, the
DOSBox debugger has surely (seriously) been very useful.

* Nuke.YKT for the Nuked OPL3 emulator. This emulator is one example
of a project of him for which accuracy of behaviors is a high priority.
Other examples: PCDoom (excluding the proprietary DMX sound library,
instead using his own DMX wrapper backed by the Apogee Sound System) and
various other source reconstruction efforts. A subset of these efforts
was used for the EDuke32-based Rednukem, NBlood and PCExhumed ports,
with the latter being co-developed with Barry Duncan.

* Braden Obrzut, for earlier reverse-engineering work on Super 3-D Noah's Ark,
which was used as a base for a DOS source reconstruction.

* Mitugu (Kou) Kurizono, David Kirschbaum, Toad Hall, Dan Lewis, Alan Modra,
Vesselin Bontchev (and possibly others) for their contributions leading to
unlzexe v0.8.

* Past and present developers of OpenTESArena, as well as other contributors,
including afritz1, Allofich, Dozayon, kcat, mdmallardi, pcercuei and Ragora.
OpenTESArena is an open-source re-implementation of The Elder Scrolls: Arena.
A component of OpenTESArena, ExeUnpacker, was modified into depklite
(mostly a port of ExeUnpacker from C++11 to C99), which is used to
unpack Keen Dreams v1.00 executables (originally packed using PKLITE).

* Craig Bruce for his CRC-32 implementation.

* John Carmack, John Romero, Jason Blochowiak, Tom Hall, Adrian Carmack and
Robert Prince for their original works on Keen Dreams, Catacomb 3-D and
Wolfenstein 3D, a lot of these used in the Catacomb Adventure Series
and Super 3-D Noah's Ark later.
* Kevin Cloud and Jay Wilbur for their contributions to Wolfenstein 3D.
* Mike Maynard, James Row, Nolan Martin, Steven Maines, Carol Ludden,
Jerry Jones, James Weiler and Judi Mangham for their additional works
on the Catacomb Adventure Series.
* Vance Kozik for Super 3-D Noah's Ark music, and Rebecca Ann Heineman for
sound code as used in SNES versions of Wolfenstein 3D and Super 3-D Noah's Ark.
More people potentially contributed to Super 3-D Noah's Ark development in
other manners, but an exact complete list was not found as of writing this.
* Shawn Green, testing the SNES version of Wolfenstein 3D which
was used as a base for the original version of Super 3-D Noah's Ark,
before porting the game from SNES to DOS. The person also
contributed to the Jaguar port of Wolfenstein 3D.

For reference, at least a small subset of code specific to the SNES port
and what is more generally known as the Mac Family of Wolfenstein 3D ports
eventually found its way into the DOS port of Super 3-D Noah's Ark.
At times, a few features originally specific to the SNES port
were re-implemented in other manners.

* Apologizes to anybody not mentioned here who deserves a credit. Guess I can
say to all of you, that you should be considered special here. :)
