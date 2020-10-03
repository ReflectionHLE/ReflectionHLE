#include <windows.h>
#include "be_filesystem_dir.h"
#include "be_st.h"

typedef struct {
	wchar_t *regex;
	HANDLE handle;
	WIN32_FIND_DATAW data;
} BE_DIR_S;

BE_DIR_T BEL_Cross_OpenDir(const wchar_t *dirname)
{
	BE_DIR_S *dir = (BE_DIR_S *)malloc(sizeof(BE_DIR_S));
	if (!dir)
		BE_ST_ExitWithErrorMsg("BEL_Cross_OpenDir: Out of memory!");
	dir->regex = (wchar_t *)malloc(sizeof(wchar_t) * (wcslen(dirname) + wcslen(L"\\*") + 1));
	if (!dir->regex)
		BE_ST_ExitWithErrorMsg("BEL_Cross_OpenDir: Out of memory!");
	memcpy(dir->regex, dirname, sizeof(wchar_t) * (wcslen(dirname) + 1));
	wcscpy(dir->regex, dirname);
	wcscat(dir->regex, L"\\*");
	dir->handle = INVALID_HANDLE_VALUE;
	return dir;
}

wchar_t *BEL_Cross_ReadDir(BE_DIR_T dir)
{
	BOOL result;
	BE_DIR_S *d = (BE_DIR_S *)dir;
	if (d->handle != INVALID_HANDLE_VALUE)
		result = FindNextFileW(d->handle, &d->data);
	else
	{
		d->handle = FindFirstFileW(d->regex, &d->data);
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
