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

// ID_VH.C

#include "id_heads.h"

#define	SCREENWIDTH		80
#define CHARWIDTH		2
#define TILEWIDTH		4
#define GRPLANES		4
#define BYTEPIXELS		4

#define SCREENXMASK		(~3)
#define SCREENXPLUS		(3)
#define SCREENXDIV		(4)

#define VIEWWIDTH		80

#define PIXTOBLOCK		4		// 16 pixels to an update block

#define UNCACHEGRCHUNK(chunk)	{MM_FreePtr(&grsegs[chunk]);grneeded[chunk]&=~ca_levelbit;}

id0_byte_t	update[UPDATEHIGH][UPDATEWIDE];

//==========================================================================

pictabletype	id0_seg *pictable;


id0_int_t	px,py;
id0_byte_t	fontcolor,backcolor;
id0_int_t	fontnumber;
id0_int_t bufferwidth,bufferheight;


//==========================================================================

void	VWL_UpdateScreenBlocks (void);

//==========================================================================

void VW_DrawPropString (id0_char_t id0_far *string)
{
	fontstruct	id0_far	*font;
	id0_int_t		width,step,height,i;
	id0_byte_t	id0_far *source, id0_far *dest, id0_far *origdest;
	id0_byte_t	ch,mask;

	font = (fontstruct id0_far *)grsegs[STARTFONT+fontnumber];
	height = bufferheight = font->height;
	dest = origdest = MK_FP(SCREENSEG,bufferofs+ylookup[py]+(px>>2));
	mask = 1<<(px&3);


	while ((ch = *string++)!=0)
	{
		width = step = font->width[ch];
		source = ((id0_byte_t id0_far *)font)+font->location[ch];
		while (width--)
		{
			VGAMAPMASK(mask);

asm	mov	ah,[BYTE PTR fontcolor]
asm	mov	bx,[step]
asm	mov	cx,[height]
asm	mov	dx,[linewidth]
asm	lds	si,[source]
asm	les	di,[dest]

vertloop:
asm	mov	al,[si]
asm	or	al,al
asm	je	next
asm	mov	[es:di],ah			// draw color

next:
asm	add	si,bx
asm	add	di,dx
asm	loop	vertloop
asm	mov	ax,ss
asm	mov	ds,ax

			source++;
			px++;
			mask <<= 1;
			if (mask == 16)
			{
				mask = 1;
				dest++;
			}
		}
	}
bufferheight = height;
bufferwidth = ((dest+1)-origdest)*4;
}


// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
void VW_DrawColorPropString (id0_char_t id0_far *string)
{
	fontstruct	id0_far	*font;
	id0_int_t		width,step,height,i;
	id0_byte_t	id0_far *source, id0_far *dest, id0_far *origdest;
	id0_byte_t	ch,mask;

	font = (fontstruct id0_far *)grsegs[STARTFONT+fontnumber];
	height = bufferheight = font->height;
	dest = origdest = MK_FP(SCREENSEG,bufferofs+ylookup[py]+(px>>2));
	mask = 1<<(px&3);


	while ((ch = *string++)!=0)
	{
		width = step = font->width[ch];
		source = ((id0_byte_t id0_far *)font)+font->location[ch];
		while (width--)
		{
			VGAMAPMASK(mask);

asm	mov	ah,[BYTE PTR fontcolor]
asm	mov	bx,[step]
asm	mov	cx,[height]
asm	mov	dx,[linewidth]
asm	lds	si,[source]
asm	les	di,[dest]

vertloop:
asm	mov	al,[si]
asm	or	al,al
asm	je	next
asm	mov	[es:di],ah			// draw color

next:
asm	add	si,bx
asm	add	di,dx

asm rcr cx,1				// inc font color
asm jc  cont
asm	inc ah

cont:
asm rcl cx,1
asm	loop	vertloop
asm	mov	ax,ss
asm	mov	ds,ax

			source++;
			px++;
			mask <<= 1;
			if (mask == 16)
			{
				mask = 1;
				dest++;
			}
		}
	}
bufferheight = height;
bufferwidth = ((dest+1)-origdest)*4;
}
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11


