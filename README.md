Ref Keen Dreams, Catacomb 3-D, Catacomb Abyss (formerly Chocolate Keen Dreams)
==============================================================================

These are ports of Keen Dreams, Catacomb 3-D (The Descent) and The Catacomb
Abyss. Although it may initially not seem obvious, the reason these ports are
available under a single project is that a lot of common code can be found.
The dimension counts may be different (Keen Dreams is a smooth-scrolling 2D
platformer game, while the Catacombs are 3D), but there's still common code
for low-level EGA output and 2D picture and font drawing routines, as well
as very similar user input and sound output routines. Files with such code
are often marked "ID Engine". A later revision of the codebase is also
found in Wolfenstein 3D.

These source ports aim to reproduce behaviors of original executables
for DOS, including bugs, at least if it's not too difficult
(e.g., anything that highly depends on the memory layout).
The Chocolate Doom source port can be considered an inspiration for that.

The term "Ref" from the ports' names comes from the word "Reflection".
These ports can be thought of reflections of the original counterparts.

Note that there may still be some differences from the originals.

-----------------------------------------
Differences from the original executables
-----------------------------------------

As of this version, you don't need to execute a separate "START" file in
order to play a game from any release. But, if relevant, you can emulate
the behaviors of skipping this in the original release, i.e., get a
message telling you should type START.

For Keen Dreams it can be done by adding the /detour command line argument,
e.g., typing "refkdreams-shar113.exe /detour" in a command prompt on Windows.
This is, by the way, the exact opposite of the behaviors of the original.

The same applies to The Catacomb Abyss, but may be a bit tricky. Basically, the
very first command line argument should be (yeah that's no mistake): ^(a@&r`
Depending on the environment, you may need to escape some characters and/or
add e.g., quotation marks.
Again, that's the opposite of the original behaviors. In addition,
we're actually talking about the behaviors of the INTRO.EXE file (letting you
choose the difficulty as of v1.13), rather than CATABYSS.EXE (the actual game).
There are also a few differences in regards to memory management when it comes
to the additional executables of LOADSCN.EXE (which includes the last textual
screen shown on quit) and INTRO.EXE: They may share some memory in this port.

Furthermore, if you want to skip the intro sequence and get right into the game
(same behaviors as executing "CATABYSS.EXE ^(a@&r`" for the original, v1.13),
simply specify /skipintro as the very first command line argument.
More arguments can be added, with behaviors similar to
what you'll get with the original DOS EXE.

Finally, about INTRO.EXE and LOADSCN.EXE, source codes for these EXEs had
to be manually restored. See "Additional sources/data used for these ports"
below for details about these codes and more.

For another possible difference, Skull 'n' Bones from Catacomb 3-D can be
slower than vanilla if VSync is in effect. This is a known limitation related
to timing, so for now this is disabled in the Catacombs.

----------------
What is included
----------------

The Keen Dreams port consists of four executables, each of them being
compatible with a different version of Keen Dreams. The original releases
supported by the port, using source codes for them, are Shareware (EGA) v1.13,
CGA v1.05, Registered (EGA) v1.93 and Shareware (EGA) v1.20.

For Catacomb 3-D, versions 1.00 and 1.22 should be supported. The source code
release has been modified for the addition of 1.00.

For The Catacomb Abyss there's currently a single executable based on the
source code release, modified to be compatible with Shareware v1.13 (QA [0]).

Note that these ports do not come with a copy of any game, which is required.
As of October 25, 2014, you can download Keen Dreams, Shareware release, v1.13,
from one of a few places online, at the least. Links for reference:

http://cd.textfiles.com/megagames2/GAMES/APOGEE/KEENDR13.ZIP  
(You want keendm.zip) http://dukeworld.duke4.net/classic%20dukeworld/share/

Similarly The Catacomb Abyss v1.13 can be downloaded. You can try any of
these links:

http://cd.textfiles.com/maxx/tothemaxpcg/ARCADE/CATABS13.ZIP  
(You want v1.13) http://www.classicdosgames.com/game/The_Catacomb_Abyss.html

--------------------------------------
Original sources/data this is based on
--------------------------------------

