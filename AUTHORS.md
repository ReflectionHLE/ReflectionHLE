# Acknowledgements

## Source codes on which this project is based on

- Original Keen Dreams source code releases (all supported versions).
- Catacomb 3-D v1.22 source code release.
- The Catacomb Abyss v1.24 source code release.
- The Catacomb Armageddon v1.02 source code release.
- The Catacomb Apocalypse v1.01 source code release.
- Wolfentein 3D and Spear of Destiny source code release.
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
Spear of Destiny, as well as the one DOS version of Super 3-D Noah's Ark.

As of Sep 26 2020, these can be found here:
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

The whole Catacombs series from the 90s, as well as Hovertank 3D,
were open-sourced by Richard Mandel of Flat Rock Software.

## Wolfenstein 3D and Spear of Destiny open-source release

Wolfenstein 3D and Spear of Destiny were open-sourced
by John Carmack while working for id Software in the 90s.

## ReflectionHLE programming

* Yoav N.

## ReflectionHLE contributors

* Source code patches: Braden Obrzut
* Application icon: Maxim Genis

## Additional credits

* David Gow, for a quite useful hint about memory wrapping for Keen Dreams,
and having general earlier experience from working with him beforehand.
Also done some research works on at least a couple of Keen games,
and worked on the Handmade Penguin tutorial.
* adurdin, quantumg, keenrush, levellord, levellass and lemm for their
differing research works on Keen games.
* Caitlin Shaw for CloneKeen, a reimplementation of Keen 1-3.
* Gerhard Stein for Commander Genius, covering a few engines
used as reimplementations of Keen 1-3 and Keen 4-6. Support
for Keen Dreams was added using Reflection Keen Dreams.
Originally started as a branch of CloneKeen known as CloneKeenPlus.

* Past and present members of the DOSBox team, or the following at the least:
harekiet, qbix79, finsterr, fanskapet, canadacow, c2woody, h-a-l-9000.
Not only the DOSBox project makes it possible to (quite faithfully) play a ton
of DOS games on more recent platforms, but its DBOPL emulator is used for AdLib
emulation; This also applies to modern source ports, not just DOSBox itself.
Originally, ReflectionHLE made use of DBOPL as well. Furthermore,
while not necessarily intended for assistance with porting, the
DOSBox debugger has surely (seriously) been very useful.

* Nuke.YKT for the Nuked OPL3 emulator. This emulator is one example
of a project of him for which accuracy of behaviors is a high priority.
Other examples: PCDoom (excluding the proprietary DMX sound library,
instead using his own DMX wrapper backed by the Apogee Sound System),
the EDuke32-based Rednukem, NBlood and PCExhumed ports,
the latter of which being co-developed with Barry Duncan.

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
Wolfenstein 3D, a lot of these used in the Catacomb Adventure Series later.
* Kevin Cloud and Jay Wilbur for their contributions to Wolfenstein 3D.
* Mike Maynard, James Row, Nolan Martin, Steven Maines, Carol Ludden,
Jerry Jones, James Weiler and Judi Mangham for their additional works
on the Catacomb Adventure Series.

* Apologizes to anybody not mentioned here who deserves a credit. Guess I can
say to all of you, that you should be considered special here. :)

