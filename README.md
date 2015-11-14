Reflection Keen
===============

This is a codebase consisting of source ports of Keen Dreams, Catacomb 3-D
(The Descent) and The Catacomb Adventure Series. Other, but similar, names,
like "Reflection Keen Dreams", may also be used to describe specific ports.
Although it may initially seem non-obvious, the reason these ports are
available under a single project is that a lot of common code can be found.
It may be true that Keen Dreams is a smooth-scrolling 2D platformer game,
while the Catacombs are 3D first person shooters, but there are still common
low-level 2D picture and font drawing routines, as well as very similar user
input and sound routines. Files with such code are often marked "ID Engine".
This is probably more noticeable if Catacomb 3-D is compared to Keen 4-6,
given that these games share the same control panel (menu) code
(including the Paddle War game) and more.

A later revision of the original codebase is also found in Wolfenstein 3D.

Nevertheless, there is still a lot in the 3D Catacomb titles which is not found
in any of the Keens. Hence, a title like "Reflection Catacomb 3-D" may also be
used again.

These source ports aim to reproduce behaviors of original executables
for DOS, including bugs, at least if it's not too difficult
(e.g., anything that greatly depends on the memory layout).
With the exception of The Catacomb Armageddon/Apocalypse, this also includes
compatibility with saved games for original DOS executables (done per version).
The Chocolate Doom source port can be considered an inspiration for that.

In fact, originally this codebase started as "Chocolate Keen Dreams". With
the addition of support for The Catacomb Abyss, and to be a bit more original,
the titles of "Ref Keen Dreams" and "Ref Catacomb Abyss" were coined. Similar
titles were later used for the other Catacombs, with no specific name for the
whole codebase before "Reflection Keen". There are chances some of these names
are still found, here and there.

-----------------------------------------
Differences from the original executables
-----------------------------------------

There are some differences when it comes to the way command-line arguments are
parsed. The fact that a single EXE may support more than one version of a game
has an influence on that. See "Command line arguments" for more information
about supported command line arguments.

There are a few modifications from the originals when it comes to the arguments
following -passorigargs. For one, in the Shareware versions of Keen Dreams,
there's no need to execute a separate "START" file in order to launch the game.
However, the behaviors can be emulated by adding the /detour argument, e.g.,
by typing "refkdreams.exe -passorigargs /detour" on Windows. This means that,
in fact, the behaviors are reversed in comparison to the originals.

