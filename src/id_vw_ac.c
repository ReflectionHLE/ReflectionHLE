/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
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

#include "id_heads.h"

//=================================
//
// CGA view manager routines
//
//=================================

//============================================================================
//
// All of these routines draw into a floating virtual screen segment in main
// memory.  bufferofs points to the origin of the drawing page in screenseg.
// The routines that write out words must take into account buffer wrapping
// and not write a word at 0xffff (which causes an exception on 386s).
//
// The direction flag should be clear
//
//============================================================================

id0_byte_t plotpixels[] = {0xC0, 0x30, 0x0C, 0x03};
id0_byte_t colorbyte[]  = {0x00, 0x55, 0xAA, 0xFF};
id0_word_t colorword[] = {0x0000, 0x5555, 0xAAAA, 0xFFFF};

//============================================================================
//
// VW_Plot (int x,y,color)
//
//============================================================================

void VW_Plot(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t color)
{
	id0_byte_t *destPtr = &screenseg[bufferofs+ylookup[y]+(x>>2)]; // byte on screen
	id0_byte_t maskOn = (colorbyte[color] & plotpixels[x&3]);
	id0_byte_t maskOff = ~plotpixels[x&3]; // mask off other pixels
	*destPtr = (((*destPtr) & maskOff) | maskOn);
}

//============================================================================
//
// VW_Vlin (int yl,yh,x,color)
//
//============================================================================

void VW_Vlin(id0_unsigned_t yl, id0_unsigned_t yh, id0_unsigned_t x, id0_unsigned_t color)
{
	id0_byte_t *destPtr = &screenseg[bufferofs+ylookup[yl]+(x>>2)]; // byte on screen
	id0_byte_t maskOn = (colorbyte[color] & plotpixels[x&3]);
	id0_byte_t maskOff = ~plotpixels[x&3]; // mask off other pixels
	for (id0_word_t loopVar = yh-yl+1; loopVar; --loopVar, destPtr += linewidth)
	{
		*destPtr = (((*destPtr) & plotpixels[x&3]) | maskOn);
	}
}

//============================================================================


//===================
//
// VW_DrawTile8
//
// xcoord in bytes (8 pixels), ycoord in pixels
// All Tile8s are in one grseg, so an offset is calculated inside it
//
// DONE
//
//===================

void VW_DrawTile8(id0_unsigned_t xcoord, id0_unsigned_t ycoord, id0_unsigned_t tile)
{
	id0_byte_t *destPtr = &screenseg[bufferofs+xcoord+ylookup[ycoord]];
	id0_byte_t *tilePtr = grsegs[STARTTILE8]+(tile<<4);
	for (int loopVar = 0; loopVar < 7; ++loopVar) {
		*(destPtr++) = *(tilePtr++);
		*(destPtr++) = *(tilePtr++);
		destPtr += (linewidth-2);
	}
	*(destPtr++) = *(tilePtr++);
	*(destPtr++) = *(tilePtr++);
}

//============================================================================
//
// VW_MaskBlock
//
// Draws a masked block shape to the screen.  bufferofs is NOT accounted for.
// The mask comes first, then the data.  Seperate unwound routines are used
// to speed drawing.
//
// Mask blocks will allways be an even width because of the way IGRAB works
//
// DONE
//
//============================================================================


// Originally (in the ASM), for wide <= UNWOUNDEDMASKS optimize code
// is in use, covering different cases based on the width.
// But there is no need for us to do so here.
//
// EXCEPTION: Despite what the comment above says, maybe the wide argument
// *is* odd for some reason. In such a case we reset the least significant bit.

#define UNWOUNDMASKS 18

VW_MaskBlock(memptr segm,id0_unsigned_t ofs,id0_unsigned_t dest,
	id0_unsigned_t wide,id0_unsigned_t height,id0_unsigned_t planesize)
{
	id0_byte_t *srcPtr = &segm[ofs];
	id0_byte_t *destPtr = &screenseg[dest];
	linedelta = linewidth-wide; // delta to start of next line
	// Emulate unwind routines behaviors
	if (wide <= UNWOUNDMASKS)
	{
		wide &= ~1; // wide should be even, but in case it isn't...
	}
	for (id0_word_t lineCounter = height, colCounter; lineCounter; --lineCounter, destPtr += linedelta)
	{
		for (colCounter = wide; colCounter; --colCounter, ++srcPtr, ++destPtr)
		{
			*destPtr = ((*destPtr) & (*srcPtr)) | srcPtr[planesize];
		}
	}
}

