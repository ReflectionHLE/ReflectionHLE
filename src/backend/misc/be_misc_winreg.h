#ifndef BE_MISC_WINREG_H
#define BE_MISC_WINREG_H

#include "../filesystem/be_filesystem_len_bounds.h"
#include "../filesystem/be_filesystem_tchar.h"

void BEL_Cross_Registry_TryGetPaths(
	const TCHAR *key, const TCHAR *value,
	TCHAR (*paths)[3][BE_CROSS_PATH_LEN_BOUND], int *count);
	
#endif
