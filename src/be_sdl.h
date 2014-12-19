#ifndef	__BE_SDL__
#define __BE_SDL__

#include <stdbool.h>
#include <stdint.h>

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
	unsigned int farPtrSegOffset;
} RefKeenConfig;

extern RefKeenConfig g_refKeenCfg;

// Direct accesses to any of these functions should be minimized

/*** General ***/
void BE_SDL_InitAll(void);
void BE_SDL_ShutdownAll(void);
void BE_SDL_HandleExit(int status); // Replacement for exit function (useful for displaying text screen)
void BE_SDL_StartKeyboardService(void (*funcPtr)(uint8_t));
void BE_SDL_StopKeyboardService(void);
void BE_SDL_GetMouseDelta(int16_t *x, int16_t *y);
uint16_t BE_SDL_GetMouseButtons(void);
void BE_SDL_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp);
uint16_t BE_SDL_GetJoyButtons(uint16_t joy);

int16_t BE_SDL_KbHit(void);
int16_t BE_SDL_BiosScanCode(int16_t command);

void BE_SDL_PollEvents(void);

// Returns an offset that should be added to the 16-bit segments of 32-bit
// far pointers present in The Catacomb Armageddon/Apocalypse saved games
// (in the case of the original DOS exes, it depends on the locations of
// modified copies of them in memory)
uint16_t BE_SDL_Compat_GetFarPtrRelocationSegOffset(void);

/*** Audio/timer (vanilla Keen kind-of has these mixed) ***/
void BE_SDL_StartAudioSDService(void (*funcPtr)(void));
void BE_SDL_StopAudioSDService(void);
void BE_SDL_LockAudioRecursively(void);
void BE_SDL_UnlockAudioRecursively(void);
// Should be used in ID_SD.C only - Frequency is about 1193182Hz/spkVal
void BE_SDL_PCSpeakerOn(uint16_t spkVal);
void BE_SDL_PCSpeakerOff(void);
// Safe alternatives for Borland's sound and nosound functions from Catacomb Abyss' gelib.c
void BE_SDL_BSound(uint16_t frequency);
void BE_SDL_BNoSound(void);
// Drop-in replacement for id_sd.c:alOut
void BE_SDL_ALOut(uint8_t reg,uint8_t val);
// Here, the actual rate is about 1193182Hz/speed
// NOTE: isALMusicOn is irrelevant for Keen Dreams (even with its music code)
void BE_SDL_SetTimer(uint16_t speed, bool isALMusicOn);
uint32_t BE_SDL_GetTimeCount(void);
void BE_SDL_SetTimeCount(uint32_t newcount);
// Use this as a replacement for busy loops waiting for some ticks to pass
// e.g., "while (TimeCount-srctimecount<timetowait)
void BE_SDL_TimeCountWaitFromSrc(uint32_t srctimecount, int16_t timetowait);
// Same as above, but instead waits to reach dsttimecount
// e.g., a replacement for "while (TimeCount<dsttimecount)"
void BE_SDL_TimeCountWaitForDest(uint32_t dsttimecount);

/*** Graphics ***/
void BE_SDL_InitGfx(void);
void BE_SDL_ShutdownGfx(void);
void BE_SDL_SetScreenStartAddress(uint16_t crtc);

// ***WARNING*** SEE WARNING BELOW BEFORE USING!!!
//
// Basically a replacement for B800:0000, points to a 80x25*2 bytes long
// buffer; Text mode only, NOT e.g., CGA graphics.
//
// ***WARNING***: After modifying this chunk, it is a MUST to call the function
// BE_SDL_MarkGfxForUpdate (used as an optimization).
uint8_t *BE_SDL_GetTextModeMemoryPtr(void);

// EGA graphics manipulations
void BE_SDL_EGASetPaletteAndBorder(const uint8_t *palette);
void BE_SDL_EGASetLineWidth(uint8_t widthInBytes);
void BE_SDL_EGASetSplitScreen(int16_t linenum);
void BE_SDL_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t planeMask);
// Same as BE_SDL_EGAUpdateGFXByte but picking specific bits out of each byte, and WITHOUT planes mask
void BE_SDL_EGAUpdateGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask);
void BE_SDL_EGAUpdateGFXBuffer(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeMask);
void BE_SDL_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff);
// Same as BE_SDL_EGAUpdateGFXByteScrToScr but with plane mask (added for Catacomb Abyss vanilla bug reproduction/workaround)
void BE_SDL_EGAUpdateGFXByteWithPlaneMaskScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t planeMask);
// Same as BE_SDL_EGAUpdateGFXByteScrToScr but picking specific bits out of each byte
void BE_SDL_EGAUpdateGFXBitsScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask);
void BE_SDL_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num);
uint8_t BE_SDL_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum);
void BE_SDL_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum);
void BE_SDL_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint8_t bitsMask);
void BE_SDL_EGAUpdateGFXPixel4bppRepeatedly(uint16_t destOff, uint8_t color, uint16_t count, uint8_t bitsMask);
void BE_SDL_EGAXorGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t planeMask);
// Like BE_SDL_EGAXorGFXByte, but:
// - OR instead of XOR.
// - All planes are updated.
// - Only specific bits are updated in each plane's byte.
void BE_SDL_EGAOrGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask);
// CGA graphics manipulations
void BE_SDL_CGAFullUpdateFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth);


//
void BE_SDL_SetBorderColor(uint8_t color);
void BE_SDL_SetScreenMode(int mode);
void BE_SDL_WaitVBL(int16_t number);
void BE_SDL_ShortSleep(void);
void BE_SDL_Delay(uint16_t msec); // Replacement for delay from dos.h
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

#endif
