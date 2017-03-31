#ifndef DEPKLITE_H
#define DEPKLITE_H

#include <stdbool.h>
#include <stdio.h>

// depklite, a derivative of OpenTESArena's ExeUnpacker.
// Used for decompressing DOS executables compressed with PKLITE.

bool depklite_unpack(FILE *fp, unsigned char *decompBuff, int buffsize);

#endif
