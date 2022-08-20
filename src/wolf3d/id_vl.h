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

// ID_VL.H

REFKEEN_NS_B

// wolf compatability

#define MS_Quit	Quit

void Quit (const id0_char_t *error);

//===========================================================================


#define SC_INDEX			0x3C4
#define SC_RESET			0
#define SC_CLOCK			1
#define SC_MAPMASK			2
#define SC_CHARMAP			3
#define SC_MEMMODE			4

#define CRTC_INDEX			0x3D4
#define CRTC_H_TOTAL		0
#define CRTC_H_DISPEND		1
#define CRTC_H_BLANK		2
#define CRTC_H_ENDBLANK		3
#define CRTC_H_RETRACE		4
#define CRTC_H_ENDRETRACE 	5
#define CRTC_V_TOTAL		6
#define CRTC_OVERFLOW		7
#define CRTC_ROWSCAN		8
#define CRTC_MAXSCANLINE 	9
#define CRTC_CURSORSTART 	10
#define CRTC_CURSOREND		11
#define CRTC_STARTHIGH		12
#define CRTC_STARTLOW		13
#define CRTC_CURSORHIGH		14
#define CRTC_CURSORLOW		15
#define CRTC_V_RETRACE		16
#define CRTC_V_ENDRETRACE 	17
#define CRTC_V_DISPEND		18
#define CRTC_OFFSET			19
#define CRTC_UNDERLINE		20
#define CRTC_V_BLANK		21
#define CRTC_V_ENDBLANK		22
#define CRTC_MODE			23
#define CRTC_LINECOMPARE 	24


#define GC_INDEX			0x3CE
#define GC_SETRESET			0
#define GC_ENABLESETRESET 	1
#define GC_COLORCOMPARE		2
#define GC_DATAROTATE		3
#define GC_READMAP			4
#define GC_MODE				5
#define GC_MISCELLANEOUS 	6
#define GC_COLORDONTCARE 	7
#define GC_BITMASK			8

#define ATR_INDEX			0x3c0
#define ATR_MODE			16
#define ATR_OVERSCAN		17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN			19
#define ATR_COLORSELECT		20

#define	STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9


//===========================================================================

#define SCREENSEG		0xa000

#define SCREENWIDTH		80			// default screen width in bytes
#define MAXSCANLINES	200			// size of ylookup table

#define CHARWIDTH		2
#define TILEWIDTH		4

//===========================================================================

extern	id0_unsigned_t	bufferofs;			// all drawing is reletive to this
extern	id0_unsigned_t	displayofs,pelpan;	// last setscreen coordinates

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	id0_boolean_t		VGAMode;
#endif

extern	id0_unsigned_t	screenseg;			// set to 0xa000 for asm convenience

extern	id0_unsigned_t	linewidth;
extern	id0_unsigned_t	ylookup[MAXSCANLINES];

extern	id0_boolean_t		screenfaded;
extern	id0_unsigned_t	bordercolor;

//===========================================================================

//
// VGA hardware routines
//

#if 0
#define VGAWRITEMODE(x) asm{\
cli;\
mov dx,GC_INDEX;\
mov al,GC_MODE;\
out dx,al;\
inc dx;\
in al,dx;\
and al,252;\
or al,x;\
out dx,al;\
sti;}

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define VGAMAPMASK(x) asm{cli;mov dx,SC_INDEX+1;mov al,x;out dx,al;sti;}
#else
#define VGAMAPMASK(x) asm{cli;mov dx,SC_INDEX;mov al,SC_MAPMASK;mov ah,x;out dx,ax;sti;}
#endif
#define VGAREADMAP(x) asm{cli;mov dx,GC_INDEX;mov al,GC_READMAP;mov ah,x;out dx,ax;sti;}
#endif
// (REFKEEN) Doing nothing
#define VGAWRITEMODE(x) {}
#define VGAMAPMASK(x) {}
#define VGAREADMAP(x) {}


