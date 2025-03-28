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

// ID_VL.C

//#include <dos.h>
//#include <alloc.h>
//#include <mem.h>
#include <string.h>
#include "id_head.h"
#include "id_vl.h"
//#pragma hdrstop

REFKEEN_NS_B

//
// SC_INDEX is expected to stay at SC_MAPMASK for proper operation
//

id0_unsigned_t	bufferofs;
id0_unsigned_t	displayofs,pelpan;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_boolean_t		VGAMode = false;
#endif

id0_unsigned_t	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

id0_unsigned_t	linewidth;
id0_unsigned_t	ylookup[MAXSCANLINES];

id0_boolean_t		screenfaded;
id0_unsigned_t	bordercolor;

// REFKEEN: Variable isn't read
//id0_boolean_t		fastpalette;				// if true, use outsb to set

id0_byte_t		id0_far	palette1[256][3],id0_far palette2[256][3];

//===========================================================================

// asm

id0_int_t	 VL_VideoID (void);
void VL_SetCRTC (id0_int_t crtc);
void VL_SetScreen (id0_int_t crtc, id0_int_t pelpan);
void VL_WaitVBL (id0_int_t vbls);

//===========================================================================


/*
=======================
=
= VL_Startup
=
=======================
*/

#if 0
void	VL_Startup (void)
{
	if ( !MS_CheckParm ("HIDDENCARD") && VL_VideoID () != 5)
		MS_Quit ("You need a VGA graphics card to run this!");

	asm	cld;				// all string instructions assume forward
}

#endif

/*
=======================
=
= VL_Startup	// WOLFENSTEIN HACK
=
=======================
*/

// FIXME (REFKEEN): Better include id_heads.h instead
extern	id0_int_t US_CheckParm(const id0_char_t *parm,const id0_char_t **strings);
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
static	const id0_char_t *ParmStrings[] = {"VGA",""};
#else
static	const id0_char_t *ParmStrings[] = {"HIDDENCARD",""};
#endif

void	VL_Startup (void)
{
	id0_int_t i,videocard;

//	asm	cld;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("VL_Startup: ");
#endif

	videocard = VL_VideoID ();
	for (i = 1;i < id0_argc;i++)
		if (US_CheckParm(id0_argv[i],ParmStrings) == 0)
		{
			videocard = 5;
			break;
		}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (videocard != 5)
	{
		BE_ST_printf("VGA not detected\nUse -VGA option to force detection\n");
		Quit (NULL);
	}
	BE_ST_printf("VGA detected\n");
#else
	if (videocard != 5)
Quit ("Improper video card!  If you really have a VGA card that I am not \n"
	  "detecting, use the -HIDDENCARD command line parameter!");
#endif

}



/*
=======================
=
= VL_Shutdown
=
=======================
*/

void	VL_Shutdown (void)
{
	VL_SetTextMode ();
}


/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void	VL_SetVGAPlaneMode (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (VGAMode)
		return;
	VGAMode = true;
#endif
	BE_ST_SetScreenMode (0x13);
	VL_DePlaneVGA ();
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	VGAMAPMASK(15);
#endif
	VL_SetLineWidth (40);
}


/*
=======================
=
= VL_SetTextMode
=
=======================
*/

void	VL_SetTextMode (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!VGAMode)
		return;
	VGAMode = false;
#endif
	BE_ST_SetScreenMode (3);
}

//===========================================================================

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo (id0_byte_t color)
{
	// REFKEEN TODO: Allow 0x10000 from a single call?
	// The original assembly code was doing 0x8000 stores of a 16-bit word
	BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(0, color, 0x8000);
	BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(0x8000, color, 0x8000);
#if 0
asm	mov	dx,GC_INDEX
asm	mov	al,GC_MODE
asm	out	dx,al
asm	inc	dx
asm	in	al,dx
asm	and	al,0xfc				// write mode 0 to store directly to video
asm	out	dx,al

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	mov	dx,SC_INDEX+1
asm	mov	al,15
asm	out	dx,al
#else
asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256
asm	out	dx,ax				// write through all four planes
#endif

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,[color]
asm	mov	ah,al
asm	mov	cx,0x8000			// 0x8000 words, clearing 8 video bytes/word
asm	xor	di,di
asm	rep	stosw
#endif
}


/*
=============================================================================

			VGA REGISTER MANAGEMENT ROUTINES

=============================================================================
*/


/*
=================
=
= VL_DePlaneVGA
=
=================
*/

