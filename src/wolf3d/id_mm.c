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

// NEWMM.C

/*
=============================================================================

		   ID software memory manager
		   --------------------------

Primary coder: John Carmack

RELIES ON
---------
Quit (char *error) function


WORK TO DO
----------
MM_SizePtr to change the size of a given pointer

Multiple purge levels utilized

EMS / XMS unmanaged routines

=============================================================================
*/

#include "id_heads.h"
//#pragma hdrstop

//#pragma warn -pro
//#pragma warn -use

REFKEEN_NS_B

/*
=============================================================================

							LOCAL INFO

=============================================================================
*/

#define LOCKBIT		0x80	// if set in attributes, block cannot be moved
#define PURGEBITS	3		// 0-3 level, 0= unpurgable, 3= purge first
#define PURGEMASK	0xfffc
#define BASEATTRIBUTES	0	// unlocked, non purgable

#define MAXUMBS		10

typedef struct mmblockstruct
{
	id0_unsigned_t	start,length;
	id0_unsigned_t	attributes;
	memptr		*useptr;	// pointer to the segment start
	struct mmblockstruct id0_far *next;
} mmblocktype;

/*
#define GETNEWBLOCK {if(!(mmnew=mmfree))Quit("MM_GETNEWBLOCK: No free blocks!")\
	;mmfree=mmfree->next;}
*/
#define GETNEWBLOCK {if(!mmfree)MML_ClearBlock();mmnew=mmfree;mmfree=mmfree->next;}

#define FREEBLOCK(x) {*x->useptr=NULL;x->next=mmfree;mmfree=x;}

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

mminfotype	mminfo;
memptr		bufferseg;
id0_boolean_t		mmerror;

