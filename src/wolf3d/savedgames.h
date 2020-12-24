#ifndef SAVEDGAMES_H
#define SAVEDGAMES_H

REFKEEN_NS_B

id0_long_t ChecksumInt16Buffer(const id0_word_t *a, int n, id0_long_t checksum);

id0_boolean_t SaveObject(BE_FILE_T file, const objtype *o);
id0_boolean_t LoadObject(BE_FILE_T file, objtype *o);

id0_long_t SaveAndChecksumGameState(BE_FILE_T file, const gametype *state, id0_long_t checksum);
id0_long_t LoadAndChecksumGameState(BE_FILE_T file, gametype *state, id0_long_t checksum);

id0_long_t SaveAndChecksumLevelRatios(BE_FILE_T file, const LRstruct *ratios, int n, id0_long_t checksum);
id0_long_t LoadAndChecksumLevelRatios(BE_FILE_T file, LRstruct *ratios, int n, id0_long_t checksum);

id0_long_t SaveAndChecksumStatObjects(BE_FILE_T file, const statobj_t *stats, int n, id0_long_t checksum);
id0_long_t LoadAndChecksumStatObjects(BE_FILE_T file, statobj_t *stats, int n, id0_long_t checksum);

id0_long_t SaveAndChecksumDoorObjects(BE_FILE_T file, const doorobj_t *doors, int n, id0_long_t checksum);
id0_long_t LoadAndChecksumDoorObjects(BE_FILE_T file, doorobj_t *doors, int n, id0_long_t checksum);

REFKEEN_NS_E

#endif
