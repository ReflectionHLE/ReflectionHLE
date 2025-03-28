/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2014-2025 NY00123
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

// ID_VW.C

#include "id_heads.h"

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWWIDTH		40

#define PIXTOBLOCK		4		// 16 pixels to an update block

// REFKEEN - GRMODE is a variable now, so change these into variables as well.
// An exception is SCREENXPLUS, which is unused.
id0_int_t SCREENXMASK, SCREENXDIV;
#if 0
#if GRMODE == EGAGR
#define SCREENXMASK		(~7)
#define SCREENXPLUS		(7)
#define SCREENXDIV		(8)
#endif

#if GRMODE == CGAGR
#define SCREENXMASK		(~3)
#define SCREENXDIV		(4)
#endif
#endif
/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

cardtype	videocard;		// set by VW_Startup
// REFKEEN - Related code won't compile as C++ as-is with unused grtype
int grmode; // TEXTGR, CGAGR, EGAGR, VGAGR
//grtype		grmode;			// CGAgr, EGAgr, VGAgr

id0_unsigned_t	bufferofs;		// hidden area to draw to before displaying
id0_unsigned_t	displayofs;		// origin of the visable screen
id0_unsigned_t	panx,pany;		// panning adjustments inside port in pixels
id0_unsigned_t	pansx,pansy;	// panning adjustments inside port in screen
							// block limited pixel values (ie 0/8 for ega x)
id0_unsigned_t	panadjust;		// panx/pany adjusted by screen resolution

id0_byte_t *screenseg;
//id0_unsigned_t	screenseg;		// normally 0xa000 / 0xb800
id0_unsigned_t	linewidth;
id0_unsigned_t	ylookup[VIRTUALHEIGHT];

id0_boolean_t		screenfaded;

pictabletype	id0_seg *pictable;
pictabletype	id0_seg *picmtable;
spritetabletype id0_seg *spritetable;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void	VWL_MeasureString (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend, id0_word_t *width, id0_word_t *height,
		fontstruct id0_seg *font);
void 	VWL_DrawCursor (void);
void 	VWL_EraseCursor (void);
void 	VWL_DBSetup (void);
void	VWL_UpdateScreenBlocks (void);


id0_int_t			bordercolor;
id0_int_t			cursorvisible;
id0_int_t			cursornumber,cursorwidth,cursorheight,cursorx,cursory;
memptr		cursorsave;
id0_unsigned_t	cursorspot;

extern	id0_unsigned_t	bufferwidth,bufferheight;	// used by font drawing stuff

//===========================================================================


/*
=======================
=
= VW_Startup
=
=======================
*/

// REFKEEN - Add new /SWMOUSE argument from the 2015 port
// (/NOASPECT and /WINDOWED don't seem to work as expected, as of Nov 12 2016)
static const id0_char_t *ParmStrings[] = {"HIDDENCARD","SWMOUSE"};
//static const id0_char_t *ParmStrings[] = {"HIDDENCARD",""};

void	VW_Startup (void)
{
	id0_int_t i;

	// Originally used for certain ASM code loops (clears direction flag)
	//asm	cld;

	videocard = NOcard/*0*/;

	for (i = 1;i < id0_argc;i++)
		// REFKEEN - Add new /SWMOUSE argument from the 2015 port
		switch (US_CheckParm(id0_argv[i],ParmStrings))
		{
		case 0:
			videocard = EGAcard;
			break;
		case 1:
			if (refkeen_current_gamever == BE_GAMEVER_KDREAMS2015)
				BE_ST_HostGfx_SetAbsMouseCursorToggle(false);
			break;
		}
#if 0
		if (US_CheckParm(id0_argv[i],ParmStrings) == 0)
		{
			videocard = EGAcard;
			break;
		}
#endif

	if (!videocard)
		videocard = VW_VideoID ();

	if (GRMODE == EGAGR)
	{
		grmode = EGAGR;
		if (videocard != EGAcard && videocard != VGAcard)
	Quit ("Improper video card!  If you really have an EGA/VGA card that I am not \n"
		  "detecting, use the -HIDDENCARD command line parameter!");
		EGAWRITEMODE(0);
	}
	else if (GRMODE == CGAGR)
	{
		grmode = CGAGR;
		if (videocard < CGAcard || videocard > VGAcard)
	Quit ("Improper video card!  If you really have a CGA card that I am not \n"
		  "detecting, use the -HIDDENCARD command line parameter!");
		MM_GetPtr ((memptr *)&screenseg,0x10000l);	// grab 64k for floating screen
	}

	cursorvisible = 0;
}

//===========================================================================

/*
=======================
=
= VW_Shutdown
=
=======================
*/