void		(* beforesort) (void);
void		(* aftersort) (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

id0_boolean_t		mmstarted;

void id0_far	*farheap;
void		*nearheap;

mmblocktype	id0_far mmblocks[MAXBLOCKS]
			,id0_far *mmhead,id0_far *mmfree,id0_far *mmrover,id0_far *mmnew;

id0_boolean_t		bombonerror;

//id0_unsigned_t	totalEMSpages,freeEMSpages,EMSpageframe,EMSpagesmapped,EMShandle;

//void		(* XMSaddr) (void);		// far pointer to XMS driver

//id0_unsigned_t	numUMBs,UMBbase[MAXUMBS];

//==========================================================================

//
// local prototypes
//

//id0_boolean_t		MML_CheckForEMS (void);
//void 		MML_ShutdownEMS (void);
//void 		MM_MapEMS (void);
//id0_boolean_t 	MML_CheckForXMS (void);
//void 		MML_ShutdownXMS (void);
void		MML_UseSpace (id0_unsigned_t segstart, id0_unsigned_t seglength);
void 		MML_ClearBlock (void);

//==========================================================================

#if 0
/*
======================
=
= MML_CheckForXMS
=
= Check for XMM driver
=
=======================
*/

id0_boolean_t MML_CheckForXMS (void)
{
	numUMBs = 0;

asm {
	mov	ax,0x4300
	int	0x2f				// query status of installed diver
	cmp	al,0x80
	je	good
	}

	return false;
good:
	return true;
}


/*
======================
=
= MML_SetupXMS
=
= Try to allocate all upper memory block
=
=======================
*/

void MML_SetupXMS (void)
{
	id0_unsigned_t	base,size;

asm	{
	mov	ax,0x4310
	int	0x2f
	mov	[WORD PTR XMSaddr],bx
	mov	[WORD PTR XMSaddr+2],es		// function pointer to XMS driver
	}

getmemory:
asm	{
	mov	ah,XMS_ALLOCUMB
	mov	dx,0xffff					// try for largest block possible
	call	[DWORD PTR XMSaddr]
	or	ax,ax
	jnz	gotone

	cmp	bl,0xb0						// error: smaller UMB is available
	jne	done;

	mov	ah,XMS_ALLOCUMB
	call	[DWORD PTR XMSaddr]		// DX holds largest available UMB
	or	ax,ax
	jz	done						// another error...
	}

gotone:
asm	{
	mov	[base],bx
	mov	[size],dx
	}
	MML_UseSpace (base,size);
	mminfo.XMSmem += size*16;
	UMBbase[numUMBs] = base;
	numUMBs++;
	if (numUMBs < MAXUMBS)
		goto getmemory;

done:;
}


/*
======================
=
= MML_ShutdownXMS
=
======================
*/

void MML_ShutdownXMS (void)
{
	id0_int_t	i;
	id0_unsigned_t	base;

	for (i=0;i<numUMBs;i++)
	{
		base = UMBbase[i];

asm	mov	ah,XMS_FREEUMB
asm	mov	dx,[base]
asm	call	[DWORD PTR XMSaddr]
	}
}
#endif

//==========================================================================

/*
======================
=
= MML_UseSpace
=
= Marks a range of paragraphs as usable by the memory manager
= This is used to mark space for the near heap, far heap, ems page frame,
= and upper memory blocks
=
======================
*/

void MML_UseSpace (id0_unsigned_t segstart, id0_unsigned_t seglength)
{
	mmblocktype id0_far *scan,id0_far *last;
	id0_unsigned_t	oldend;
	id0_long_t		extra;

	scan = last = mmhead;
	mmrover = mmhead;		// reset rover to start of memory

//
// search for the block that contains the range of segments
//
	while (scan->start+scan->length < segstart)
	{
		last = scan;
		scan = scan->next;
	}

//
// take the given range out of the block
//
	oldend = scan->start + scan->length;
	extra = oldend - (segstart+seglength);
	if (extra < 0)
		Quit ("MML_UseSpace: Segment spans two blocks!");

	if (segstart == scan->start)
	{
		last->next = scan->next;			// unlink block
		FREEBLOCK(scan);
		scan = last;
	}
	else
		scan->length = segstart-scan->start;	// shorten block

	if (extra > 0)
	{
		GETNEWBLOCK;
		mmnew->useptr = NULL;

		mmnew->next = scan->next;
		scan->next = mmnew;
		mmnew->start = segstart+seglength;
		mmnew->length = extra;
		mmnew->attributes = LOCKBIT;
	}

}

//==========================================================================

/*
====================
=
= MML_ClearBlock
=
= We are out of blocks, so free a purgable block
=
====================
*/

void MML_ClearBlock (void)
{
	mmblocktype id0_far *scan/*,id0_far *last*/;

	scan = mmhead->next;

	while (scan)
	{
		if (!(scan->attributes&LOCKBIT) && (scan->attributes&PURGEBITS) )
		{
			MM_FreePtr(scan->useptr);
			return;
		}
		scan = scan->next;
	}

	Quit ("MM_ClearBlock: No purgable blocks!");
}


//==========================================================================

/*
===================
=
= MM_Startup
=
= Grabs all space from turbo with malloc/farmalloc
= Allocates bufferseg misc buffer
=
===================
*/

//static	id0_char_t *ParmStrings[] = {"noems","noxms",""}; // Unused var

void MM_Startup (void)
{
	id0_int_t i;
	id0_unsigned_long_t length;
	void id0_far 	*start;
	id0_unsigned_t 	segstart,seglength/*,endfree*/;

	if (mmstarted)
		MM_Shutdown ();


	mmstarted = true;
	bombonerror = true;
//
// set up the linked list (everything in the free list;
//
	mmhead = NULL;
	mmfree = &mmblocks[0];
	for (i=0;i<MAXBLOCKS-1;i++)
		mmblocks[i].next = &mmblocks[i+1];
	mmblocks[i].next = NULL;

//
// locked block of all memory until we punch out free space
//
	GETNEWBLOCK;
	mmhead = mmnew;				// this will allways be the first node
	mmnew->start = 0;
	mmnew->length = 0xffff;
	mmnew->attributes = LOCKBIT;
	mmnew->next = NULL;
	mmrover = mmhead;


//
// get all available near conventional memory segments
//
	length=BE_Cross_Bcoreleft();
	start = (void id0_far *)(nearheap = BE_Cross_Bmalloc(length));

	length -= 16-BE_Cross_GetPtrNormalizedOff(start); // REFKEEN - Offset is NORMALIZED, thus < 16
	length -= SAVENEARHEAP;
	seglength = length / 16;			// now in paragraphs
	segstart = BE_Cross_GetPtrNormalizedSeg(start)+(BE_Cross_GetPtrNormalizedOff(start)+15)/16;
#if 0
	length -= 16-(FP_OFF(start)&15);
	length -= SAVENEARHEAP;
	seglength = length / 16;			// now in paragraphs
	segstart = FP_SEG(start)+(FP_OFF(start)+15)/16;
#endif
	MML_UseSpace (segstart,seglength);
	mminfo.nearheap = length;

//
// get all available far conventional memory segments
//
	length=BE_Cross_Bfarcoreleft();
	start = farheap = BE_Cross_Bfarmalloc(length);

	length -= 16-BE_Cross_GetPtrNormalizedOff(start); // REFKEEN - Offset is NORMALIZED, thus < 16
	length -= SAVEFARHEAP;
	seglength = length / 16;			// now in paragraphs
	segstart = BE_Cross_GetPtrNormalizedSeg(start)+(BE_Cross_GetPtrNormalizedOff(start)+15)/16;
#if 0
	length -= 16-(FP_OFF(start)&15);
	length -= SAVEFARHEAP;
	seglength = length / 16;			// now in paragraphs
	segstart = FP_SEG(start)+(FP_OFF(start)+15)/16;
#endif
	MML_UseSpace (segstart,seglength);
	mminfo.farheap = length;
	mminfo.mainmem = mminfo.nearheap + mminfo.farheap;

//
// allocate the misc buffer
//
	mmrover = mmhead;		// start looking for space after low block

	MM_GetPtr (&bufferseg,BUFFERSIZE);
}

//==========================================================================

/*
====================
=
= MM_Shutdown
=
= Frees all conventional, EMS, and XMS allocated
=
====================
*/

void MM_Shutdown (void)
{
  if (!mmstarted)
	return;

  BE_Cross_Bfarfree (farheap);
  BE_Cross_Bfree (nearheap);
//  MML_ShutdownXMS ();
}

//==========================================================================

// REFKEEN: Move Wolf3D declaration out of function for namespaced C++ builds
id0_boolean_t SetViewSize(id0_unsigned_t width, id0_unsigned_t height);

/*
====================
=
= MM_GetPtr
=
= Allocates an unlocked, unpurgable block
=
====================
*/

void MM_GetPtr (memptr *baseptr,id0_unsigned_long_t size)
{
	mmblocktype id0_far *scan,id0_far *lastscan,id0_far *endscan
				,id0_far *purge,id0_far *next;
	id0_int_t			search;
	id0_unsigned_t	needed,startseg;

	needed = (size+15)/16;		// convert size from bytes to paragraphs

	GETNEWBLOCK;				// fill in start and next after a spot is found
	mmnew->length = needed;
	mmnew->useptr = baseptr;
	mmnew->attributes = BASEATTRIBUTES;

tryagain:
	for (search = 0; search<3; search++)
	{
	//
	// first search:	try to allocate right after the rover, then on up
	// second search: 	search from the head pointer up to the rover
	// third search:	compress memory, then scan from start
		if (search == 1 && mmrover == mmhead)
			search++;

		switch (search)
		{
		case 0:
			lastscan = mmrover;
			scan = mmrover->next;
			endscan = NULL;
			break;
		case 1:
			lastscan = mmhead;
			scan = mmhead->next;
			endscan = mmrover;
			break;
		case 2:
			MM_SortMem ();
			lastscan = mmhead;
			scan = mmhead->next;
			endscan = NULL;
			break;
		}

		startseg = lastscan->start + lastscan->length;

		while (scan != endscan)
		{
			if (scan->start - startseg >= needed)
			{
			//
			// got enough space between the end of lastscan and
			// the start of scan, so throw out anything in the middle
			// and allocate the new block
			//
				purge = lastscan->next;
				lastscan->next = mmnew;
				mmnew->start /*= *(id0_unsigned_t *)baseptr*/ = startseg;
				*baseptr = BE_Cross_BGetPtrFromSeg(startseg);
				mmnew->next = scan;
				while ( purge != scan)
				{	// free the purgable block
					next = purge->next;
					FREEBLOCK(purge);
					purge = next;		// purge another if not at scan
				}
				mmrover = mmnew;
				return;	// good allocation!
			}

			//
			// if this block is purge level zero or locked, skip past it
			//
			if ( (scan->attributes & LOCKBIT)
				|| !(scan->attributes & PURGEBITS) )
			{
				lastscan = scan;
				startseg = lastscan->start + lastscan->length;
			}


			scan=scan->next;		// look at next line
		}
	}

	if (bombonerror)
	{

extern id0_char_t configname[];
// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
extern	id0_boolean_t	insetupscaling;
extern	id0_int_t	viewsize;
//id0_boolean_t SetViewSize (id0_unsigned_t width, id0_unsigned_t height);
#define HEIGHTRATIO		0.50
//
// wolf hack -- size the view down
//
		if (!insetupscaling && viewsize>10)
		{
mmblocktype	id0_far *savedmmnew;
			savedmmnew = mmnew;
			viewsize -= 2;
			SetViewSize (viewsize*16,viewsize*16*HEIGHTRATIO);
			mmnew = savedmmnew;
			goto tryagain;
		}

//		BE_Cross_unlink_rewritable(configname);
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
		BE_Cross_unlink_rewritable(configname);
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11
		Quit ("MM_GetPtr: Out of memory!");
	}
	else
		mmerror = true;
}

//==========================================================================

/*
====================
=
= MM_FreePtr
=
= Deallocates an unlocked, purgable block
=
====================
*/

void MM_FreePtr (memptr *baseptr)
{
	mmblocktype id0_far *scan,id0_far *last;

	last = mmhead;
	scan = last->next;

	if (baseptr == mmrover->useptr)	// removed the last allocated block
		mmrover = mmhead;

	while (scan->useptr != baseptr && scan)
	{
		last = scan;
		scan = scan->next;
	}

	if (!scan)
		Quit ("MM_FreePtr: Block not found!");

	last->next = scan->next;

	FREEBLOCK(scan);
}
//==========================================================================

/*
=====================
=
= MM_SetPurge
=
= Sets the purge level for a block (locked blocks cannot be made purgable)
=
=====================
*/

void MM_SetPurge (memptr *baseptr, id0_int_t purge)
{
	mmblocktype id0_far *start;

	start = mmrover;

	do
	{
		if (mmrover->useptr == baseptr)
			break;

		mmrover = mmrover->next;

		if (!mmrover)
			mmrover = mmhead;
		else if (mmrover == start)
			Quit ("MM_SetPurge: Block not found!");

	} while (1);

	mmrover->attributes &= ~PURGEBITS;
	mmrover->attributes |= purge;
}

//==========================================================================

/*
=====================
=
= MM_SetLock
=
= Locks / unlocks the block
=
=====================
*/

void MM_SetLock (memptr *baseptr, id0_boolean_t locked)
{
	mmblocktype id0_far *start;

	start = mmrover;

	do
	{
		if (mmrover->useptr == baseptr)
			break;

		mmrover = mmrover->next;

		if (!mmrover)
			mmrover = mmhead;
		else if (mmrover == start)
			Quit ("MM_SetLock: Block not found!");

	} while (1);

	mmrover->attributes &= ~LOCKBIT;
	mmrover->attributes |= locked*LOCKBIT;
}

//==========================================================================

/*
=====================
=
= MM_SortMem
=
= Throws out all purgable stuff and compresses movable blocks
=
=====================
*/

void MM_SortMem (void)
{
	mmblocktype id0_far *scan,id0_far *last,id0_far *next;
	id0_unsigned_t	start,length,source,dest;
	id0_int_t			playing;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!mmstarted)
		return;
#endif
	//
	// lock down a currently playing sound
	//
	playing = SD_SoundPlaying ();
	if (playing)
	{
		switch (SoundMode)
		{
		case sdm_PC:
			playing += STARTPCSOUNDS;
			break;
		case sdm_AdLib:
			playing += STARTADLIBSOUNDS;
			break;
		}
		MM_SetLock((memptr *)&audiosegs[playing],true);
	}


	SD_StopSound();

	if (beforesort)
		beforesort();

	scan = mmhead;

	last = NULL;		// shut up compiler warning

	while (scan)
	{
		if (scan->attributes & LOCKBIT)
		{
		//
		// block is locked, so try to pile later blocks right after it
		//
			start = scan->start + scan->length;
		}
		else
		{
			if (scan->attributes & PURGEBITS)
			{
			//
			// throw out the purgable block
			//
				next = scan->next;
				FREEBLOCK(scan);
				last->next = next;
				scan = next;
				continue;
			}
			else
			{
			//
			// push the non purgable block on top of the last moved block
			//
				if (scan->start != start)
				{
					length = scan->length;
					source = scan->start;
					dest = start;
					while (length > 0xf00)
					{
						memmove(BE_Cross_BGetPtrFromSeg(dest), BE_Cross_BGetPtrFromSeg(source), 0xf00*16);
						//movedata(source,0,dest,0,0xf00*16);
						length -= 0xf00;
						source += 0xf00;
						dest += 0xf00;
					}
					memmove(BE_Cross_BGetPtrFromSeg(dest), BE_Cross_BGetPtrFromSeg(source), length*16);
					//movedata(source,0,dest,0,length*16);

					scan->start = start;
					//*(id0_unsigned_t *)scan->useptr = start;
					*(scan->useptr) = BE_Cross_BGetPtrFromSeg(start);
				}
				start = scan->start + scan->length;
			}
		}

		last = scan;
		scan = scan->next;		// go to next block
	}

	mmrover = mmhead;

	if (aftersort)
		aftersort();

	if (playing)
		MM_SetLock((memptr *)&audiosegs[playing],false);
}


