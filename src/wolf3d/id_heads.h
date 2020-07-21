/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020 NY00123
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

// ID_GLOB.H


//#include <ALLOC.H>
#include <ctype.h>
//#include <DOS.H>
#include <errno.h>
//#include <fcntl.h>
//#include <IO.H>
//#include <MEM.H>
//#include <PROCESS.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <VALUES.H>
//#include <DIR.H>

#include <stdint.h>
#include <stdbool.h>

#define __ID_GLOB__
#include "version.h"

//--------------------------------------------------------------------------

// *** ALPHA RESTORATION ***
// Originally defined in even earlier revisions (Keen Dreams, Catacomb 3-D)
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define EXTENSION	"WL1"
#endif

// REFKEEN moved to bottom (where id0_char_t and more are defined)
// - also modified type for different platforms
//extern	id0_char_t		id0_far signon;

#define	introscn	signon

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#include "gfxv_n3d.h"
#include "audion3d.h"
//#include "mapsn3d.h" // Unused anyway
#elif (defined JAPAN)
//#ifdef JAPAN
#ifdef JAPDEMO
#include "foreign/japan/gfxv_wj1.h"
#else
#include "foreign/japan/gfxv_wj6.h"
#endif
#include "audiowl6.h"
#include "mapswl6.h"
#else

#ifndef SPEAR

// *** SHAREWARE/REGISTERED V1.4 APOGEE RESTORATION ***
// Include GFXV_APO.H instead of GFXV_WL6.H
#ifdef GOODTIMES
#include "gfxv_wl6.h"
#else
#include "gfxv_apo.h"
#endif
#include "audiowl6.h"
#include "mapswl6.h"

#else

#ifndef SPEARDEMO
#include "gfxv_sod.h"
#include "audiosod.h"
#include "mapssod.h"
#else
#include "gfxv_sdm.h"
#include "audiosdm.h"
#include "mapssdm.h"
#endif

#endif
#endif
//-----------------


#define GREXT	"VGA"

//
//	ID Engine
//	Types.h - Generic types, #defines, etc.
//	v1.0d1
//

// REFKEEN - Define these for a bit more consistent style
#define id0_argc g_be_argc
#define id0_argv g_be_argv

typedef bool id0_boolean_t;

// WARNING:
//
// Should be UNSIGNED for VWL_MeasureString (high scores table) in Keen Dreams
// but SIGNED in Catacomb Abyss for which casting is done in VWL_MeasureString
// (and Keen Dreams' loadscn2)
//
// BUT, to reduce the amount of compilation warnings in C and errors in C++,
// we still use plain char here, and fix Keen Dreams' VWL_MeasureString instead
typedef char id0_char_t;

typedef uint8_t id0_unsigned_char_t;

typedef int16_t id0_int_t;
typedef uint16_t id0_unsigned_t;
typedef int32_t id0_long_t;
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

extern id0_char_t *signon;

#include "id_mm.h"
#include "id_pm.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"


void	Quit (id0_char_t *error);		// defined in user program

//
// replacing refresh manager with custom routines
//

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define	PORTTILESWIDE		22
#define	PORTTILESHIGH		14
#else
#define	PORTTILESWIDE		20      // all drawing takes place inside a
#define	PORTTILESHIGH		13		// non displayed port of this size
#endif

#define UPDATEWIDE			PORTTILESWIDE
#define UPDATEHIGH			PORTTILESHIGH

#define	MAXTICS				10
#define DEMOTICS			4

#define	UPDATETERMINATE	0x0301

extern	id0_unsigned_t	mapwidth,mapheight,tics;
extern	id0_boolean_t		compatability;

extern	id0_byte_t		*updateptr;
extern	id0_unsigned_t	uwidthtable[UPDATEHIGH];
extern	id0_unsigned_t	blockstarts[UPDATEWIDE*UPDATEHIGH];

extern	id0_byte_t		fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

