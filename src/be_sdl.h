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
} ChocolateKeenConfig;

extern ChocolateKeenConfig g_chocolateKeenCfg;

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

void BE_SDL_PollEvents(void);

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

/*** Graphics ***/
void BE_SDL_InitGfx(void);
void BE_SDL_ShutdownGfx(void);
void BE_SDL_SetScreenStartAddress(uint16_t crtc);
uint8_t *BE_SDL_GetTextModeMemoryPtr(void);
uint8_t *BE_SDL_GetCGAMemoryPtr(void);
// EGA graphics manipulations
void BE_SDL_EGASetPaletteAndBorder(const uint8_t *palette);
void BE_SDL_EGASetLineWidth(uint8_t widthInBytes);
void BE_SDL_EGASetSplitScreen(int16_t linenum);
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
void BE_SDL_SetBorderColor(uint8_t color);
void BE_SDL_SetScreenMode(int mode);
void BE_SDL_WaitVBL(int16_t number);
void BE_SDL_ShortSleep(void);
void BE_SDL_Delay(uint16_t msec); // Replacement for delay from dos.h
// Use this ONLY in Catacombs' CalcTics (from ThreeDRefresh):
// While ID_RF's RF_Refresh (from Keen Dreams) indirectly leads to wait for
// vsync on original hardware in VW_SetScreen, along with an addition wait
// from RF_CalcTics, Catacombs' ThreeDRefresh and CalcTics don't do either.
// This implies visual glitches are probably expected if the framerate goes
// a bit above 70fps, and the game's PlayLoop may execute for many times on
// sufficiently fast machines (at least 500 times a second). And so:
//
// Use this ONLY in Catacombs' CalcTics (from ThreeDRefresh), to simulate
// some wait relevant on older machines
void BE_SDL_ThreeDRefreshSleep(void);

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
