Reflection Keen
===============

This is a project consisting of source ports of Keen Dreams,
Catacomb 3-D (The Descent) and The Catacomb Adventure Series.

You may encounter some names like "Reflection Keen Dreams" or "Ref KDreams",
here and there. These are used to described the specific ports (by game).

------------
Terms of use
------------

Reflection Keen and the DBOPL emulator are released under the GNU GPLv2+.
See LICENSE for more details.

Furthermore, the included CRC-32 implementation is in the public domain. The
same applies to the modified unlzexe source (based on UNLZEXE.DOC from v0.4).

Note that these terms do not cover any original game data required for usage
with Reflection Keen, which should be obtained separately. Check below for
compatible "game data" in order to find a way to obtain such data.

---------------------------------------------------------------------
How is Keen Dreams (a 2D platformer) related to the Catacombs (FPSes)
---------------------------------------------------------------------

A lot of the original Keen Dreams code base, with changes, was re-used by
id Software not only in the later Keen games from 1991 (i.e., Keen 4-6), but
also in Catacomb 3-D. This should explain why do Keen 4-6 and Catacomb 3-D seem
to share features like the control panel (menu), including the Paddle War game.

While the later Catacomb games may have the control panel and other features
removed or replaced, they're still clear derivatives of Catacomb 3-D.

The Wolfenstein 3D sources are also based on Catacomb 3-D, and again share
a lot of similarities.

------------------------------------
Comparisons to original DOS versions
------------------------------------

These source ports aim to reproduce behaviors of original executables
for DOS, including bugs, at least if it's not too difficult
(e.g., anything that greatly depends on the memory layout).
With the exception of The Catacomb Armageddon/Apocalypse, this also includes
compatibility with saved games for original DOS executables (done per version).
The Chocolate Doom source port can be considered an inspiration for that.

In fact, originally this codebase started as a Keen Dreams port titled
"Chocolate Keen Dreams". With the addition of support for The Catacomb Abyss,
and also in order to be a bit more original, the titles of "Ref Keen Dreams"
and "Ref Catacomb Abyss" were coined. Similar titles were later used for
the other Catacombs, with no specific name for the codebase. There are
chances some of these names are still found, here and there.

As of this release, the whole codebase has the title of "Reflection Keen".

------------------------------------------------------------------------
Differences from the original releases related to command line arguments
------------------------------------------------------------------------