void VL_DePlaneVGA (void)
{

#if 0 // REFKEEN TODO: This might be redundant
//
// change CPU addressing to non linear mode
//

//
// turn off chain 4 and odd/even
//
	outportb (SC_INDEX,SC_MEMMODE);
	outportb (SC_INDEX+1,(inportb(SC_INDEX+1)&~8)|4);

	outportb (SC_INDEX,SC_MAPMASK);		// leave this set throughought

//
// turn off odd/even and set write mode 0
//
	outportb (GC_INDEX,GC_MODE);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~0x13);

//
// turn off chain
//
	outportb (GC_INDEX,GC_MISCELLANEOUS);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~2);
#endif

//
// clear the entire buffer space, because int 10h only did 16 k / plane
//
	VL_ClearVideo (0);

#if 0 // REFKEEN TODO: This might be redundant
//
// change CRTC scanning from doubleword to byte mode, allowing >64k scans
//
	outportb (CRTC_INDEX,CRTC_UNDERLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)&~0x40);

	outportb (CRTC_INDEX,CRTC_MODE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)|0x40);
#endif
}

//===========================================================================

/*
====================
=
= VL_SetLineWidth
=
= Line witdh is in WORDS, 40 words is normal width for vgaplanegr
=
====================
*/

void VL_SetLineWidth (id0_unsigned_t width)
{
	id0_int_t i,offset;

//
// set wide virtual screen
//
	BE_ST_VGASetLineWidth(width*2);

//
// set up lookup tables
//
	linewidth = width*2;

	offset = 0;

	for (i=0;i<MAXSCANLINES;i++)
	{
		ylookup[i]=offset;
		offset += linewidth;
	}
}

/*
====================
=
= VL_SetSplitScreen
=
====================
*/

void VL_SetSplitScreen (id0_int_t linenum)
{
	VL_WaitVBL (1);
	linenum=linenum*2-1;
	// Bit 9 of the VGA's "Line Compare" field was originally cleared
	BE_ST_EGASetSplitScreen(linenum & ~512);
#if 0
	outportb (CRTC_INDEX,CRTC_LINECOMPARE);
	outportb (CRTC_INDEX+1,linenum % 256);
	outportb (CRTC_INDEX,CRTC_OVERFLOW);
	outportb (CRTC_INDEX+1, 1+16*(linenum/256));
	outportb (CRTC_INDEX,CRTC_MAXSCANLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1) & (255-64));
#endif
}


/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (id0_int_t red, id0_int_t green, id0_int_t blue)
{
	BE_ST_VGAFillPalette (red, green, blue, 0, 255);
#if 0
	id0_int_t	i;

	outportb (PEL_WRITE_ADR,0);
	for (i=0;i<256;i++)
	{
		outportb (PEL_DATA,red);
		outportb (PEL_DATA,green);
		outportb (PEL_DATA,blue);
	}
#endif
}

//===========================================================================

/*
=================
=
= VL_SetColor
=
=================
*/

void VL_SetColor	(id0_int_t color, id0_int_t red, id0_int_t green, id0_int_t blue)
{
	BE_ST_VGASetPaletteColor (red, green, blue, color);
#if 0
	outportb (PEL_WRITE_ADR,color);
	outportb (PEL_DATA,red);
	outportb (PEL_DATA,green);
	outportb (PEL_DATA,blue);
#endif
}

//===========================================================================

/*
=================
=
= VL_GetColor
=
=================
*/

void VL_GetColor	(id0_int_t color, id0_int_t *red, id0_int_t *green, id0_int_t *blue)
{
	id0_byte_t out_r, out_g, out_b;
	BE_ST_VGAGetPaletteColor (&out_r, &out_g, &out_b, color);
	*red = out_r;
	*green = out_g;
	*blue = out_b;
#if 0
	outportb (PEL_READ_ADR,color);
	*red = inportb (PEL_DATA);
	*green = inportb (PEL_DATA);
	*blue = inportb (PEL_DATA);
#endif
}

//===========================================================================

/*
=================
=
= VL_SetPalette
=
= If fast palette setting has been tested for, it is used
= (some cards don't like outsb palette setting)
=
=================
*/

void VL_SetPalette (id0_byte_t id0_far *palette)
{
	BE_ST_VGASetPalette (palette);
#if 0
	id0_int_t	i;

//	outportb (PEL_WRITE_ADR,0);
//	for (i=0;i<768;i++)
//		outportb(PEL_DATA,*palette++);

	asm	mov	dx,PEL_WRITE_ADR
	asm	mov	al,0
	asm	out	dx,al
	asm	mov	dx,PEL_DATA
	asm	lds	si,[palette]

	asm	test	[ss:fastpalette],1
	asm	jz	slowset
//
// set palette fast for cards that can take it
//
	asm	mov	cx,768
	asm	rep outsb
	asm	jmp	done

//
// set palette slowly for some video cards
//
slowset:
	asm	mov	cx,256
setloop:
	asm	lodsb
	asm	out	dx,al
	asm	lodsb
	asm	out	dx,al
	asm	lodsb
	asm	out	dx,al
	asm	loop	setloop

done:
	asm	mov	ax,ss
	asm	mov	ds,ax
#endif

}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
= This does not use the port string instructions,
= due to some incompatabilities
=
=================
*/

