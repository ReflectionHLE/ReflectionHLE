#include "be_filesystem_len_bounds.h"
#include "be_filesystem_mods.h"
#include "be_filesystem_string_ops.h"
#ifdef REFKEEN_PLATFORM_WINDOWS
#include <windows.h>
#endif

TCHAR g_be_modPath[BE_CROSS_PATH_LEN_BOUND];

void BE_Cross_SetModPath(const char *path)
{
#ifdef REFKEEN_PLATFORM_WINDOWS
	if (MultiByteToWideChar(CP_UTF8, 0, path, -1, g_be_modPath,
	                         BE_Cross_ArrayLen(g_be_modPath)))
		g_be_modPath[BE_Cross_ArrayLen(g_be_modPath) - 1] = _T('\0');
	else
		g_be_modPath[0] = _T('\0');
#else
	BE_Cross_safeandfastcstringcopy(g_be_modPath,
	                                g_be_modPath + BE_Cross_ArrayLen(g_be_modPath),
	                                path);
#endif
}