void VL_Startup (void);
void VL_Shutdown (void);

void VL_SetVGAPlane (void);
void VL_SetTextMode (void);
void VL_DePlaneVGA (void);
void VL_SetVGAPlaneMode (void);
void VL_ClearVideo (id0_byte_t color);

void VL_SetLineWidth (id0_unsigned_t width);
void VL_SetSplitScreen (id0_int_t linenum);

void VL_WaitVBL (id0_int_t vbls);
void VL_CrtcStart (id0_int_t crtc);
void VL_SetScreen (id0_int_t crtc, id0_int_t pelpan);

void VL_FillPalette (id0_int_t red, id0_int_t green, id0_int_t blue);
void VL_SetColor	(id0_int_t color, id0_int_t red, id0_int_t green, id0_int_t blue);
void VL_GetColor	(id0_int_t color, id0_int_t *red, id0_int_t *green, id0_int_t *blue);
void VL_SetPalette (id0_byte_t id0_far *palette);
void VL_GetPalette (id0_byte_t id0_far *palette);
void VL_FadeOut (id0_int_t start, id0_int_t end, id0_int_t red, id0_int_t green, id0_int_t blue, id0_int_t steps);
void VL_FadeIn (id0_int_t start, id0_int_t end, id0_byte_t id0_far *palette, id0_int_t steps);
void VL_ColorBorder (id0_int_t color);

void VL_Plot (id0_int_t x, id0_int_t y, id0_int_t color);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void VL_Hlin (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t color);
#else
void VL_Hlin (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t width, id0_unsigned_t color);
#endif
void VL_Vlin (id0_int_t x, id0_int_t y, id0_int_t height, id0_int_t color);
void VL_Bar (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t height, id0_int_t color);

void VL_MungePic (id0_byte_t id0_far *source, id0_unsigned_t width, id0_unsigned_t height);
void VL_DrawPicBare (id0_int_t x, id0_int_t y, id0_byte_t id0_far *pic, id0_int_t width, id0_int_t height);
void VL_MemToLatch (id0_byte_t id0_far *source, id0_int_t width, id0_int_t height, id0_unsigned_t dest);
void VL_ScreenToScreen (id0_unsigned_t source, id0_unsigned_t dest,id0_int_t width, id0_int_t height);
void VL_MemToScreen (id0_byte_t id0_far *source, id0_int_t width, id0_int_t height, id0_int_t x, id0_int_t y);
void VL_MaskedToScreen (id0_byte_t id0_far *source, id0_int_t width, id0_int_t height, id0_int_t x, id0_int_t y);

// *** S3DNA RESTORATION ***
// REFKEEN: Always declare VL_LatchToScreen, but not FillCharAttr
//#ifdef GAMEVER_NOAH3D
void VL_LatchToScreen (id0_unsigned_t source, id0_int_t width, id0_int_t height, id0_int_t x, id0_int_t y);
#ifdef GAMEVER_NOAH3D
void FillCharAttr (id0_byte_t c, id0_byte_t attr, id0_int_t count);
#else
void VL_DrawTile8String (id0_char_t *str, id0_char_t id0_far *tile8ptr, id0_int_t printx, id0_int_t printy);
void VL_DrawLatch8String (id0_char_t *str, id0_unsigned_t tile8ptr, id0_int_t printx, id0_int_t printy);
void VL_SizeTile8String (id0_char_t *str, id0_int_t *width, id0_int_t *height);
#endif
void VL_DrawPropString (id0_char_t *str, id0_unsigned_t tile8ptr, id0_int_t printx, id0_int_t printy);
void VL_SizePropString (id0_char_t *str, id0_int_t *width, id0_int_t *height, id0_char_t id0_far *font);

#define VL_TestPaletteSet() // REFKEEN: Variable fastpalette isn't read
//void VL_TestPaletteSet (void);

REFKEEN_NS_E
