/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2025 NY00123
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

// ID_MM.H

#ifndef __ID_CA__

#define __ID_CA__

REFKEEN_NS_B

#define SAVENEARHEAP	0x400		// space to leave in data segment
#define SAVEFARHEAP		0			// space to leave in far heap

#define	BUFFERSIZE		0x1000		// miscelanious, allways available buffer

// *** APOGEE+FORMGEN VERSIONS RESTORATION ***
// Looks like less blocks were used for Shareware 1.2/1.4 Apogee
// and SOD FormGen versions, and even less in Shareware 1.1 Apogee.
// On the other hand, more were used in Shareware 1.0 Apogee.
#if (GAMEVER_WOLFREV > GV_WR_SODFG14A)
#define MAXBLOCKS		700
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define MAXBLOCKS		1200
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
#define MAXBLOCKS		500
#else
#define MAXBLOCKS		600
#endif


//--------

#define	EMS_INT			0x67

#define	EMS_STATUS		0x40
#define	EMS_GETFRAME	0x41
#define	EMS_GETPAGES	0x42
#define	EMS_ALLOCPAGES	0x43
#define	EMS_MAPPAGE		0x44
#define	EMS_FREEPAGES	0x45
#define	EMS_VERSION		0x46

//--------

#define	XMS_INT			0x2f
#define	XMS_CALL(v)		_AH = (v);\
						asm call [DWORD PTR XMSDriver]

#define	XMS_VERSION		0x00

#define	XMS_ALLOCHMA	0x01
#define	XMS_FREEHMA		0x02

#define	XMS_GENABLEA20	0x03
#define	XMS_GDISABLEA20	0x04
#define	XMS_LENABLEA20	0x05
#define	XMS_LDISABLEA20	0x06
#define	XMS_QUERYA20	0x07

#define	XMS_QUERYFREE	0x08
#define	XMS_ALLOC		0x09
#define	XMS_FREE		0x0A
#define	XMS_MOVE		0x0B
#define	XMS_LOCK		0x0C
#define	XMS_UNLOCK		0x0D
#define	XMS_GETINFO		0x0E
#define	XMS_RESIZE		0x0F

#define	XMS_ALLOCUMB	0x10
#define	XMS_FREEUMB		0x11

//==========================================================================

typedef void id0_seg * memptr;

typedef struct
{
	id0_long_t	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;

//==========================================================================

extern	mminfotype	mminfo;
extern	memptr		bufferseg;
extern	id0_boolean_t		mmerror;

extern	void		(* beforesort) (void);
extern	void		(* aftersort) (void);

//==========================================================================

void MM_Startup (void);
void MM_Shutdown (void);
void MM_MapEMS (void);

void MM_GetPtr (memptr *baseptr,id0_unsigned_long_t size);
void MM_FreePtr (memptr *baseptr);

void MM_SetPurge (memptr *baseptr, id0_int_t purge);
void MM_SetLock (memptr *baseptr, id0_boolean_t locked);
void MM_SortMem (void);

void MM_ShowMemory (void);

id0_long_t MM_UnusedMemory (void);
id0_long_t MM_TotalFree (void);

void MM_BombOnError (id0_boolean_t bomb);

void MML_UseSpace (id0_unsigned_t segstart, id0_unsigned_t seglength);

REFKEEN_NS_E

#endif
