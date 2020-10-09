/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#include "refkeen_config.h"

#include "../filesystem/be_filesystem_file_ops.h"
#include "../gamedefs/be_gamedefs_structs.h"
#include "be_cross.h"
#include "be_startup.h"
#include "be_st.h"
#include "crc32/crc32.h"

#define REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS 1

// Loads a file originally embedded into the EXE (for DOS) to a newly allocated
// chunk of memory. Should be freed with BE_Cross_free_mem_loaded_embedded_rsrc.
// Returns chunk size if successful, or a negative number in case of failure.
int BE_Cross_load_embedded_rsrc_to_mem(const char *filename, void **ptr)
{
	BE_FILE_T fp = BE_Cross_open_readonly_for_reading(filename);
	if (!fp)
		return -1;

	int filesize = BE_Cross_FileLengthFromHandle(fp);
	*ptr = malloc(filesize);
	bool success = (*ptr) && (fread(*ptr, filesize, 1, fp) == 1);
	if (!success)
		free(*ptr); // This is OK with NULL

	fclose(fp);
	return success ? filesize : -1;
}

// Frees file loaded using BE_Cross_load_embedded_rsrc_to_mem. Accepts a NULL pointer.
void BE_Cross_free_mem_loaded_embedded_rsrc(void *ptr)
{
	free(ptr);
}

void *BE_Cross_BmallocFromEmbeddedData(const char *name, uint16_t *pSize)
{
	const BE_EmbeddedGameFileDetails_T *embeddedFile = g_be_current_exeFileDetails->embeddedFiles;
	if (embeddedFile)
		for (; embeddedFile->fileDetails.filenames && BE_Cross_strcasecmp(name, embeddedFile->fileDetails.filenames); ++embeddedFile)
			;

	if (!embeddedFile || !(embeddedFile->fileDetails.filenames))
		BE_ST_ExitWithErrorMsg("BE_Cross_BmallocFromEmbeddedData: Unrecognized embedded data name!");

#ifdef REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS
	if (Crc32_ComputeBuf(0, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize) != embeddedFile->fileDetails.crc32)
		BE_ST_ExitWithErrorMsg("BE_Cross_BmallocFromEmbeddedData: Unexpectedly got the wrong CRC32!");
#endif

	void *ptr = BE_Cross_Bmalloc(embeddedFile->fileDetails.filesize);
	if (ptr)
	{
		memcpy(ptr, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize);
		if (pSize)
			*pSize = embeddedFile->fileDetails.filesize;
	}
	return ptr;
}

void *BE_Cross_BfarmallocFromEmbeddedData(const char *name, uint32_t *pSize)
{
	const BE_EmbeddedGameFileDetails_T *embeddedFile = g_be_current_exeFileDetails->embeddedFiles;
	if (embeddedFile)
		for (; embeddedFile->fileDetails.filenames && BE_Cross_strcasecmp(name, embeddedFile->fileDetails.filenames); ++embeddedFile)
			;

	if (!embeddedFile || !(embeddedFile->fileDetails.filenames))
		BE_ST_ExitWithErrorMsg("BE_Cross_BfarmallocFromEmbeddedData: Unrecognized embedded data name!");

#ifdef REFKEEN_ENABLE_EMBEDDED_FILES_CRC32_CHECKS
	if (Crc32_ComputeBuf(0, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize) != embeddedFile->fileDetails.crc32)
		BE_ST_ExitWithErrorMsg("BE_Cross_BfarmallocFromEmbeddedData: Unexpectedly got the wrong CRC32!");
#endif

	void *ptr = BE_Cross_Bfarmalloc(embeddedFile->fileDetails.filesize);
	if (ptr)
	{
		memcpy(ptr, g_be_current_exeImage + embeddedFile->offset, embeddedFile->fileDetails.filesize);
		if (pSize)
			*pSize = embeddedFile->fileDetails.filesize;
	}
	return ptr;
}