void	VW_Shutdown (void)
{
	VW_SetScreenMode (TEXTGR);
	if (GRMODE == EGAGR)
		VW_SetLineWidth (80);
}

//===========================================================================

/*
========================
=
= VW_SetScreenMode
= Call BIOS to set TEXT / CGAgr / EGAgr / VGAgr
=
========================
*/

void VW_SetScreenMode (id0_int_t grmode)
{
	switch (grmode)
	{
	  case TEXTGR:
		BE_ST_SetScreenMode(3);
		screenseg=BE_ST_GetTextModeMemoryPtr();
		break;
	  case CGAGR:
		BE_ST_SetScreenMode(4);
		// screenseg is actually a main mem buffer
		break;
	  case EGAGR:
		BE_ST_SetScreenMode(0xd);
		// REFKEEN no need to obtain screenseg
		// - different EGA planes are accessed with new functions
		break;
#ifdef VGAGAME
	  case VGAGR:{
		  id0_char_t extern VGAPAL;	// deluxepaint vga pallet .OBJ file
		  void id0_far *vgapal = &VGAPAL;
		  SetCool256 ();		// custom 256 color mode
		  screenseg=0xa000;
		  _ES = FP_SEG(vgapal);
		  _DX = FP_OFF(vgapal);
		  _BX = 0;
		  _CX = 0x100;
		  _AX = 0x1012;
		  geninterrupt(0x10);			// set the deluxepaint pallet

		  break;
#endif
	}
	VW_SetLineWidth(SCREENWIDTH);
}

/*
=============================================================================

							SCREEN FADES

=============================================================================
*/

id0_char_t colors[7][17]=
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,0},
 {0,0,0,0,0,0,0,0,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0},
 {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f}};
// REFKEEN - Patch for 2015 port
static id0_char_t fakecgacolors[17] =
{0,0x1b,0x1d,0x1f,0,0x1b,0x1d,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0};

void VW_ColorBorder (id0_int_t color)
{
	BE_ST_SetBorderColor(color);
	bordercolor = color;
}

void VW_SetDefaultColors(void)
{
	if (GRMODE != EGAGR)
		return;
	// REFKEEN - Patch for 2015 port, use either colors[3] or fakecgacolors
	id0_char_t *palette = fakecgamode ? fakecgacolors : colors[3];
	palette[16] = bordercolor;
	BE_ST_EGASetPaletteAndBorder((id0_byte_t *)palette);
	screenfaded = false;
}


void VW_FadeOut(void)
{
	if ((GRMODE != EGAGR) || fakecgamode) // REFKEEN - Patch for 2015 port
		return;
	id0_int_t i;

	for (i=3;i>=0;i--)
	{
	  colors[i][16] = bordercolor;
	  BE_ST_EGASetPaletteAndBorder((id0_byte_t *)&colors[i]);
	  VW_WaitVBL(6);
	}
	screenfaded = true;
}


void VW_FadeIn(void)
{
	if ((GRMODE != EGAGR) || fakecgamode) // REFKEEN - Patch for 2015 port
		return;
	id0_int_t i;

	for (i=0;i<4;i++)
	{
	  colors[i][16] = bordercolor;
	  BE_ST_EGASetPaletteAndBorder((id0_byte_t *)&colors[i]);
	  VW_WaitVBL(6);
	}
	screenfaded = false;
}

void VW_FadeUp(void)
{
	if ((GRMODE != EGAGR) || fakecgamode) // REFKEEN - Patch for 2015 port
		return;
	id0_int_t i;

	for (i=3;i<6;i++)
	{
	  colors[i][16] = bordercolor;
	  BE_ST_EGASetPaletteAndBorder((id0_byte_t *)&colors[i]);
	  VW_WaitVBL(6);
	}
	screenfaded = true;
}

void VW_FadeDown(void)
{
	if ((GRMODE != EGAGR) || fakecgamode) // REFKEEN - Patch for 2015 port
		return;
	id0_int_t i;

	for (i=5;i>2;i--)
	{
	  colors[i][16] = bordercolor;
	  BE_ST_EGASetPaletteAndBorder((id0_byte_t *)&colors[i]);
	  VW_WaitVBL(6);
	}
	screenfaded = false;
}



//===========================================================================

/*
====================
=
= VW_SetLineWidth
=
= Must be an even number of bytes
=
====================
*/

void VW_SetLineWidth (id0_int_t width)
{
  id0_int_t i,offset;

  if (GRMODE == EGAGR)
//
// set wide virtual screen
//
	BE_ST_EGASetLineWidth(width); // Ported from ASM

//
// set up lookup tables
//
  linewidth = width;

  offset = 0;

  for (i=0;i<VIRTUALHEIGHT;i++)
  {
	ylookup[i]=offset;
	offset += width;
  }
}