void VL_GetPalette (id0_byte_t id0_far *palette)
{
	BE_ST_VGAGetPalette (palette);
#if 0
	id0_int_t	i;

	outportb (PEL_READ_ADR,0);
	for (i=0;i<768;i++)
		*palette++ = inportb(PEL_DATA);
#endif
}


//===========================================================================

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (id0_int_t start, id0_int_t end, id0_int_t red, id0_int_t green, id0_int_t blue, id0_int_t steps)
{
	id0_int_t		i,j,orig,delta;
	id0_byte_t	id0_far *origptr, id0_far *newptr;

	VL_WaitVBL(1);
	VL_GetPalette (&palette1[0][0]);
	memcpy (palette2,palette1,768);
//	_fmemcpy (palette2,palette1,768);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		origptr = &palette1[start][0];
		newptr = &palette2[start][0];
		for (j=start;j<=end;j++)
		{
			orig = *origptr++;
			delta = red-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = green-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = blue-orig;
			*newptr++ = orig + delta * i / steps;
		}

		VL_WaitVBL(1);
		VL_SetPalette (&palette2[0][0]);
	}

//
// final color
//
	VL_FillPalette (red,green,blue);

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	screenfaded = true;
#endif
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (id0_int_t start, id0_int_t end, id0_byte_t id0_far *palette, id0_int_t steps)
{
	id0_int_t		i,j,delta;

	// Prevent out-of-bound array access in loop
	const id0_byte_t id0_far * const _palette1 = &palette1[0][0];
	id0_byte_t id0_far * const _palette2 = &palette2[0][0];

	VL_WaitVBL(1);
	VL_GetPalette (&palette1[0][0]);
	memcpy (&palette2[0][0],&palette1[0][0],sizeof(palette1));
//	_fmemcpy (&palette2[0][0],&palette1[0][0],sizeof(palette1));

	start *= 3;
	end = end*3+2;

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		for (j=start;j<=end;j++)
		{
			delta = palette[j]-_palette1[j];
			_palette2[j] = _palette1[j] + delta * i / steps;
		}

		VL_WaitVBL(1);
		VL_SetPalette (&palette2[0][0]);
	}

//
// final color
//
	VL_SetPalette (palette);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	screenfaded = false;
#endif
}



/*
=================
=
= VL_TestPaletteSet
=
= Sets the palette with outsb, then reads it in and compares
= If it compares ok, fastpalette is set to true.
=
=================
*/

#if 0 // REFKEEN: fastpalette isn't read
void VL_TestPaletteSet (void)
{
	id0_int_t	i;

	for (i=0;i<768;i++)
		palette1[0][i] = i;

	fastpalette = true;
	VL_SetPalette (&palette1[0][0]);
	VL_GetPalette (&palette2[0][0]);
	if (memcmp (&palette1[0][0],&palette2[0][0],768))
//	if (_fmemcmp (&palette1[0][0],&palette2[0][0],768))
		fastpalette = false;
}
#endif


/*
==================
=
= VL_ColorBorder
=
==================
*/

void VL_ColorBorder (id0_int_t color)
{
	BE_ST_SetBorderColor(color);
	bordercolor = color;
}



/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

id0_byte_t	pixmasks[4] = {1,2,4,8};
id0_byte_t	leftmasks[4] = {15,14,12,8};
id0_byte_t	rightmasks[4] = {1,3,7,15};


/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (id0_int_t x, id0_int_t y, id0_int_t color)
{
	BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(bufferofs+(ylookup[y]+(x>>2)), color, pixmasks[x&3]);
#if 0
	id0_byte_t mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);
	*(id0_byte_t id0_far *)MK_FP(SCREENSEG,bufferofs+(ylookup[y]+(x>>2))) = color;
	VGAMAPMASK(15);
#endif
}


