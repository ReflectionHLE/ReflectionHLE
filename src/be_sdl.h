#ifndef	__BE_SDL__
#define __BE_SDL__

#include <stdbool.h>
#include <stdint.h>

// WARNING: This struct is used by BE_SDL ONLY

typedef enum { VSYNC_AUTO, VSYNC_OFF, VSYNC_ON } VSyncSettingType;
typedef enum { SCALE_ASPECT, SCALE_FILL } ScaleTypeSettingType;

typedef struct
{
	bool isFullscreen;
	int fullWidth, fullHeight;
	int winWidth, winHeight;
	int displayNum;
	int sdlRendererDriver;
	VSyncSettingType vSync;
	bool isBilinear;
	ScaleTypeSettingType scaleType;
	int scaleFactor;
	bool autolockCursor;
	int sndSampleRate;
	bool disableSoundSubSystem;
	struct
	{
		bool isEnabled;
		int actionMappings[8]; // Buttons/triggers
		int movementMapping; // D-pad, left stick or right stick (or none)
		int menuMouseMapping; // One of the two sticks (or none)
	} altControlScheme;
	unsigned int farPtrSegOffset; // Actually used just in The Catacomb Armageddon/Apocalypse
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

// Various controller schemes are saved in a stack, so it's straight-forward
// to revert to an arbitrary preceding scheme when desired.
//
// Note: Do push a new scheme before the first time picking a new scheme
// (an internal default as filled for the very first one before the push).
void BE_SDL_AltControlScheme_Push(void);
void BE_SDL_AltControlScheme_Pop(void);
// Replace current controller scheme using any of these
void BE_SDL_AltControlScheme_PrepareFaceButtonsDOSScancodes(const char *scanCodes);
void BE_SDL_AltControlScheme_PreparePageScrollingControls(int prevPageScan, int nextPageScan);
void BE_SDL_AltControlScheme_PrepareMenuControls(void);
void BE_SDL_AltControlScheme_PrepareInGameControls(int primaryScanCode, int secondaryScanCode, int upScanCode, int downScanCode, int leftScanCode, int rightScanCode);
void BE_SDL_AltControlScheme_PrepareInputWaitControls(void); // When there's a simple wait for user input, not anything specific...
void BE_SDL_AltControlScheme_PrepareTextInput(void);


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
void BE_SDL_PrepareForManualAudioSDServiceCall(void);
bool BE_SDL_IsEmulatedOPLChipReady(void);
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
// e.g., "while (TimeCount-srctimecount<timetowait)"
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
void BE_SDL_EGASetPelPanning(uint8_t panning);
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
// Replacements for functions that print to (emulated) text mode memory
// (Note: Probably just partially implemented)
void BE_SDL_puts(const char *str);
void BE_SDL_printf(const char *str, ...);
void BE_SDL_vprintf(const char *str, va_list args);
void BE_SDL_cprintf(const char *str, ...); // Non-standard

void BE_SDL_MarkGfxForUpdate(void);

// Internally used by BE_SDL

typedef enum EmulatedKeyScancode_T {
     EMULATEDKEYSCANCODE_ESC = 1,
     EMULATEDKEYSCANCODE_1,
     EMULATEDKEYSCANCODE_2,
     EMULATEDKEYSCANCODE_3,
     EMULATEDKEYSCANCODE_4,
     EMULATEDKEYSCANCODE_5,
     EMULATEDKEYSCANCODE_6,
     EMULATEDKEYSCANCODE_7,
     EMULATEDKEYSCANCODE_8,
     EMULATEDKEYSCANCODE_9,
     EMULATEDKEYSCANCODE_0, // 0Bh
     EMULATEDKEYSCANCODE_MINUS,
     EMULATEDKEYSCANCODE_EQUALS,
     EMULATEDKEYSCANCODE_BSPACE,
     EMULATEDKEYSCANCODE_TAB,
     EMULATEDKEYSCANCODE_Q, // 10h
     EMULATEDKEYSCANCODE_W,
     EMULATEDKEYSCANCODE_E,
     EMULATEDKEYSCANCODE_R,
     EMULATEDKEYSCANCODE_T,
     EMULATEDKEYSCANCODE_Y,
     EMULATEDKEYSCANCODE_U,
     EMULATEDKEYSCANCODE_I,
     EMULATEDKEYSCANCODE_O,
     EMULATEDKEYSCANCODE_P,
     EMULATEDKEYSCANCODE_LBRACKET,
     EMULATEDKEYSCANCODE_RBRACKET,
     EMULATEDKEYSCANCODE_ENTER,
     EMULATEDKEYSCANCODE_LCTRL,
     EMULATEDKEYSCANCODE_A, // 1Eh
     EMULATEDKEYSCANCODE_S,
     EMULATEDKEYSCANCODE_D,
     EMULATEDKEYSCANCODE_F,
     EMULATEDKEYSCANCODE_G,
     EMULATEDKEYSCANCODE_H,
     EMULATEDKEYSCANCODE_J,
     EMULATEDKEYSCANCODE_K,
     EMULATEDKEYSCANCODE_L,
     EMULATEDKEYSCANCODE_SEMICOLON,
     EMULATEDKEYSCANCODE_QUOTE,
     EMULATEDKEYSCANCODE_GRAVE,
     EMULATEDKEYSCANCODE_LSHIFT,
     EMULATEDKEYSCANCODE_BACKSLASH,
     EMULATEDKEYSCANCODE_Z, // 2Ch
     EMULATEDKEYSCANCODE_X,
     EMULATEDKEYSCANCODE_C,
     EMULATEDKEYSCANCODE_V,
     EMULATEDKEYSCANCODE_B,
     EMULATEDKEYSCANCODE_N,
     EMULATEDKEYSCANCODE_M,
     EMULATEDKEYSCANCODE_COMMA,
     EMULATEDKEYSCANCODE_PERIOD,
     EMULATEDKEYSCANCODE_SLASH,
     EMULATEDKEYSCANCODE_RSHIFT,
     EMULATEDKEYSCANCODE_PRINTSCREEN, // 37h but kind of special
     EMULATEDKEYSCANCODE_LALT,
     EMULATEDKEYSCANCODE_SPACE,
     EMULATEDKEYSCANCODE_CAPSLOCK,
     EMULATEDKEYSCANCODE_F1, // 3Bh
     EMULATEDKEYSCANCODE_F2,
     EMULATEDKEYSCANCODE_F3,
     EMULATEDKEYSCANCODE_F4,
     EMULATEDKEYSCANCODE_F5,
     EMULATEDKEYSCANCODE_F6,
     EMULATEDKEYSCANCODE_F7,
     EMULATEDKEYSCANCODE_F8,
     EMULATEDKEYSCANCODE_F9,
     EMULATEDKEYSCANCODE_F10,
     EMULATEDKEYSCANCODE_NUMLOCK, // 45h
     EMULATEDKEYSCANCODE_SCROLLLOCK,
     EMULATEDKEYSCANCODE_KP_7,
     EMULATEDKEYSCANCODE_KP_8,
     EMULATEDKEYSCANCODE_KP_9,
     EMULATEDKEYSCANCODE_KP_MINUS,
     EMULATEDKEYSCANCODE_KP_4,
     EMULATEDKEYSCANCODE_KP_5,
     EMULATEDKEYSCANCODE_KP_6,
     EMULATEDKEYSCANCODE_KP_PLUS,
     EMULATEDKEYSCANCODE_KP_1,
     EMULATEDKEYSCANCODE_KP_2,
     EMULATEDKEYSCANCODE_KP_3,
     EMULATEDKEYSCANCODE_KP_0,
     EMULATEDKEYSCANCODE_KP_PERIOD, // 53h
     // A couple of "special" keys (scancode E0h sent first)
     EMULATEDKEYSCANCODE_KP_DIVIDE = 0x35,
     EMULATEDKEYSCANCODE_KP_ENTER = 0x1C,
     // Back to a few "non-special" keys
     EMULATEDKEYSCANCODE_F11 = 0x57,
     EMULATEDKEYSCANCODE_F12 = 0x58,
     // And again special keys
     EMULATEDKEYSCANCODE_INSERT = 0x52,
     EMULATEDKEYSCANCODE_DELETE = 0x53,
     EMULATEDKEYSCANCODE_HOME = 0x47,
     EMULATEDKEYSCANCODE_END = 0x4F,
     EMULATEDKEYSCANCODE_PAGEUP = 0x49,
     EMULATEDKEYSCANCODE_PAGEDOWN = 0x51,
     EMULATEDKEYSCANCODE_LEFT = 0x4B,
     EMULATEDKEYSCANCODE_RIGHT = 0x4D,
     EMULATEDKEYSCANCODE_UP = 0x48,
     EMULATEDKEYSCANCODE_DOWN = 0x50,
     EMULATEDKEYSCANCODE_RALT = 0x38,
     EMULATEDKEYSCANCODE_RCTRL = 0x1D,
     // Two extra kes
     EMULATEDKEYSCANCODE_LESSTHAN = 0x56,
     EMULATEDKEYSCANCODE_KP_MULTIPLY = 0x37,
     // This one is different from all the rest (6 scancodes sent on press ONLY)
     EMULATEDKEYSCANCODE_PAUSE = 0xE1,
} EmulatedKeyScancode_T;

#endif
