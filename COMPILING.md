# Building from the source code

## Supported platforms differing from Android

You will need CMake, a compatible C or C++ compiler and SDL3.
Additionally, SpeexDSP is required for audio resampling.

While it should be possible to build the code as C,
you'll have to build more separate executables,
due to the lack of C++ namespaces.

This was tested with GCC for Linux and Windows builds,
the latter of which being done with Mingw-w64. It was
further tested with Clang and LLVM for Linux.

## Android

This was only tested with building from Linux, but chances are
the instructions should be mostly similar for other host platforms.

Right now, you won't be able to use SpeexDSP for audio resampling.
You'll need CMake and a copy of the SDL3 source tree.
As of writing this, SDL 3.2.30 was found to be compatible.

See SDL3's docs for requirements related to the Android environment,
like supported SDK and NDK versions.

src/android-project/reflectionhle/jni needs either a directory named "SDL"
with the SDL3 sources, or a symbolic link (symlink)
named "SDL" and pointing at such a directory.

It should also have a symlink named "ReflectionHLE", pointing at the
ReflectionHLE source tree (outside of the "src" directory). This can
be a proper directory if it has a full copy of all required files,
outside of android-project itself.

src/android-project/reflectionhle/src/main/java should have a copy of
the Java source tree for the SDL Activity, including (but not limited to)
org/libsdl/app/SDLActivity.java and all subdirectories (like org/libsdl/app).

If setup as expected, you can run gradlew from the android-project dir,
say with assembleRelease, assembleDebug, installRelease or installDebug
as a command-line argument.

You should first tell the process started by gradlew where to locate the
Android SDK, say by setting an environment variable named `ANDROID_HOME`.
Exact details will depend on the SDK files used for development.
