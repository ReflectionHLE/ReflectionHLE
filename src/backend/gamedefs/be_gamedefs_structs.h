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

#ifndef BE_GAMEDEFS_STRUCTS_H
#define BE_GAMEDEFS_STRUCTS_H

#include "refkeen_config.h"
#include "../filesystem/be_filesystem_tchar.h"
#include "be_cross.h"
#include "be_features.h"
#include "be_gamever.h"

// Describes a file originally embedded somewhere (in an EXE file's image)
typedef struct {
	BE_GameFileDetails_T fileDetails;
	int offset; // Location of file/chunk in the EXE image
} BE_EmbeddedGameFileDetails_T;

typedef enum {
	BE_EXECOMPRESSION_NONE, BE_EXECOMPRESSION_LZEXE9X,
#ifdef ENABLE_PKLITE
	BE_EXECOMPRESSION_PKLITE105,
#endif
} BE_ExeCompression_T;

typedef struct {
	const BE_EmbeddedGameFileDetails_T *embeddedFiles;
	const char *subDescription; // NULL if there's nothing to add
	const char *exeNames; // NULL if all we want is a main function pointer
	void (*mainFuncPtr)(void);
	void (*embeddedFilesLoaderFuncPtr)(void);
	int decompExeImageSize;
	BE_ExeCompression_T compressionType;
	bool passArgsToMainFunc;
} BE_EXEFileDetails_T;

typedef struct {
	const BE_GameFileDetails_T *reqFiles;
	const BE_EXEFileDetails_T *exeFiles;
	const TCHAR *writableFilesDir;
	const char *customInstDescription;
	void (**patcherFuncPtrs)(void);
	int audioDeviceFlags;
	BE_GameVer_T verId;
} BE_GameVerDetails_T;

extern const BE_GameVerDetails_T *g_be_gamever_ptrs[];

#endif