//============================================================================
//
// VW_ScreenToScreen
//
// Basic block copy routine.  Copies one block of screen memory to another,
// bufferofs is NOT accounted for.
//
// DONE
//
//============================================================================

void VW_ScreenToScreen(id0_unsigned_t source, id0_unsigned_t dest,
	id0_unsigned_t wide, id0_unsigned_t height)
{
	id0_byte_t *srcPtr = &screenseg[source];
	id0_byte_t *destPtr = &screenseg[dest];
	for (id0_word_t lineCounter = height; lineCounter; --lineCounter, srcPtr += linewidth, destPtr += linewidth) {
		memcpy(destPtr, srcPtr, wide);
	}
}

//============================================================================
//
// VW_MemToScreen
//
// Basic block drawing routine. Takes a block shape at segment pointer source
// of width by height data, and draws it to dest in the virtual screen,
// based on linewidth.  bufferofs is NOT accounted for.
// There are four drawing routines to provide the best optimized code while
// accounting for odd segment wrappings due to the floating screens.
//
// DONE
//
//============================================================================

void VW_MemToScreen(memptr source, id0_unsigned_t dest,
	id0_unsigned_t wide,id0_unsigned_t height)
{
	id0_byte_t *srcPtr = source;
	id0_byte_t *destPtr = &screenseg[dest];
	// NOTE: Using just one loop instead of four drawing routines now
	for (id0_word_t lineCounter = height; lineCounter; --lineCounter, srcPtr += wide, destPtr += linewidth) {
		memcpy(destPtr, srcPtr, wide);
	}
}

//===========================================================================
//
// VW_ScreenToMem
//
// Copies a block of video memory to main memory, in order from planes 0-3.
// This could be optimized along the lines of VW_MemToScreen to take advantage
// of word copies, but this is an infrequently called routine.
//
// DONE
//
//===========================================================================

void VW_ScreenToMem(id0_unsigned_t source, memptr dest,
	id0_unsigned_t wide, id0_unsigned_t height)
{
	id0_byte_t *srcPtr = &screenseg[source];
	id0_byte_t *destPtr = dest;
	for (id0_word_t lineCounter = height; lineCounter; --lineCounter, srcPtr += linewidth, destPtr += wide) {
		memcpy(destPtr, srcPtr, wide);
	}
}


//===========================================================================
//
//                    MISC CGA ROUTINES
//
//===========================================================================

//==============
//
// VW_SetScreen
//
// DONE
//
//==============

VW_SetScreen (id0_unsigned_t crtc, id0_unsigned_t pelpan)
{
	BE_SDL_SetScreenStartAddress(crtc);
}

#if NUMFONT+NUMFONTM

//===========================================================================
//
// GENERAL FONT DRAWING ROUTINES
//
//===========================================================================

int16_t px, py; // proportional character drawing coordinates
id0_byte_t fontcolor = 15; // 0-15 mapmask value

/*
 * offsets in font structure
 */
//#define pcharheight		0	// lines high
//#define charloc			2	// pointers to every character
//#define charwidth		514	// every character's width in pixels


//id0_word_t propchar; // the character number to shift // UNUSED VARIABLE
char *stringptr;

id0_word_t fontcolormask; // font color expands into this

#define BUFFWIDTH 100
#define BUFFHEIGHT 32 // must be twice as high as font for masked fonts

id0_byte_t databuffer[BUFFWIDTH*BUFFHEIGHT];

id0_word_t bufferwidth; // bytes with valid info / line
id0_word_t bufferheight; // number of lines currently used

id0_word_t bufferbyte;
id0_word_t bufferbit;

id0_word_t screenspot; // where the buffer is going

id0_word_t bufferextra; // add at end of a line copy
id0_word_t screenextra;

//======================
//
// Macros to table shift a byte of font
//
//======================

