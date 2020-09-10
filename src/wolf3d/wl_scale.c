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

// WL_SCALE.C

#include "wl_def.h"
//#pragma hdrstop

#define OP_RETF	0xcb

/*
=============================================================================

						  GLOBALS

=============================================================================
*/

t_compscale id0_seg *scaledirectory[MAXSCALEHEIGHT+1];
id0_byte_t			*fullscalefarcall[MAXSCALEHEIGHT+1];
//id0_long_t			fullscalefarcall[MAXSCALEHEIGHT+1];

id0_int_t			maxscale,maxscaleshl2;

// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
id0_boolean_t	insetupscaling;
#endif

/*
=============================================================================

						  LOCALS

=============================================================================
*/

t_compscale 	id0_seg *work;
id0_unsigned_t BuildCompScale (id0_int_t height, memptr *finalspot);

id0_int_t			stepbytwo;

//===========================================================================

/*
==============
=
= BadScale
=
==============
*/

void id0_far BadScale (void)
{
	Quit ("BadScale called!");
}


/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling (id0_int_t maxscaleheight)
{
	id0_int_t		i,x,y;
	id0_byte_t	id0_far *dest;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	insetupscaling = true;
#endif

	maxscaleheight/=2;			// one scaler every two pixels

	maxscale = maxscaleheight-1;
	maxscaleshl2 = maxscale<<2;

//
// free up old scalers
//
	for (i=1;i<MAXSCALEHEIGHT;i++)
	{
		if (scaledirectory[i])
			MM_FreePtr ((memptr *)&scaledirectory[i]);
		if (i>=stepbytwo)
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			i++;
#else
			i += 2;
#endif
	}
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	memset (scaledirectory,0,sizeof(scaledirectory));
#endif

	MM_SortMem ();

//
// build the compiled scalers
//
	stepbytwo = viewheight/2;	// save space by double stepping
	MM_GetPtr ((memptr *)&work,20000);

	for (i=1;i<=maxscaleheight;i++)
	{
		BuildCompScale (i*2,(memptr *)&scaledirectory[i]);
		if (i>=stepbytwo)
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			i++;
#else
			i+= 2;
#endif
	}
	MM_FreePtr ((memptr *)&work);

//
// compact memory and lock down scalers
//
	MM_SortMem ();
	for (i=1;i<=maxscaleheight;i++)
	{
		MM_SetLock ((memptr *)&scaledirectory[i],true);
		fullscalefarcall[i] = (id0_byte_t *)scaledirectory[i];
#if 0
		fullscalefarcall[i] = (id0_unsigned_t)scaledirectory[i];
		fullscalefarcall[i] <<=16;
#endif
		fullscalefarcall[i] += scaledirectory[i]->codeofs[0];
		if (i>=stepbytwo)
		{
			scaledirectory[i+1] = scaledirectory[i];
			fullscalefarcall[i+1] = fullscalefarcall[i];
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			i++;
#else
			scaledirectory[i+2] = scaledirectory[i];
			fullscalefarcall[i+2] = fullscalefarcall[i];
			i+=2;
#endif
		}
	}
	scaledirectory[0] = scaledirectory[1];
	fullscalefarcall[0] = fullscalefarcall[1];

//
// check for oversize wall drawing
//
	for (i=maxscaleheight;i<MAXSCALEHEIGHT;i++)
		// REFKEEN: This will be checked in ScalePost separately
		fullscalefarcall[i] = 0;
		//fullscalefarcall[i] = (id0_long_t)BadScale;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	insetupscaling = false;
#endif
}

//===========================================================================

/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
= Call with
= ---------
= DS:SI		Source for scale
= ES:DI		Dest for scale
=
= Calling the compiled scaler only destroys AL
=
========================
*/

id0_unsigned_t BuildCompScale (id0_int_t height, memptr *finalspot)
{
	id0_byte_t		id0_far *code;

	//id0_int_t			i;
	id0_long_t		fix,step;
	id0_unsigned_t	src/*,totalscaled*/,totalsize;
	id0_int_t			startpix,endpix,toppix;


	step = ((id0_long_t)height<<16) / 64;
	code = &work->code[0];
	toppix = (viewheight-height)/2;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = (id0_word_t)((id0_longword_t)fix>>16);
//		startpix = fix>>16;
		fix += step;
		endpix = (id0_word_t)((id0_longword_t)fix>>16);
//		endpix = fix>>16;

		if (endpix>startpix)
			work->width[src] = endpix-startpix;
		else
			work->width[src] = 0;

//
// mark the start of the code
//
		// (REFKEEN) Kind of backwards compatibility
		work->codeofs[src] = (code-(id0_byte_t *)work);
		//work->codeofs[src] = FP_OFF(code);

//
// compile some code if the source pixel generates any screen pixels
//
		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= viewheight || src == 64)
			continue;

	//
	// mov al,[si+src]
	//
		*code++ = 0x8a;
		*code++ = 0x44;
		*code++ = src;

		for (;startpix<endpix;startpix++)
		{
			if (startpix >= viewheight)
				break;						// off the bottom of the view area
			if (startpix < 0)
				continue;					// not into the view area

		//
		// mov [es:di+heightofs],al
		//
			*code++ = 0x26;
			*code++ = 0x88;
			*code++ = 0x85;
			// REFKEEN - Safe unaligned accesses
			id0_unsigned_t temp = startpix*SCREENBWIDE;
			memcpy(code, &temp, 2);
			code += 2;
			//*((id0_unsigned_t id0_far *)code)++ = startpix*SCREENBWIDE;
		}

	}

