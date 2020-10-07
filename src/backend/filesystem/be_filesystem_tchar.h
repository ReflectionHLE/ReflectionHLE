#ifndef BE_FILESYSTEM_CHAR_H
#define BE_FILESYSTEM_CHAR_H

#include "refkeen_config.h"

#ifdef REFKEEN_PLATFORM_WINDOWS

#include "tchar.h"

#else

#define _T(x) x
#define TCHAR char
#define _tcslen strlen
#define _tcscmp strcmp
#define _tfopen fopen
#define _tremove remove
#define _tmkdir mkdir

#endif

#endif