- There are some differences when it comes to the way command-line arguments
are parsed. The fact that a single EXE may support more than one version of
a game has an influence on that. See "Command line arguments" for more
information about supported command line arguments.
- In the original shareware releases of Keen Dreams, versions 1.13 and 1.20,
the /DETOUR command line argument should be passed in order to start the game.
This is not required in Reflection Keen Dreams, and in fact, leads to the same
behaviors as *omitting* this argument in any of these original DOS releases.
- The same applies to The Catacomb Adventure Series, regarding this command
line argument (yeah, that's no mistake): ^(a@&r`

----------------------------------------------------------------------------
Differences from the original releases related to usage of multiple DOS EXEs
----------------------------------------------------------------------------

There are some original game releases that have more than one EXE
(per release). Not all behaviors of such EXEs are implemented, but
when they are, there may be a few technical differences.

- For one, no separate EXE is actually used in each Reflection Keen port.
As an example, in Keen Dreams v1.13, the behaviors of KDREAMS.EXE (the game)
and LOADSCN.EXE (showing ordering information on quit) are present
in a single Reflection Keen Dreams EXE.
- There are a few differences with the memory management, when
it comes to LOADSCN.EXE and INTRO.EXE from The Catacomb Abyss v1.13.
Basically, they may share some memory in the Reflection Catacomb Abyss port.
- Some of these additional EXES, like INTRO.EXE and LOADSCN.EXE from
The Catacomb Abyss v1.13, lack original source codes as of this
release; So such sources had to be manually reconstructed.
See "Additional sources/data used for these ports"
below for details about these codes and more.
- Currently, ports of DEMOCAT.EXE/HINTCAT.EXE from The Catacomb Adventure
Series are *not* implemented. This is especially noticeable when you
start up The Catacomb Abyss v1.13 and then press on the "F10" key.

----------------------------------------------
One more difference from the original releases
----------------------------------------------

This one is specific to The Catacomb Armageddon/Apocalypse. If the
cheat code letting one cycle through ceiling/sky colors is used while there
is flashing (a lightning), this can lead to so-called "undefined behaviors". It
may seem harmless, but a crash and/or other unexpected side-effects may occur.

----------------
What is included
----------------

Reflection Keen consists of ports of Keen Dreams and all 3D Catacomb games.

Note that these ports do not come with a copy of any game, which is required.
Look below for mentions of "game data" to find possible sources for these.

----------------------------------------------
Supported game data, described by DOS versions
----------------------------------------------

- Keen Dreams: Shareware (EGA) v1.13, CGA v1.05, Registered (EGA) v1.93
and Shareware (EGA) v1.20. There's also partial support for the data
from the 2015 re-release (Steam / IndieGameStand).
- Catacomb 3-D (The Descent): Versions 1.00 and 1.22.
- The Catacomb Abyss: Versions 1.13 (QA [0], Shareware) and 1.24 (rev 1).
- The Catacomb Armageddon: Version 1.02 (rev 1).
- The Catacomb Apocalypse: Version 1.01 (rev 1).

Note that Keen Dreams v1.20 hasn't been widely available so far, although
it is briefly mentioned in the Apogee FAQ. Furthermore, based on the source
codes release, the differences between v1.13, v1.93 and v1.20 are quite small,
and the two releases of v1.20 and v1.93 share the same static game data
(e.g., common EGA and map headers).

----------------------------------------------------------------------------
Where can I get compatible Keen Dreams game data (updated as of Oct 12 2016)
----------------------------------------------------------------------------

You can download a Shareware release of Keen Dreams, v1.13, from one of a few
places online, at the least. Links for reference:

- http://cd.textfiles.com/megagames2/GAMES/APOGEE/KEENDR13.ZIP
- (You want keendm.zip) http://dukeworld.duke4.net/classic%20dukeworld/share/

-------------------------------------
Partially compatible Keen Dreams data
-------------------------------------

2015 had a re-release of Keen Dreams on Steam, IndieGameStand
and possibly also another store.
Note that while Reflection Keen is partially compatible with the 2015
data, it is still recommended to use data from any of the supported
DOS versions. In particular, the Reflection Keen Dreams codebase
is known to be internally different from the re-release's
codebase in many ways.

Links to stores' pages:
- http://store.steampowered.com/app/356200/
- https://indiegamestand.com/store/1317/keen-dreams/

--------------------------------------------------------------------------
Where can I get compatible Catacombs game data (updated as of Oct 12 2016)
--------------------------------------------------------------------------

The Catacomb Abyss v1.13 can be downloaded as Shareware from the net. You can
try any of these links:

- http://cd.textfiles.com/maxx/tothemaxpcg/ARCADE/CATABS13.ZIP
- (You want v1.13) https://www.classicdosgames.com/game/The_Catacomb_Abyss.html

Copies of all games from the Catacomb series, including Catacomb 3-D and
The Catacomb Adventure Series, are currently available from GOG.com under
the "Catacombs Pack". On the Windows platform and under normal circumstances,
Reflection Keen can auto-detect and read the data from this pack
(assuming it was installed from setup_catacombs_pack_2.1.0.5.exe).

Link to the pack on GOG.com: https://www.gog.com/game/catacombs_pack

------------------------------------------------------------
Running any of the games using the corresponding source port
------------------------------------------------------------

If you used an installer for the games, creating shortcut/launcher icons
on the way, then selecting the correct icon should be sufficient.

Reflection Keen should consist of five executables, one per game, capable of
launching all supported versions of the given game:

- reflection-kdreams(.exe) for Keen Dreams.
- reflection-cat3d(.exe) for Catacomb 3-D (The Descent).
- reflection-catabyss(.exe) for The Catacomb Abyss.
- reflection-catarm(.exe) for The Catacomb Armageddon.
- reflection-catapoc(.exe) for The Catacomb Apocalypse.

If you want to manually start a game on Windows, make sure that SDL2.dll
and libspeexdsp-1.dll reside with the exe of choice.

Check above for "game data" in order to find a way to obtain compatible data.

----------------------------
The Reflection Keen launcher
----------------------------

When you start any of the Reflection Keen ports using the corresponding
shortcut/launcher icon or EXE file, you should get a window with an
internal Reflection Keen launcher. It should be possible to navigate
through that launcher with a keyboard, a mouse and/or a touchscreen.

It's also possible to use a game controller for navigation, although
there are good chances you won't be able to do so, at least initially.
See "Adding an unsupported game controller" below for more details.

Note that a US keyboard layout is currently assumed for the launcher
(mostly for consistency with the games themselves, including DOS versions).

You can use the keyboard's letter keys, as well as a few other keys, for
selecting menu items. This is probably mostly useful when you manually want
to add a game installation.

Similarly, the keyboard can be used to enter command line arguments for the
game (after selecting "Set arguments for game" in the main menu). Press on
the "Enter" key to confirm the changes to the arguments, or "Esc" to cancel.

It's also possible to show an on-screen keyboard, in case you're not using
an actual keyboard. Click/Touch on the "..." button on the top-right of the
launcher window in order to show it.

In case you're using a supported game controller with the Xbox 360 controller
layout, press on the Y button to show the on-screen keyboard. While the
on-screen keyboard is shown, press on the controller's X button to toggle
shifted keys on/off. While editing command-line arguments, select
the "Ent" key in the on-screen keyboard to confirm the changes.
To discard the changes, press on the controller's B or Back button.

--------------------------------------
Where is the configuration file stored
--------------------------------------

Each EXE should generate its own configuration file, having a filename ending
with ".cfg".

These are the default locations for the configuration file:

- Windows: %APPDATA%\reflection-keen.
- Linux: $XDG_CONFIG_HOME/reflection-keen, if $XDG_CONFIG_HOME is defined
and filled. Otherwise, ~/.config/reflection-keen.
- Android: This is separated per installed Android app. For Keen Dreams,
this is the folder Android/data/net.duke4.ny.reflection_kdreams/files/appdata
within the shared storage, as often accessible when connecting an
Android-powered device to a PC/Mac using a USB cable. Similar folder
names are used for the other Reflection Keen apps (games).

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
- Android: Same as the location of the configuration file.

The -datadir command line argument can be used to override this path, but
again ensure you're consistent with your choice. Otherwise,
you may get unexpected behaviors, as with the cfg dir.

------------------------------------------------------------------
A few configuration file settings not accessible from the launcher
------------------------------------------------------------------

- (Not available on all platforms) "fullres" can be edited to specify a
fullscreen resolution, with the form {width}x{height}, just like the value
of the "windowres" setting. Clearly, it's not necessary to pick a windowed
resolution from the launcher, in case the window is resizable. The reason
there's no separate menu item for the full screen resolution, is that using
any resolution other than the desktop's may lead to unexpected behaviors on
certain environments, like desktop icons getting messed up.

The following settings are not even written to the cfg file by default:

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
- "farptrsegoffset" can be used for (some) level of saved game compatibility
in The Catacomb Armageddon/Apocalypse (doesn't apply to Abyss). Using just any
of the original DOS executables for one of these titles, it is possible that
a game saved at some point will fail to properly load later, due to
the location of a modified copy of the DOS EXE in the address space.
The value given to "farptrsegoffset" is a hexadecimal value that can
be used to control this location (as if a DOS executable were used).
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
A classic controller scheme, and a modern one. The latter is currently
the default.

Following is a short explanation for the presence of these two schemes:

- The classic controller scheme is used to (somewhat) mimic the behaviors of
the original DOS versions of the games. These versions have basic support for
2-buttons joysticks, which are vastly different from a lot of the game
controllers in use these days, and have great limitations. In particular,
there is often the need to go through a process called "calibration", and
a separate keyboard is still required for various purposes (e.g., text input).
Other games, like Wolfenstein 3D and later releases of Keen 4-6,
also have support for all 4 buttons of the Gravis PC GamePad,
but there are still clear limits.
- The modern controller scheme is a better fit for various game controllers
used these days, with an emphasis on the Xbox 360 controller layout.
It makes it possible to exclusively use such a controller, with
no other input device (like a keyboard) required. There are still
some limitations, though. Look for "Limitations" below for details.

Note that with the modern scheme, each of the games will behave like there are
0 joysticks. Instead, keyboard/mouse emulation is used behind the scenes.

---------------------------------------
Classic controller scheme - Limitations
---------------------------------------

With a few exceptions in mind, any game controller that can be used in general
should be usable by Reflection Keen with that scheme. However, there isn't
really a way to configure/select features of the controller (like buttons).
Limitations from the original DOS versions apply, too. As stated above, this
includes the need to calibrate a joystick, as well as the fact that a keyboard
device (or another alternative) is still mandatory for various purposes.

--------------------------------------
Modern controller scheme - Limitations
--------------------------------------

Unfortunately, due to historical reasons, a lot of game controllers for
PCs lack a standard layout that requires zero configuration from the user.
You may wish to check out "Adding an unsupported game controller" if
your controller cannot be used. Checking if it is usable in
the launcher is a good way to verify this.

With the modern scheme in mind, it is currently assumed the controller has
the following features (although some of these are configurable).
The Xbox 360 controller layout should be covered.

- A d-pad (if there's no d-pad, maybe you can fake one using an analog stick).
- Four face buttons (A, B, X, Y on the Xbox 360 controller).
- Two shoulder/bumper buttons.
- Two trigger buttons (they don't have to be analog).
- Reserved Back button (Esc key replacement) and Start button (Pause key).

It's also possible to take advantage of clickable analog sticks.

However, you *cannot* take advantage of guide button clicks as
technically possible with the Xbox 360 controller.

The Reflection Keen launcher should be used for configuring in-game actions
for the controller. Any in-game configuration setting is irrelevant,
and it's actually better to not touch it. A possible issue with this
is described below.

As stated above, with the modern controller scheme, 0 joysticks are detected
in-game, and keyboard/mouse emulation is in use.

While it is still possible to configure the keyboard in-game, as in
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

- Use the SDL2 Gamepad Tool available from General Arcade as of Oct 12 2016:
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

--------------------------
Multi-touch screen support
--------------------------

Version 0.16.0 of Reflection Keen introduced support for multi-touch input,
along with support for Android.

- On Windows and Linux, multi-touch input is disabled by default, because there
are chances such input may arrive from a multi-touch trackpad
(at least on Macs).
- On Android, multi-touch input is auto-detected by default. This means
that touch controls should be shown while touch input is in use, and hidden
once you begin using a different input device (like a game controller).
There may be exceptions to this, e.g., pressing on an Android device's "Back"
button shouldn't immediately hide touch controls.

You may permanently disable, or permanently enable, multi-touch input,
in case you have issues. Note that if you permanently *enable* multi-touch
input, then a mouse will behave more-or-less like a finger.

As with the modern controller scheme, if you take advantage of multi-touch
input, then, keyboard/mouse emulation is in use.

Also, you probably do *not* want to mess with the in-game input settings
while using multi-touch input (say, any in-game keyboard configuration).

----------------------------------
Android - Granting access to files
----------------------------------

As stated above, on all supported versions of Android, the source ports
can write files to directories which are specific to the Android apps. For
Keen Dreams, this is Android/data/net.duke4.ny.reflection_kdreams/files/appdata
within the shared storage (as accessible from a PC/Mac, using a USB cable).
It should be similar for the other ports.

However, read-only access to more directories is desired, so you have
the freedom to prepare a copy of a supported game (say Keen Dreams v1.13)
anywhere in the shared storage. You can then select this directory from
the Reflection Keen launcher, making it possible to play the game.

This is not an issue with versions of Android preceding 6.0. Either you grant
the app the permission to do so on installation (if required), or the app
isn't installed at all.

Beginning from Android v6.0, though, you don't have to grant this permission
on installation. You'll be asked to do so when you start the corresponding
Reflection Keen port, at least until you ask to permanently grant or deny
the permission. If it is granted, you can select a game directory as in
versions of Android preceding 6.0.

Otherwise, though, there are still app-specific directories
that may be accessed. Again assuming Keen Dreams for now, it's the
directory Android/data/net.duke4.ny.reflection_kdreams/files/user_gameinsts
within the shared storage.

You'll have to create this "user_gameinsts" directory (per app), though.

----------------------
Command line arguments
----------------------

Adding the single argument of -? should tell the details. For instance,
on Windows, in case reflection-kdreams.exe is chosen, the command
should look like this: "reflection-kdreams -?"

In case something goes wrong, these are the supported arguments common to
all games (i.e., all executables):

-gamever {VER}: Select game version supported by this executable.
-passorigargs {...}: Pass all following arguments to the original game port.
-datadir {...}: Specify an alternative path for game data (separated by ver.).
-cfgdir {...}: Specify an alternative path for new cfg files (not old CONFIG).
-fulllauncher: Show a fullscreen launcher window.
-softlauncher: Show a software-rendered launcher window (not fullscreen).

Te following argument is specific to the Catacomb Adventure Series:

-skipintro: Skip what is found in the original intro EXE and start game.

Note: The path passed to -datadir or -cfgdir is assumed to exist.

------------------------------------------------------------
Building the ports from the source codes - Linux executables
------------------------------------------------------------

Prerequisites:

- Development files for SDL 2.0.2+ are required, as well as GNU make, the
GNU Compiler Collection's C compiler (i.e. gcc), and development files for one
of select supported libraries used for resampling (list is given below).
In practice, at the moment only SDL 2.0 dev files, "make" and "gcc" are
(probably) required.
- Note that SDL 2.0.4 or later is recommended, due to fixes to mouse cursor
handling in windowed mode, especially when it comes to relative mouse motion.

Building EXEs - the basics:

- Before building a new EXE (or more than one, say for all ports), it is
recommended for you to enter "make clean", in order to remove any
object/executable file possibly built beforehand.
- If nothing goes wrong, and you don't mind giving up
resampling capabilities, then a single "make RESAMPLER=NONE" command
should build various executables, each of them supporting a different game.

Using a resampling library:

- One of seven resampling libraries can be selected, by setting the appropriate
RESAMPLER variable for "make". As in the case of SDL 2.0, you'll need the
appropriate development files.
Currently supported libraries for resampling: LIBSWRESAMPLE, LIBAVRESAMPLE,
LIBAVCODEC (its resampling API was deprecated for LIBAVRESAMPLE), LIBRESAMPLE,
LIBSOXR, LIBSPEEXDSP and LIBSAMPLERATE. As stated above, NONE is also an
option if you don't want to add any dependency on a resampling library.
- Currently RESAMPLER=LIBSPEEXDSP is the default choice.

Building just one EXE:

- It is possible to build just the Keen Dreams executable, by typing "make"
while in the "src/kdreams" subtree (you can still pass e.g., RESAMPLER=NONE).
- Similarly, to build just Catacomb 3-D, you can type "make" while in the
"src/id91_11/cat_all/cat3d" subtree.
For The Catacomb Abyss, it is "src/id91_11/cat_all/catabyss", and there are
two more such subdirectories for The Catacomb Armageddon and Apocalypse.

--------------------------------------------------------------
Building the ports from the source codes - Windows executables
--------------------------------------------------------------

Although untested, MinGW can be used for this, and then you should
be able to follow the above instructions for building Linux executables
with (virtually) no change.


If you try to cross-compile from a Linux environment, then you may wish
to set PLATFORM=WINDOWS and BINPREFIX accordingly.

-----------------------------------------------------------
Building the ports from the source codes - Android packages
-----------------------------------------------------------

This was tested on a Linux environment, although the instructions should
probably apply on other platforms as well.

Prerequisites (includes some preparation):

- In terms of development tools, a little mix of old and new was used.
The old Android NDK version r8d was used, along with Android SDK Tools 25.2.2,
Android SDK Platform-tools 24.0.3 and Android SDK Build-tools 19.1.
The Target Android API level is currently 24, while the minimum is 10.
The minimum of 10 is also used for the native code.
A compatible Java 8 installation is required. On the other hand, "ant" is
still used to build the Java projects and create the APK packages,
rather than "gradle".
- Within the SDK directory, you should look for the file android-support-v4.jar
as found under {sdk-dir}/extras/android/support/v7/appcompat/libs, and store
a copy of it within the "src/android-lib/libs" directory in the refkeen tree.
You may have to create the "libs" subdirectory beforehand.
- Do NOT copy the android-support-v4.jar file from
{sdk-dir}/extras/android/support/v4 or any other location.
- Full SDL 2.0.2+ sources are required. SDL 2.0.5 pre-release is recommended.
- In the refkeen source tree, the directory src/android-lib/jni should have
a copy of (or a symlink to) the SDL2 sources, named "SDL".

Preparing a resampling library:

- One of three resampling libraries can be selected. As in the case of SDL 2.0,
you'll need full sources. A list of known compatible versions is given below.
Currently supported libraries for resampling: LIBSAMPLERATE, LIBSOXR and
LIBSPEEXDSP. NONE is also an option if you don't want to add
any dependency on a resampling library.
- Currently RESAMPLER=LIBSPEEXDSP is the default choice.
- If a resampling library is used, then src/android-lib/jni should contain
a copy of (or a symlink to) the corresponding resampling library
sources, using one of these names for the directory/symlink
(depending on the library in use): samplerate, soxr, speexdsp.

Building native library code:

- To build the native SDL2 code, and (optionally) a resampling library,
enter the src/android-lib directory inside a compatible shell (e.g., "bash")
and then type /path/to/ndk-build clean, followed by /path/to/ndk-build.
You may optionally pass arguments like -j #n.

Building native game code:

- The native code of each game shall be built separately. For instance, in
the case of Keen Dreams, enter the src/kdreams/android-project directory.
- You can then use ndk-build in src/kdreams/android-project to build the
native Reflection Keen Dreams code as usual. Again, though, it's safer
to run "ndk-build clean" first. You can also set the RESAMPLER variable if you
want to use a different resampling library (or disable resampling altogether).
Example: ndk-build RESAMPLER=LIBSOXR

Building the Java code and creating an APK file (assuming Keen Dreams only):

- Create a new local.properties file at src/android-lib, and fill it with
the path to the SDK, like this: sdk.dir=/path/to/adt-bundle-linux-x86_64/sdk
- For Keen Dreams, copy src/android-lib/local.properties
into src/kdreams/android-project.
- While under src/kdreams/android-project, type "ant clean" and then "ant".
If there is no unexpected issue, you should get a RefkeenActivity-debug.apk
package in src/kdreams/android-project/bin.

Installing the APK package:

- If you have a compatible Android device connected via USB,
*and* USB debugging is enabled, then you may be able
to install the app by typing "ant installd".

Supported versions/archives of resampling libraries: soxr-0.1.2.Source.tar.xz,
speexdsp-SpeexDSP-1.2rc3.tar.gz, libsamplerate-0.1.8.tar.gz.

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

As of October 12 2016, these can be found here:
https://bitbucket.org/NY00123/gamesrc-ver-recreation/

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

* sulix, for a quite useful hint about memory wrapping for Keen Dreams, and
having earlier experience with him in general. Also done some research works on
at least a couple of Keen games, and worked on the Handmade Penguin tutorial.
* adurdin, quantumg, keenrush, levellord, levellass and lemm for their
differing research works on Keen games.
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

* doomjedi for his nice application icon used by Reflection Keen
(technically a few minor edits are in use).

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

Oct 12, 2016 (v0.16.0):

- BREAKING CHANGE: The file HELP.TXT is now required in order to
play The Catacomb Abyss.
- Regarding The Catacomb Abyss v1.13, the expected/extracted EGAHEAD.ABS and
MTEMP.TMP files should be a little bit smaller. Previously, whole segments
would be extracted from the CATABYSS.EXE file, leading to a little
bit larger files (with filesizes divisible by 16 bytes).
- Added support for multi-touch input, along with an Android port;
Currently requires Android 2.3.3 (API level 10) or later and targets 7.0 (24).
- Some "modern" game controller input adjustments, including changes to the
handling of debug keys.
- Some launcher adjustments; Examples: The ability to set (emulated)
command-line arguments for the game, showing/hiding an on-screen keyboard for
text search/input, access to details about files from supported game versions.
- "autolock" setting was replaced with the "mousegrab" setting, which
behaves somewhat more (but not exactly) like the mouse grab setting
from Chocolate Doom.
- Added a "absmousemotion" setting, optionally letting one seamlessly move
the mouse cursor out of the window in Keen Dreams' control panel. This is
disabled by default, since it technically behaves a bit differently
from vanilla Keen Dreams.
- If multi-touch input is used, and the /NOMOUSE option is *not* passed
to Keen Dreams, then the mouse cursor in the control panel can be used
with a single finger, in a matter similar to enabling "absmousemotion" for
an actual mouse. That is, the ship cursor (more precisely, its top-left corner)
is shown where the finger resides.
- Big Endian fixes (including fixed sound support).
- Misc. graphics output optimizations. This includes changes to emulated EGA
memory layout; Using one byte per pixel, rather than four monochromatic
sections for the four EGA planes. Reason is this is better for the Catacombs,
and it also didn't seem to make things worse for Keen Dreams.
- Screen is refreshed more often, even if there's no actual on-screen update.
This is done for somewhat better Steam Overlay support and other situations.
- Minor DBOPL fixes.
- Support better resampling of audio (from the OPL rate of 49716Hz to a
different rate), using a resampling library. Currently SpeexDSP is used.
- Fix some unaligned memory accesses, as well as a few buffer overflows.
- Fix duplicated game controllers on startup, in launcher.
- Other code modifications.

Nov 20, 2015 (v0.13.0):

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
- The EXE filenames are also longer, just to reduce the chances of weird
conflicts. Names without file extension: reflection-kdreams, reflection-cat3d,
reflection-catabyss, reflection-catarm and reflection-catapoc.
- The cfg files were renamed in a similar manner (for the sake of consistency).
This may technically look like a breaking change on its own, only the cfg files
and more should be relocated to different directories anyway.
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
