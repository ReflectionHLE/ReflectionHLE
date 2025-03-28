/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2016-2025 NY00123
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

// NEWMM.H

#ifndef __ID_MM__

#define __ID_MM__

#ifndef __TYPES__
#include "id_types.h"
#endif

void Quit (const id0_char_t *error);


//==========================================================================

#define SAVENEARHEAP	0x400		// space to leave in data segment
#define SAVEFARHEAP		0			// space to leave in far heap

#define	BUFFERSIZE		0x1000		// miscelanious, allways available buffer

extern id0_int_t	MAXBLOCKS; // REFKEEN: The macro was replaced with a var

//==========================================================================

typedef void id0_seg * memptr;

typedef struct
{
	id0_long_t	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;

//==========================================================================

extern	mminfotype	mminfo;
extern	memptr		bufferseg;
extern	id0_boolean_t		bombonerror;

extern	void		(* beforesort) (void);
extern	void		(* aftersort) (void);

//==========================================================================

void MM_Startup (void);
void MM_Shutdown (void);
//void MM_MapEMS (void);

void MM_GetPtr (memptr *baseptr,id0_unsigned_long_t size);
void MM_FreePtr (memptr *baseptr);

void MM_SetPurge (memptr *baseptr, id0_int_t purge);
void MM_SetLock (memptr *baseptr, id0_boolean_t locked);
void MM_SortMem (void);

void MM_ShowMemory (void);

id0_long_t MM_UnusedMemory (void);
id0_long_t MM_TotalFree (void);

// REFKEEN - New functions, used while swapping CGA/EGA graphics from the 2015 port
id0_unsigned_t MM_GetAttributes (memptr *baseptr);
void MM_SetAttributes (memptr *baseptr, id0_unsigned_t attributes);

#endif
