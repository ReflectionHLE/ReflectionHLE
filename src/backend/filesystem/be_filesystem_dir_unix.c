#include <dirent.h>
#include "be_filesystem_dir.h"

BE_DIR_T BEL_Cross_OpenDir(const char *dirname) { return opendir(dirname); }

char *BEL_Cross_ReadDir(BE_DIR_T dir)
{
	struct dirent *direntry = readdir((DIR *)dir);
	return direntry ? direntry->d_name : NULL;
}

void BEL_Cross_RewindDir(BE_DIR_T dir) { rewinddir((DIR *)dir); }

void BEL_Cross_CloseDir(BE_DIR_T dir) { closedir((DIR *)dir); }