The same applies to The Catacomb Adventure Series. The very first command line
argument (after -passorigargs) should be (yeah that's no mistake): ^(a@&r`
Depending on the environment in use, you may need to escape some characters
and/or add e.g., quotation marks. Again, that's the opposite of the
original behaviors.

In addition, a portion of the games' versions' came with multiple executables.
Not all behaviors of these are ported, but this is at least partially the case.
Following are descriptions of separate executables with ported behaviors:
- When you quit Keen Dreams v1.13 (Shareware release) within the game, you
should get a textual screen showing (now outdated) ordering information.
In the original release, this is done via a separate LOADSCN.EXE program,
while the game itself is run from KDREAMS.EXE.
- With a few differences, the same applies to The Catacomb Abyss v1.13
(another Shareware release) via a similar-yet-different LOADSCN.EXE program.
- Each of the episodes from The Catacomb Adventure Series begins with an
intro sequence, being run with an EXE separate from the one used for the game
itself. The EXE pairs (of intro and game) are as follows:
[INTRO.EXE, CATABYSS.EXE] for The Catacomb Abyss v1.13,
[CATABYSS.EXE, ABYSGAME.EXE] for The Catacomb Abyss v1.24,
[CATARM.EXE, ARMGAME.EXE] for The Catacomb Armageddon v1.02
and [CATAPOC.EXE, APOCGAME.EXE] for The Catacomb Apocalypse v1.01.

There are also a few differences in regards to memory management when
it comes to the additional executables of LOADSCN.EXE and INTRO.EXE:
They may share some memory in this port.

Finally, about INTRO.EXE (or registered CATABYSS.EXE/CATARM.EXE/CATAPOC.EXE)
and LOADSCN.EXE, source codes for these EXEs had to be manually reconstructed.
See "Additional sources/data used for these ports" below for details about
these codes and more.

For another difference which applies to The Catacomb Armageddon/Apocalypse, if
the cheat code letting one cycle through ceiling/sky colors is used while there
is flashing (a lightning), this can lead to so-called "undefined behaviors". It
may seem harmless, but a crash and/or other unexpected side-effects may occur.

----------------
What is included
----------------

Reflection Keen consists of ports of Keen Dreams and all 3D Catacomb games.

The Keen Dreams port is run from a single executable, compatible with four
DOS releases. The original releases supported by the port, using source codes
for them, are Shareware (EGA) v1.13, CGA v1.05, Registered (EGA) v1.93
and Shareware (EGA) v1.20.

For Catacomb 3-D, versions 1.00 and 1.22 should be supported. The original
source code release was modified for the addition of 1.00.

For The Catacomb Abyss, versions 1.13 (QA [0], Shareware) and 1.24 (rev 1)
are supported. The source code release was modified for support
of v1.13. Furthermore, The Catacomb Armageddon v1.02 (rev 1)
and The Catacomb Apocalypse v1.01 (rev 1) are supported.

Note that these ports do not come with a copy of any game, which is required.
Look below for mentions of "game data" to find possible sources for these.

Also note that Keen Dreams v1.20 hasn't been widely available so far, although
it is briefly mentioned in the Apogee FAQ. Furthermore, based on the source
codes release, the differences between v1.13, v1.93 and v1.20 are quite small,
and the two releases of v1.20 and v1.93 share the same static game data
(e.g., common EGA and map headers).

--------------
Current status
--------------

- The mouse can be used for control panel navigation in Keen Dreams, while the
keyboard is usable basically everywhere in all supported games. The mouse can
also be used for (partial) in-game controls in the supported Catacomb titles.
- Basic joystick support is in, being kind-of similar to the DOS days so it
doesn't remind of more recent titles. However, while currently experimental
and disabled by default, you can also check "Alternative Controller Schemes"
below.
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
- With the exception of The Catacomb Armageddon/Apocalypse,
saved game compatibility is implemented per game release separately (e.g., the
refkdreams-cga105 executable should be compatible with Keen Dreams CGA v1.05),
but there can still be some new non-vanilla bugs. Due to technical limitations,
by default saved games for The Catacomb Armageddon/Apocalypse aren't compatible
with the original DOS executables (Armageddon v1.02 / Apocalypse v1.01), unless
you're lucky. In fact, even with any of the original DOS EXEs, there's no
guarantee that a game being saved will work later while using the exact same
EXE (some data in the saved game depends on the location of a modified copy
of the DOS executable in the address space). If you still want some form of
compatibility, though, there is a way to control this. See "Configuration file"
below for the hidden farptrsegoffset setting.

----------------------------------------------------------------------------
Where can I get compatible Keen Dreams game data (updated as of Nov 14 2015)
----------------------------------------------------------------------------

You can download a Shareware release of Keen Dreams, v1.13, from one of a few
places online, at the least. Links for reference:

http://cd.textfiles.com/megagames2/GAMES/APOGEE/KEENDR13.ZIP  
(You want keendm.zip) http://dukeworld.duke4.net/classic%20dukeworld/share/

-----------------------------------
New *incompatible* Keen Dreams data
-----------------------------------

As a side-note, there is a new official port of Keen Dreams available on Steam,
IndieGameStand and possibly also another store.
Note that Reflection Keen is technically *not* compatible with the data of this
port, but you may still be interested. EGA and CGA graphics are both included.

Links to stores' pages:
http://store.steampowered.com/app/356200/
https://indiegamestand.com/store/1317/keen-dreams/

--------------------------------------------------------------------------
Where can I get compatible Catacombs game data (updated as of Nov 14 2015)
--------------------------------------------------------------------------

The Catacomb Abyss v1.13 can be downloaded as Shareware from the net. You can
try any of these links:

http://cd.textfiles.com/maxx/tothemaxpcg/ARCADE/CATABS13.ZIP  
(You want v1.13) http://www.classicdosgames.com/game/The_Catacomb_Abyss.html

Copies of all games from the Catacomb series, including Catacomb 3-D and
The Catacomb Adventure Series, are currently available from GOG.com under
the "Catacombs Pack". On the Windows platform and under normal circumstances,
Reflection Keen can auto-detect and read the data from this pack
(assuming it was installed from setup_catacombs_pack_2.1.0.5.exe).

Link to the pack on GOG.com: http://www.gog.com/game/catacombs_pack

------------------------------------------------------------
Running any of the games using the corresponding source port
------------------------------------------------------------

Reflection Keen should consist of five executables, one per game, capable of
launching all supported versions of the given game:
- refkdreams(.exe) for Keen Dreams.
- refcat3d(.exe) for Catacomb 3-D (The Descent).
- refcatabyss(.exe) for The Catacomb Abyss.
- refcatarm(.exe) for The Catacomb Armageddon.
- refcatapoc(.exe) for The Catacomb Apocalypse.

If you used an installer for the games, creating shortcuts in the way, clicking
on the correct shortcut should be sufficient.

If you want to manually start a game on Windows, make sure that SDL2.dll
resides with the exe of choice.

Check above for "game data" in order to find a way to obtain compatible data.

----------------------------
The Reflection Keen launcher
----------------------------

When you start any of the Reflection Keen ports using the corresponding
shortcut or EXE file, you should get a window with a launcher. It should be
possible to navigate through that launcher with a keyboard and/or mouse.

While a US layout keyboard is currently assumed for the launcher
(mostly for consistency with the games where it's also done, just like in DOS),
you can use the keyboard's letter keys for selecting menu items. This is
probably mostly useful when you manually want to add a game installation.

It's also possible to use a game controller for navigation, although
there are good chances you won't be able to do so, initially.
See "Adding an unsupported game controller" below for more details.

--------------------------------------
Where is the configuration file stored
--------------------------------------

Each EXE should generate its own configuration file, having a filename ending
with ".cfg".

These are the default locations for the configuration file:
- Windows: %APPDATA%\reflection-keen.
- Linux: $XDG_CONFIG_HOME/reflection-keen, if $XDG_CONFIG_HOME is defined
and filled. Otherwise, ~/.config/reflection-keen.

The -cfgdir command line argument can be used to override this path, but
make sure to be consistent with your choice. Otherwise, you may get
unexpected behaviors.

------------------------------------------
Where are other newly written files stored
------------------------------------------

There are other files that may be written by each source port, like files
generated by the original DOS versions of the supported games. These are
generally split into separate directories, based on original versions
of the games (although there may be exceptions).

These are the default locations for the various files:
- Windows: %APPDATA%\reflection-keen.
- Linux: $XDG_DATA_HOME/reflection-keen, if $XDG_DATA_HOME is defined
and filled. Otherwise, ~/.local/share/reflection-keen.

The -datadir command line argument can be used to override this path, but
again ensure you're consistent with your choice. Otherwise,
you may get unexpected behaviors, as with the cfg dir.

------------------------------------------------------------------
A few configuration file settings not accessible from the launcher
------------------------------------------------------------------

- "launcherexeargs" can be used to pass command-line arguments to the game
itself. Used only if the launcher is shown beforehand.
- "fullres" can be edited to specify a fullscreen resolution, with the form
<width>x<height>, just like the value of this "windowres". Clearly, it's not
necessary to pick a windowed resolution from the launcher, since the window
should be resizable. The reason there's no separate menu item for the full
screen resolution, is that using any resolution other than the desktop's may
lead to unexpected behaviors on certain environments, like desktop icons
getting messed up.
- "manualgamevermode" can be used if one wants to replace some game data.
By default, checksum and filesize verifications are done, so Reflection Keen
won't let you mistakenly start a game with modified data, even if the DOS EXE
or another original game file is OK. Given that none of the DOS versions
has such a verification (at least in general), "manualgamevermode" is an
unofficial path for doing the same here. Note that it is an UNOFFICIAL feature.
You may wish to make a full backup of the directory containing
"newly written files" as mentioned above before using this.
Also note that no automatic detection of game installations, other than what's
manually added, is done, in case you set "manualgamevermode" to "true".

The following setting is not even written to the cfg by default:
- "farptrsegoffset" can be used for (some) level of saved game compatibility
in The Catacomb Armageddon/Apocalypse (doesn't apply to Abyss). Using just any
of the original DOS executables for one of these titles, it is possible that
a game saved at some point will fail to properly load (or even close to that)
later, due to the location of a modified copy of the DOS EXE in the address
space. The value given to "farptrsegoffset" is a hexadecimal value that can be
used to control this location (as if a DOS executable were used).
If you have a saved game coming from a default setup of DOSBox 0.74, or some
official SVN build of DOSBox from 2014, chances are farptrsegoffset=1A2 is the
desired setting.

MOST IMPORTANT: If a Catacomb Armageddon/Apocalypse game is saved using the
source port, it will not properly load once farptrsegoffset is modified!

--------------------
There is some bug...
--------------------

Naturally, bugs tend to happen in the porting process, and also later.
But, if it's rather a bug found in an original game release which is
reproduced, most chances are it won't be fixed, since these ports aim
to preserve behaviors of original releases, including bugs.

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
this assisted with the porting of Keen Dreams with CGA graphics later.

------------------------
Game controllers support
------------------------

There are two ways in which game controllers can be used in each game:
A classic controller scheme, and a modern one. The latter is currently the
default.

Following here is an explanation why do these schemes exist (either as an
experimental feature or not), and why should these be considered "alternative":
- Back in the beginning of the 90s, the various Keen and Catacomb titles were
originally released with support for 2-button joysticks. This is related to
limitations of the game port connector found in various IBM PCs and compatible
setups. A little bit later releases of Keen 4-6 were also made available,
with support for the Gravis PC GamePad, including all of its 4 buttons.
- In all releases, though, it was assumed a keyboard is present for various
purposes (like text input). While a joystick could be used as an alternative
for gameplay, the keyboard was practically a mandatory input device.
- More advanced game controllers were later released. Initially with game port
connectors, they were later substituted with USB controllers. Game controllers
also became closer in layouts and features to the gamepads commonly used
with various gaming consoles.
- However, one thing didn't change. There wasn't a standard layout for game
controllers that programmers could depend on with the PCs, the way it's been
done with keyboards and mice for many years. For instance, two gamepads may
look similar, with each of them having a d-pad, but one may report the
d-pad to be be a pair of analog axes (same as analog stick), while
the other gamepad reports it as four digital buttons.
- Hence, while keyboards and mice have generally been usable "out of the box"
with PCs for many years, and the same applies to game controllers for consoles,
this cannot be said about game controllers for PCs, at least for some time.
- At some later point, Microsoft introduced XInput, making it possible to
easily use Xbox 360 controllers with PC games, as well as other XInput-capable
controllers. As a side-effect, they defined a standard game controller layout
for PCs.
- The same cannot be said about other, non-XInput capable controllers, though.
The lack of a standard layout still applies to them. Hence, a mapping table
is required if one wants to map each such controller's layout to some
common layout, so it can be used "out of the box".
- The SDL_GameController API from SDL2 was created to do exactly that, with
support for multiple platforms. This is basically a wrapper over the earlier
SDL_Joystick API, with the wrapper itself doing the mapping above.
- The modern controller scheme takes advantage of this API for a more
straightforward controller support. In addition, while the keyboard was
mandatory back in the days, various functionalities that originally required
a keyboard are now accessible from game controllers using this scheme.

---------------------------------------
Classic controller scheme - Limitations
---------------------------------------

With a few exceptions in mind, any game controller that can be used in general
should be usable by Reflection Keen with that scheme. However, there isn't
really a way to configure/select features of the controller (like buttons).
Limitations from the original DOS versions apply, too. This includes
the need to calibrate a joystick, as well as the fact that a keyboard device
(or another alternative) is still mandatory for various purposes.

--------------------------------------
Modern controller scheme - Limitations
--------------------------------------

With this scheme in mind, it is currently assumed the controller has the
following features (although some of these are configurable):
- A d-pad (if there's no d-pad, maybe you can fake one using an analog stick).
- Four face buttons (A, B, X, Y on the Xbox 360 controller).
- Two shoulder/bumper buttons.
- Two trigger buttons (they don't have to be analog).
- Reserved Back button (Esc key replacement) and Start button (Pause key).

Furthermore, you cannot take advantage of analog stick clicks or guide button
clicks as technically possible with the Xbox 360 controller.

In contrary to the situation with the classic controller scheme, the
Reflection Keen launcher should be used for configuring in-game actions
for the controller. Any in-game configuration setting is irrelevant,
and it's actually better to not touch it. A possible issue with this
is described below.

The game itself should detect a total of 0 joysticks while the modern
controller scheme is in use. With some exceptions, controller events
(like button presses/releases) are translated to keyboard or mouse events
that are parsed in the same ways as in the original DOS releases.

Note that while it is still possible to configure the keyboard in-game, as in
the original DOS releases, this can lead to side effects. For instance, if you
configure 'Space' to be the key for shooting a fireball in the game Catacomb
3-D, it will be used not just for shooting, but also for drinking potions.
This is the case in the original DOS releases, as well as the source port.
While the controller is used, this can further lead to a similar effect
when any of the controller's features (e.g., a button) is configured
either for shooting or potion drinking.

There may still be cases where a keyboard is required, but hopefully there
aren't really. An on-screen keyboard should be displayed in-game for
text input, and it's also possible to show a keyboard
for entering debug keys (e.g., cheat codes).

---------------------------------------------------------------------------
Modern controller scheme / Launcher - Adding an unsupported game controller
---------------------------------------------------------------------------

Given the need to map layouts as described above, not all game controllers are
supported with this scheme out-of-the-box, even if any of them can be used
as a joystick while the classic controller scheme is in use. It is
expected that all XInput-capable controllers are supported
on Windows, along with a few more.

SDL2 should have a small built-in mapping table for other controllers and
non-Windows platforms. In addition, a file named gamecontrollerdb.txt can
be prepared in the location of the Reflection Keen cfg file
(see "Where is the configuration file stored" above for details).
This file can have additional controller mappings.

Such a mapping can be prepared for an unsupported game controller.
There are a few ways to do this:
- Use the SDL2 Gamepad Tool available from General Arcade as of Mar 12 2015:
http://generalarcade.com/gamepadtool/
After using it, simply copy the contents of the newly generated
gamecontrollerdb.txt file to the one bundled with any of the
Reflection Keen source ports (or use it as-is if one doesn't exist).
- Alternatively start Steam (http://store.steampowered.com/) in Big Picture
mode, then choose Settings -> Controllers and configure the mapping. Once
that's done, it's expected that you can optionally upload the mapping so other
users of the same controller can take advantage of it. Either way, though,
the directory where Steam is installed should have a config/config.vdf file,
with a new "SDL_GamepadBind" entry. The value of this entry (following the
mention of the string "SDL_GamepadBind") should be copied to the end of the
file gamecontrollerdb.txt bundled with one of the Reflection Keen ports.
- Use test/controllermap.c (and controllermap.bmp) from the SDL2
sources, on which the SDL2 Gamepad Tool above may possibly be
based (source code only, no EXE): https://hg.libsdl.org/SDL/
This should write output to a command prompt or similar, including a mapping
that can be added to gamecontrollerdb.txt,

----------------------
Command line arguments
----------------------

Adding the single argument of -? should tell the details. For instance,
on Windows, in case refkdreams.exe is chosen, the command should look
like this: "refkdreams -?"

In case something goes wrong, these are the supported arguments common to
all games (i.e., all executables):

-gamever <VER>: Select game version supported by this executable.
-passorigargs <...>: Pass all following arguments to the original game port.
-datadir <...>: Specify an alternative path for game data (separated by ver.).
-cfgdir <...>: Specify an alternative path for new cfg files (not old CONFIG).
-fulllauncher: Show a fullscreen launcher window.
-softlauncher: Show a software-rendered launcher window (not fullscreen).

Te following argument is specific to the Catacomb Adventure Series:

-skipintro: Skip what is found in the original intro EXE and start game.

Note: The path passed to -datadir or -cfgdir is assumed to exist.

----------------------------------------
Building the ports from the source codes
----------------------------------------

- Development files for SDL 2.0.2+ are required, as well as GNU make and the
GNU Compiler Collection's C compiler (i.e. gcc). In practice, at the moment
only SDL 2.0 dev files, "make" and "gcc" are probably required. If nothing goes
wrong, a single "make" command should build various executables, each of them
supporting a different game. "make clean" removes any generated object or
executable file, assuming the *exact* same arguments have been passed
to "make" as before, with the exception of the "clean" argument.
- It is possible to build just the Keen Dreams executable by typing "make"
while in the "src/kdreams" subtree.
- Similarly, to build just Catacomb 3-D, you can type "make" while in the
"src/id91_11/cat_all/cat3d" subtree.
For The Catacomb Abyss it is "src/id91_11/cat_all/catabyss", and there are
two more such subdirectories for The Catacomb Armageddon and Apocalypse.
- MinGW can be used as well (tested on Linux). If you try to cross-compile
then you may wish to set PLATFORM=WINDOWS and BINPREFIX accordingly.

---------------------------------
Original sources this is based on
---------------------------------

- Original Keen Dreams source code releases (all supported versions).
- Catacomb 3-D v1.22 source code release.
- The Catacomb Abyss v1.24 source code release.
- The Catacomb Armageddon v1.02 source code release.
- The Catacomb Apocalypse v1.01 source code release.
- The DBOPL emulator from the DOSBox project.
- A CRC-32 implementation.
- unlzexe v0.8.

--------------------------------------------
Additional sources/data used for these ports
--------------------------------------------

- Minor source code modifications, for reproduction of Catacomb 3-D v1.00 and
The Catacomb Abyss v1.13.
- Reconstructed source codes for INTRO.EXE/CATABYSS.EXE/CATARM.EXE/CATAPOC.EXE
(intro with title screen and credits) and LOADSCN.EXE (screens shown on quit),
a couple of DOS programs distributed with the Catacomb Adventure Series
(or at least specific versions of the episodes).

As of December 20 2014, these can be found here:
https://bitbucket.org/NY00123/gamesrc-ver-recreation/

------------
Terms of use
------------

Reflection Keen and the DBOPL emulator are released under the GNU GPLv2+.
See LICENSE for more details.

Furthermore, the included CRC-32 implementation is in the public domain. The
same applies to the modified unlzexe source (based on UNLZEXE.DOC from v0.4).

Note that these terms do not cover any original game data required for usage
with Reflection Keen, which should be obtained separately. Check above
for "game data" in order to find a way to obtain compatible data.

---------------------------------------------------------------------------
Based on the README.md file of the original Keen Dreams source code release
---------------------------------------------------------------------------

The original Keen Dreams source code release was made possible by
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

* Mitugu (Kou) Kurizono, David Kirschbaum, Toad Hall, Dan Lewis, Alan Modra,
Vesselin Bontchev (and possibly others) for their contributions leading to
unlzexe v0.8.

* Craig Bruce for his CRC-32 implementation.

* Richard Mandel for releasing original source codes for the complete
Catacomb series and Hovertank 3D.

* John Carmack, John Romero, Jason Blochowiak, Tom Hall, Adrian Carmack and
Robert Prince for their original works for Keen Dreams and Catacomb 3-D, a lot
of these used in the Catacomb Adventure Series later.
* Mike Maynard, James Row, Nolan Martin, Steven Maines, Carol Ludden,
Jerry Jones, James Weiler and Judi Mangham for their additional works
for the Catacomb Adventure Series.

* Apologizes to anybody not mentioned here who deserves a credit. Guess I can
say to all of you, that you should be considered special here. :)

---------
Changelog
---------

??? ??, 2015 (v0.13.0):
- A minor breaking change: The disablesndsubsystem setting was changed to
sndsubsystem, for avoiding double negatives and being more consistent with
newly added code (the launcher).
- A new launcher is now in. By default, it is shown before starting any game,
but command line arguments can be used to skip it. It is now the way to
configure the Reflection Keen ports as an alternative to manually
editing cfg files. This may be mostly useful for configuring
game controllers with the modern controller scheme, but not only.
- Reflection Keen may detect and use existing game installations from more
than one location: Current working directory, GOG.com installation of
the Catacombs for windows, or a directory manually selected from the launcher.
There are also checksum and filesize validations for game data.
- Embedded resources like MAPHEAD.KDR are now written externally. They can
be extracted from original DOS executables automatically and then used by
Reflection Keen. However, the games still behave as if these resources
were linked into the EXEs. For each game, they're loaded to memory
by Reflection Keen before entering the game's main function.
- Some Reflection Keen EXEs unifications were made. To summarize,
there's now a single EXE per game, possibly supporting
multiple original versions of the game.
- Due to the above change, new files are now written to separate directories,
based on the game version.
- Files can be accessed by filenames in a case insensitive-manner.
Case sensitivity still applies to folder names, though!
- By default, the Reflection Keen cfg files are now written to a specific
directory depending on the current user. It may be overridden using the -cfgdir
command-line argument. For instance, on Linux, the default location is
$XDG_CONFIG_HOME/reflection-keen, if $XDG_CONFIG_HOME is defined
and filled, or ~/.config/reflection-keen otherwise.
- Reflection Keen looks for gamecontrollerdb.txt in the same place where
the cfg files are stored.
- Other files are similarly written to a possible different directory depending
on the current user, by default. -datadir can be specified
to override this. For another Linux example, the default location
is $XDG_DATA_HOME/reflection-keen or ~/.local/share/reflection-keen.
- Improvements were made to the modern game controller scheme. The ability to
configure it from the newly added launcher is one thing, but there's more. Some
of these may be a bit too technical, but a clear example may be the ability
to show an on-screen keyboard for entering debug keys (e.g., cheat codes).
- As a consequence, the modern game controller scheme is now the default.
- Changes were made to "key repeat/delay" behaviors. These are now done in
software (using SDL_GetTicks for timing), and should be similar while using a
game controller in the launcher. This is also the case while using a controller
in-game with the modern controller scheme, as long as keyboard key presses
are emulated internally, or an on-screen keyboard is in use.
- While a bit technical, BE_SDL* functions were renamed BE_ST*, emphasizing
the fact they may contain platform/framework specific code, but it doesn't have
to be SDL (e.g., in case there's no available port of SDL on some platform).
Filenames were similarly renamed, so there are now be_st.h,
be_st_sdl.h and be_st_sdl*.c.
- More fixes and other changes.

Mar 13, 2015 (v0.10.0):
- After some time of not having one title covering the whole codebase,
"Reflection Keen" is finally in use.
- Experimental support for alternative controller schemes is implemented. This
should theoretically make it more comfortable to use game controllers with the
Xbox 360 Controller layout. Taking advantage of the SDL_GameController API,
a mapping table is used so this is theoretically not limited just to XInput
controllers. This is currently disabled by default, though, and at
least one edit to the generated cfg file is expected. Furthermore,
as a side-effect SDL 2.0.2+ is known to be required now.
- Alright, it's probably better to consider this unofficial, but there is now
partially-tested support for big-endian architectures (or bi-endian archs in
big-endian modes). It's a bit difficult to test this with the lack of
appropriate hardware, though (in particular audio support is totally untested).
In addition, commonly used architectures of these days (x86, x86-64, ARM)
generally operate in little-endian modes. So, again, better assume that
big-endian modes aren't officially supported.
- It should be possible to disable the sound subsystem by enabling
the "disablesndsubsystem" setting in the generated cfg file. The code
changes also include a fix for an infinite loop in SD_WaitSoundDone
in case the sound subsystem is disabled. Furthermore, in such a case
a silent PC Speaker is emulated, but the OPL emulator isn't running.
- Code can optionally be built as C++ now, using g++ version 4:4.8.2-1ubuntu6
(should be based on v4.8.2). It is still built as C by default, but the ability
to build the code as C++ assisted with catching at least a few bugs.
- Added a fix (or possibly more than one) for Catacomb 3-D crashes. Note
that the way this was done, a vanilla Catacomb 3-D rendering glitch is *not*
reproduced now. It may be possible with more efforts, although this obviously
depends on the original EXE's layout.
- As in the original DOS executables, there shouldn't be a noticeable fizzle
fade after loading a saved game in any of the Catacomb Adventure Series titles
(except for maybe the first time after starting the application).

Dec 20, 2014 (v0.9.12):
- Complete support for the Catacomb Adventure Series has been integrated.
In addition to version 1.13 of The Catacomb Abyss (Shareware release),
there is support for Abyss v1.24, Armageddon v1.02 and Apocalypse v1.01.
- It shall be noted that while it's possible to save and load games for The
Catacomb Armageddon/Apocalypse, most chances are they won't be compatible with
any of the original DOS executables. A new, hidden farptrsegoffset setting can
be added to the cfg file in order to make this somewhat more feasible, although
there's no guarantee it'll always work with the same value, given the structure
of the saved games (doesn't apply to Abyss).
- For another note, if the cheat code letting one cycle through ceiling/sky
colors is used while there is flashing (a lightning), this can lead to
so-called "undefined behaviors". It may look like there's no harm,
but a crash and/or other unexpected side-effects may occur.
- It turns out VSync was still enabled in the preceding release (v0.9.6) by
default for The Catacomb Abyss (and Catacomb 3-D), as well as Keen Dreams with
CGA graphics. Now, though, some adjustments were made in regards to timing, so
Skull 'n' Bones from Catacomb 3-D can be played back at a rate closer to the
original while VSync is toggled on, and not just while it's off. Furthermore,
VSync is again toggled on by default for all supported games, with the
exception of the CGA release of Keen Dreams (where it's off by default).

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
