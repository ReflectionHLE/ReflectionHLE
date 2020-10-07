#ifndef BE_FILESYSTEM_MKDIR_H
#define BE_FILESYSTEM_MKDIR_H

#include "be_filesystem_tchar.h"

#ifdef REFKEEN_PLATFORM_WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#endif

// WARNING: Do *not* assume this is recursive!!
static inline void BEL_Cross_mkdir(const TCHAR *path)
{
#ifdef REFKEEN_PLATFORM_WINDOWS
	_tmkdir(path);
#else
	_tmkdir(path, 0755);
#endif
}

#endif
