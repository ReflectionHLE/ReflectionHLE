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

#ifndef BE_CROSS_MEM_INTERNAL_H
#define BE_CROSS_MEM_INTERNAL_H

#include "be_cross.h"

// A gap between the near and far heaps
#define EMULATED_GAP_BETWEEN_HEAPS_PARAGRAPHS 103
// Lengths in paragraphs of the different sections
#define EMULATED_FIRST_PARAGRAPHS 4096
#define EMULATED_NEAR_PARAGRAPHS 213
#define EMULATED_FAR_PARAGRAPHS 28222
#define EMULATED_EMS_PARAGRAPHS 4096

#define EMULATED_CONVENTIONAL_SIZE (16 * (EMULATED_FIRST_PARAGRAPHS + EMULATED_NEAR_PARAGRAPHS + EMULATED_GAP_BETWEEN_HEAPS_PARAGRAPHS + EMULATED_FAR_PARAGRAPHS))
#define EMULATED_EMS_SIZE (16 * (EMULATED_EMS_PARAGRAPHS))

extern uint8_t g_be_emulatedMemSpace[EMULATED_CONVENTIONAL_SIZE + EMULATED_EMS_SIZE];

#endif // BE_CROSS_MEM_INTERNAL_H
