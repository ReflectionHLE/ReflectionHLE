/* Copyright (C) 2014-2015 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef	_BE_ST_
#define _BE_ST_

#include <stdbool.h>
#include <stdint.h>

#include "be_st_sdl.h"
#include "be_launcher.h"

#define BE_ST_MAXJOYSTICKS 8

/*** General ***/
void BE_ST_InitCommon(void); // Before game or launcher
void BE_ST_PrepareForGameStartup(void); // Before game
void BE_ST_ShutdownAll(void); // After game
void BE_ST_HandleExit(int status); // Replacement for exit function (useful for displaying text screen)
void BE_ST_QuickExit(void); // Where the usual exit handler isn't sufficient: Saves last settings, shutdowns subsystems and then exits immediately
void BE_ST_StartKeyboardService(void (*funcPtr)(uint8_t));
void BE_ST_StopKeyboardService(void);
void BE_ST_GetMouseDelta(int16_t *x, int16_t *y);
uint16_t BE_ST_GetMouseButtons(void);
void BE_ST_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp);
uint16_t BE_ST_GetJoyButtons(uint16_t joy);

int16_t BE_ST_KbHit(void);
int16_t BE_ST_BiosScanCode(int16_t command);

// Used internally, or alternatively for new errors: Logs to emulated text
// screen *and* possibly also somewhere else (e.g., actual stdout). Note that
// this function does *not* access a variable number of arguments like printf,
// since format specifiers may differ between the two methods of outputs.
void BE_ST_ExitWithErrorMsg(const char *msg);


/*** Alternative controller schemes ***/
// The XInput Controller layout, using details given by the
// SDL_GameController API, is used as a base for the controller layout.
typedef enum {
	BE_ST_CTRL_BUT_A = 0,
	BE_ST_CTRL_BUT_B,
	BE_ST_CTRL_BUT_X,
	BE_ST_CTRL_BUT_Y,
	BE_ST_CTRL_BUT_BACK,
	BE_ST_CTRL_BUT_GUIDE, // Shouldn't be used, but defined for consistency with SDL 2.0
	BE_ST_CTRL_BUT_START,
	BE_ST_CTRL_BUT_LSTICK,
	BE_ST_CTRL_BUT_RSTICK,
	BE_ST_CTRL_BUT_LSHOULDER,
	BE_ST_CTRL_BUT_RSHOULDER,
	BE_ST_CTRL_BUT_DPAD_UP,
	BE_ST_CTRL_BUT_DPAD_DOWN,
	BE_ST_CTRL_BUT_DPAD_LEFT,
	BE_ST_CTRL_BUT_DPAD_RIGHT,
	BE_ST_CTRL_BUT_MAX,
} BE_ST_ControllerDigiButton;

typedef enum {
	BE_ST_CTRL_AXIS_LX = 0,
	BE_ST_CTRL_AXIS_LY,
	BE_ST_CTRL_AXIS_RX,
	BE_ST_CTRL_AXIS_RY,
	BE_ST_CTRL_AXIS_LTRIGGER,
	BE_ST_CTRL_AXIS_RTRIGGER,
	BE_ST_CTRL_AXIS_MAX,
} BE_ST_ControllerAxis;

typedef enum {
	BE_ST_CTRL_MAP_NONE = 0, // Should be set to 0 for e.g., default initializations
	BE_ST_CTRL_MAP_KEYSCANCODE,
	BE_ST_CTRL_MAP_MOUSEBUTTON,
	BE_ST_CTRL_MAP_MOUSEMOTION,
	BE_ST_CTRL_MAP_OTHERMAPPING,
	//BE_ST_CONTROLLER_MAPPING_TEXTINPUTTOGGLE,
	//BE_ST_CONTROLLER_MAPPING_DEBUGKEYSTOGGLE,
} BE_ST_ControllerSingleMapClass;

// BE_ST_ControllerMapping is what defines a mapping, consisting of arrays
// of BE_ST_ControllerSingleMap. Note that an array element may have
// a pointer to another BE_ST_ControllerMapping.

struct BE_ST_ControllerMapping;

typedef struct {
	const struct BE_ST_ControllerMapping *otherMappingPtr;
	int val;
	int secondaryVal; // Used for mouse motion emulation
	BE_ST_ControllerSingleMapClass mapClass;
} BE_ST_ControllerSingleMap;