//==========================================================================

/*
=====================
=
= MM_ShowMemory
=
=====================
*/

// *** ALPHA RESTORATION ***
// A few differences, basically appears to be the Catacomb 3-D
// revision, and practically (in terms of compiled code) also the same
// as the Keen Dreams revision, except for call to VW_SetDefaultColors.
void MM_ShowMemory (void)
{
	mmblocktype id0_far *scan;
	id0_unsigned_t color,temp,x,y;
	id0_long_t	end/*,owner*/;
	//id0_char_t    scratch[80],str[10];

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VW_SetDefaultColors();
	VW_SetLineWidth(40);
	temp = bufferofs;
	bufferofs = 0;
	VW_SetScreen (0,0);
#else
	temp = bufferofs;
	bufferofs = displayofs;
#endif
	scan = mmhead;

	end = -1;

	while (scan)
	{
		if (scan->attributes & PURGEBITS)
			color = 5;		// dark purple = purgable
		else
			color = 9;		// medium blue = non purgable
		if (scan->attributes & LOCKBIT)
			color = 12;		// red = locked
		if (scan->start<=end)
			Quit ("MM_ShowMemory: Memory block order currupted!");
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		end = scan->start+scan->length-1;
		VW_Hlin(scan->start,(id0_unsigned_t)end,0,color);
		VW_Plot(scan->start,0,15);
		if (scan->next->start > end+1)
			VW_Hlin(end+1,scan->next->start,0,0);	// black = free
#else
		end = scan->length-1;
		y = scan->start/320;
		x = scan->start%320;
		VW_Hlin(x,x+end,y,color);
		VW_Plot(x,y,15);
		if (scan->next && scan->next->start > end+1)
			VW_Hlin(x+end+1,x+(scan->next->start-scan->start),y,0);	// black = free
#endif

		scan = scan->next;
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	IN_Ack();
	VW_SetLineWidth(64);
#else
	VW_FadeIn ();
	IN_Ack();
#endif

	bufferofs = temp;
}

// *** SHAREWARE V1.0 APOGEE RESTORATION *** (but looks unused in ALL versions)
#if 0 // REFKEEN: So, don't compile this
//#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
//==========================================================================

/*
=====================
=
= MM_DumpData
=
=====================
*/

void MM_DumpData (void)
{
	mmblocktype id0_far *scan,id0_far *best;
	id0_long_t	lowest,oldlowest;
	id0_unsigned_t	owner;
	id0_char_t	lock,purge;
	FILE	*dumpfile;


	BE_Cross_Bfree (nearheap);
	dumpfile = fopen ("MMDUMP.TXT","w");
	if (!dumpfile)
		Quit ("MM_DumpData: Couldn't open MMDUMP.TXT!");

	lowest = -1;
	do
	{
		oldlowest = lowest;
		lowest = 0xffff;

		scan = mmhead;
		while (scan)
		{
			owner = (id0_unsigned_t)scan->useptr;

			if (owner && owner<lowest && owner > oldlowest)
			{
				best = scan;
				lowest = owner;
			}

			scan = scan->next;
		}

		if (lowest != 0xffff)
		{
			if (best->attributes & PURGEBITS)
				purge = 'P';
			else
				purge = '-';
			if (best->attributes & LOCKBIT)
				lock = 'L';
			else
				lock = '-';
			fprintf (dumpfile,"0x%p (%c%c) = %u\n"
			,(id0_unsigned_t)lowest,lock,purge,best->length);
		}

	} while (lowest != 0xffff);

	fclose (dumpfile);
	Quit ("MMDUMP.TXT created.");
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10

//==========================================================================


/*
======================
=
= MM_UnusedMemory
=
= Returns the total free space without purging
=
======================
*/

id0_long_t MM_UnusedMemory (void)
{
	id0_unsigned_t free;
	mmblocktype id0_far *scan;

	free = 0;
	scan = mmhead;

	while (scan->next)
	{
		free += scan->next->start - (scan->start + scan->length);
		scan = scan->next;
	}

	return free*16l;
}

//==========================================================================


/*
======================
=
= MM_TotalFree
=
= Returns the total free space with purging
=
======================
*/

id0_long_t MM_TotalFree (void)
{
	id0_unsigned_t free;
	mmblocktype id0_far *scan;

	free = 0;
	scan = mmhead;

	while (scan->next)
	{
		if ((scan->attributes&PURGEBITS) && !(scan->attributes&LOCKBIT))
			free += scan->length;
		free += scan->next->start - (scan->start + scan->length);
		scan = scan->next;
	}

	return free*16l;
}

//==========================================================================

/*
=====================
=
= MM_BombOnError
=
=====================
*/

void MM_BombOnError (id0_boolean_t bomb)
{
	bombonerror = bomb;
}

REFKEEN_NS_E