//
// retf
//
	*code++ = 0xcb;

	// (REFKEEN) Kind of backwards compatibility
	totalsize = (code-(id0_byte_t *)work);
	//totalsize = FP_OFF(code);
	MM_GetPtr (finalspot,totalsize);
	memcpy ((id0_byte_t id0_seg *)(*finalspot),(id0_byte_t id0_seg *)work,totalsize);
//	_fmemcpy ((id0_byte_t id0_seg *)(*finalspot),(id0_byte_t id0_seg *)work,totalsize);

	return totalsize;
}

// (REFKEEN) Replacement for functions generated by BuildCompScale
void ExecuteCompScale(const id0_byte_t *codePtr, id0_unsigned_t destOff, const id0_byte_t *srcPtr, id0_byte_t mask)
{
	while (*codePtr != 0xcb)
	{
		//
		// mov al, [si+src] ;We fetch src from code
		//
		id0_byte_t srcVal = srcPtr[*(codePtr += 2)];
		++codePtr;
		while (*codePtr == 0x26)
		{
			//
			// mov [es:di+heightofs],al
			//
			id0_unsigned_t heightofs;
			memcpy(&heightofs, codePtr += 3, sizeof(heightofs)); // REFKEEN - Safe unaligned accesses
			codePtr += sizeof(heightofs);
			BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destOff+heightofs, srcVal, mask);
		}
	}
}

/*
=======================
=
= ScaleLine
=
= linescale should have the high id0_word_t set to the segment of the scaler
=
=======================
*/

// REFKEEN: Port ScaleLine from ASM and define some things a bit differently
extern	id0_int_t			slinex,slinewidth;
extern	id0_byte_t		*linecmds;
extern	id0_byte_t		*linescale, *lineshape;
//extern	id0_unsigned_t	id0_far *linecmds;
//extern	id0_long_t		linescale;
extern	id0_unsigned_t	maskword;
// *** PRE-V1.4 APOGEE RESTORATION *** - There were apparently some unused
// variables here (or maybe an array). Also brute forcing order of appearances
// of mask1,mask2,mask3 in the EXE layout.
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
extern	id0_byte_t	mask1,mask2,mask3;
id0_unsigned_t unusedscalevar1, unusedscalevar2, unusedscalevar3, unusedscalevar4, unusedscalevar5, unusedscalevar6;
#endif
//id0_byte_t	mask1,mask2,mask3;


void id0_near ScaleLine (void)
{
	id0_byte_t *cmdptr = linecmds;
	id0_word_t cmd;
	id0_int_t linescaleEnd, scalecodeOff, shapetop;
	int i;

        id0_byte_t masks[] = {
		mapmasks1[slinex & 3][slinewidth - 1],
		mapmasks2[slinex & 3][slinewidth - 1],
		mapmasks3[slinex & 3][slinewidth - 1],
		0
	};

	memcpy(&cmd, cmdptr, 2);
	while (cmd)
	{
		memcpy(&linescaleEnd, linescale + cmd, 2);
		id0_byte_t scalerPreFarRet = *(linescale + linescaleEnd); // save old value
		*(linescale + linescaleEnd) = OP_RETF; // patch a RETF in

		memcpy(&scalecodeOff, cmdptr + 4, 2);
		memcpy(&scalecodeOff, linescale + scalecodeOff, 2);
		memcpy(&shapetop, cmdptr + 2, 2);
		cmdptr += 6;

		for (i = 0; masks[i]; ++i)
			ExecuteCompScale(linescale + scalecodeOff, (slinex >> 2) + bufferofs + i, lineshape + shapetop, masks[i]);

		*(linescale + linescaleEnd) = scalerPreFarRet; // unpatch the RETF
		memcpy(&cmd, cmdptr, 2);
	}
#if 0
asm	mov	cx,WORD PTR [linescale+2]
asm	mov	es,cx						// segment of scaler

asm	mov bp,WORD PTR [linecmds]
asm	mov	dx,SC_INDEX+1				// to set SC_MAPMASK

asm	mov	bx,[slinex]
asm	mov	di,bx
asm	shr	di,2						// X in bytes
asm	add	di,[bufferofs]
asm	and	bx,3
asm	shl	bx,3
asm	add	bx,[slinewidth]				// bx = (pixel*8+pixwidth)
asm	mov	al,BYTE [mapmasks3-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ds,WORD PTR [linecmds+2]
asm	or	al,al
asm	jz	notthreebyte				// scale across three bytes
asm	jmp	threebyte
notthreebyte:
asm	mov	al,BYTE PTR ss:[mapmasks2-1+bx]	// -1 because pixwidth of 1 is first
asm	or	al,al
asm	jnz	twobyte						// scale across two bytes

//
// one byte scaling
//
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	out	dx,al						// set map mask register

scalesingle:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	dl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	call ss:[linescale]				// scale the segment of pixels

asm	mov	es,cx						// segment of scaler
asm	mov	BYTE PTR es:[bx],dl			// unpatch the RETF
asm	jmp	scalesingle					// do the next segment


//
// done
//
linedone:
asm	mov	ax,ss
asm	mov	ds,ax
return;

//
// two byte scaling
//
twobyte:
asm	mov	ss:[mask2],al
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask1],al

scaledouble:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	cl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,ss:[mask1]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask2]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	dec	di

