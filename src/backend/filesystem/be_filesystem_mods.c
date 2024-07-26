#include "be_filesystem_len_bounds.h"
#include "be_filesystem_mods.h"
#include "be_filesystem_string_ops.h"

TCHAR g_be_modPath[BE_CROSS_PATH_LEN_BOUND];

void BE_Cross_SetModPath(const char *path)
{
	// FIXME: That doesn't properly convert UTF-8 on Windows
	BEL_Cross_safeandfastcstringcopytoctstring(g_be_modPath,
	                                           g_be_modPath + BE_Cross_ArrayLen(g_be_modPath),
	                                           path);
}