typedef struct BE_ST_ControllerMapping {
	// Set to non-NULL if toggling non-mapped action
	// (generally digital buttons only, but MAY include the analog triggers)
	struct BE_ST_ControllerMapping *prevMapping;

	BE_ST_ControllerSingleMap buttons[BE_ST_CTRL_BUT_MAX];
	BE_ST_ControllerSingleMap axes[BE_ST_CTRL_AXIS_MAX][2];
	bool showUi;
} BE_ST_ControllerMapping;

// Various controller schemes are saved in a stack, so it's straight-forward
// to revert to an arbitrary preceding scheme when desired.
//
// Note: Do push a new scheme before the first time picking a new scheme
// (an internal default as filled for the very first one before the push).
void BE_ST_AltControlScheme_Push(void);
void BE_ST_AltControlScheme_Pop(void);
// Replace current controller scheme using any of these
void BE_ST_AltControlScheme_PrepareControllerMapping(const BE_ST_ControllerMapping *mapping);

// HACK - Pass corresponding pointer to PrepareControllerMapping for on-screen keyboard
// (can also be used to go from one mapping to another, e.g., showing debug keys in-game)
extern BE_ST_ControllerMapping g_beStControllerMappingTextInput;
extern BE_ST_ControllerMapping g_beStControllerMappingDebugKeys;


// Used when loading controller scheme stuff from cfg
enum {
	BE_ST_CTRL_CFG_BUTMAP_BEFOREFIRST = -1, /* The actual first entry is to be numbered 0 */
#ifdef REFKEEN_VER_KDREAMS
	BE_ST_CTRL_CFG_BUTMAP_JUMP,
	BE_ST_CTRL_CFG_BUTMAP_THROW,
	BE_ST_CTRL_CFG_BUTMAP_STATS,
#else
	BE_ST_CTRL_CFG_BUTMAP_FIRE,
	BE_ST_CTRL_CFG_BUTMAP_STRAFE,
	BE_ST_CTRL_CFG_BUTMAP_DRINK,
	BE_ST_CTRL_CFG_BUTMAP_BOLT, // Zapper in the Adventures Series
	BE_ST_CTRL_CFG_BUTMAP_NUKE, // Xterminator in the Adventures Series
	BE_ST_CTRL_CFG_BUTMAP_FASTTURN,
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	BE_ST_CTRL_CFG_BUTMAP_SCROLLS,
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS,
#endif
	BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS,
	BE_ST_CTRL_CFG_BUTMAP_AFTERLAST,
};

// Used by launcher for controller button selection
void BE_ST_Launcher_WaitForControllerButton(BEMenuItem *menuItem);


void BE_ST_PollEvents(void);

// Launcher loop
void BE_ST_Launcher_RunEventLoop(void);

// Returns an offset that should be added to the 16-bit segments of 32-bit
// far pointers present in The Catacomb Armageddon/Apocalypse saved games
// (in the case of the original DOS exes, it depends on the locations of
// modified copies of them in memory)
uint16_t BE_ST_Compat_GetFarPtrRelocationSegOffset(void);

/*** Audio/timer (vanilla Keen kind-of has these mixed) ***/
void BE_ST_StartAudioSDService(void (*funcPtr)(void));
void BE_ST_StopAudioSDService(void);
void BE_ST_LockAudioRecursively(void);
void BE_ST_UnlockAudioRecursively(void);
void BE_ST_PrepareForManualAudioSDServiceCall(void);
bool BE_ST_IsEmulatedOPLChipReady(void);
// Should be used in ID_SD.C only - Frequency is about 1193182Hz/spkVal
void BE_ST_PCSpeakerOn(uint16_t spkVal);
void BE_ST_PCSpeakerOff(void);
// Safe alternatives for Borland's sound and nosound functions from Catacomb Abyss' gelib.c
void BE_ST_BSound(uint16_t frequency);
void BE_ST_BNoSound(void);
// Drop-in replacement for id_sd.c:alOut
void BE_ST_ALOut(uint8_t reg,uint8_t val);
// Here, the actual rate is about 1193182Hz/speed
// NOTE: isALMusicOn is irrelevant for Keen Dreams (even with its music code)
void BE_ST_SetTimer(uint16_t speed, bool isALMusicOn);
uint32_t BE_ST_GetTimeCount(void);
void BE_ST_SetTimeCount(uint32_t newcount);
// Use this as a replacement for busy loops waiting for some ticks to pass
// e.g., "while (TimeCount-srctimecount<timetowait)"
void BE_ST_TimeCountWaitFromSrc(uint32_t srctimecount, int16_t timetowait);
// Same as above, but instead waits to reach dsttimecount
// e.g., a replacement for "while (TimeCount<dsttimecount)"
void BE_ST_TimeCountWaitForDest(uint32_t dsttimecount);