- Original Keen Dreams source code releases for all supported versions,
including static data (e.g., game text and audio/graphics/maps header files).
- Catacomb 3-D source code release for v1.22, including static data.
- The Catacomb Abyss source code release for v1.24, including static data.
- The DBOPL emulator from the DOSBox project.

--------------------------------------------
Additional sources/data used for these ports
--------------------------------------------

- Minor source code modifications and static data additions, for reproduction
of Catacomb 3-D v1.00.
- Minor source code modifications and static data additions, for reproduction
of The Catacomb Abyss Shareware v1.13.
- Reconstructed source codes for the INTRO.EXE and LOADSCN.EXE programs
distributed with The Catacomb Abyss v1.13.
- As of November 30 2014, these can be found here:
https://bitbucket.org/NY00123/gamesrc-ver-recreation/

------------
Terms of use
------------

Ref Keen Dreams, Ref Catacomb 3-D, Ref Catacomb Abyss and the DBOPL emulator
are released under the GNU GPLv2+. See LICENSE for more details.

Note that this does not cover the static data included for usage with Ref Keen
Dreams. However, as an exception to the terms above you may link and distribute
that data for the purposes of compatibility with the original game.

---------------------------------------------------------------------------
Based on the README.md file of the original Keen Dreams source code release
---------------------------------------------------------------------------

The original Keen Draems source code release was made possible by
a crowdfunding effort. It is brought to you by Javier M. Chavez
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

-----------------
Some more credits
-----------------

* sulix, for a hint about memory wrapping, as well as having earlier experience
with him in general. Also done research on Keen.
* adurdin, quantumg, keenrush, levellord, levellass and lemm for their
research works.
* Caitlin Shaw for CloneKeen, a reimplementation of Keen 1-3.
* gerstrong for Commander Genius, containing a few engines used as
reimplementations of Keen 1-3 and Keen 4-6. Originally started
as a branch of CloneKeen known as CloneKeenPlus.

* Past and present members of the DOSBox team, or the following at the least:
harekiet, qbix79, finsterr, fanskapet, canadacow, c2woody, h-a-l-9000.
Not only the DOSBox project makes it possible to (quite faithfully) play a ton
of DOS games on more recent platforms, but its DBOPL emulator is used for AdLib
emulation; This also applies to modern source ports, not just DOSBox itself.
Furthermore, while not necessarily intended for assistance with porting,
the DOSBox debugger has surely (seriously) been very useful.

* Richard Mandel for releasing original source codes for the complete
Catacomb series and Hovertank 3D.

* John Carmack, John Romero, Jason Blochowiak, Tom Hall, Adrian Carmack and
Robert Prince for their original works for Keen Dreams and Catacomb 3-D, a lot
of these used in the Catacomb Adventure Series (Catacomb Abyss included) later.
* Mike Maynard, James Row, Nolan Martin, Steven Maines, Carol Ludden,
Jerry Jones, James Weiler and Judi Mangham for their additional works
for the Catacomb Adventure Series.

* Apologizes to anybody not mentioned here who deserves a credit. Guess I can
say to all of you, that you should be considered special here. :)

------------------------------------------------------
Why are CGA graphics supported in the Keen Dreams port
------------------------------------------------------

Since there were originally releases compatible with the CGA, and we have the
source code(s) for these, then why not support these?
In fact, CGA graphics were supported before EGA in this port.