//===========================================================================

/*
====================
=
= VW_ClearVideo
=
====================
*/

void	VW_ClearVideo (id0_int_t color)
{
	// REFKEEN - A bit of code re-ordering done
	// (originally there was code common to CGA and EGA)
	if (GRMODE == EGAGR)
	{
		EGAWRITEMODE(2);
		EGAMAPMASK(15);

		BE_ST_EGAUpdateGFXBufferFrom4bitsPixel(0, color, 0xffff);

		EGAWRITEMODE(0);
	}
	else if (GRMODE == CGAGR)
	{
		memset(screenseg, color, 0xffff);
	}
}

//===========================================================================

#if NUMPICS>0

/*
====================
=
= VW_DrawPic
=
= X in bytes, y in pixels, chunknum is the #defined picnum
=
====================
*/

void VW_DrawPic(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t chunknum)
{
	id0_int_t	picnum = chunknum - STARTPICS;
	memptr source;
	id0_unsigned_t dest,width,height;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VW_MemToScreen(source,dest,width,height);
}

#endif

#if NUMPICM>0

/*
====================
=
= VW_DrawMPic
=
= X in bytes, y in pixels, chunknum is the #defined picnum
=
====================
*/

void VW_DrawMPic(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t chunknum)
{
	id0_int_t	picnum = chunknum - STARTPICM;
	memptr source;
	id0_unsigned_t dest,width,height;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VW_MaskBlock(source,0,dest,width,height,width*height);
}

#endif

//===========================================================================

#if NUMSPRITES>0

/*
====================
=
= VW_DrawSprite
=
= X and Y in pixels, it will match the closest shift possible
=
= To do:
= Add vertical clipping!
= Make the shifts act as center points, rather than break points
=
====================
*/

void VW_DrawSprite(id0_int_t x, id0_int_t y, id0_unsigned_t chunknum)
{
	spritetabletype id0_far *spr;
	void/*spritetype*/ id0_seg	*block;
	id0_unsigned_t	dest,shift;

	spr = &spritetable[chunknum-STARTSPRITES];
	block = /*(spritetype id0_seg *)*/grsegs[chunknum];

	y+=spr->orgy>>G_P_SHIFT;
	x+=spr->orgx>>G_P_SHIFT;

	if (GRMODE == EGAGR)
		shift = (x&7)/2;
	else if (GRMODE == CGAGR)
		shift = 0;

	dest = bufferofs + ylookup[y];
	if (x>=0)
		dest += x/SCREENXDIV;
	else
		dest += (x+1)/SCREENXDIV;

	VW_MaskBlock (block,VW_GetSpriteShiftSourceOffset(block,shift),dest,
		VW_GetSpriteShiftWidth(block,shift),spr->height,VW_GetSpriteShiftPlaneSize(block,shift));
	//VW_MaskBlock (block,block->sourceoffset[shift],dest,
	//	block->width[shift],spr->height,block->planesize[shift]);
}

#endif

// REFKEEN - GRMODE is a variable now, so EGA and CGA versions of functions
// are defined for all time. Hence, they have been renamed.
// Correct functions are selected based on game version.

/*
==================
=
= VW_Hlin
=
==================
*/


//#if GRMODE == EGAGR

id0_unsigned_char_t leftmask_ega[8] = {0xff,0x7f,0x3f,0x1f,0xf,7,3,1};
id0_unsigned_char_t rightmask_ega[8] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

void VW_Hlin_EGA(id0_unsigned_t xl, id0_unsigned_t xh, id0_unsigned_t y, id0_unsigned_t color)
{
  id0_unsigned_t dest,xlb,xhb,maskleft,maskright,mid;

	xlb=xl/8;
	xhb=xh/8;

	EGAWRITEMODE(2);
	EGAMAPMASK(15);

	maskleft = leftmask_ega[xl&7];
	maskright = rightmask_ega[xh&7];

	mid = xhb-xlb-1;
	dest = bufferofs+ylookup[y]+xlb;

  if (xlb==xhb)
  {
  	//
  	// entire line is in one byte
  	//

	maskleft&=maskright;

	BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(dest, color, maskleft);
	return;
  }

	//
	// draw left side
	//
	BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(dest++, color, maskleft);

	//
	// draw middle
	//
	BE_ST_EGAUpdateGFXBufferFrom4bitsPixel(dest, color, mid); // no masking
	dest += mid;

	//
	// draw right side
	//
	BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(dest, color, maskright);
}
#//endif


//#if GRMODE == CGAGR