#define SHIFTNOXOR \
	currshift = currshifttable[*srcPtr]; /* table shift into two bytes */ \
	*(destPtr++) |= (currshift & 0xFF); /* or with first byte */ \
	*destPtr = (currshift >> 8); /* replace next byte */ \
	srcPtr++; /* next source byte */


/*** LOOKS LIKE THIS ONE IS UNUSED ***/
#if 0 
#define SHIFTWITHXOR \
	currshift = ~currshifttable[*srcPtr]; /* table neg of shift into two bytes */ \
	*(destPtr++) &= (currshift & 0xFF); /* and with first byte */ \
	*destPtr = (currshift >> 8); /* replace next byte */ \
	srcPtr++; /* next source byte */
#endif


//=======================
//
// VWL_XORBuffer
//
// Pass buffer start in SI (somewhere in databuffer)
// Draws the buffer to the screen buffer
//
//========================

void VWL_XORBuffer(id0_byte_t *buffer)
{
	fontcolormask = colorword[fontcolor];
	id0_byte_t *srcPtr = buffer;
	id0_byte_t *destPtr = &screenseg[screenspot];
	id0_word_t modbuffwidth = bufferwidth;
	if (!modbuffwidth)
	{
		return; // nothing to draw
	}
	if (modbuffwidth & 1) // odd
	{
		// clear the last byte so word draws can be used
		for (id0_word_t line = 0; line < BUFFHEIGHT; ++line) {
			// Yes, it's databuffer here; Not input buffer ptr
			databuffer[BUFFWIDTH*line+modbuffwidth] = 0;
		}
		++modbuffwidth;
	}
	screenextra = linewidth-modbuffwidth;
	bufferextra = BUFFWIDTH-modbuffwidth;
	// We next divide this by 2 (words to copy)
	modbuffwidth >>= 1;
	for (id0_word_t lineCounter = bufferheight, colPairCounter; lineCounter; --lineCounter, srcPtr += bufferextra, destPtr += screenextra)
	{
		for (colPairCounter = modbuffwidth; colPairCounter; --colPairCounter, srcPtr += 2, destPtr += 2)
		{
			*(id0_word_t *)destPtr ^= ((*(id0_word_t *)srcPtr) & fontcolormask);
		}
	}
}

//============================================================================
//
// NON MASKED FONT DRAWING ROUTINES
//
//============================================================================

#if NUMFONT

//==================
//
// ShiftPropChar
//
// Call with BX = character number (0-255)
// Draws one character to the buffer at bufferbyte/bufferbit, and adjusts
// them to the new position
//
//==================

void ShiftPropChar(id0_word_t charnum)
{
	// WARNING: Later [es:bx] is the source while [di] (ds:di) is the dest

/*
 * find character location, width, and height
 */

	// Font to use
	// TODO (CHOCO KEEN): Using tight packing for struct
	fontstruct *currfont = (fontstruct *)(grsegs[STARTFONT]);
	// Width in pixels
	id0_word_t adjustedwidth = (currfont->width[charnum]) & 0xFF;
	// Location of data in the font
	id0_int_t fontloc = currfont->location[charnum];
	// Going to point at character data
	id0_byte_t *srcPtr = (id0_byte_t *)currfont+fontloc;

/*
 * look up which shift table to use, based on bufferbit
 */
	id0_unsigned_t *currshifttable = shifttabletable[bufferbit];
	id0_unsigned_t currshift;

	id0_byte_t *destPtr = &databuffer[bufferbyte];
	id0_word_t oldbufferbit = bufferbit;
	// add twice because pixel == two bits
	bufferbit = (oldbufferbit+2*adjustedwidth) & 7; // new bit position
	bufferbyte = (oldbufferbit >> 3); // new byte position

	switch ((adjustedwidth+3)>>2)
	{
	case 1: // one byte character
		for (id0_word_t loopvar = currfont->height; loopvar; --loopvar, destPtr += BUFFWIDTH-1)
		{
			SHIFTNOXOR
		}
		break;
	case 2: // two byte character
		for (id0_word_t loopvar = currfont->height; loopvar; --loopvar, destPtr += BUFFWIDTH-2)
		{
			SHIFTNOXOR
			SHIFTNOXOR
		}
		break;
	case 3: // three byte character
		for (id0_word_t loopvar = currfont->height; loopvar; --loopvar, destPtr += BUFFWIDTH-3)
		{
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
		}
		break;
	case 4: // four byte character
		for (id0_word_t loopvar = currfont->height; loopvar; --loopvar, destPtr += BUFFWIDTH-4)
		{
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
		}
		break;
	case 5: // five byte character
		for (id0_word_t loopvar = currfont->height; loopvar; --loopvar, destPtr += BUFFWIDTH-5)
		{
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
		}
		break;
	case 6: // six byte character
		for (id0_word_t loopvar = currfont->height; loopvar; --loopvar, destPtr += BUFFWIDTH-6)
		{
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
			SHIFTNOXOR
		break;
		}
	}
}

