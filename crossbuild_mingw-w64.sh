#!/bin/sh

if [ -z "$SDLCONFIGBIN" ]; then
    export SDLCONFIGBIN=sdl2-config
fi

#NOTE: This script was tested with an archive of SDL 2.0.3 ready for Mingw-w64
#(32-bit). It seemed to work since we didn't recompile the libraries.
if [ "$1" = "" ]; then
	echo
	echo "Usage:"
	echo "./crossbuild_mingw-w64.sh /path/to/SDL-mingw <Args for \"make\">"
	echo
	echo "Example:"
	echo "./crossbuild_mingw-w64.sh ~/dev/SDL-2.0.1 -j4"
	echo
else
	SDL_PATH=$1
	shift 1
	make BINPREFIX=i686-w64-mingw32- SDLCONFIG="$SDL_PATH/bin/$SDLCONFIGBIN --prefix=$SDL_PATH" PLATFORM=WINDOWS "$@"
fi
