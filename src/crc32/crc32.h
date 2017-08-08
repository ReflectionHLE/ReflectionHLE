#ifndef CRC32_H
#define CRC32_H

#include <stdio.h>
#include <stdint.h>

int Crc32_ComputeFile( FILE *file, uint32_t *outCrc32 );

/*----------------------------------------------------------------------------*\
 * Note: Pass 0 as the value of inCrc32. For more details see crc32.c.
\*----------------------------------------------------------------------------*/
uint32_t Crc32_ComputeBuf( uint32_t inCrc32, const void *buf,
                                       size_t bufLen );

#endif
