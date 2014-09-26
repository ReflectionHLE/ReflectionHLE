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
// Frequency is about 1193182Hz/spkVal
void BE_SDL_PCSpeakerOn(id0_word_t spkVal);
void BE_SDL_PCSpeakerOff(void);
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
id0_byte_t *BE_SDL_GetEGAMemoryPtr(void);
void BE_SDL_SetBorderColor(id0_byte_t color);
void BE_SDL_SetScreenMode(int mode);
void BE_SDL_WaitVBL(id0_int_t number);
void BE_SDL_ShortSleep(void);

void BE_SDL_textcolor(int color);
void BE_SDL_textbackground(int color);
void BE_SDL_clrscr(void);
void BE_SDL_MoveTextCursorTo(int x, int y);
// Replacement for puts function that prints to emulated text mode memory
void BE_SDL_puts(const char *str);
// Limited replacement for printf, does NOT handle formatting (apart from '\n')
void BE_SDL_simplified_printf(const char *str);

void BE_SDL_MarkGfxForUpdate(void);
void BE_SDL_MarkGfxForPendingUpdate(void); // Marks only if BE_SDL_MarkGfxForUpdate has been called, or after setting emulated video mode

#endif
