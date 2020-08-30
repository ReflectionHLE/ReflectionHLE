/* Copyright (C) 2014-2020 NY00123
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

#ifndef BE_ST_H
#define BE_ST_H

#include <stdbool.h>
#include <stdint.h>

// On-screen touch controls are scaled such that the largest square
// fitting in the window has the dimensions of 140x140 (scaled) pixels
#define BE_ST_TOUCHCONTROL_MAX_WINDOW_DIM 130

/*** General ***/
void BE_ST_InitCommon(void); // Before game or launcher
void BE_ST_PrepareForGameStartupWithoutAudio(void); // Before game, excludes the audio
void BE_ST_InitAudio(void);
void BE_ST_ShutdownAll(void); // After game
void BE_ST_HandleExit(int status); // Replacement for exit function (useful for displaying text screen)
void BE_ST_QuickExit(void); // Where the usual exit handler isn't sufficient: Saves last settings, shutdowns subsystems and then exits immediately

// Sets/Unsets interrupt handler for emulated keyboard events
void BE_ST_StartKeyboardService(void (*funcPtr)(uint8_t));
void BE_ST_StopKeyboardService(void);
// Reset emulated mouse state (a possibly partial replacement for x86 int 33h,0)
void BE_ST_ResetEmuMouse(void);
// Set/Get emulated mouse cursor position
void BE_ST_SetEmuMousePos(int16_t x, int16_t y);
void BE_ST_GetEmuMousePos(int16_t *x, int16_t *y);
// Gets emulated mouse motion accumulated since a preceding call to
// this function (if any). Note that the output (pointer) arguments
// are optional; To ignore any of the motion axes,
// pass NULL for the corresponding argument.
void BE_ST_GetEmuAccuMouseMotion(int16_t *optX, int16_t *optY);
// Gets emulated mouse buttons states (pressed/released) as bit flags.
uint16_t BE_ST_GetEmuMouseButtons(void);
// Gets the current emulated joystick axes states for the
// given (emulated) joystick. Note that these axes don't have the
// usual kind of scale (it's roughly gameport joystick emulation),
// and some process of "calibration" might be required. Further note
// that the output (pointer) arguments are optional; To ignore
// any of the axes, pass NULL for the corresponding argument.
void BE_ST_GetEmuJoyAxes(uint16_t joy, uint16_t *optX, uint16_t *optY);
// Gets emulated joystick buttons states (pressed/released)
// as bit flags, for the given (emulated) joystick.
uint16_t BE_ST_GetEmuJoyButtons(uint16_t joy);

// Replacement for Borland's kbhit function, checking if a key
// (of emulated keyboard) has recently been pressed. The key can
// be cleared by a call to BE_ST_BiosScanCode with command == 0.
int16_t BE_ST_KbHit(void);
// ***NON-EQUIVALENT*** replacement for Borland's bioskey function.
// - If command == 0, the scan code of last pressed key
// (from emulated keyboard) is returned. If no key is
// pressed, the function waits for key press. Note that
// a following call to this function with command == 1 is
// expected to return 0, if no other key is pressed in-between.
// - If command == 1, the function returns the scan code of
// last key pressed, if there's any, or 0 if such code is not stored
// (e.g., right after a call to the same function with command == 0).
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

typedef struct {
	const char **xpmImage;
	int xpmWidth, xpmHeight;
	int xpmPosX, xpmPosY;
	void *miscData; // Used to store e.g., texture data
} BE_ST_OnscreenTouchControl;

// Same as above, but invisible (touch regions defined) and with some mapping...
typedef struct {
	BE_ST_ControllerSingleMap mapping;
	const char **xpmImage;
	int xpmWidth, xpmHeight;
	int xpmPosX, xpmPosY;
} BE_ST_TouchControlSingleMap;

typedef struct BE_ST_ControllerMapping {
	// Fill this for some default action. Examples of ways to trigger it:
	// - Pressing on a game controller button (or trigger)
	// with no mapped action.
	// - Pressing with mouse, or finger on a touchscreen,
	// outside touch controls or on-screen keyboard.
	// - Pressing on game controller's back button while
	// an on-screen keyboard is shown.
	BE_ST_ControllerSingleMap defaultMapping;

	BE_ST_OnscreenTouchControl *onScreenTouchControls;
	BE_ST_TouchControlSingleMap *touchMappings;

	BE_ST_ControllerSingleMap buttons[BE_ST_CTRL_BUT_MAX];
	BE_ST_ControllerSingleMap axes[BE_ST_CTRL_AXIS_MAX][2];
	bool showUi;
	bool absoluteFingerPositioning;
	bool grabMouse;
} BE_ST_ControllerMapping;

