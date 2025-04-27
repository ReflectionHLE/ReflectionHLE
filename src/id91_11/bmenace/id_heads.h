/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 * Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
 *
 * Copyright (C) 2014-2025 NY00123
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// ID_GLOB.H


//#include <ALLOC.H>
#include <ctype.h>
//#include <DOS.H>
#include <errno.h>
//#include <FCNTL.H>
//#include <IO.H>
//#include <MEM.H>
//#include <PROCESS.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <SYS\STAT.H>
#include <time.h>

#include "refkeen.h"

#include <stdint.h>
#include <stdbool.h>

#define __ID_GLOB__

//--------------------------------------------------------------------------

//EXTENSION is defined in VERSION.H, which is included by GFXE_BM.H and AUDIOBM.H
//#define	EXTENSION	"C3D"

// REFKEEN moved to bottom (where id0_char_t and more are defined)
// - also modified type for different platforms
//extern	id0_char_t id0_far introscn;

#include "gfxe_bm.h"
#include "audiobm.h"

REFKEEN_NS_B

//--------------------------------------------------------------------------

#define BIOMENACE	//this used to be CAT3D

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

#define GRMODE	EGAGR

#if GRMODE == EGAGR
#define GREXT	"EGA"
#endif
#if GRMODE == CGAGR
#define GREXT	"CGA"
#endif

//#define PROFILE

//
//	ID Engine
//	Types.h - Generic types, #defines, etc.
//	v1.0d1
//

#ifndef	__TYPES__
#define	__TYPES__

// REFKEEN - Define these for a bit more consistent style
#define id0_argc g_be_argc
#define id0_argv g_be_argv

typedef	bool id0_boolean_t;

// WARNING:
//
// Should be UNSIGNED for VWL_MeasureString (high scores table) in Keen Dreams
// but SIGNED in Catacomb Abyss for which casting is done in VWL_MeasureString
// (and Keen Dreams' loadscn2). Should also be signed in Catacomb Abyss and
// Armageddon's function SpawnWallSkeleton, for the arrays xofs and yofs.
//
// BUT, to reduce the amount of compilation warnings in C and errors in C++,
// we still use plain char here, and fix Keen Dreams' VWL_MeasureString
// and the Catacombs' SpawnWallSkeleton instead.
typedef char id0_char_t;
//typedef int8_t id0_char_t;

typedef int8_t id0_signed_char_t;
typedef uint8_t id0_unsigned_char_t;
typedef int16_t id0_int_t;
typedef uint16_t id0_unsigned_t;
typedef int32_t id0_long_t;
typedef int32_t id0_signed_long_t;
typedef uint32_t id0_unsigned_long_t;

typedef uint8_t id0_byte_t;
typedef uint16_t id0_word_t;
typedef uint32_t id0_longword_t;
//typedef uint8_t * Ptr;

typedef	struct
		{
			id0_int_t	x,y;
		} Point;
typedef	struct
		{
			Point	ul,lr;
		} Rect;

#ifdef __cplusplus
#define	id0_nil_t	NULL
#else
#define	id0_nil_t	((void *)0)
#endif

// REFKEEN - Used for documentation
#define id0_far
#define id0_huge
#define id0_seg

#endif

extern id0_char_t *introscn;

REFKEEN_NS_E

#include "id_mm.h"
#include "id_ca.h"
#include "id_vw.h"
#include "id_rf.h"	//added refresh manager back in for BioMenace
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "altcontroller.h" // REFKEEN: New addition

REFKEEN_NS_B

void	Quit (const id0_char_t *error);		// defined in user program

REFKEEN_NS_E