//==========================================================================


/*
=================
=
= VL_MungePic
=
=================
*/

void VL_MungePic (id0_byte_t id0_far *source, id0_unsigned_t width, id0_unsigned_t height)
{
	id0_unsigned_t	x,y,plane,size,pwidth;
	id0_byte_t		id0_seg *temp, id0_far *dest, id0_far *srcline;

	size = width*height;

	if (width&3)
		MS_Quit ("VL_MungePic: Not divisable by 4!");

//
// copy the pic to a temp buffer
//
	MM_GetPtr (&(memptr)temp,size);
	_fmemcpy (temp,source,size);

//
// munge it back into the original buffer
//
	dest = source;
	pwidth = width/4;

	for (plane=0;plane<4;plane++)
	{
		srcline = temp;
		for (y=0;y<height;y++)
		{
			for (x=0;x<pwidth;x++)
				*dest++ = *(srcline+x*4+plane);
			srcline+=width;
		}
	}

	MM_FreePtr (&(memptr)temp);
}

void VWL_MeasureString (id0_char_t id0_far *string, id0_word_t *width, id0_word_t *height
	, fontstruct id0_seg *font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((id0_byte_t id0_far *)string)];	// proportional width
}

void	VW_MeasurePropString (id0_char_t id0_far *string, id0_word_t *width, id0_word_t *height)
{
	VWL_MeasureString(string,width,height,(fontstruct id0_seg *)grsegs[STARTFONT+fontnumber]);
}

void	VW_MeasureMPropString  (id0_char_t id0_far *string, id0_word_t *width, id0_word_t *height)
{
	VWL_MeasureString(string,width,height,(fontstruct id0_seg *)grsegs[STARTFONTM+fontnumber]);
}



/*
=============================================================================

				Double buffer management routines

=============================================================================
*/

// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***

#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
// An unknown do-nothing stub - at least until the recreation
// of alpha version from March, hinting that this is
// a do-nothing VGA implemenation of VW_SetDefaultColors
// (used in MM_ShowMemory, as in Catacomb 3-D)
void VW_SetDefaultColors (void)
{
}

// Some v1.0 specific function, guessing it's VW_InitDoubleBuffer
void VW_InitDoubleBuffer (void)
{
	displayofs = 0;
	bufferofs = linewidth*224;
	VL_SetScreen (displayofs,0);
}

// Another v1.0 specific, but unused, function, guessing name from Catacomb 3-D
void VW_FixRefreshBuffer (void)
{
	VL_ScreenToScreen (displayofs,bufferofs,linewidth,160);
}

// Another unused do-nothing stub, again guessing its name from Catacomb 3-D
void VW_QuitDoubleBuffer (void)
{
}
#endif

/*
=======================
=
= VW_MarkUpdateBlock
=
= Takes a pixel bounded block and marks the tiles in bufferblocks
= Returns 0 if the entire block is off the buffer screen
=
=======================
*/

id0_int_t VW_MarkUpdateBlock (id0_int_t x1, id0_int_t y1, id0_int_t x2, id0_int_t y2)
{
	id0_int_t	x,y,xt1,yt1,xt2,yt2,nextline;
	id0_byte_t *mark;

	xt1 = x1>>PIXTOBLOCK;
	yt1 = y1>>PIXTOBLOCK;

	xt2 = x2>>PIXTOBLOCK;
	yt2 = y2>>PIXTOBLOCK;

	if (xt1<0)
		xt1=0;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	else if (xt1>=UPDATEWIDE-1)
#else
	else if (xt1>=UPDATEWIDE)
#endif
		return 0;

	if (yt1<0)
		yt1=0;
	else if (yt1>UPDATEHIGH)
		return 0;

	if (xt2<0)
		return 0;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	else if (xt2>=UPDATEWIDE-1)
		xt2 = UPDATEWIDE-2;
#else
	else if (xt2>=UPDATEWIDE)
		xt2 = UPDATEWIDE-1;
#endif

	if (yt2<0)
		return 0;
	else if (yt2>=UPDATEHIGH)
		yt2 = UPDATEHIGH-1;

	mark = updateptr + uwidthtable[yt1] + xt1;
	nextline = UPDATEWIDE - (xt2-xt1) - 1;

	for (y=yt1;y<=yt2;y++)
	{
		for (x=xt1;x<=xt2;x++)
			*mark++ = 1;			// this tile will need to be updated

		mark += nextline;
	}

	return 1;
}

