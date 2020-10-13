#include "refkeen_config.h" // Unicode
#include "be_filesystem_dir.h"
#include "be_st.h"

#include <windows.h>

typedef struct {
	TCHAR *regex;
	HANDLE handle;
	WIN32_FIND_DATA data;
} BE_DIR_S;

BE_DIR_T BEL_Cross_OpenDir(const TCHAR *dirname)
{
	BE_DIR_S *dir = (BE_DIR_S *)malloc(sizeof(BE_DIR_S));
	if (!dir)
		BE_ST_ExitWithErrorMsg("BEL_Cross_OpenDir: Out of memory!");
	dir->regex = (TCHAR *)malloc(sizeof(TCHAR) * (_tcslen(dirname) + _tcslen(_T("\\*")) + 1));
	if (!dir->regex)
		BE_ST_ExitWithErrorMsg("BEL_Cross_OpenDir: Out of memory!");
	memcpy(dir->regex, dirname, sizeof(TCHAR) * (_tcslen(dirname) + 1));
	_tcscpy(dir->regex, dirname);
	_tcscat(dir->regex, _T("\\*"));
	dir->handle = INVALID_HANDLE_VALUE;
	return dir;
}

TCHAR *BEL_Cross_ReadDir(BE_DIR_T dir)
{
	BOOL result;
	BE_DIR_S *d = (BE_DIR_S *)dir;
	if (d->handle != INVALID_HANDLE_VALUE)
		result = FindNextFile(d->handle, &d->data);
	else
	{
		d->handle = FindFirstFile(d->regex, &d->data);
		result = (d->handle != INVALID_HANDLE_VALUE);
	}
	return result ? d->data.cFileName : NULL;
}

void BEL_Cross_RewindDir(BE_DIR_T dir)
{
	BE_DIR_S *d = (BE_DIR_S *)dir;
	if (d->handle != INVALID_HANDLE_VALUE)
	{
		if (!FindClose(d->handle))
			// FIXME: Error code is missing
			BE_ST_ExitWithErrorMsg("BEL_Cross_RewindDir: FindClose failed!");
		d->handle = INVALID_HANDLE_VALUE;
	}
}

void BEL_Cross_CloseDir(BE_DIR_T dir)
{
	BE_DIR_S *d = (BE_DIR_S *)dir;
	if (d->handle != INVALID_HANDLE_VALUE)
		if (!FindClose(d->handle))
			// FIXME: Error code is missing
			BE_ST_ExitWithErrorMsg("BEL_Cross_CloseDir: FindClose failed!");
	free(d->regex);
	free(d);
}
