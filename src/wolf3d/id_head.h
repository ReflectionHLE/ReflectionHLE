/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2022 NY00123
 *
 * This file is part of Wolfenstein 3D.
 *
 * Wolfenstein 3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Wolfenstein 3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an addendum, you can further use the Wolfenstein 3D Source Code under
 * the terms of the limited use software license agreement for Wolfenstein 3D.
 * See id-wolf3d.txt for these terms.
 */

// ID_HEAD.H

#include "refkeen.h"

#define	EXTENSION	"WLF"
#define WOLF

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

#define GRMODE	VGAGR

// *** S3DNA RESTORATION ***
// Unused, but fpos_t is found in the debugging symbols for ID_VL.C, which is
// the only place from which ID_HEAD.H is included, and it's mentioned in
// STDIO.H only (possibly done while adding the FillCharAttr function)
#ifdef GAMEVER_NOAH3D
#include <stdio.h>
#endif
#include "version.h"

#include <stdint.h>
#include <stdbool.h>

REFKEEN_NS_B

// REFKEEN - Define these for a bit more consistent style
#define id0_argc g_be_argc
#define id0_argv g_be_argv

typedef bool id0_boolean_t;

// WARNING:
//
// Should be UNSIGNED for VWL_MeasureString (high scores table) in Keen Dreams
// but SIGNED in Catacomb Abyss for which casting is done in VWL_MeasureString
// (and Keen Dreams' loadscn2). Should also be signed in Wolfenstein 3D for
// the fields of struct atkinf.
//
// BUT, to reduce the amount of compilation warnings in C and errors in C++,
// we still use plain char here, and fix Keen Dreams' VWL_MeasureString
// and Wolfenstein 3D's atkinf instead.
typedef char id0_char_t;

typedef uint8_t id0_unsigned_char_t;

typedef int16_t id0_int_t;
typedef uint16_t id0_unsigned_t;

typedef uint8_t id0_byte_t;
//typedef uint16_t id0_word_t;
//typedef uint32_t id0_long_t;
//typedef uint8_t * Ptr;

typedef	struct
		{
			id0_int_t	x,y;
		} Point;

typedef	struct
		{
			Point	ul,lr;
		} Rect;

#define id0_far // REFKEEN - Use for documentation

void	Quit (const id0_char_t *error);		// defined in user program

REFKEEN_NS_E