asm	mov	es,WORD PTR ss:[linescale+2] // segment of scaler
asm	mov	BYTE PTR es:[bx],cl			// unpatch the RETF
asm	jmp	scaledouble					// do the next segment


//
// three byte scaling
//
threebyte:
asm	mov	ss:[mask3],al
asm	mov	al,BYTE PTR ss:[mapmasks2-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask2],al
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask1],al

scaletriple:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	cl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,ss:[mask1]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask2]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask3]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	dec	di
asm	dec	di

asm	mov	es,WORD PTR ss:[linescale+2] // segment of scaler
asm	mov	BYTE PTR es:[bx],cl			// unpatch the RETF
asm	jmp	scaletriple					// do the next segment
#endif


}


/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

static	id0_long_t		longtemp;

void ScaleShape (id0_int_t xcenter, id0_int_t shapenum, id0_unsigned_t height)
{
	t_compshape	id0_seg *shape;
	t_compscale id0_seg *comptable;
	id0_unsigned_t	scale,srcx,stopx,tempx;
	id0_int_t			t;
	id0_unsigned_t	id0_far *cmdptr;
	id0_boolean_t		leftvis,rightvis;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (shapenum >= SPR_LAST)
	{
		sprintf (str, "ScaleShape: Invalid sprite %u!", shapenum);
		Quit (str);
	}
#endif

	shape = PM_GetSpritePage (shapenum);

	scale = height>>3;						// low three bits are fractional
	if (!scale || scale>maxscale)
		return;								// too close or far away
	comptable = scaledirectory[scale];

	linescale = (id0_byte_t *)comptable;
	lineshape = (id0_byte_t *)shape;
//	*(((id0_unsigned_t *)&linescale)+1)=(id0_unsigned_t)comptable;	// seg of far call
//	*(((id0_unsigned_t *)&linecmds)+1)=(id0_unsigned_t)shape;		// seg of shape

//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = &shape->dataofs[31-stopx];

	while ( --srcx >=stopx && slinex>0)
	{
		linecmds = (id0_byte_t *)shape + *cmdptr--;
//		(id0_unsigned_t)linecmds = *cmdptr--;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		if (slinewidth == 1)
		{
			slinex--;
			if (slinex<viewwidth)
			{
				if (wallheight[slinex] >= height)
					continue;		// obscured by closer wall
				ScaleLine ();
			}
			continue;
		}

		//
		// handle multi pixel lines
		//
		if (slinex>viewwidth)
		{
			slinex -= slinewidth;
			slinewidth = viewwidth-slinex;
			if (slinewidth<1)
				continue;		// still off the right side
		}
		else
		{
			if (slinewidth>slinex)
				slinewidth = slinex;
			slinex -= slinewidth;
		}


		leftvis = (wallheight[slinex] < height);
		rightvis = (wallheight[slinex+slinewidth-1] < height);

		if (leftvis)
		{
			if (rightvis)
				ScaleLine ();
			else
			{
				while (wallheight[slinex+slinewidth-1] >= height)
					slinewidth--;
				ScaleLine ();
			}
		}
		else
		{
			if (!rightvis)
				continue;		// totally obscured

			while (wallheight[slinex] >= height)
			{
				slinex++;
				slinewidth--;
			}
			ScaleLine ();
			break;			// the rest of the shape is gone
		}
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = &shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx && (slinex+=slinewidth)<viewwidth)
	{
		linecmds = (id0_byte_t *)shape + *cmdptr++;
//		(id0_unsigned_t)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		if (slinewidth == 1)
		{
			if (slinex>=0 && wallheight[slinex] < height)
			{
				ScaleLine ();
			}
			continue;
		}

		//
		// handle multi pixel lines
		//
		if (slinex<0)
		{
			if (slinewidth <= -slinex)
				continue;		// still off the left edge

			slinewidth += slinex;
			slinex = 0;
		}
		else
		{
			if (slinex + slinewidth > viewwidth)
				slinewidth = viewwidth-slinex;
		}


		leftvis = (wallheight[slinex] < height);
		rightvis = (wallheight[slinex+slinewidth-1] < height);

		if (leftvis)
		{
			if (rightvis)
			{
				ScaleLine ();
			}
			else
			{
				while (wallheight[slinex+slinewidth-1] >= height)
					slinewidth--;
				ScaleLine ();
				break;			// the rest of the shape is gone
			}
		}
		else
		{
			if (rightvis)
			{
				while (wallheight[slinex] >= height)
				{
					slinex++;
					slinewidth--;
				}
				ScaleLine ();
			}
			else
				continue;		// totally obscured
		}
	}
}