//id0_unsigned_char_t pixmask[4] = {0xc0,0x30,0x0c,0x03}; // REFKEEN - Unused variable
id0_unsigned_char_t leftmask_cga[4] = {0xff,0x3f,0x0f,0x03};
id0_unsigned_char_t rightmask_cga[4] = {0xc0,0xf0,0xfc,0xff};
extern id0_unsigned_char_t colorbyte[4];

//
// could be optimized for rep stosw
//
void VW_Hlin_CGA(id0_unsigned_t xl, id0_unsigned_t xh, id0_unsigned_t y, id0_unsigned_t color)
{
	id0_unsigned_t dest,xlb,xhb,mid;
	id0_byte_t maskleft,maskright;

	color = colorbyte[color];	// expand 2 color bits to 8

	xlb=xl/4;
	xhb=xh/4;

	maskleft = leftmask_cga[xl&3];
	maskright = rightmask_cga[xh&3];

	mid = xhb-xlb-1;
	dest = bufferofs+ylookup[y]+xlb;

	if (xlb==xhb)
	{
		//
		// entire line is in one byte
		//
		maskleft&=maskright;
		// mask out pixels; 'or' in color
		screenseg[dest] = (screenseg[dest] & ~maskleft) | ((id0_byte_t)color & maskleft);

		return;
	}

	//
	// draw left side
	//

	// mask out pixels; 'or' in color
	screenseg[dest] = (screenseg[dest] & ~maskleft) | ((id0_byte_t)color & maskleft);
	++dest;

	//
	// draw middle
	//

	BE_Cross_Wrapped_MemSet(screenseg, &screenseg[dest], (id0_byte_t)color, mid);
	//memset(&screenseg[dest], color, mid);
	dest += mid; // dest is a 16-bit unsigned so it wraps "automagically" (and can "safely" be used as an offset)

	//
	// draw right side
	//

	// mask out pixels; 'or' in color
	screenseg[dest] = (screenseg[dest] & ~maskright) | ((id0_byte_t)color & maskright);
}
//#endif


/*
==================
=
= VW_Bar
=
= Pixel addressable block fill routine
=
==================
*/

//#if GRMODE == CGAGR

void VW_Bar_CGA (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t width, id0_unsigned_t height,
	id0_unsigned_t color)
{
	id0_unsigned_t xh = x+width-1;

	while (height--)
		VW_Hlin_CGA (x,xh,y++,color);
}

//#endif


//#if	GRMODE == EGAGR

void VW_Bar_EGA (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t width, id0_unsigned_t height,
	id0_unsigned_t color)
{
	id0_unsigned_t dest,xh,xlb,xhb,maskleft,maskright,mid;

	xh = x+width-1;
	xlb=x/8;
	xhb=xh/8;

	EGAWRITEMODE(2);
	EGAMAPMASK(15);

	maskleft = leftmask_ega[x&7];
	maskright = rightmask_ega[xh&7];

	mid = xhb-xlb-1;
	dest = bufferofs+ylookup[y]+xlb;

	if (xlb==xhb)
	{
		//
		// entire line is in one byte
		//

		maskleft&=maskright;

		do
		{
			BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(dest, color, maskleft);
			dest += linewidth; // down to next line
			--height;
		} while (height);

		return;
	}

	id0_unsigned_t bytesToAdd = linewidth-mid-1;
	do
	{
		//
		// draw left side
		//
		BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(dest++, color, maskleft);

		//
		// draw middle
		//
		BE_ST_EGAUpdateGFXBufferFrom4bitsPixel(dest, color, mid); // no masking
		dest += mid;

		//
		// draw right side
		//
		BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(dest, color, maskright);

		dest += bytesToAdd; // move to start of next line
		--height;
	} while (height);
}

//#endif

//==========================================================================

/*
==================
=
= VW_MeasureString
=
==================
*/

#if NUMFONT+NUMFONTM>0
void
VWL_MeasureString (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend, id0_word_t *width, id0_word_t *height, fontstruct id0_seg *font)
{
	// (REFKEEN) Modifications from the original:
	// - All input strings are now const.
	// - An additional "optsend" argument marking one char past end
	// of string. Set to NULL for original behaviors.
	// - Related to modification to US_Print and US_CPrint, properly taking
	// care of C string literals as inputs.
	//
	// - Keen Dreams exclusive change: Sync with ID_VW.C from the Catacombs
	// and cast each character from input string to an unsigned 8-bit int,
	// because id0_char_t may be defined to be char, which can be signed.
	*height = font->height;
#if 0
	for (*width = 0;*string;string++)
		*width += font->width[*string];		// proportional width
#endif
	if (optsend)
	{
		for (*width = 0;string!=optsend;string++)
			*width += font->width[*((id0_byte_t id0_far *)string)];		// proportional width
	}
	else
	{
		for (*width = 0;*string;string++)
			*width += font->width[*((id0_byte_t id0_far *)string)];		// proportional width
	}
}

