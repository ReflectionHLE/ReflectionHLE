#ifndef CRC32_H
#define CRC32_H

#include <stdio.h>
#include <stdint.h>

int Crc32_ComputeFile( FILE *file, uint32_t *outCrc32 );

#endif
