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

extern	char		far signon;

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

typedef uint8_t id0_byte_t;
typedef uint16_t id0_word_t;
typedef uint32_t id0_longword_t;
//typedef uint8_t * Ptr;

typedef	struct
		{
			int	x,y;
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

#include "id_mm.h"
#include "id_pm.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"


void	Quit (char *error);		// defined in user program

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

extern	unsigned	mapwidth,mapheight,tics;
extern	boolean		compatability;

extern	byte		*updateptr;
extern	unsigned	uwidthtable[UPDATEHIGH];
extern	unsigned	blockstarts[UPDATEWIDE*UPDATEHIGH];

extern	byte		fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

