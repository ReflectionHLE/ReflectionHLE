/* Copyright (C) 2014-2021 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#include "be_st.h"
#include "be_st_egavga_lookup_tables.h"
#include "be_st_sdl_private.h"
#include "be_video.h"
#include "be_video_emu.h"

BEVideoEmuMem g_sdlVidMem;

BEVideoEmuHostScrMem g_sdlHostScrMem, g_sdlHostScrMemCache;

uint16_t g_sdlScreenStartAddress = 0;
int g_sdlScreenMode = 3;
int g_sdlTexWidth, g_sdlTexHeight;
uint8_t g_sdlPelPanning = 0;
int g_sdlPixLineWidth = 8*40; // Originally stored a byte, while measuring this in bytes instead of pixels
int16_t g_sdlSplitScreenLine = -1;
int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
bool g_sdlTxtCursorEnabled = true;
int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

uint8_t *BE_ST_GetTextModeMemoryPtr(void)
{
	return g_sdlVidMem.text;
}

uint32_t g_sdlEGACurrBGRAPalette[256], g_sdlEGACurrBGRAPaletteCache[256];
uint32_t g_sdlEGALastBGRABorderColor;
uint8_t g_overscanBorderColorIndex, g_overscanBorderColorIndexCache;

/* Gets a value represeting 6 EGA signals determining a color number and
 * returns it in a "Blue Green Red Intensity" 4-bit format.
 * Usually, the 6 signals represented by the given input mean:
 * "Blue Green Red Secondary-Blue Secondary-Green Secondary-Red". However, for
 * the historical reason of compatibility with CGA monitors, on the 200-lines
 * modes used by Keen the Secondary-Green signal is treated as an Intensity
 * one and the two other intensity signals are ignored.
 */
static int BEL_ST_ConvertEGASignalToEGAEntry(int color)
{
	return (color & 7) | ((color & 16) >> 1);
}


void BE_ST_SetScreenStartAddress(uint16_t crtc)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlScreenStartAddress != crtc);
	g_sdlScreenStartAddress = crtc;
}

void BE_ST_SetScreenStartAddressHiPart(uint8_t crtc_hi)
{
	BE_ST_SetScreenStartAddress((g_sdlScreenStartAddress & 0xFF) | (crtc_hi << 8));
}