/*
=================
=
= VL_Hlin
=
=================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void VL_Hlin (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t color)
#else
void VL_Hlin (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t width, id0_unsigned_t color)
#endif
{
	id0_unsigned_t		xbyte;
	id0_unsigned_t		destoff;
//	id0_byte_t			id0_far *dest;
	id0_byte_t			leftmask,rightmask;
	id0_int_t				midbytes;

	xbyte = x>>2;
	leftmask = leftmasks[x&3];
	rightmask = rightmasks[(x+width-1)&3];
	// *** ALPHA RESTORATION ***
	// This one is a bit weird
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	midbytes = ((x+width+3)>>2) - xbyte + (-2);
#else
	midbytes = ((x+width+3)>>2) - xbyte - 2;
#endif

	destoff = bufferofs+ylookup[y]+xbyte;
//	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+xbyte);

	if (midbytes<0)
	{
	// all in one byte
		BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff, color, leftmask&rightmask);
#if 0
		VGAMAPMASK(leftmask&rightmask);
		*dest = color;
		VGAMAPMASK(15);
#endif
		return;
	}

	BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff++, color, leftmask);
	BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(destoff, color, midbytes);
	destoff += midbytes;
	BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff, color, rightmask);
#if 0
	VGAMAPMASK(leftmask);
	*dest++ = color;

	VGAMAPMASK(15);
	_fmemset (dest,color,midbytes);
	dest+=midbytes;

	VGAMAPMASK(rightmask);
	*dest = color;

	VGAMAPMASK(15);
#endif
}


/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (id0_int_t x, id0_int_t y, id0_int_t height, id0_int_t color)
{
	id0_byte_t	/*id0_far *dest,*/mask;
	id0_unsigned_t	destoff;

	mask = pixmasks[x&3];

	destoff = bufferofs+ylookup[y]+(x>>2);
#if 0
	VGAMAPMASK(mask);

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));
#endif

	while (height--)
	{
		BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff, color, mask);
		destoff += linewidth;
#if 0
		*dest = color;
		dest += linewidth;
#endif
	}

	VGAMAPMASK(15);
}


/*
=================
=
= VL_Bar
=
=================
*/

void VL_Bar (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t height, id0_int_t color)
{
	id0_unsigned_t	destoff;
//	id0_byte_t	id0_far *dest;
	id0_byte_t	leftmask,rightmask;
	id0_int_t		midbytes,linedelta;

	leftmask = leftmasks[x&3];
	rightmask = rightmasks[(x+width-1)&3];
	midbytes = ((x+width+3)>>2) - (x>>2) - 2;
	linedelta = linewidth-(midbytes+1);

	destoff = bufferofs+ylookup[y]+(x>>2);
//	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));

	if (midbytes<0)
	{
	// all in one byte
		while (height--)
		{
			BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff, color, leftmask&rightmask);
			destoff += linewidth;
		}
#if 0
		VGAMAPMASK(leftmask&rightmask);
		while (height--)
		{
			*dest = color;
			dest += linewidth;
		}
		VGAMAPMASK(15);
#endif
		return;
	}

	while (height--)
	{
		BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff++, color, leftmask);
		BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(destoff, color, midbytes);
		destoff += midbytes;
		BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(destoff, color, rightmask);
		destoff += linedelta;
#if 0
		VGAMAPMASK(leftmask);
		*dest++ = color;

		VGAMAPMASK(15);
		_fmemset (dest,color,midbytes);
		dest+=midbytes;

		VGAMAPMASK(rightmask);
		*dest = color;

		dest+=linedelta;
#endif
	}

	VGAMAPMASK(15);
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch (id0_byte_t id0_far *source, id0_int_t width, id0_int_t height, id0_unsigned_t dest)
{
	id0_unsigned_t	count;
	id0_byte_t	plane/*,mask*/;

	count = ((width+3)/4)*height;
//	mask = 1;
	for (plane = 0; plane<4 ; plane++)
	{
		BE_ST_VGAUpdateGFXBufferInPlane(dest, source, count, plane);
#if 0
		VGAMAPMASK(mask);
		mask <<= 1;

asm	mov	cx,count
asm mov ax,SCREENSEG
asm mov es,ax
asm	mov	di,[dest]
asm	lds	si,[source]
asm	rep movsb
asm mov	ax,ss
asm	mov	ds,ax
#endif

		source+= count;
	}
}


//===========================================================================


/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen (id0_byte_t id0_far *source, id0_int_t width, id0_int_t height, id0_int_t x, id0_int_t y)
{
	id0_unsigned_t		screenoff,destoff;
//	id0_byte_t    id0_far *screen,id0_far *dest,mask;
	id0_int_t		plane;

	width>>=2;
	destoff = bufferofs+ylookup[y]+(x>>2);

	for (plane = 0; plane<4; plane++)
		for (y=0, screenoff = destoff;
		     y < height; y++, screenoff += linewidth, source += width)
			BE_ST_VGAUpdateGFXBufferInPlane(screenoff, source, width, plane);
#if 0
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
	mask = 1 << (x&3);

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			_fmemcpy (screen,source,width);
	}
