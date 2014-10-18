/* Copyright (C) 2014 NY00123
 *
 * This file is part of Chocolate Keen Dreams.
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

#ifndef	__BE_SDL__
#define __BE_SDL__

// WARNING: This struct is used by BE_SDL ONLY

typedef struct
{
	bool isFullscreen;
	int fullWidth, fullHeight;
	int winWidth, winHeight;
	int displayNum;
	int sdlRendererDriver;
	enum { VSYNC_AUTO, VSYNC_OFF, VSYNC_ON } vSync;
	bool isBilinear;
	enum { SCALE_ASPECT, SCALE_FILL } scaleType;
	int scaleFactor;
	bool autolockCursor;
	int sndSampleRate;
} ChocolateKeenConfig;

extern ChocolateKeenConfig g_chocolateKeenCfg;

// Direct accesses to any of these functions should be minimized

/*** General ***/
void BE_SDL_InitAll(void);
void BE_SDL_ShutdownAll(void);
void BE_SDL_HandleExit(int status); // Replacement for exit function (useful for displaying text screen)
void BE_SDL_StartKeyboardService(void (*funcPtr)(id0_byte_t));
void BE_SDL_StopKeyboardService(void);
void BE_SDL_GetMouseDelta(id0_int_t *x, id0_int_t *y);
id0_word_t BE_SDL_GetMouseButtons(void);
void BE_SDL_GetJoyAbs(id0_word_t joy, id0_word_t *xp, id0_word_t *yp);
id0_word_t BE_SDL_GetJoyButtons(id0_word_t joy);

void BE_SDL_PollEvents(void);

/*** Audio/timer (vanilla Keen kind-of has these mixed) ***/
void BE_SDL_StartAudioSDService(void (*funcPtr)(void));
void BE_SDL_StopAudioSDService(void);
void BE_SDL_LockAudioRecursively(void);
void BE_SDL_UnlockAudioRecursively(void);
// Should be used in ID_SD.C only - Frequency is about 1193182Hz/spkVal
void BE_SDL_PCSpeakerOn(id0_word_t spkVal);
void BE_SDL_PCSpeakerOff(void);
// Safe alternatives for Borland's sound and nosoudn functions from Catacomb Abyss' gelib.c
void BE_SDL_BSound(id0_word_t frequency);
void BE_SDL_BNoSound(void);
// Drop-in replacement for id_sd.c:alOut
void BE_SDL_ALOut(id0_byte_t reg,id0_byte_t val);
// Here, the actual rate is about 1193182Hz/speed
// NOTE: isALMusicOn is irrelevant for Keen Dreams (even with its music code)
void BE_SDL_SetTimer(id0_word_t speed, bool isALMusicOn);
id0_longword_t BE_SDL_GetTimeCount(void);
void BE_SDL_SetTimeCount(id0_longword_t newcount);

/*** Graphics ***/
void BE_SDL_InitGfx(void);
void BE_SDL_ShutdownGfx(void);
void BE_SDL_SetScreenStartAddress(id0_unsigned_t crtc);
id0_byte_t *BE_SDL_GetTextModeMemoryPtr(void);
id0_byte_t *BE_SDL_GetCGAMemoryPtr(void);
// EGA graphics manipulations
void BE_SDL_EGASetPaletteAndBorder(const id0_char_t *palette);
void BE_SDL_EGASetLineWidth(id0_byte_t widthInBytes);
void BE_SDL_EGASetSplitScreen(id0_int_t linenum);
void BE_SDL_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask);
// Same as BE_SDL_EGAUpdateGFXByte but picking specific bits out of each byte, and WITHOUT planes mask
void BE_SDL_EGAUpdateGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask);
void BE_SDL_EGAUpdateGFXBuffer(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t mask);
void BE_SDL_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff);
// Same as BE_SDL_EGAUpdateGFXByteScrToScr but picking specific bits out of each byte
void BE_SDL_EGAUpdateGFXBitsScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask);
void BE_SDL_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num);
uint8_t BE_SDL_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum);
void BE_SDL_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum);
void BE_SDL_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint16_t mask);
void BE_SDL_EGAUpdateGFXPixel4bppRepeatedly(uint16_t destOff, uint8_t color, uint16_t count, uint16_t mask);
void BE_SDL_EGAXorGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask);
// Like BE_SDL_EGAXorGFXByte, but:
// - OR instead of XOR.
// - Only specific bits are updated in each plane's byte.
void BE_SDL_EGAOrGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask);


//
void BE_SDL_SetBorderColor(id0_byte_t color);
void BE_SDL_SetScreenMode(int mode);
void BE_SDL_WaitVBL(id0_int_t number);
void BE_SDL_ShortSleep(void);
void BE_SDL_Delay(id0_unsigned_t msec); // Replacement for delay from dos.h

void BE_SDL_textcolor(int color);
void BE_SDL_textbackground(int color);
void BE_SDL_clrscr(void);
void BE_SDL_MoveTextCursorTo(int x, int y);
void BE_SDL_ToggleTextCursor(bool isEnabled);
// Replacement for puts function that prints to emulated text mode memory
void BE_SDL_puts(const char *str);
// Limited replacement for printf, does NOT handle formatting (apart from '\n')
void BE_SDL_simplified_printf(const char *str);

void BE_SDL_MarkGfxForUpdate(void);
void BE_SDL_MarkGfxForPendingUpdate(void); // Marks only if BE_SDL_MarkGfxForUpdate has been called, or after setting emulated video mode

#endif
