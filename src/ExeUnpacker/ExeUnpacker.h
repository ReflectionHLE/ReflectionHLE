#ifndef EXE_UNPACKER_H
#define EXE_UNPACKER_H

#include <stdbool.h>
#include <stdio.h>

// EXE unpacker, adapted from OpenTESArena.
// Used for decompressing DOS executables compressed with PKLITE.

bool ExeUnpacker_unpack(FILE *fp, unsigned char *decompBuff, int buffsize);

#endif
