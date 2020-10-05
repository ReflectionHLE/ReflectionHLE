#ifndef BE_FILESYSTEM_DIR_H
#define BE_FILESYSTEM_DIR_H

#include "be_filesystem_char.h"

typedef void *BE_DIR_T;

BE_DIR_T BEL_Cross_OpenDir(const TCHAR *dirname);
// Returns a pointer to the following directory entry's filename if any
// is found, or a NULL pointer otherwise. This pointer should be assumed
// to get invalidated after any other function call involving dir,
// but it is valid to modify the string's contents beforehand.
TCHAR *BEL_Cross_ReadDir(BE_DIR_T dir);

void BEL_Cross_RewindDir(BE_DIR_T dir);
void BEL_Cross_CloseDir(BE_DIR_T dir);

#endif // BE_FILESYSTEM_DIR_H
