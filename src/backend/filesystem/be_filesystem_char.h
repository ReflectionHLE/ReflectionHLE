#ifndef BE_FILESYSTEM_CHAR_H
#define BE_FILESYSTEM_CHAR_H

#include "refkeen_config.h"

#ifdef REFKEEN_PLATFORM_WINDOWS

#define _T(x) L##x
#define TCHAR wchar_t
#define _tcslen wcslen
#define _tcscmp wcscmp
#define _tfopen _wfopen
#define _tremove _wremove
#define _tmkdir _wmkdir

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