void	VW_MeasurePropString (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend, id0_word_t *width, id0_word_t *height)
{
	VWL_MeasureString(string,optsend,width,height,(fontstruct id0_seg *)grsegs[STARTFONT]);
}

void	VW_MeasureMPropString  (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend, id0_word_t *width, id0_word_t *height)
{
	VWL_MeasureString(string,optsend,width,height,(fontstruct id0_seg *)grsegs[STARTFONTM]);
}


#endif


/*
=============================================================================

							CGA stuff

=============================================================================
*/

// REFKEEN - GRMODE is a variable now, so the following is always defined
//#if GRMODE == CGAGR

//#define CGACRTCWIDTH	40 // REFKEEN - Unused macro

/*
==========================
=
= VW_CGAFullUpdate
=
==========================
*/

void VW_CGAFullUpdate (void)
{
#if 0
	id0_byte_t	*update;
	id0_boolean_t	halftile;
	id0_unsigned_t	x,y,middlerows,middlecollumns;
#endif

	displayofs = bufferofs+panadjust;


	BE_ST_CGAUpdateGFXBufferFromWrappedMem(screenseg, screenseg+displayofs, linewidth);
#if 0
	uint8_t *srcPtr = &screenseg[displayofs];
	uint8_t *destPtr = BE_ST_GetCGAMemoryPtr();

	id0_unsigned_t linePairsToCopy = 100; // pairs of scan lines to copy

	do
	{
		BE_Cross_WrappedToLinear_MemCopy(destPtr, screenseg, srcPtr, 80);
		//memcpy(destPtr, srcPtr, 80);
		BE_Cross_Wrapped_Add(screenseg, &srcPtr, linewidth);
		//srcPtr += linewidth;
		destPtr += 0x2000; // go to the interlaced bank

		BE_Cross_WrappedToLinear_MemCopy(destPtr, screenseg, srcPtr, 80);
		//memcpy(destPtr, srcPtr, 80);
		BE_Cross_Wrapped_Add(screenseg, &srcPtr, linewidth);
		//srcPtr += linewidth;
		destPtr -= (0x2000 - 80); // go to the non interlaced bank
	} while (--linePairsToCopy);
#endif

	// clear out the update matrix
	memset(baseupdateptr, 0, UPDATEWIDE*UPDATEHIGH);

	updateptr = baseupdateptr;
	// REFKEEN - Safe unaligned accesses
	*(updateptr + UPDATEWIDE*PORTTILESHIGH) = 1;
	*(updateptr + UPDATEWIDE*PORTTILESHIGH + 1) = 3;
	//*(id0_unsigned_t *)(updateptr + UPDATEWIDE*PORTTILESHIGH) = UPDATETERMINATE;

#if 0
	id0_byte_t	*update;
	id0_boolean_t	halftile;
	id0_unsigned_t	x,y,middlerows,middlecollumns;

	displayofs = bufferofs+panadjust;

asm	mov	ax,0xb800
asm	mov	es,ax

asm	mov	si,[displayofs]
asm	xor	di,di

asm	mov	bx,100				// pairs of scan lines to copy
asm	mov	dx,[linewidth]
asm	sub	dx,80

asm	mov	ds,[screenseg]
asm	test	si,1
asm	jz	evenblock

//
// odd source
//
asm	mov	ax,39				// words accross screen
copytwolineso:
asm	movsb
asm	mov	cx,ax
asm	rep	movsw
asm	movsb
asm	add	si,dx
asm	add	di,0x2000-80		// go to the interlaced bank
asm	movsb
asm	mov	cx,ax
asm	rep	movsw
asm	movsb
asm	add	si,dx
asm	sub	di,0x2000			// go to the non interlaced bank

asm	dec	bx
asm	jnz	copytwolineso
asm	jmp	blitdone

//
// even source
//
evenblock:
asm	mov	ax,40				// words accross screen
copytwolines:
asm	mov	cx,ax
asm	rep	movsw
asm	add	si,dx
asm	add	di,0x2000-80		// go to the interlaced bank
asm	mov	cx,ax
asm	rep	movsw
asm	add	si,dx
asm	sub	di,0x2000			// go to the non interlaced bank

asm	dec	bx
asm	jnz	copytwolines

blitdone:
asm	mov	ax,ss
asm	mov	ds,ax
asm	mov	es,ax

asm	xor	ax,ax				// clear out the update matrix
asm	mov	cx,UPDATEWIDE*UPDATEHIGH/2

asm	mov	di,[baseupdateptr]
asm	rep	stosw

	updateptr = baseupdateptr;
	*(id0_unsigned_t *)(updateptr + UPDATEWIDE*PORTTILESHIGH) = UPDATETERMINATE;
#endif
}