As a possible preparation for Commander Keen 4-6, some engine code considered
to be shared with these was originally ported from the Catacomb 3-D source code
(https://github.com/FlatRockSoft/Catacomb3D), with no actual testing.
CGA routines were preferred, given possible complications with the EGA
bit planes and multiple read/write modes. While Catacomb 3-D is EGA-only,
this assisted with the porting of Keen Dreams CGA later.

--------------
Current status
--------------

- The mouse can be used for control panel navigation in Keen Dreams, while the
keyboard is usable basically everywhere in all supported games. The mouse can
also be used for (partial) in-game controls in the supported Catacomb titles.
- Basic joystick support is in, although it's kind-of similar to the DOS days
so it doesn't remind of more recent titles.
- AdLib and PC Speaker sound emulations are built-in, including music playback
for Catacomb 3-D. AdLib emulation is possible thanks to the DBOPL emulator
from DOSBox.
- CGA/EGA graphics and text mode emulations (on an emulated VGA adapter)
are in.
- Games are playable using 64-bit Linux executables (tested on Ubuntu 14.04),
as well as 32-bit Windows executables, but these haven't yet been entirely
tested. New non-vanilla bugs are possible!
- There should be compatibility with the configuration files as generated by
the original DOS executables, but this hasn't been entirely tested.
- Saved game compatibility is implemented per game release separately
(e.g., the CGA executable should be compatible with Keen Dreams CGA v1.05),
but there can still be some new non-vanilla bugs.

----------------------
Making Keen Dreams run
----------------------

This requires one of the following releases: Keen Dreams Shareware v1.13,
CGA v1.05, Registered v1.93, Shareware v1.20.

For the Shareware release, v1.13, and on Windows desktops, you should simply
drop the files SDL2.dll and refkdreams-shar113.exe along with the files coming
with this Shareware release (v1.13), and then launch refkdreams-shar113.exe.

You should be able to find a copy of the Shareware release online.
Check "What is included" above for download links.

Similarly, refkdreams-cga105.exe should be used if playability of
the CGA release (v1.05) is desired.

Other versions should be supported in a similar manner. Note that v1.20
hasn't been widely available so far, although it is briefly mentioned in
the Apogee FAQ. Furthermore, based on the source codes release the differences
between v1.13, v1.93 and v1.20 are quite small, and the two releases of v1.20
and v1.93 share the same static game data (e.g., common EGA and map headers).

-------------------------------------
Making Catacomb 3-D (The Descent) run
-------------------------------------

This requires either Catacomb 3-D v1.00 or Catacomb 3-D: The Descent v1.22.

On Windows desktops, you should simply drop the files SDL2.dll and
refcat-100.exe or refcat-122.exe (depending on the version) along with
the files coming with the corresponding release of the game for DOS.

-----------------------------
Making The Catacomb Abyss run
-----------------------------

This requires the following release: Catacomb Abyss Shareware v1.13 (QA [0]).

On Windows desktops, you should simply drop the files SDL2.dll and
refcatabyss.exe along with the files coming with the shareware release.

------------------------------------------------
How to do *this and that* or: Configuration file
------------------------------------------------

On first launch of Ref Keen Dreams, a configuration file named refkdreams.cfg
should be generated. For Ref Catacomb 3-D the filename is refcat3d.cfg, and
for Ref Catacomb Abyss it is refcatabyss.cfg.
You can edit it to fit your needs. Currently available settings:
- "fullscreen" toggle. Value should be either "true" or "false".
- "fullres" for fullscreen window resolution. By default the value is "0x0",
implying desktop resolution, but it can be any other (but supported)
specific resolution, say (under some setups) "1024x768".
- "windowres" is similar, but for non-fullscreen windows. The default of "0x0"
refers to dimensions which are kind-of proportional to the desktop's.
- "displaynum" may be useful if you have multiple monitors,
but it's totally untested as of now.
- "sdlrenderer" lets you pick software rendering if you wish. Value should be
"auto" for hardware accelerated rendering (if supported) and "software"
for the other choice.
Note: Technically at least one more value should be acceptable,
but the values given above probably suffice for now.
- "vsync" lets you toggle Sync to VBlank. Valid values are "auto", "off", "on".
The default of "auto" implies "off" with CGA graphics and "on" with EGA within
Keen Dreams, since these probably represent the behaviors of the original
releases with a real CGA/EGA/VGA. For now it implies "off" in the Catacombs.
- "bilinear" toggles bilinear filtering. Valid values are "true" and "false".
If set to "true", consider taking a look at the "scalefactor" setting.
- "scaletype" can be used to toggle aspect ratio correction. Good e.g.,
in case screen burn-in is a concern. Valid values are "aspect" and "fill".
- "scalefactor" can be used for two-step scaling. When different from 1, and
with "bilinear=true", on compatible hardware the game's graphical contents to
be displayed should first be zoomed in each dimension by the given scale
factor, and then the zoomed pic should be scaled to the window with bilinear
filtering applied. This is done so scaling artifacts coming from non-bilinear
interpolation are less noticeable, while the output is not too blurry.
- "autolock" tells if the window should have full control of the mouse
(setting has no effect for fullscreen windows). Even if disabled, the cursor
should be hidden while in the middle of the window. Valid values are "true"
and "false".
- "sndsamplerate" lets you specify a sound sample rate (in Hz). Currently,
the default is 49716, which is close to the rate of OPL chips generating AdLib
sounds. It's also more efficient, as no interpolation is required,
since the rate of 49716Hz is used for AdLib emulation internally.

--------------------
There is some bug...
--------------------

This is still new, and bugs tend to happen in the porting process.
But, if it's rather a bug found in an original game released which is
reproduced, most chances are it won't be fixed, since these ports aim
to preserve behaviors of original releases, including bugs.

----------------------------------------
Building the ports from the source codes
----------------------------------------

- Development files for SDL 2.0 are required, as well as GNU make and the
GNU Compiler Collection's C compiler (i.e. gcc). In practice, at the moment
only SDL 2.0 dev files, "make" and "gcc" are probably required. If nothing goes
wrong, a single "make" command should build seven executables, one for each
original supported version of Keen Dreams for DOS, two more for Catacomb 3-D
and another one for The Catacomb Abyss. "make clean" removes any generated
object or executable file, assuming the *exact* same arguments have been
passed to "make" as before, with the exception of the "clean" argument.
- It is possible to build just one executable of Keen Dreams by going through
a specific Makefile in the "src/kdreams" subtree, e.g., "make -f Makefile.cga".
- Similarly, to build just Catacomb 3-D, you can type "make" while in the
"src/id91_11/cat3d" subtree (or pick a specific version as for Keen Dreams).
For The Catacomb Abyss it is "src/id91_11/catabyss".
- MinGW can be used as well (tested on Linux). If you try to cross-compile
then you may wish to set PLATFORM=WINDOWS and BINPREFIX accordingly.

---------
Changelog
---------

Nov 30, 2014 (v0.9.6):
- Support for Catacomb 3-D (The Descent) is now in. This includes support for
versions 1.00 and 1.22, as well as saved game compatibility with each of these
separately (although this can be buggy as usual). A lot of ID Engine code is
shared between the two, while an earlier revision of the ID Engine used by
Keen Dreams remains separate.
- Intro and exit screens are added for Catacomb Abyss. The intro can be skipped
by adding the /skipintro command line argument (same as launching vanilla
CATABYSS.EXE Shareware v1.13 with a seemingly-random pattern as an argument).
- Known non-vanilla bug/limitation: Skull 'n' Bones is a bit slower than
vanilla if VSync is toggled on (given a refresh rate of 60Hz), so it's disabled
by default in Catacomb 3-D for now. For the sake of consistency it's also
disabled in Catacomb Abyss. It is enabled in Keen Dreams by default if
EGA graphics are used, though.
- The Catacomb Abyss Vanilla bug reproduction: Numbers not shown in HUD on
startup immediately.
- A few more misc. changes.

Oct 25, 2014 (v0.9.0):
- RENAMING OF PORT: The Keen Dreams port has been renamed "Ref Keen Dreams".
This is done to be a just a little bit more original, and also follows the
addition of a new source port called "Ref Catacomb Abyss" (see next point).
Note that the source codes still have internal mentions of "CHOCO" or similar,
but from the user's point of view "Ref" is the new prefix for the two ports.
- Support for Catacomb Abyss (which shares a lot of code with Keen Dreams)
has been added. It is based on the original source code release from June 2014,
while modified to be compatible with the data from the Shareware release,
v1.13 (QA [0]).
- Apart from major modifications like ports of 3D scaling routines, there are
also some minor changes, like the support of a 640x200 graphics mode emulation
(for Catacomb Abyss help section, internally similar to 320x200) and overscan
borders flashing (technically supported since v0.8.10, but actually used now).
- There should be compatibility with saved games of the exact same Catacomb
Abyss release, but this can be buggy as usual. Furthermore, the way saved games
names are displayed in the corresponding dialogs (via the F3 or F4 key) may
seem a bit weird. This can be improved, but for now we have that.
- Another related complication is the fact that the Catacomb Abyss port is
compatible with a platform where case-sensitive filesystems are commonly used,
while under DOS this is not the case and filenames tend to be renamed uppercase
automatically. As of this version of the Catacomb Abyss port, newly saved
games' filenames are automatically converted to uppercase. The same applies
when typing a name for game loading (even if lowercase filenames are shown).
This shouldn't be a problem on case-insensitive filesystems, even if they're
case-preserving.
- Partial compatibility breakage: chocolate-keen-dreams.cfg is renamed
refkdreams.cfg. Similarly refcatabyss.cfg is used for Catacomb Abyss.

Oct 11, 2014 (v0.8.10):
- Fixed finale text printing (more generally, modified some functions
to better handle original code leading to undefined behaviors per the
C standard, or at least to crashes).
- Fixed King Boobus Tuber explosion.
- Added compatibility with the registered release, v1.93, and the Shareware
release, v1.20. Taking a look at the source codes release, they are both
almost identical to v1.13.
- Overscan borders are drawn now, even if permanently colored black (MM_SortMem
flashes the borders, but chances are the function is never called in practice).
- More miscellaneous fixes.

Oct 9, 2014 (v0.8.8):
- EGA graphics emulation has been added, and Chocolate Keen Dreams should be
compatible with the Shareware release of Keen Dreams, version 1.13.
- This includes compatibility with saved games (each Chocolate Keen exe should
be compatible with saved games of the corresponding release for DOS only).
- Two separate executables are used for the CGA and Shareware releases.
- chocolate-keen-dreams-cga.exe has been renamed to the shorter filename of
chocolate-kdreams-cga.exe. There's also chocolate-kdreams-shar.exe now.
- Difference from behaviors of vanilla Keen: For the Shareware release,
you don't need to select a "START" executable in order to launch the game.
But you can still get the message telling you need to type START by adding the
command-line argument of /detour (exact opposite of original behaviors).
- High scores table glitch is fixed.
- Other miscellaneous fixes.

Oct 3, 2014 (v0.8.2):
- Bilinear filtering can now be toggled on if hardware acceleration is in use.
- In case off-screen rendering is supported, it can be used for two-step
scaling of the graphical output: First nearest-neighbor interpolation with
the added "scalefactor" setting, and then bilinear.
- Sync to VBlank can be manually toggled now (if supported on the used setup).
By default "vsync=auto" is used, currently implying "off" (with CGA graphics).
- Live sound interpolation has been implemented, and it's possible to specify
a different sound sample rate, while the rate used for OPL emulation is still
49716Hz. Currently 49716 is the default value
- Fixed a bug: One cfg setting or more not written to file when it's expected.
- As of this version, the cfg file is (re)written on launch. This is done so
new settings can appear in the file if it's not up-to-date.

Sep 29 2014 (v0.8.1):
- Compatibility with saved games from vanilla Keen Dreams CGA v1.05 is in. As
expected, it can still be buggy (in fact, vanilla Keen also has its limits/bugs
with this).
- As a possibly indirect consequence, a vanilla Keen bug in
StateMachine/DoActor leading to a crash in Chocolate Keen Dreams
has been found (NULL pointer dereference). Workaround is applied now.
- The environment variables from last release are gone. Instead, you should
have the configuration file chocolate-keen-dreams.cfg to fiddle with (created
on launch if it doesn't exist).
- Custom fullscreen and windowed resolutions can be set in the configuration
file. Furthermore, aspect correction can be toggled off (e.g., in case screen
burn-in is a concern).
- Mouse cursor lock can be toggled off (from the cfg). By default the cursor is
now unlocked in a non-fullscreen window. Chances are the behaviors may still
feel a bit off, but at least we have that.
- Loading window with bars should display (while loading a map), although it's
shown for about a short moment on sufficiently fast machines.

Sep 26 2014 (v0.8.0):
- Initial release, based on Keen Dreams CGA v1.05. Should be playable with
sound effects and CGA graphics on Linux and Windows desktops. Game saving and
more is totally untested and may be incompatible with original data generated
by the corresponding DOS executable (saved games are known to be incompatible).

NY00123