//============================================================================

//==================
//
// VW_DrawPropString
//
// Draws a C string of characters at px/py and advances px
//
//==================

void VW_DrawPropString (id0_char_t id0_far *string)
{
	/*
	 * proportional spacing, which clears the buffer ahead of it, so only
	 * clear the first column
	 */
	for (id0_word_t line = 0; line < BUFFHEIGHT; ++line) {
		databuffer[BUFFWIDTH*line] = 0;
	}
	/*
	 * shift the characters into the buffer
	 */
	bufferbit = (((unsigned)px&3) << 1); // one pixel == two bits
	bufferbyte = 0;
	id0_char_t *stringPtr = string;
	do
	{
		id0_byte_t currChAsUnsigned = *(stringPtr++);
		if (!currChAsUnsigned)
		{
			break;
		}
		ShiftPropChar(currChAsUnsigned);
	} while (true);

	/*
	 * calculate position to draw buffer on screen
	 */

	// px>>2: x location in bytes
	screenspot = (ylookup[py]+bufferofs+panadjust) + (px>>2);

	// advance px
	px += ((bufferbyte << 2) | (bufferbit >> 1)); // two bits == one pixels

	// draw it
	bufferwidth = (bufferbit & 7) ? (bufferbyte+1) : bufferbyte;
	// TODO (CHOCO KEEN): Using tight packing for struct
	fontstruct *currfont = (fontstruct *)(grsegs[STARTFONT]);
	bufferheight = currfont->height;
	VWL_XORBuffer(databuffer);
}

#endif // if NUMFONT

/* Seemingly unused functions; VW_DrawMPropString Can't be used as-is
 * as-is anyway (due to lack of a CGA implementation of BufferToScreen)
 */

#if 0

;============================================================================
;
; MASKED FONT DRAWING ROUTINES
;
;============================================================================

#if NUMFONTM

DATASEG

mshiftdrawtable	dw      0,mshift1wide,mshift2wide,mshift3wide


CODESEG

;==================
;
; ShiftMPropChar
;
; Call with BX = character number (0-255)
; Draws one character to the buffer at bufferbyte/bufferbit, and adjusts
; them to the new position
;
;==================

PROC	ShiftMPropChar	NEAR

	mov	es,[grsegs+STARTFONTM*2]	;segment of font to use

;
; find character location, width, and height
;
	mov	si,[es:charwidth+bx]
	and	si,0ffh					;SI hold width in pixels
	shl	bx,1
	mov	bx,[es:charloc+bx]		;BX holds pointer to character data

;
; look up which shift table to use, based on bufferbit
;
	mov	di,[bufferbit]
	shl	di,1
	mov	bp,[shifttabletable+di]	;BP holds pointer to shift table

	mov	di,OFFSET databuffer
	add	di,[bufferbyte]			;DI holds pointer to buffer

;
; advance position by character width
;
	mov	cx,[bufferbit]
	add	cx,si					;new bit position
	mov	ax,cx
	and	ax,7
	mov	[bufferbit],ax			;new bit position
	mov	ax,cx
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	[bufferbyte],ax			;new byte position

	add	si,7
	shr	si,1
	shr	si,1
	shr	si,1					;bytes the character is wide
	shl	si,1                    ;*2 to look up in shiftdrawtable

	mov	cx,[es:pcharheight]
	mov	dx,BUFFWIDTH
	jmp	[ss:mshiftdrawtable+si]	;procedure to draw this width