//#endif

/*
=============================================================================

					   CURSOR ROUTINES

These only work in the context of the double buffered update routines

=============================================================================
*/

/*
====================
=
= VWL_DrawCursor
=
= Background saves, then draws the cursor at cursorspot
=
====================
*/

void VWL_DrawCursor (void)
{
	cursorspot = bufferofs + ylookup[cursory+pansy]+(cursorx+pansx)/SCREENXDIV;
	VW_ScreenToMem(cursorspot,cursorsave,cursorwidth,cursorheight);
	VWB_DrawSprite(cursorx,cursory,cursornumber);
}


//==========================================================================


/*
====================
=
= VWL_EraseCursor
=
====================
*/

void VWL_EraseCursor (void)
{
	VW_MemToScreen(cursorsave,cursorspot,cursorwidth,cursorheight);
	VW_MarkUpdateBlock ((cursorx+pansx)&SCREENXMASK,cursory+pansy,
		( (cursorx+pansx)&SCREENXMASK)+cursorwidth*SCREENXDIV-1,
		cursory+pansy+cursorheight-1);
}


//==========================================================================


/*
====================
=
= VW_ShowCursor
=
====================
*/

void VW_ShowCursor (void)
{
	cursorvisible++;
}


//==========================================================================

/*
====================
=
= VW_HideCursor
=
====================
*/

void VW_HideCursor (void)
{
	cursorvisible--;
}

//==========================================================================

/*
====================
=
= VW_MoveCursor
=
====================
*/

void VW_MoveCursor (id0_int_t x, id0_int_t y)
{
	cursorx = x;
	cursory = y;
}

//==========================================================================

/*
====================
=
= VW_SetCursor
=
= Load in a sprite to be used as a cursor, and allocate background save space
=
====================
*/

void VW_SetCursor (id0_int_t spritenum)
{
	if (cursornumber)
	{
		MM_SetLock (&grsegs[cursornumber],false);
		MM_FreePtr (&cursorsave);
	}

	cursornumber = spritenum;

	CA_CacheGrChunk (spritenum);
	MM_SetLock (&grsegs[cursornumber],true);

	cursorwidth = spritetable[spritenum-STARTSPRITES].width+1;
	cursorheight = spritetable[spritenum-STARTSPRITES].height;

	MM_GetPtr (&cursorsave,cursorwidth*cursorheight*5);
}


/*
=============================================================================

				Double buffer management routines

=============================================================================
*/

/*
======================
=
= VW_InitDoubleBuffer
=
======================
*/

void VW_InitDoubleBuffer (void)
{
	if (GRMODE == EGAGR)
		VW_SetScreen_EGA (displayofs+panadjust,0);			// no pel pan
}


/*
======================
=
= VW_FixRefreshBuffer
=
= Copies the view page to the buffer page on page flipped refreshes to
= avoid a one frame shear around pop up windows
=
======================
*/

void VW_FixRefreshBuffer (void)
{
	if (GRMODE == EGAGR)
		VW_ScreenToScreen (displayofs,bufferofs,PORTTILESWIDE*4*CHARWIDTH_EGA,
			PORTTILESHIGH*16);
}


/*
======================
=
= VW_QuitDoubleBuffer
=
======================
*/

void VW_QuitDoubleBuffer (void)
{
}


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
	else if (xt1>=UPDATEWIDE-1)
		return 0;

	if (yt1<0)
		yt1=0;
	else if (yt1>UPDATEHIGH)
		return 0;

	if (xt2<0)
		return 0;
	else if (xt2>=UPDATEWIDE-1)
		xt2 = UPDATEWIDE-2;

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


/*
===========================
=
= VW_UpdateScreen
=
= Updates any changed areas of the double buffer and displays the cursor
=
===========================
*/

void VW_UpdateScreen (void)
{
	if (cursorvisible>0)
		VWL_DrawCursor();

	if (GRMODE == EGAGR)
	{
		VWL_UpdateScreenBlocks();

		BE_ST_SetScreenStartAddress(displayofs+panadjust); // Ported from ASM
	}
	else if (GRMODE == CGAGR)
	{
		VW_CGAFullUpdate();
	}

	if (cursorvisible>0)
		VWL_EraseCursor();
}



void VWB_DrawTile8 (id0_int_t x, id0_int_t y, id0_int_t tile)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x&SCREENXMASK,y,(x&SCREENXMASK)+7,y+7))
		VW_DrawTile8 (x/SCREENXDIV,y,tile);
}

