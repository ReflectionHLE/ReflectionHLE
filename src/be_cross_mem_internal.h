/* Copyright (C) 2014-2021 NY00123
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