/*** Graphics ***/
void BE_ST_InitGfx(void);
void BE_ST_ShutdownGfx(void);
void BE_ST_SetScreenStartAddress(uint16_t crtc);

void BE_ST_MarkGfxForUpdate(void);
void BE_ST_Launcher_MarkGfxCache(void);

// ***WARNING*** SEE WARNING BELOW BEFORE USING!!!
//
// Basically a replacement for B800:0000, points to a 80x25*2 bytes long
// buffer; Text mode only, NOT e.g., CGA graphics.
//
// ***WARNING***: After modifying this chunk, it is a MUST to call the function
// BE_ST_MarkGfxForUpdate (used as an optimization).
uint8_t *BE_ST_GetTextModeMemoryPtr(void);

// ***WARNING***: Ensure BE_ST_Launcher_MarkGfxCache is called after drawing.
uint8_t *BE_ST_Launcher_GetGfxPtr(void);

// EGA graphics manipulations
void BE_ST_EGASetPaletteAndBorder(const uint8_t *palette);
void BE_ST_EGASetLineWidth(uint8_t widthInBytes);
void BE_ST_EGASetSplitScreen(int16_t linenum);
void BE_ST_EGASetPelPanning(uint8_t panning);
void BE_ST_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t planeMask);
// Same as BE_ST_EGAUpdateGFXByte but picking specific bits out of each byte, and WITHOUT planes mask
void BE_ST_EGAUpdateGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask);
void BE_ST_EGAUpdateGFXBuffer(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeMask);
void BE_ST_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff);
// Same as BE_ST_EGAUpdateGFXByteScrToScr but with plane mask (added for Catacomb Abyss vanilla bug reproduction/workaround)
void BE_ST_EGAUpdateGFXByteWithPlaneMaskScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t planeMask);
// Same as BE_ST_EGAUpdateGFXByteScrToScr but picking specific bits out of each byte
void BE_ST_EGAUpdateGFXBitsScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask);
void BE_ST_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num);
uint8_t BE_ST_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum);
void BE_ST_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum);
void BE_ST_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint8_t bitsMask);
void BE_ST_EGAUpdateGFXPixel4bppRepeatedly(uint16_t destOff, uint8_t color, uint16_t count, uint8_t bitsMask);
void BE_ST_EGAXorGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t planeMask);
// Like BE_ST_EGAXorGFXByte, but:
// - OR instead of XOR.
// - All planes are updated.
// - Only specific bits are updated in each plane's byte.
void BE_ST_EGAOrGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask);
// CGA graphics manipulations
void BE_ST_CGAFullUpdateFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth);

//
void BE_ST_Launcher_Prepare(void);
void BE_ST_Launcher_Shutdown(void);
void BE_ST_Launcher_RefreshSelectGameMenuContents(void);

//
void BE_ST_SetBorderColor(uint8_t color);
void BE_ST_SetScreenMode(int mode);
void BE_ST_WaitVBL(int16_t number);
void BE_ST_ShortSleep(void);
void BE_ST_Delay(uint16_t msec); // Replacement for delay from dos.h
void BE_ST_textcolor(int color);
void BE_ST_textbackground(int color);
void BE_ST_clrscr(void);
void BE_ST_MoveTextCursorTo(int x, int y);
void BE_ST_ToggleTextCursor(bool isEnabled);
// Replacements for functions that print to (emulated) text mode memory
// (Note: Probably just partially implemented)
void BE_ST_puts(const char *str);
void BE_ST_printf(const char *str, ...);
void BE_ST_vprintf(const char *str, va_list args);
void BE_ST_cprintf(const char *str, ...); // Non-standard

// Used with BE_ST