void VWB_DrawTile8M (id0_int_t x, id0_int_t y, id0_int_t tile)
{
	id0_int_t xb;

	x+=pansx;
	y+=pansy;
	xb = x/SCREENXDIV; 			// use intermediate because VW_DT8M is macro
	if (VW_MarkUpdateBlock (x&SCREENXMASK,y,(x&SCREENXMASK)+7,y+7))
		VW_DrawTile8M (xb,y,tile);
}

void VWB_DrawTile16 (id0_int_t x, id0_int_t y, id0_int_t tile)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x&SCREENXMASK,y,(x&SCREENXMASK)+15,y+15))
		VW_DrawTile16 (x/SCREENXDIV,y,tile);
}

void VWB_DrawTile16M (id0_int_t x, id0_int_t y, id0_int_t tile)
{
	id0_int_t xb;

	x+=pansx;
	y+=pansy;
	xb = x/SCREENXDIV;		// use intermediate because VW_DT16M is macro
	if (VW_MarkUpdateBlock (x&SCREENXMASK,y,(x&SCREENXMASK)+15,y+15))
		VW_DrawTile16M (xb,y,tile);
}

#if NUMPICS
void VWB_DrawPic (id0_int_t x, id0_int_t y, id0_int_t chunknum)
{
// mostly copied from drawpic
	id0_int_t	picnum = chunknum - STARTPICS;
	memptr source;
	id0_unsigned_t dest,width,height;

	x+=pansx;
	y+=pansy;
	x/= SCREENXDIV;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	if (VW_MarkUpdateBlock (x*SCREENXDIV,y,(x+width)*SCREENXDIV-1,y+height-1))
		VW_MemToScreen(source,dest,width,height);
}
#endif

#if NUMPICM>0
void VWB_DrawMPic(id0_int_t x, id0_int_t y, id0_int_t chunknum)
{
// mostly copied from drawmpic
	id0_int_t	picnum = chunknum - STARTPICM;
	memptr source;
	id0_unsigned_t dest,width,height;

	x+=pansx;
	y+=pansy;
	x/=SCREENXDIV;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = picmtable[picnum].width;
	height = picmtable[picnum].height;

	if (VW_MarkUpdateBlock (x*SCREENXDIV,y,(x+width)*SCREENXDIV-1,y+height-1))
		VW_MaskBlock(source,0,dest,width,height,width*height);
}
#endif


void VWB_Bar (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t height, id0_int_t color)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x,y,x+width,y+height-1) )
		VW_Bar (x,y,width,height,color);
}


#if NUMFONT
void VWB_DrawPropString	 (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend)
{
	id0_int_t x,y;
	x = px+pansx;
	y = py+pansy;
	VW_DrawPropString (string,optsend);
//	VW_MarkUpdateBlock(0,0,320,200);
	VW_MarkUpdateBlock(x,y,x+bufferwidth*8-1,y+bufferheight-1);
}
#endif


// REFKEEN - Unused function
#if 0
//#if NUMFONTM
void VWB_DrawMPropString (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend)
{
	id0_int_t x,y;
	x = px+pansx;
	y = py+pansy;
	VW_DrawMPropString (string,optsend);
	VW_MarkUpdateBlock(x,y,x+bufferwidth*8-1,y+bufferheight-1);
}
#endif

#if NUMSPRITES
void VWB_DrawSprite(id0_int_t x, id0_int_t y, id0_int_t chunknum)
{
	spritetabletype id0_far *spr;
	void/*spritetype*/ id0_seg	*block;
	id0_unsigned_t	dest,shift,width,height;

	x+=pansx;
	y+=pansy;

	spr = &spritetable[chunknum-STARTSPRITES];
	block = /*(spritetype id0_seg *)*/grsegs[chunknum];

	y+=spr->orgy>>G_P_SHIFT;
	x+=spr->orgx>>G_P_SHIFT;


	if (GRMODE == EGAGR)
		shift = (x&7)/2;
	else if (GRMODE == CGAGR)
		shift = 0;

	dest = bufferofs + ylookup[y];
	if (x>=0)
		dest += x/SCREENXDIV;
	else
		dest += (x+1)/SCREENXDIV;

	width = VW_GetSpriteShiftWidth(block,shift)/*block->width[shift]*/;
	height = spr->height;

	if (VW_MarkUpdateBlock (x&SCREENXMASK,y,(x&SCREENXMASK)+width*SCREENXDIV-1
		,y+height-1))
		VW_MaskBlock (block,VW_GetSpriteShiftSourceOffset(block,shift)/*block->sourceoffset[shift]*/,dest,
			width,height,VW_GetSpriteShiftPlaneSize(block,shift)/*block->planesize[shift]*/);
}
#endif

void VWB_Plot (id0_int_t x, id0_int_t y, id0_int_t color)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x,y,x,y))
		VW_Plot(x,y,color);
}

