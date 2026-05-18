#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool Unlzexe_unpack(FILE *ifile, unsigned char *obuff, int buffsize,
                    uint16_t *oextramempara);
