#ifndef BE_FILESYSTEM_DIR_H
#define BE_FILESYSTEM_DIR_H

#include "refkeen_config.h"

#ifdef REFKEEN_PLATFORM_WINDOWS
#include <shlwapi.h>
#endif
#ifdef REFKEEN_PLATFORM_UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "be_filesystem_tchar.h"

typedef void *BE_DIR_T;

BE_DIR_T BEL_Cross_OpenDir(const TCHAR *dirname);
// Returns a pointer to the following directory entry's filename if any
// is found, or a NULL pointer otherwise. This pointer should be assumed
// to get invalidated after any other function call involving dir,
// but it is valid to modify the string's contents beforehand.
TCHAR *BEL_Cross_ReadDir(BE_DIR_T dir);

void BEL_Cross_RewindDir(BE_DIR_T dir);
void BEL_Cross_CloseDir(BE_DIR_T dir);

static inline bool BEL_Cross_IsDir(const TCHAR *path)
{
#ifdef REFKEEN_PLATFORM_WINDOWS
	return PathIsDirectoryW(path);
#endif
#ifdef REFKEEN_PLATFORM_UNIX
	struct stat info;
	return ((stat(path, &info) == 0) && S_ISDIR(info.st_mode));
#endif
}


#endif // BE_FILESYSTEM_DIR_H
