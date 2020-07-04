/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2014-2020 NY00123
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
//#include <fcntl.h>
//#include <IO.H>
//#include <MEM.H>
//#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>

#include "refkeen.h"

#include <stdint.h>
#include <stdbool.h>

#define __ID_GLOB__

#define	EXTENSION	"KDR"

#include "graphkdr.h"
#include "audiokdr.h"

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

// REFKEEN - Change GRMODE from a macro to a variable
// TODO - Defined below so we have id0_int_t (even though we could've used int...)
//extern id0_int_t GRMODE;
//#define GRMODE	CGAGR
//#define GRMODE	EGAGR

// REFKEEN - Unused since transforming GRMODE into a variable
#if 0
#if GRMODE == EGAGR
#define GREXT	"EGA"
#endif
#if GRMODE == CGAGR
#define GREXT	"CGA"
#endif
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
// (and Keen Dreams' loadscn2)
//
// BUT, to reduce the amount of compilation warnings in C and errors in C++,
// we still use plain char here, and fix Keen Dreams' VWL_MeasureString instead
typedef char id0_char_t;
//typedef uint8_t id0_char_t;

typedef int8_t id0_signed_char_t;
typedef uint8_t id0_unsigned_char_t;
typedef int16_t id0_short_t; // Possibly used in kd_main.c and external decompression routines for Keen Dreams
typedef int16_t id0_int_t;
typedef uint16_t id0_unsigned_t;
typedef uint16_t id0_unsigned_int_t; // Found in shareware release v1.13
typedef int32_t id0_long_t;
typedef uint32_t id0_unsigned_long_t;

typedef uint8_t id0_byte_t;
typedef uint16_t id0_word_t;
typedef uint32_t id0_longword_t;
// REFKEEN - Used just in Keen Dreams' SD_StartMusic, but there's no music
// in the original game and it was commented that Ptr shouldn't be used...
//typedef uint8_t * id0_ptr_t;

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

// TODO (REFKEEN): These should really be removed, but just for now and to document...
#define id0_far
#define id0_huge
#define id0_seg

#endif

// REFKEEN - Change GRMODE from a macro to a variable
extern id0_int_t GRMODE;

// REFKEEN - New variable, used as an alternative to refkeen_current_gamever
// on various occassions (e.g., behaviors common to versions 1.00 and 1.05)
extern int current_gamever_int;

#include "id_mm.h"
#include "id_ca.h"
#include "id_vw.h"
#include "id_rf.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
