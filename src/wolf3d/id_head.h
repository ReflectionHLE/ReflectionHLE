// ID_HEAD.H


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

void	Quit (id0_char_t *error);		// defined in user program