void VWB_DrawTile8 (id0_int_t x, id0_int_t y, id0_int_t tile)
{
	if (VW_MarkUpdateBlock (x,y,x+7,y+7))
		LatchDrawChar(x,y,tile);
}

void VWB_DrawTile8M (id0_int_t x, id0_int_t y, id0_int_t tile)
{
	if (VW_MarkUpdateBlock (x,y,x+7,y+7))
		VL_MemToScreen (((id0_byte_t id0_far *)grsegs[STARTTILE8M])+tile*64,8,8,x,y);
}


void VWB_DrawPic (id0_int_t x, id0_int_t y, id0_int_t chunknum)
{
	id0_int_t	picnum = chunknum - STARTPICS;
	id0_unsigned_t width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	if (VW_MarkUpdateBlock (x,y,x+width-1,y+height-1))
		VL_MemToScreen (grsegs[chunknum],width,height,x,y);
}



void VWB_DrawPropString	 (id0_char_t id0_far *string)
{
	id0_int_t x;
	x=px;
	VW_DrawPropString (string);
	VW_MarkUpdateBlock(x,py,px-1,py+bufferheight-1);
}


void VWB_Bar (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t height, id0_int_t color)
{
	if (VW_MarkUpdateBlock (x,y,x+width,y+height-1) )
		VW_Bar (x,y,width,height,color);
}

void VWB_Plot (id0_int_t x, id0_int_t y, id0_int_t color)
{
	if (VW_MarkUpdateBlock (x,y,x,y))
		VW_Plot(x,y,color);
}