;
; one byte character
;
mshift1wide:
	dec	dx

EVEN
@@loop1m:
	SHIFTWITHXOR
	add	di,dx			; next line in buffer

	loop	@@loop1m

	mov	cx,[es:pcharheight]

EVEN
@@loop1:
	SHIFTNOXOR
	add	di,dx			; next line in buffer
	loop	@@loop1

	ret

;
; two byte character
;
mshift2wide:
	dec	dx
	dec	dx
EVEN
@@loop2m:
	SHIFTWITHXOR
	SHIFTWITHXOR
	add	di,dx			; next line in buffer

	loop	@@loop2m

	mov	cx,[es:pcharheight]

EVEN
@@loop2:
	SHIFTNOXOR
	SHIFTNOXOR
	add	di,dx			; next line in buffer
	loop	@@loop2

	ret

;
; three byte character
;
mshift3wide:
	sub	dx,3
EVEN
@@loop3m:
	SHIFTWITHXOR
	SHIFTWITHXOR
	SHIFTWITHXOR
	add	di,dx			; next line in buffer

	loop	@@loop3m

	mov	cx,[es:pcharheight]

EVEN
@@loop3:
	SHIFTNOXOR
	SHIFTNOXOR
	SHIFTNOXOR
	add	di,dx			; next line in buffer
	loop	@@loop3

	ret


ENDP

;============================================================================

;==================
;
; VW_DrawMPropString
;
; Draws a C string of characters at px/py and advances px
;
;==================



PROC	VW_DrawMPropString	string:DWORD
PUBLIC	VW_DrawMPropString
USES	SI,DI

;
; clear out the first byte of the buffer, the rest will automatically be
; cleared as characters are drawn into it
;
	mov	es,[grsegs+STARTFONTM*2]
	mov	dx,[es:pcharheight]
	mov	di,OFFSET databuffer
	mov	ax,ds
	mov	es,ax
	mov	bx,BUFFWIDTH-1

	mov	cx,dx
	mov	al,0ffh
@@maskfill:
	stosb				; fill the mask part with $ff
	add	di,bx
	loop	@@maskfill

	mov	cx,dx
	xor	al,al
@@datafill:
	stosb				; fill the data part with $0
	add	di,bx
	loop	@@datafill

;
; shift the characters into the buffer
;
	mov	ax,[px]
	and	ax,7
	mov	[bufferbit],ax
	mov	[bufferbyte],0

	mov	ax,[WORD string]
	mov	[stringptr],ax
	mov	ax,[WORD string+2]
	mov	[stringptr+2],ax

@@shiftone:
	mov	es,[stringptr+2]
	mov	bx,[stringptr]
	inc	[stringptr]
	mov	bx,[es:bx]
	xor	bh,bh
	or	bl,bl
	jz	@@allshifted
	call	ShiftMPropChar
	jmp	@@shiftone

@@allshifted:
;
; calculate position to draw buffer on screen
;
	mov	bx,[py]
	shl	bx,1
	mov	di,[ylookup+bx]
	add	di,[bufferofs]

	mov	ax,[px]
	shr	ax,1
	shr	ax,1
	shr	ax,1		;x location in bytes
	add	di,ax
	mov	[screenspot],di

;
; advance px
;
	mov	ax,[bufferbyte]
	shl	ax,1
	shl	ax,1
	shl	ax,1
	or	ax,[bufferbit]
	add	[px],ax

;
; draw it
;
	mov	ax,[bufferbyte]
	test	[bufferbit],7
	jz	@@go
	inc	ax				;so the partial byte also gets drawn
@@go:
	mov	[bufferwidth],ax
	mov	es,[grsegs+STARTFONTM*2]
	mov	ax,[es:pcharheight]
	mov	[bufferheight],ax

	mov	si,OFFSET databuffer
	call	BufferToScreen		; cut out mask
								; or in data
	call	BufferToScreen		; SI is still in the right position in buffer

	ret

ENDP
#endif // VW_DrawMPropString

#endif // if NUMFONTM

#endif // if fonts