// Various controller schemes are saved in a stack, so it's straight-forward
// to revert to an arbitrary preceding scheme when desired.
void BE_ST_AltControlScheme_Push(void);
void BE_ST_AltControlScheme_Pop(void);
// Use this when a main function is called
void BE_ST_AltControlScheme_Reset(void);
// Replace current controller scheme using any of these
void BE_ST_AltControlScheme_PrepareControllerMapping(const BE_ST_ControllerMapping *mapping);
// Use this for absolute mouse cursor position update (e.g., for touch input)
// Assumes dimensions of 320x200 are used as a base
void BE_ST_AltControlScheme_UpdateVirtualMouseCursor(int x, int y);

/*** Call this before entering game code (added for textures management) ***/
void BE_ST_AltControlScheme_InitTouchControlsUI(BE_ST_OnscreenTouchControl *onScreenTouchControls);

// HACK - Pass corresponding pointer to PrepareControllerMapping for on-screen keyboard
// (can also be used to go from one mapping to another, e.g., showing debug keys in-game)
extern BE_ST_ControllerMapping g_beStControllerMappingTextInput;
extern BE_ST_ControllerMapping g_beStControllerMappingDebugKeys;


// Used when loading controller scheme stuff from cfg
enum {
	BE_ST_CTRL_CFG_BUTMAP_BEFOREFIRST = -1, /* The actual first entry is to be numbered 0 */
#ifdef REFKEEN_HAS_VER_KDREAMS
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
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	BE_ST_CTRL_CFG_BUTMAP_SCROLLS,
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES)
	BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS,
#endif
	BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS,
	BE_ST_CTRL_CFG_BUTMAP_AFTERLAST,
};

// Set callback function, to be called when the app is shut down in a way
// originally not supported by a given game, say clicking on a window's
// "close" button. Can be used for saving settings.
// There may still be cases in which the callback is *not* called, though.
void BE_ST_SetAppQuitCallback(void (*funcPtr)(void));

void BE_ST_PollEvents(void);

// Returns an offset that should be added to the 16-bit segments
// of 32-bit far pointers read from/written to files. Saved games from
// The Catacomb Armageddon/Apocalypse are good examples of such files.
// (In the case of the original DOS exes, the offset depends on the
// locations of modified copies of the far pointers in memory.)
uint16_t BE_ST_Compat_GetFarPtrRelocationSegOffset(void);

/*** Audio/timer (vanilla Keen kind-of has these mixed) ***/
void BE_ST_StartAudioAndTimerInt(void (*funcPtr)(void));
void BE_ST_StopAudioAndTimerInt(void);
void BE_ST_LockAudioRecursively(void);
void BE_ST_UnlockAudioRecursively(void);
bool BE_ST_IsEmulatedOPLChipReady(void);
// WARNING about using BE_ST_PCSpeaker*:
//
// You MUST call BE_ST_LockAudioRecursively before calling any of these
// functions. Afterwards, you MUST call BE_ST_UnlockAudioRecursively.
// If you don't, unexpected behaviors may be reproduced
// (e.g., an audio callback thread hang).
//
// For BE_ST_PCSpeakerSetInvFreq: The frequency is about 1193182Hz/spkInvFreq.
// To turn the PC Speaker off, call BE_ST_PCSpeakerSetConstVal(0).
void BE_ST_PCSpeakerSetInvFreq(uint16_t spkInvFreq);
void BE_ST_PCSpeakerSetConstVal(bool isUp);
// Used for playback from digitized sound data in signed 16-bit int format.
// Do NOT assume the data is copied; You ***must*** call BE_ST_StopSoundEffect.
// You can also use BE_ST_StartAudioAndTimerInt to set a callback function,
// to be called when reading of sound data is complete. This can happen
// a bit before actual sound playback is complete, in case
// some mechanism of resampling is in use.
void BE_ST_PlayS16SoundEffect(int16_t *data, int numOfSamples);
void BE_ST_StopSoundEffect(void);
// Safe alternatives for Borland's sound and nosound functions from Catacomb Abyss' gelib.c
void BE_ST_BSound(uint16_t frequency);
void BE_ST_BNoSound(void);
// Sends data to emulated OPL2/3 chip (e.g., AdLib), where "reg" is
// written to the address/status port and "val" is written to the data
// port. For the purpose of OPL emulation, this attempts to cover small
// delays (measured in microseconds) as given by the AdLib manual.
void BE_ST_OPL2Write(uint8_t reg,uint8_t val);
// Sets the rate in which the timer interrupt is called, to about
// 1193182Hz/rateVal. May also have an effect on sound generation.
void BE_ST_SetTimer(uint16_t rateVal);
// Resets to 0 an internal counter of calls to timer interrupt,
// and returns the original counter's value
int BE_ST_TimerIntClearLastCalls(void);
// Attempts to wait for a given amount of calls to timer interrupt.
// It may wait a bit more in practice (e.g., due to Sync to VBlank).
// This is taken into account into a following call to the same function,
// which may actually be a bit shorter than requested (as a consequence).
void BE_ST_TimerIntCallsDelayWithOffset(int nCalls);