void VWB_Hlin (id0_int_t x1, id0_int_t x2, id0_int_t y, id0_int_t color)
{
	if (VW_MarkUpdateBlock (x1,y,x2,y))
		VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (id0_int_t y1, id0_int_t y2, id0_int_t x, id0_int_t color)
{
	if (VW_MarkUpdateBlock (x,y1,x,y2))
		VW_Vlin(y1,y2,x,color);
}

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void VH_UpdateScreen (void);
#endif

// *** ALPHA RESTORATION ***
// It looks like VW_UpdateScreen was originally defined to VH_UpdateScreen,
// then got modified to that v1.0 revision with the split screen case and
// finally was replaced as a simple wrapper (not a macro) over VH_UpdateScreen
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void VW_UpdateScreen (void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	extern id0_boolean_t screensplit;
	if (screensplit)
		VH_UpdateSplitScreen ();
	else
#endif
		VH_UpdateScreen ();
}
#endif


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/

/*
=====================
=
= LatchDrawPic
=
=====================
*/

void LatchDrawPic (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t picnum)
{
	id0_unsigned_t wide, height, source;

	wide = pictable[picnum-STARTPICS].width;
	height = pictable[picnum-STARTPICS].height;
	source = latchpics[2+picnum-LATCHPICS_LUMP_START];

	VL_LatchToScreen (source,wide/4,height,x*8,y);
}


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem (void)
{
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	id0_int_t	i,j,p,m,width,height;
#else
	id0_int_t	i,j,p,m,width,height,start,end;
#endif
	id0_byte_t	id0_far *src;
	id0_unsigned_t	destoff;

//
// tile 8s
//
	latchpics[0] = freelatch;
	CA_CacheGrChunk (STARTTILE8);
	src = (id0_byte_t id0_seg *)grsegs[STARTTILE8];
	destoff = freelatch;

	for (i=0;i<NUMTILE8;i++)
	{
		VL_MemToLatch (src,8,8,destoff);
		src += 64;
		destoff +=16;
	}
	UNCACHEGRCHUNK (STARTTILE8);

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// Do compile in v1.0
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
//#if 0	// ran out of latch space!
//
// tile 16s
//
	src = (id0_byte_t id0_seg *)grsegs[STARTTILE16];
	latchpics[1] = destoff;

	for (i=0;i<NUMTILE16;i++)
	{
		CA_CacheGrChunk (STARTTILE16+i);
		src = (id0_byte_t id0_seg *)grsegs[STARTTILE16+i];
		VL_MemToLatch (src,16,16,destoff);
		destoff+=64;
		if (src)
			UNCACHEGRCHUNK (STARTTILE16+i);
	}
#endif

//
// pics
//
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	for (i=LATCHPICS_LUMP_START;i<=LATCHPICS_LUMP_END;i++)
#else
	start = LATCHPICS_LUMP_START;
	end = LATCHPICS_LUMP_END;

	for (i=start;i<=end;i++)
#endif
	{
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		latchpics[2+i-LATCHPICS_LUMP_START] = destoff;
#else
		latchpics[2+i-start] = destoff;
#endif
		CA_CacheGrChunk (i);
		width = pictable[i-STARTPICS].width;
		height = pictable[i-STARTPICS].height;
		VL_MemToLatch (grsegs[i],width,height,destoff);
		destoff += width/4 *height;
		UNCACHEGRCHUNK(i);
	}

	EGAMAPMASK(15);
}

//==========================================================================

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
===================
=
= FizzleFade
=
= returns true if aborted
=
===================
*/

extern	ControlInfo	c;

id0_boolean_t FizzleFade (id0_unsigned_t source, id0_unsigned_t dest,
	id0_unsigned_t width,id0_unsigned_t height, id0_unsigned_t frames, id0_boolean_t abortable)
{
	id0_int_t			pixperframe;
	id0_unsigned_t	drawofs,pagedelta;
	id0_byte_t 		mask,maskb[8] = {1,2,4,8};
	id0_unsigned_t	x,y,p,frame;
	id0_long_t		rndval;

	pagedelta = dest-source;
	rndval = 1;
	y = 0;
	pixperframe = 64000/frames;

	IN_StartAck ();

	TimeCount=frame=0;
	do	// while (1)
	{
		if (abortable && IN_CheckAck () )
			return true;

		asm	mov	es,[screenseg]

		for (p=0;p<pixperframe;p++)
		{
			//
			// seperate random value into x/y pair
			//
			asm	mov	ax,[WORD PTR rndval]
			asm	mov	dx,[WORD PTR rndval+2]
			asm	mov	bx,ax
			asm	dec	bl
			asm	mov	[BYTE PTR y],bl			// low 8 bits - 1 = y xoordinate
			asm	mov	bx,ax
			asm	mov	cx,dx
			asm	mov	[BYTE PTR x],ah			// next 9 bits = x xoordinate
			asm	mov	[BYTE PTR x+1],dl
			//
			// advance to next random element
			//
			asm	shr	dx,1
			asm	rcr	ax,1
			asm	jnc	noxor
			asm	xor	dx,0x0001
			asm	xor	ax,0x2000
noxor:
			asm	mov	[WORD PTR rndval],ax
			asm	mov	[WORD PTR rndval+2],dx

			if (x>width || y>height)
				continue;
			drawofs = source+ylookup[y] + (x>>2);

			//
			// copy one pixel
			//
			mask = x&3;
			VGAREADMAP(mask);
			mask = maskb[mask];
			VGAMAPMASK(mask);

			asm	mov	di,[drawofs]
			asm	mov	al,[es:di]
			asm add	di,[pagedelta]
			asm	mov	[es:di],al

			if (rndval == 1)		// entire sequence has been completed
				return false;
		}
		frame++;
		while (TimeCount<frame)		// don't go too fast
		;
	} while (1);


}
#endif // GAMEVER_NOAH3D