void BE_ST_SetBorderColor(uint8_t color)
{
	if (g_sdlScreenMode == 0x13)
		g_overscanBorderColorIndex = color;
	else
		g_overscanBorderColorIndex = BEL_ST_ConvertEGASignalToEGAEntry(color);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGASetPaletteAndBorder(const uint8_t *palette)
{
	for (int entry = 0; entry < 16; ++entry)
		g_sdlEGACurrBGRAPalette[entry] = g_sdlEGABGRAScreenColors[BEL_ST_ConvertEGASignalToEGAEntry(palette[entry])];
	g_overscanBorderColorIndex = BEL_ST_ConvertEGASignalToEGAEntry(palette[16]);
	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_VGAForceRefresh(void)
{
	g_overscanBorderColorIndexCache = g_overscanBorderColorIndex^0xFF;
	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_VGASetPaletteColor_NoRefresh(uint8_t r, uint8_t g, uint8_t b, int index)
{
	g_sdlEGACurrBGRAPalette[index] = 0xFC000000 | (r << 18) | (g << 10) | (b << 2);
}

void BE_ST_VGASetPaletteColor(uint8_t r, uint8_t g, uint8_t b, int index)
{
	BEL_ST_VGASetPaletteColor_NoRefresh(r, g, b, index);
	BEL_ST_VGAForceRefresh();
}

void BE_ST_VGAGetPaletteColor(uint8_t *r, uint8_t *g, uint8_t *b, int index)
{
	uint32_t col = g_sdlEGACurrBGRAPalette[index];
	*r = col >> 18;
	*g = col >> 10;
	*b = col >> 2;
}

void BE_ST_VGASetPalette(const uint8_t *palette)
{
	for (int entry = 0; entry < 256; ++entry, palette += 3)
		BE_ST_VGASetPaletteColor(palette[0], palette[1], palette[2], entry);
	BEL_ST_VGAForceRefresh();
}

void BE_ST_VGAGetPalette(uint8_t *palette)
{
	for (int entry = 0; entry < 256; ++entry, palette += 3)
		BE_ST_VGAGetPaletteColor(palette, palette + 1, palette + 2, entry);
}

void BE_ST_VGAFillPalette(uint8_t r, uint8_t g, uint8_t b, int first, int last)
{
	for (int entry = first; entry <= last; ++entry)
		BEL_ST_VGASetPaletteColor_NoRefresh(r, g, b, entry);
	BEL_ST_VGAForceRefresh();
}

void BE_ST_EGASetPelPanning(uint8_t panning)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlPelPanning != panning);
	g_sdlPelPanning = panning;
}

static void BEL_ST_SetLineWidth(int widthInBytes, int pixelsPerAddr)
{
	// TODO: Replace BEL_ST_SetLineWidth with function accepting half width instead?
	widthInBytes &= ~1; // Based on the way the EGA/VGA's CRTC "offset" register was used
	g_sdlDoRefreshGfxOutput |= (g_sdlPixLineWidth != pixelsPerAddr*widthInBytes);
	g_sdlPixLineWidth = pixelsPerAddr*widthInBytes;
}

void BE_ST_EGASetLineWidth(uint8_t widthInBytes)
{
	BEL_ST_SetLineWidth(widthInBytes, 8);
}

void BE_ST_VGASetLineWidth(uint8_t widthInBytes)
{
	BEL_ST_SetLineWidth(widthInBytes, 4);
}

void BE_ST_EGASetSplitScreen(int16_t linenum)
{
	// VGA only for now (200-lines graphics modes)
	if (g_sdlTexHeight == GFX_TEX_HEIGHT)
	{
		// Because 200-lines modes are really double-scanned to 400,
		// a linenum of x was originally replaced with 2x-1 in
		// Catacomb Abss's id_vw.c.
		// In practice, it should've probably been 2x+1, and this is how
		// we "correct" it here (one less black line in Catacomb Abyss
		// before gameplay begins in a map, above the status bar).
		g_sdlSplitScreenLine = linenum/2;
	}
	else
		g_sdlSplitScreenLine = linenum;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXByteInPlane(uint16_t destOff, uint8_t srcVal, uint16_t planeNum)
{
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_egaplane_repeat[1 << planeNum]) | (g_be_st_lookup_linear_to_egaplane[srcVal] << planeNum);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXByteInPlane(uint16_t destOff, uint8_t srcVal, uint16_t planeNum)
{
	g_sdlVidMem.gfxByByte[4*destOff + planeNum] = srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_LinearToWrapped_MemCopy
static void BEL_ST_LinearToEGAPlane_MemCopy(uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num, uint16_t planeNum)
{
	uint64_t planeInvRepeatedMask = ~g_be_st_lookup_egaplane_repeat[1 << planeNum];
	uint64_t *planeDstPtr = &g_sdlVidMem.egaGfx[planeDstOff];
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		for (int i = 0; i < num; ++i, ++planeDstPtr, ++linearSrc)
			*planeDstPtr = ((*planeDstPtr) & planeInvRepeatedMask) | (g_be_st_lookup_linear_to_egaplane[*linearSrc] << planeNum);
	}
	else
	{
		for (int i = 0; i < bytesToEnd; ++i, ++planeDstPtr, ++linearSrc)
			*planeDstPtr = ((*planeDstPtr) & planeInvRepeatedMask) | (g_be_st_lookup_linear_to_egaplane[*linearSrc] << planeNum);
		planeDstPtr = g_sdlVidMem.egaGfx;
		for (int i = 0; i < num-bytesToEnd; ++i, ++planeDstPtr, ++linearSrc)
			*planeDstPtr = ((*planeDstPtr) & planeInvRepeatedMask) | (g_be_st_lookup_linear_to_egaplane[*linearSrc] << planeNum);
	}
	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_LinearToVGAPlane_MemCopy(uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num, uint16_t planeNum)
{
	uint8_t *planeDstPtr = &g_sdlVidMem.gfxByByte[4*planeDstOff + planeNum];
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		for (int i = 0; i < num; ++i, planeDstPtr += 4, ++linearSrc)
			*planeDstPtr = *linearSrc;
	}
	else
	{
		for (int i = 0; i < bytesToEnd; ++i, planeDstPtr += 4, ++linearSrc)
			*planeDstPtr = *linearSrc;
		planeDstPtr = &g_sdlVidMem.gfxByByte[planeNum];
		for (int i = 0; i < num-bytesToEnd; ++i, planeDstPtr += 4, ++linearSrc)
			*planeDstPtr = *linearSrc;
	}
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToLinear_MemCopy
static void BEL_ST_EGAPlaneToLinear_MemCopy(uint8_t *linearDst, uint16_t planeSrcOff, uint16_t num, uint16_t planeNum)
{
	const uint64_t *planeSrcPtr = &g_sdlVidMem.egaGfx[planeSrcOff];
	uint16_t bytesToEnd = 0x10000-planeSrcOff;
	if (num <= bytesToEnd)
	{
		for (int i = 0; i < num; ++i, ++linearDst, ++planeSrcPtr)
			*linearDst = BEL_ST_Lookup_EGAPlaneToLinear((*planeSrcPtr)>>planeNum);
	}
	else
	{
		for (int i = 0; i < bytesToEnd; ++i, ++linearDst, ++planeSrcPtr)
			*linearDst = BEL_ST_Lookup_EGAPlaneToLinear((*planeSrcPtr)>>planeNum);
		planeSrcPtr = g_sdlVidMem.egaGfx;
		for (int i = 0; i < num-bytesToEnd; ++i, ++linearDst, ++planeSrcPtr)
			*linearDst = BEL_ST_Lookup_EGAPlaneToLinear((*planeSrcPtr)>>planeNum);
	}
	//No need to since we just read screen data
	//g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToWrapped_MemCopy
static void BEL_ST_EGAVGAPlaneToAllPlanes_MemCopy(
	uint16_t planeDstOff, uint16_t planeSrcOff, uint16_t num, int pixelsPerAddr)
{
	uint8_t *vidMemPtr = g_sdlVidMem.gfxByByte;
	uint16_t srcBytesToEnd = 0x10000-planeSrcOff;
	uint16_t dstBytesToEnd = 0x10000-planeDstOff;
	if (num <= srcBytesToEnd)
	{
		// Source is linear: Same as BE_Cross_LinearToWrapped_MemCopy here
		if (num <= dstBytesToEnd)
			memcpy(
				vidMemPtr + pixelsPerAddr*planeDstOff,
				vidMemPtr + pixelsPerAddr*planeSrcOff,
				pixelsPerAddr*num);
		else
		{
			memcpy(
				vidMemPtr + pixelsPerAddr*planeDstOff,
				vidMemPtr + pixelsPerAddr*planeSrcOff,
				pixelsPerAddr*dstBytesToEnd);
			memcpy(
				vidMemPtr,
				vidMemPtr + pixelsPerAddr*(planeSrcOff+dstBytesToEnd),
				pixelsPerAddr*(num-dstBytesToEnd));
		}
	}
	// Otherwise, check if at least the destination is linear
	else if (num <= dstBytesToEnd)
	{
		// Destination is linear: Same as
		// BE_Cross_WrappedToLinear_MemCopy, non-linear source
		memcpy(
			vidMemPtr + pixelsPerAddr*planeDstOff,
			vidMemPtr + pixelsPerAddr*planeSrcOff,
			pixelsPerAddr*srcBytesToEnd);
		memcpy(
			vidMemPtr + pixelsPerAddr*(planeDstOff+srcBytesToEnd),
			vidMemPtr,
			pixelsPerAddr*(num-srcBytesToEnd));
	}
	// BOTH buffers have wrapping. We don't check separately if
	// srcBytesToEnd==dstBytesToEnd (in such a case planeDstOff==planeSrcOff...)
	else if (srcBytesToEnd <= dstBytesToEnd)
	{
		memcpy(
			vidMemPtr + pixelsPerAddr*planeDstOff,
			vidMemPtr + pixelsPerAddr*planeSrcOff,
			pixelsPerAddr*srcBytesToEnd);
		memcpy(
			vidMemPtr + pixelsPerAddr*(planeDstOff+srcBytesToEnd),
			vidMemPtr,
			pixelsPerAddr*(dstBytesToEnd-srcBytesToEnd));
		memcpy(
			vidMemPtr,
			vidMemPtr + pixelsPerAddr*(dstBytesToEnd-srcBytesToEnd),
			pixelsPerAddr*(num-dstBytesToEnd));
	}
	else // srcBytesToEnd > dstBytesToEnd
	{
		memcpy(
			vidMemPtr + pixelsPerAddr*planeDstOff,
			vidMemPtr + pixelsPerAddr*planeSrcOff,
			pixelsPerAddr*dstBytesToEnd);
		memcpy(
			vidMemPtr,
			vidMemPtr + pixelsPerAddr*(planeSrcOff+dstBytesToEnd),
			pixelsPerAddr*(srcBytesToEnd-dstBytesToEnd));
		memcpy(
			vidMemPtr + pixelsPerAddr*(srcBytesToEnd-dstBytesToEnd),
			vidMemPtr,
			pixelsPerAddr*(num-srcBytesToEnd));
	}
	g_sdlDoRefreshGfxOutput = true;
}

// A similar analogue of memset
static void BEL_ST_EGAVGAPlane_MemSet(uint16_t planeDstOff, uint8_t value, uint16_t num, int pixelsPerAddr)
{
	uint8_t *vidMemPtr = g_sdlVidMem.gfxByByte;
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		memset(vidMemPtr + pixelsPerAddr*planeDstOff, value, pixelsPerAddr*num);
	}
	else
	{
		memset(vidMemPtr + pixelsPerAddr*planeDstOff, value, pixelsPerAddr*bytesToEnd);
		memset(vidMemPtr, value, pixelsPerAddr*(num-bytesToEnd));
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXBufferInPlane(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeNum)
{
	BEL_ST_LinearToEGAPlane_MemCopy(destOff, srcPtr, num, planeNum);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXBufferInPlane(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeNum)
{
	BEL_ST_LinearToVGAPlane_MemCopy(destOff, srcPtr, num, planeNum);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff)
{
	g_sdlVidMem.egaGfx[destOff] = g_sdlVidMem.egaGfx[srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr but with a specific plane (added for Catacomb Abyss vanilla bug reproduction/workaround)
void BE_ST_EGAUpdateGFXByteInPlaneScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t planeNum)
{
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_egaplane_repeat[1<<planeNum]) | (g_sdlVidMem.egaGfx[srcOff] & g_be_st_lookup_egaplane_repeat[1<<planeNum]);
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr but picking specific bits out of each byte
void BE_ST_EGAUpdateGFXBitsInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_egaplane_bitsmask[bitsMask]) | (g_sdlVidMem.egaGfx[srcOff] & g_be_st_lookup_egaplane_bitsmask[bitsMask]);
	g_sdlDoRefreshGfxOutput = true;
}


void BE_ST_EGAUpdateGFXBufferInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_ST_EGAVGAPlaneToAllPlanes_MemCopy(destOff, srcOff, num, 8);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_ST_EGAVGAPlaneToAllPlanes_MemCopy(destOff, srcOff, num, 4);
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t BE_ST_EGAFetchGFXByteFromPlane(uint16_t destOff, uint16_t planeNum)
{
	return BEL_ST_Lookup_EGAPlaneToLinear(g_sdlVidMem.egaGfx[destOff]>>planeNum);
}

uint8_t BE_ST_VGAFetchGFXByteFromPlane(uint16_t destOff, uint16_t planeNum)
{
	return g_sdlVidMem.gfxByByte[4*destOff + planeNum];
}

void BE_ST_EGAFetchGFXBufferFromPlane(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planeNum)
{
	BEL_ST_EGAPlaneToLinear_MemCopy(destPtr, srcOff, num, planeNum);
}

void BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(uint16_t destOff, uint8_t color, uint8_t bitsMask)
{
	color &= 0xF; // We may get a larger value in The Catacombs Armageddon (sky color)
	g_sdlVidMem.egaGfx[destOff] = (g_sdlVidMem.egaGfx[destOff] & ~g_be_st_lookup_egaplane_bitsmask[bitsMask]) | (g_be_st_lookup_egaplane_repeat[color] & g_be_st_lookup_egaplane_bitsmask[bitsMask]);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXBitsFrom8bitsPixel(uint16_t destOff, uint8_t color, uint8_t bitsMask)
{
	bitsMask &= 0xF; // TODO add assert instead?
	g_sdlVidMem.vgaGfx[destOff] = (g_sdlVidMem.vgaGfx[destOff] & ~g_be_st_lookup_vgaplane_bitsmask[bitsMask]) | (g_be_st_lookup_vgaplane_repeat[color] & g_be_st_lookup_vgaplane_bitsmask[bitsMask]);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXBufferFrom4bitsPixel(uint16_t destOff, uint8_t color, uint16_t count)
{
	color &= 0xF; // We may get a larger value in The Catacombs Armageddon (sky color)
	BEL_ST_EGAVGAPlane_MemSet(destOff, color, count, 8);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(uint16_t destOff, uint8_t color, uint16_t count)
{
	BEL_ST_EGAVGAPlane_MemSet(destOff, color, count, 4);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAXorGFXByteByPlaneMask(uint16_t destOff, uint8_t srcVal, uint16_t planeMask)
{
	g_sdlVidMem.egaGfx[destOff] ^= ((g_be_st_lookup_egaplane_repeat[planeMask] & g_be_st_lookup_egaplane_bitsmask[srcVal]));
	g_sdlDoRefreshGfxOutput = true;
}



void BE_ST_CGAUpdateGFXBufferFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth)
{
	const uint8_t *endSegPtr = segPtr + 0x10000;
	uint8_t *cgaHostPtr = g_sdlHostScrMem.cgaGfx;
	// Remember this is a wrapped copy, offInSegPtr points inside a 65536-bytes long segment beginning at segPtr;
	// But we still want to skip some bytes, assuming byteLineWidth sources bytes per line (picking 80 out of these)
	int lineBytesLeft = byteLineWidth - GFX_TEX_WIDTH/4;
	for (int line = 0, byteInLine; line < GFX_TEX_HEIGHT; ++line)
	{
		for (byteInLine = 0; byteInLine < GFX_TEX_WIDTH/4; ++byteInLine, ++offInSegPtr, offInSegPtr = (offInSegPtr == endSegPtr) ? segPtr : offInSegPtr)
		{
			*cgaHostPtr++ = ((*offInSegPtr) & 0xC0) >> 6;
			*cgaHostPtr++ = ((*offInSegPtr) & 0x30) >> 4;
			*cgaHostPtr++ = ((*offInSegPtr) & 0x0C) >> 2;
			*cgaHostPtr++ = ((*offInSegPtr) & 0x03);
		}
		offInSegPtr += lineBytesLeft;
		if (offInSegPtr >= endSegPtr)
		{
			offInSegPtr = segPtr + (uint16_t)(offInSegPtr-segPtr);
		}
	}
	g_sdlDoRefreshGfxOutput |= memcmp(g_sdlHostScrMemCache.cgaGfx, g_sdlHostScrMem.cgaGfx, sizeof(g_sdlHostScrMem.cgaGfx));
	memcpy(g_sdlHostScrMemCache.cgaGfx, g_sdlHostScrMem.cgaGfx, sizeof(g_sdlHostScrMem.cgaGfx));
}


void BE_ST_SetScreenMode(int mode)
{
	g_overscanBorderColorIndex = 0;
	g_sdlEGALastBGRABorderColor = g_sdlEGABGRAScreenColors[0];
	BEL_ST_VGAForceRefresh();
	switch (mode)
	{
	case 3:
	{
		g_sdlTexWidth = VGA_TXT_TEX_WIDTH;
		g_sdlTexHeight = VGA_TXT_TEX_HEIGHT;
		g_sdlTxtCursorPosX = g_sdlTxtCursorPosY = 0;
		// REFKEEN - Re-use BE_ST_clrscr with default text and background
		// colors, but ensure we preserve the previously set colors
		// for functions like BE_ST_cprintf (required for DEMOCAT)
		int origTxtColor = g_sdlTxtColor, origTxtBackground = g_sdlTxtBackground;
		g_sdlTxtColor = 7;
		g_sdlTxtBackground = 0;
		g_sdlTxtCursorEnabled = true;
		BE_ST_clrscr();
		g_sdlTxtColor = origTxtColor;
		g_sdlTxtBackground = origTxtBackground;
		break;
	}
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlHostScrMem.cgaGfx, 0, sizeof(g_sdlHostScrMem.cgaGfx));
		g_sdlHostScrMemCache.cgaGfx[0] = g_sdlHostScrMem.cgaGfx[0]^0xFF; // Force refresh
		break;
	case 0xD:
	case 0x13:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		if (mode == 0x13)
			memcpy(g_sdlEGACurrBGRAPalette, g_sdlVGADefaultBGRAScreenColors, sizeof(g_sdlVGADefaultBGRAScreenColors));
		else
			memcpy(g_sdlEGACurrBGRAPalette, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlPelPanning = 0;
		g_sdlPixLineWidth = 8*40;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if ((mode == 0xD) && (g_sdlScreenMode != 0xE))
		{
			memset(g_sdlVidMem.egaGfx, 0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx, 0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	case 0xE:
		g_sdlTexWidth = 2*GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPalette, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlPelPanning = 0;
		g_sdlPixLineWidth = 8*80;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if (g_sdlScreenMode != 0xD)
		{
			memset(g_sdlVidMem.egaGfx,  0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx,  0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	}
	g_sdlScreenMode = mode;
	BEL_ST_SetGfxOutputRects(true);
	BEL_ST_RecreateTexture();
}

void BE_ST_textcolor(int color)
{
	g_sdlTxtColor = color;
}

void BE_ST_textbackground(int color)
{
	g_sdlTxtBackground = color;
}

void BE_ST_clrscr(void)
{
	uint8_t *currMemByte = g_sdlVidMem.text;
	for (int i = 0; i < TXT_COLS_NUM*TXT_ROWS_NUM; ++i)
	{
		*(currMemByte++) = ' ';
		*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_MoveTextCursorTo(int x, int y)
{
	g_sdlTxtCursorPosX = x;
	g_sdlTxtCursorPosY = y;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_ToggleTextCursor(bool isEnabled)
{
	g_sdlTxtCursorEnabled = isEnabled;
}

static uint8_t *BEL_ST_CheckForTextCursorOverflow(uint8_t *currMemByte)
{
	// Go to next line
	if (g_sdlTxtCursorPosX == TXT_COLS_NUM)
	{
		g_sdlTxtCursorPosX = 0; // Carriage return
		++g_sdlTxtCursorPosY; // Line feed
		currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
	}
	// Shift screen by one line
	if (g_sdlTxtCursorPosY == TXT_ROWS_NUM)
	{
		--g_sdlTxtCursorPosY;
		// Scroll one line down
		uint8_t lastAttr = g_sdlVidMem.text[sizeof(g_sdlVidMem.text)-1];
		memmove(g_sdlVidMem.text, g_sdlVidMem.text+2*TXT_COLS_NUM, sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM);
		currMemByte = g_sdlVidMem.text+sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM;
		// New empty line
		for (int i = 0; i < TXT_COLS_NUM; ++i)
		{
			*(currMemByte++) = ' ';
			*(currMemByte++) = lastAttr;
		}
		currMemByte -= 2*TXT_COLS_NUM; // Go back to beginning of line
	}

	return currMemByte;
}

void BE_ST_RepeatCharWithColorAttributes(uint8_t ch, uint8_t attr, int count)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	while (count--)
	{
		*currMemByte++ = ch;
		*currMemByte++ = attr;
		currMemByte = BEL_ST_CheckForTextCursorOverflow(currMemByte);
	}
}

static uint8_t *BEL_ST_printchar(uint8_t *currMemByte, char ch, bool iscolored, bool requirecrchar)
{
	if (ch == '\t')
	{
		int nextCursorPosX = (g_sdlTxtCursorPosX & ~7) + 8;
		for (; g_sdlTxtCursorPosX != nextCursorPosX; ++g_sdlTxtCursorPosX)
		{
			*(currMemByte++) = ' ';
			*currMemByte = iscolored ? (g_sdlTxtColor | (g_sdlTxtBackground << 4)) : *currMemByte;
			++currMemByte;
		}
	}
	else if (ch == '\r')
	{
		if (!requirecrchar)
			return currMemByte; // Do nothing

		g_sdlTxtCursorPosX = 0; // Carriage return
		currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
	}
	else if (ch == '\n')
	{
		if (!requirecrchar)
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
		}
		++g_sdlTxtCursorPosY;
		currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	}
	else
	{
		*(currMemByte++) = ch;
		*currMemByte = iscolored ? (g_sdlTxtColor | (g_sdlTxtBackground << 4)) : *currMemByte;
		++currMemByte;
		++g_sdlTxtCursorPosX;
	}

	return BEL_ST_CheckForTextCursorOverflow(currMemByte);
}

static uint8_t *BEL_ST_printnumber(uint8_t *currMemByte, int64_t n, bool iscolored)
{
	// 64 bit number can be up to 20 base-10 characters in length
	char buffer[21] = {0};
	char *c = &buffer[21]-1;

	if (n < 0) // negative
	{
		do
		{
			*--c = '0' - (n % 10);
		}
		while (n /= 10);

		*--c = '-';
	}
	else // positive
	{
		do
		{
			*--c = '0' + (n % 10);
		}
		while (n /= 10);
	}

	do
	{
		currMemByte = BEL_ST_printchar(currMemByte, *c, iscolored, false);
	}
	while (*++c);

	return currMemByte;
}

void BE_ST_puts(const char *str)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		currMemByte = BEL_ST_printchar(currMemByte, *str, false, false);
	}
	BEL_ST_printchar(currMemByte, '\n', false, false);

	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_vprintf_impl(const char *format, va_list args, bool iscolored, bool requirecrchar);

void BE_ST_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	BEL_ST_vprintf_impl(format, args, false, false);
	va_end(args);
}

void BE_ST_cprintf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	BEL_ST_vprintf_impl(format, args, true, true);
	va_end(args);
}

void BE_ST_vprintf(const char *format, va_list args)
{
	BEL_ST_vprintf_impl(format, args, false, false);
}

// There's no colored version of vprintf in the original codebase

static void BEL_ST_vprintf_impl(const char *format, va_list args, bool iscolored, bool requirecrchar)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	while (*format)
	{
		if (*format == '%')
		{
			switch (*(++format))
			{
			case '%':
				currMemByte = BEL_ST_printchar(currMemByte, '%', iscolored, requirecrchar);
				break;
			case 's':
			{
				for (const char *str = va_arg(args, char *); *str; ++str)
				{
					currMemByte = BEL_ST_printchar(currMemByte, *str, iscolored, requirecrchar);
				}
				break;
			}
			case 'u':
				currMemByte = BEL_ST_printnumber(currMemByte, va_arg(args, unsigned int), iscolored);
				break;
			case 'd':
				currMemByte = BEL_ST_printnumber(currMemByte, va_arg(args, int), iscolored);
				break;
			case 'l':
			{
				switch(*(++format))
				{
				case 'u':
					currMemByte = BEL_ST_printnumber(currMemByte, va_arg(args, unsigned long), iscolored);
					break;
				case 'd':
					currMemByte = BEL_ST_printnumber(currMemByte, va_arg(args, long), iscolored);
					break;
				default:
				{
					// Do NOT constify this cause of hack...
					char errorMsg[] = "BEL_ST_vprintf_impl: Unsupported format specifier flag: lX\n";
					errorMsg[strlen(errorMsg)-2] = *format; // Hack
					BE_ST_ExitWithErrorMsg(errorMsg);
				}
				}
				break;
			}
			default:
			{
				// Do NOT constify this cause of hack...
				char errorMsg[] = "BEL_ST_vprintf_impl: Unsupported format specifier flag: X\n";
				errorMsg[strlen(errorMsg)-2] = *format; // Hack
				BE_ST_ExitWithErrorMsg(errorMsg);
			}
			}
		}
		else
		{
			currMemByte = BEL_ST_printchar(currMemByte, *format, iscolored, requirecrchar);
		}
		++format;
	}
	g_sdlDoRefreshGfxOutput = true;
}