typedef enum BE_ST_ScanCode_T {
     BE_ST_SC_ESC = 1,
     BE_ST_SC_1,
     BE_ST_SC_2,
     BE_ST_SC_3,
     BE_ST_SC_4,
     BE_ST_SC_5,
     BE_ST_SC_6,
     BE_ST_SC_7,
     BE_ST_SC_8,
     BE_ST_SC_9,
     BE_ST_SC_0, // 0Bh
     BE_ST_SC_MINUS,
     BE_ST_SC_EQUALS,
     BE_ST_SC_BSPACE,
     BE_ST_SC_TAB,
     BE_ST_SC_Q, // 10h
     BE_ST_SC_W,
     BE_ST_SC_E,
     BE_ST_SC_R,
     BE_ST_SC_T,
     BE_ST_SC_Y,
     BE_ST_SC_U,
     BE_ST_SC_I,
     BE_ST_SC_O,
     BE_ST_SC_P,
     BE_ST_SC_LBRACKET,
     BE_ST_SC_RBRACKET,
     BE_ST_SC_ENTER,
     BE_ST_SC_LCTRL,
     BE_ST_SC_A, // 1Eh
     BE_ST_SC_S,
     BE_ST_SC_D,
     BE_ST_SC_F,
     BE_ST_SC_G,
     BE_ST_SC_H,
     BE_ST_SC_J,
     BE_ST_SC_K,
     BE_ST_SC_L,
     BE_ST_SC_SEMICOLON,
     BE_ST_SC_QUOTE,
     BE_ST_SC_GRAVE,
     BE_ST_SC_LSHIFT,
     BE_ST_SC_BACKSLASH,
     BE_ST_SC_Z, // 2Ch
     BE_ST_SC_X,
     BE_ST_SC_C,
     BE_ST_SC_V,
     BE_ST_SC_B,
     BE_ST_SC_N,
     BE_ST_SC_M,
     BE_ST_SC_COMMA,
     BE_ST_SC_PERIOD,
     BE_ST_SC_SLASH,
     BE_ST_SC_RSHIFT,
     BE_ST_SC_PRINTSCREEN, // 37h but kind of special
     BE_ST_SC_LALT,
     BE_ST_SC_SPACE,
     BE_ST_SC_CAPSLOCK,
     BE_ST_SC_F1, // 3Bh
     BE_ST_SC_F2,
     BE_ST_SC_F3,
     BE_ST_SC_F4,
     BE_ST_SC_F5,
     BE_ST_SC_F6,
     BE_ST_SC_F7,
     BE_ST_SC_F8,
     BE_ST_SC_F9,
     BE_ST_SC_F10,
     BE_ST_SC_NUMLOCK, // 45h
     BE_ST_SC_SCROLLLOCK,
     BE_ST_SC_KP_7,
     BE_ST_SC_KP_8,
     BE_ST_SC_KP_9,
     BE_ST_SC_KP_MINUS,
     BE_ST_SC_KP_4,
     BE_ST_SC_KP_5,
     BE_ST_SC_KP_6,
     BE_ST_SC_KP_PLUS,
     BE_ST_SC_KP_1,
     BE_ST_SC_KP_2,
     BE_ST_SC_KP_3,
     BE_ST_SC_KP_0,
     BE_ST_SC_KP_PERIOD, // 53h
     // A couple of "special" keys (scancode E0h sent first)
     BE_ST_SC_KP_DIVIDE = 0x35,
     BE_ST_SC_KP_ENTER = 0x1C,
     // Back to a few "non-special" keys
     BE_ST_SC_F11 = 0x57,
     BE_ST_SC_F12 = 0x58,
     // And again special keys
     BE_ST_SC_INSERT = 0x52,
     BE_ST_SC_DELETE = 0x53,
     BE_ST_SC_HOME = 0x47,
     BE_ST_SC_END = 0x4F,
     BE_ST_SC_PAGEUP = 0x49,
     BE_ST_SC_PAGEDOWN = 0x51,
     BE_ST_SC_LEFT = 0x4B,
     BE_ST_SC_RIGHT = 0x4D,
     BE_ST_SC_UP = 0x48,
     BE_ST_SC_DOWN = 0x50,
     BE_ST_SC_RALT = 0x38,
     BE_ST_SC_RCTRL = 0x1D,
     // Two extra keys
     BE_ST_SC_LESSTHAN = 0x56,
     BE_ST_SC_KP_MULTIPLY = 0x37,
     // This one is different from all the rest (6 scancodes sent on press ONLY)
     BE_ST_SC_PAUSE = 0xE1,

     // SPECIAL - Used to mark maximum, may have to update if 0xFF is actually used
     BE_ST_SC_MAX = 0xFF,
} BE_ST_ScanCode_T;

#endif
