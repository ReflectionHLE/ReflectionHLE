#ifndef BE_FILESYSTEM_DIR_H
#define BE_FILESYSTEM_DIR_H

#include "refkeen.h"

typedef void *BE_DIR_T;

#ifdef REFKEEN_PLATFORM_WINDOWS

BE_DIR_T BEL_Cross_OpenDir(const wchar_t *dirname);
// Returns a pointer to the following directory entry's filename if any
// is found, or a NULL pointer otherwise. This pointer should be assumed
// to get invalidated after any other function call involving dir,
// but it is valid to modify the string's contents beforehand.
wchar_t *BEL_Cross_ReadDir(BE_DIR_T dir);

#else

BE_DIR_T BEL_Cross_OpenDir(const char *dirname);
// Returns a pointer to the following directory entry's filename if any
// is found, or a NULL pointer otherwise. This pointer should be assumed
// to get invalidated after any other function call involving dir, but
// but it is valid to modify the string's contents beforehand.
char *BEL_Cross_ReadDir(BE_DIR_T dir);

#endif

void BEL_Cross_RewindDir(BE_DIR_T dir);
void BEL_Cross_CloseDir(BE_DIR_T dir);

#endif // BE_FILESYSTEM_DIR_H
