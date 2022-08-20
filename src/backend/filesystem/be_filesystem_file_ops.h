/* Copyright (C) 2014-2022 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef BE_FILESYSTEM_FILE_OPS_H
#define BE_FILESYSTEM_FILE_OPS_H

#include "be_cross.h"
#include "be_filesystem_len_bounds.h"
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