/*** Graphics ***/
void BE_ST_InitGfx(void);
void BE_ST_ShutdownGfx(void);

void BE_ST_MarkGfxForUpdate(void);

// ***WARNING*** SEE WARNING BELOW BEFORE USING!!!
//
// Basically a replacement for B800:0000, points to a 80x25*2 bytes long
// buffer; Text mode only, NOT e.g., CGA graphics.
//
// ***WARNING***: After modifying this chunk, it is a MUST to call the function
// BE_ST_MarkGfxForUpdate (used as an optimization).
uint8_t *BE_ST_GetTextModeMemoryPtr(void);

bool BE_ST_HostGfx_CanToggleAspectRatio(void);
bool BE_ST_HostGfx_GetAspectRatioToggle(void);
void BE_ST_HostGfx_SetAspectRatioToggle(bool aspectToggle);

bool BE_ST_HostGfx_CanToggleFullScreen(void);
bool BE_ST_HostGfx_GetFullScreenToggle(void);
void BE_ST_HostGfx_SetFullScreenToggle(bool fullScreenToggle);
void BE_ST_HostGfx_ToggleFullScreen(void);

// NOT NECESSARILY DEFINED FOR ALL GAMES!
void BE_ST_HostGfx_SetAbsMouseCursorToggle(bool cursorToggle);

// 256-color graphics manipulations (e.g., VGA mode 0x13)
void BE_ST_VGASetPaletteColor(uint8_t r, uint8_t g, uint8_t b, int index);
void BE_ST_VGAGetPaletteColor(uint8_t *r, uint8_t *g, uint8_t *b, int index);
void BE_ST_VGASetPalette(const uint8_t *palette);
void BE_ST_VGAGetPalette(uint8_t *palette);
void BE_ST_VGAFillPalette(uint8_t r, uint8_t g, uint8_t b, int first, int last);
void BE_ST_VGAUpdateGFXByteInPlane(uint16_t destOff, uint8_t srcVal, uint16_t planeNum);
void BE_ST_VGAUpdateGFXBufferInPlane(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeNum);
void BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num);
uint8_t BE_ST_VGAFetchGFXByteFromPlane(uint16_t destOff, uint16_t planeNum);

// 16-color graphics manipulations (say, EGA modes 0xD-0xE),
// with a portion of the functions also applying to 256-color graphics
void BE_ST_EGASetPaletteAndBorder(const uint8_t *palette);
void BE_ST_EGASetLineWidth(uint8_t widthInBytes);
void BE_ST_EGASetSplitScreen(int16_t linenum);
void BE_ST_EGASetPelPanning(uint8_t panning);
void BE_ST_EGAUpdateGFXByteInPlane(uint16_t destOff, uint8_t srcVal, uint16_t planeNum);
// Same as BE_ST_EGAUpdateGFXByteInPlane but picking specific bits out of each byte, and covering ALL planes
void BE_ST_EGAUpdateGFXBufferInPlane(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeNum);
void BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff);
// Same as BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr but with a specific plane (added for Catacomb Abyss vanilla bug reproduction/workaround)
void BE_ST_EGAUpdateGFXByteInPlaneScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t planeNum);
// Same as BE_ST_EGAUpdateGFXByteInAllPlanesScrToScr but picking specific bits out of each byte
void BE_ST_EGAUpdateGFXBitsInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask);
void BE_ST_EGAUpdateGFXBufferInAllPlanesScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num);
uint8_t BE_ST_EGAFetchGFXByteFromPlane(uint16_t destOff, uint16_t planeNum);
void BE_ST_EGAFetchGFXBufferFromPlane(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planeNum);
void BE_ST_EGAUpdateGFXBitsFrom4bitsPixel(uint16_t destOff, uint8_t color, uint8_t bitsMask);
void BE_ST_EGAUpdateGFXBufferFrom4bitsPixel(uint16_t destOff, uint8_t color, uint16_t count);
void BE_ST_EGAXorGFXByteByPlaneMask(uint16_t destOff, uint8_t srcVal, uint16_t planeMask);

// 4-color graphics manipulations (e.g., CGA mode 4)
void BE_ST_CGAUpdateGFXBufferFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth);

void BE_ST_SetScreenStartAddress(uint16_t crtc);
void BE_ST_SetBorderColor(uint8_t color);
void BE_ST_SetScreenMode(int mode);
void BE_ST_WaitForNewVerticalRetraces(int16_t number);
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

// MUST be included here (since be_st_sdl.h depends on be_st.h)
#include "be_st_sdl.h"

#endif // BE_ST_H