void VWB_Hlin (id0_int_t x1, id0_int_t x2, id0_int_t y, id0_int_t color)
{
	x1+=pansx;
	x2+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x1,y,x2,y))
		VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (id0_int_t y1, id0_int_t y2, id0_int_t x, id0_int_t color)
{
	x+=pansx;
	y1+=pansy;
	y2+=pansy;
	if (VW_MarkUpdateBlock (x,y1,x,y2))
		VW_Vlin(y1,y2,x,color);
}


//===========================================================================

/*** REFKEEN - A few shared CGA/EGA variable definitions moved from id_vw_ac.c/id_vw_ae.c ***/
/* SIGNED */ id0_int_t px, py; // proportional character drawing coordinates
//pdrawmode
id0_byte_t fontcolor = 15; // 0-15 mapmask value

id0_word_t bufferwidth; // bytes with valid info / line
id0_word_t bufferheight; // number of lines currently used

id0_word_t bufferbyte;
id0_word_t bufferbit;

id0_word_t screenspot; // where the buffer is going

id0_word_t bufferextra; // add at end of a line copy
id0_word_t screenextra;

// REFKEEN - New variable for 2015 port data;
// **MUST** be set to false while DOS version data is in use!!
// (There isn't necessarily a separate check for game version in each such case)
id0_boolean_t	fakecgamode = false;

// (REFKEEN) Used for patching version-specific stuff
id0_int_t	SCREENWIDTH, CHARWIDTH, TILEWIDTH;
id0_int_t	WHITE, BLACK, FIRSTCOLOR, SECONDCOLOR, F_BLACK, F_FIRSTCOLOR, F_SECONDCOLOR;

void (*VW_MaskBlock)(memptr segm,id0_unsigned_t ofs,id0_unsigned_t dest,
	id0_unsigned_t wide,id0_unsigned_t height,id0_unsigned_t planesize);
void (*VW_MemToScreen)(memptr source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);
void (*VW_ScreenToMem)(id0_unsigned_t source,memptr dest,id0_unsigned_t width,id0_unsigned_t height);
void (*VW_ScreenToScreen)(id0_unsigned_t source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);

void (*VW_DrawTile8)(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t tile);

void (*VW_DrawPropString) (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend);
// REFKEEN - Unused function
//void (*VW_DrawMPropString) (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend);

void (*VW_Plot)(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t color);
void (*VW_Hlin)(id0_unsigned_t xl, id0_unsigned_t xh, id0_unsigned_t y, id0_unsigned_t color);
void (*VW_Vlin)(id0_unsigned_t yl, id0_unsigned_t yh, id0_unsigned_t x, id0_unsigned_t color);
void (*VW_Bar) (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t width, id0_unsigned_t height,
	id0_unsigned_t color);

void RefKeen_Patch_id_vw(void)
{
	// Note: Most functions are rather patched in id_vw_ac or id_vw_ae.
	// The definitions of all function pointers are given here, though,
	// since each function pointer may be defined in exactly one place.
	//
	// Additionally, GRMODE *must* be patched first.
	if (GRMODE == CGAGR)
	{
		SCREENWIDTH = SCREENWIDTH_CGA;
		CHARWIDTH = CHARWIDTH_CGA;
		TILEWIDTH = TILEWIDTH_CGA;

		WHITE = WHITE_CGA;
		BLACK = BLACK_CGA;
		FIRSTCOLOR = FIRSTCOLOR_CGA;
		SECONDCOLOR = SECONDCOLOR_CGA;
		F_BLACK = F_BLACK_CGA;
		F_FIRSTCOLOR = F_FIRSTCOLOR_CGA;
		F_SECONDCOLOR = F_SECONDCOLOR_CGA;

		SCREENXMASK = (~3);
		SCREENXDIV = 4;

		VW_Hlin = VW_Hlin_CGA;
		VW_Bar = VW_Bar_CGA;
	}
	else
	{
		SCREENWIDTH = SCREENWIDTH_EGA;
		CHARWIDTH = CHARWIDTH_EGA;
		TILEWIDTH = TILEWIDTH_EGA;

		WHITE = WHITE_EGA;
		BLACK = BLACK_EGA;
		FIRSTCOLOR = FIRSTCOLOR_EGA;
		SECONDCOLOR = SECONDCOLOR_EGA;
		F_BLACK = F_BLACK_EGA;
		F_FIRSTCOLOR = F_FIRSTCOLOR_EGA;
		F_SECONDCOLOR = F_SECONDCOLOR_EGA;

		SCREENXMASK = (~7);
		SCREENXDIV = 8;

		VW_Hlin = VW_Hlin_EGA;
		VW_Bar = VW_Bar_EGA;
	}
}
