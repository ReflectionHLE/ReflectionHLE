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
 
#ifndef BE_FILESYSTEM_FILE_OPS_H
#define BE_FILESYSTEM_FILE_OPS_H

#include "be_cross.h"
#include "be_filesystem_path_len_bound.h"
#include "be_filesystem_tchar.h"

typedef enum {
	BE_FILE_REQUEST_READ, BE_FILE_REQUEST_OVERWRITE, BE_FILE_REQUEST_DELETE
} BE_FileRequest_T;

// Attempts to apply a read, overwrite or delete request for a
// (possibly-existing) file from given directory, in a case-insensitive manner.
//
// OPTIONAL ARGUMENT (used internally): outfullpath, if not NULL, should
// point to an out buffer which is BE_CROSS_PATH_LEN_BOUND chars long.
BE_FILE_T BEL_Cross_apply_file_action_in_dir(
	const char *filename, BE_FileRequest_T request,
	const TCHAR *searchdir, TCHAR (*outfullpath)[BE_CROSS_PATH_LEN_BOUND]);
	
#endif