#endif
}

//==========================================================================


/*
=================
=
= VL_MaskedToScreen
=
= Masks a block of main memory to the screen.
=
=================
*/

void VL_MaskedToScreen (id0_byte_t id0_far *source, id0_int_t width, id0_int_t height, id0_int_t x, id0_int_t y)
{
	// REFKEEN: Looks like a clone of VL_MemToScreen
	VL_MemToScreen (source, width, height, x, y);
#if 0
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	id0_byte_t    id0_far *screen,id0_far *maskptr,id0_far *dest,mask;
#else
	id0_byte_t    id0_far *screen,id0_far *dest,mask;
	id0_byte_t	id0_far *maskptr;
#endif
	id0_int_t		plane;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
//	mask = 1 << (x&3);

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	maskptr = source;
#else
//	maskptr = source;
#endif

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			_fmemcpy (screen,source,width);
	}
#endif
}

//==========================================================================

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreen (id0_unsigned_t source, id0_int_t width, id0_int_t height, id0_int_t x, id0_int_t y)
{
	id0_unsigned_t destOff = ((id0_unsigned_t)x>>2)+ylookup[y]+bufferofs;
	id0_unsigned_t srcOff = source;
	for (id0_unsigned_t i = height; i; --i, destOff += linewidth, srcOff += width)
		BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(destOff, srcOff, width);
#if 0
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	di,[y]				// dest = bufferofs+ylookup[y]+(x>>2)
asm	shl	di,1
asm	mov	di,[WORD PTR ylookup+di]
asm	add	di,[bufferofs]
asm	mov	ax,[x]
asm	shr	ax,2
asm	add	di,ax

asm	mov	si,[source]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
#endif
}


//===========================================================================

#if 0

/*
=================
=
= VL_ScreenToScreen
=
=================
*/

void VL_ScreenToScreen (id0_unsigned_t source, id0_unsigned_t dest,id0_int_t width, id0_int_t height)
{
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	si,[source]
asm	mov	di,[dest]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	si,bx
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
}


#endif

/*
=============================================================================

						STRING OUTPUT ROUTINES

=============================================================================
*/


#if 0 // REFKEEN: Unused functions

/*
===================
=
= VL_DrawTile8String
=
===================
*/

void VL_DrawTile8String (id0_char_t *str, id0_char_t id0_far *tile8ptr, id0_int_t printx, id0_int_t printy)
{
	id0_int_t		i;
	id0_unsigned_t	id0_far *dest,id0_far *screen,id0_far *src;

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[printy]+(printx>>2));

	while (*str)
	{
		src = (id0_unsigned_t id0_far *)(tile8ptr + (*str<<6));
		// each character is 64 bytes

		VGAMAPMASK(1);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(2);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(4);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(8);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;

		str++;
		printx += 8;
		dest+=2;
	}
}



/*
===================
=
= VL_DrawLatch8String
=
===================
*/

void VL_DrawLatch8String (id0_char_t *str, id0_unsigned_t tile8ptr, id0_int_t printx, id0_int_t printy)
{
	id0_int_t		i;
	id0_unsigned_t	src,dest;

	dest = bufferofs+ylookup[printy]+(printx>>2);

	VGAWRITEMODE(1);
	VGAMAPMASK(15);

	while (*str)
	{
		src = tile8ptr + (*str<<4);		// each character is 16 latch bytes

asm	mov	si,[src]
asm	mov	di,[dest]
asm	mov	dx,[linewidth]

asm	mov	ax,SCREENSEG
asm	mov	ds,ax

asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx

asm	mov	ax,ss
asm	mov	ds,ax

		str++;
		printx += 8;
		dest+=2;
	}

	VGAWRITEMODE(0);
}


/*
===================
=
= VL_SizeTile8String
=
===================
*/

void VL_SizeTile8String (id0_char_t *str, id0_int_t *width, id0_int_t *height)
{
	*height = 8;
	*width = 8*strlen(str);
}

#endif // REFKEEN: Unused functions

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D

void FillCharAttr (id0_byte_t c, id0_byte_t attr, id0_int_t count)
{
	BE_ST_RepeatCharWithColorAttributes(c, attr, count);
#if 0
asm	mov	ah,9
asm	mov	al,[c]
asm	mov	bh,0
asm	mov	bl,[attr]
asm	mov	cx,[count]
asm	int	0x10
#endif
}

#endif

REFKEEN_NS_E
