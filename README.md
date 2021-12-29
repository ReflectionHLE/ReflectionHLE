# ReflectionHLE

ReflectionHLE, known for years as Reflection Keen, is a project consisting
of source ports of DOS games, all being inspired by the Chocolate Doom port.
These ports use a common codebase, consisting of the ReflectionHLE backend
and third-party code.

The source ports in question are:
- Reflection Keen Dreams (KDreams), a Keen Dreams port.
- Reflection Catacomb 3-D (Cat3D), which includes ports of
Catacomb 3-D (The Descent) and The Catacomb Adventure Series.
- Reflection Wolfenstein 3D (Wolf3D), covering ports of Wolfenstein 3D,
Spear of Destiny and Super 3-D Noah's Ark (DOS version).

## Terms of use

All corresponding license documents should be found under LICENSES.

Since the covered ports are based on multiple codebases with
varying licenses, the terms are described for each source port
as a whole, as well as the differing components.

Note that these terms do not cover any original game data,
which should be obtained separately.

The terms applying to the source ports (i.e., the executables):

- Reflection KDreams, Reflection Cat3D and Reflection Wolf3D
are all released under the GNU GPLv2+. See gpl-2.0.txt for more details.

The terms applying to each component separately:

- The ported Keen Dreams, Catacomb 3-D, The Catacomb Adventures Series and
Wolfenstein 3D codebases are released under the GNU GPLv2+. See gpl-2.0.txt.

- The ported Wolfenstein 3D code is also available under the original
terms for the Wolfenstein 3D sources from 1995. See id-wolf3d.txt.

- The ReflectionHLE backend is released under the 3-Clause BSD License.
See bsd-3-clause-template.txt for a general license template. For the exact
contents you should consult the top of any relevant file from the source code,
although the template's form was originally used as a base.

- Nuked OPL3 is released under the GNU LGPLv2.1+. See lgpl-2.1.txt.

- The included depklite implementation, a modification of ExeUnpacker,
is released under the MIT License. See mit-opentesarena.txt for details.

- The included CRC-32 implementation is in the public domain. The same
applies to the modified unlzexe source (based on UNLZEXE.DOC from v0.4).

## How to run a game

Note that ReflectionHLE does not include game data, which you'll have to
obtain separately.

Depending on the environment (e.g., operating system) in which ReflectionHLE
is used, you might be able to use the ReflectionHLE launcher and/or
a command-line interface. Even if you prefer to skip the launcher,
it may assist you when you want to tell ReflectionHLE where
to locate compatible game data.

ReflectionHLE may autodetect existing game installations
in specific locations which are internally scanned. You can
also use the launcher to add other locations with game installations.

### Using an application icon or executable

If you start ReflectionHLE by pressing on an application icon or executable,
the ReflectionHLE launcher should appear. Here, you can see which
game versions are supported, and which game files are
required for each such version.

In addition to autodetected game installations, you can use the launcher
to select a directory with compatible game data. Once it's confirmed
such data is found, it'll be remembered for later uses.

Note that if you use an external launcher or shortcut which adds additional
command-line arguments, this may change the behaviors. More details are
given under following instructions for using a command-line interface.

### Using a command-line interface

You can also start ReflectionHLE from a command-line interface,
like Windows' Command Prompt. Doing so without passing additional arguments
should generally be the same as using an application icon, albeit there
can be differences that vary by the environment.

If you pass at least one additional argument, a game can be started without
going through the launcher, unless you use -fulllauncher or -softlauncher.

If -passorigargs is used, the arguments that follow it are passed
to the game as-is. The exact meanings of them can vary by the game.

You can use -gamever <VER> for selecting a specific game version.
To show a list of supported game versions, use -listgamevers
as the only command-line argument.

To read about more options, you can use -? as the sole command-line argument.

### Skipping the launcher

If you simply want to start a game while skipping the launcher,
you can use the command-line arguments -gamever <VER>,
or alternatively, use -passorigargs with no other argument.

Note that you should first make sure that ReflectionHLE knows where to
find compatible game data. See above for locating data via the launcher.

## How to build from the source code

See COMPILING.md.

## Acknowledgements

See AUTHORS.md.