/*
=======================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

void SimpleScaleShape (id0_int_t xcenter, id0_int_t shapenum, id0_unsigned_t height)
{
	t_compshape	id0_seg *shape;
	t_compscale id0_seg *comptable;
	id0_unsigned_t	scale,srcx,stopx,tempx;
	id0_int_t			t;
	id0_unsigned_t	id0_far *cmdptr;
	id0_boolean_t		leftvis,rightvis;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (shapenum >= SPR_LAST)
	{
		sprintf (str, "SimpleScaleShape: Invalid sprite %u!", shapenum);
		Quit (str);
	}
#endif

	shape = PM_GetSpritePage (shapenum);

	scale = height>>1;
	comptable = scaledirectory[scale];

	linescale = (id0_byte_t *)comptable;
	lineshape = (id0_byte_t *)shape;
//	*(((id0_unsigned_t *)&linescale)+1)=(id0_unsigned_t)comptable;	// seg of far call
//	*(((id0_unsigned_t *)&linecmds)+1)=(id0_unsigned_t)shape;		// seg of shape

//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = &shape->dataofs[31-stopx];

	while ( --srcx >=stopx )
	{
		linecmds = (id0_byte_t *)shape + *cmdptr--;
//		(id0_unsigned_t)linecmds = *cmdptr--;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		slinex -= slinewidth;
		ScaleLine ();
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = &shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx )
	{
		linecmds = (id0_byte_t *)shape + *cmdptr++;
//		(id0_unsigned_t)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		ScaleLine ();
		slinex+=slinewidth;
	}
}




//
// bit mask tables for drawing scaled strips up to eight pixels wide
//
// down here so the STUPID inline assembler doesn't get confused!
//


id0_byte_t	mapmasks1[4][8] = {
{1 ,3 ,7 ,15,15,15,15,15},
{2 ,6 ,14,14,14,14,14,14},
{4 ,12,12,12,12,12,12,12},
{8 ,8 ,8 ,8 ,8 ,8 ,8 ,8} };

id0_byte_t	mapmasks2[4][8] = {
{0 ,0 ,0 ,0 ,1 ,3 ,7 ,15},
{0 ,0 ,0 ,1 ,3 ,7 ,15,15},
{0 ,0 ,1 ,3 ,7 ,15,15,15},
{0 ,1 ,3 ,7 ,15,15,15,15} };

id0_byte_t	mapmasks3[4][8] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1},
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,3},
{0 ,0 ,0 ,0 ,0 ,1 ,3 ,7} };


id0_unsigned_t	wordmasks[8][8] = {
{0x0080,0x00c0,0x00e0,0x00f0,0x00f8,0x00fc,0x00fe,0x00ff},
{0x0040,0x0060,0x0070,0x0078,0x007c,0x007e,0x007f,0x807f},
{0x0020,0x0030,0x0038,0x003c,0x003e,0x003f,0x803f,0xc03f},
{0x0010,0x0018,0x001c,0x001e,0x001f,0x801f,0xc01f,0xe01f},
{0x0008,0x000c,0x000e,0x000f,0x800f,0xc00f,0xe00f,0xf00f},
{0x0004,0x0006,0x0007,0x8007,0xc007,0xe007,0xf007,0xf807},
{0x0002,0x0003,0x8003,0xc003,0xe003,0xf003,0xf803,0xfc03},
{0x0001,0x8001,0xc001,0xe001,0xf001,0xf801,0xfc01,0xfe01} };

id0_int_t			slinex,slinewidth;
id0_byte_t		*linecmds;
id0_byte_t		*linescale, *lineshape;
//id0_unsigned_t	id0_far *linecmds;
//id0_long_t		linescale;
id0_unsigned_t	maskword;

