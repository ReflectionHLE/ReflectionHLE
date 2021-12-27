#ifndef BE_INPUT_H
#define BE_INPUT_H

#include "be_st.h"

#define BE_ST_EMU_MOUSE_XRANGE 640
#define BE_ST_EMU_MOUSE_YRANGE 200

// Using example of values from here:
// http://www.intel-assembler.it/portale/5/program-joystick-port-210h/program-joystick-port-210h.asp
#define BE_ST_EMU_JOYSTICK_RANGEMIN 8
#define BE_ST_EMU_JOYSTICK_RANGECENTER 330
#define BE_ST_EMU_JOYSTICK_RANGEMAX 980
// This one is for init with no joysticks, for Keen Dreams v1.00
// (It requires a large value, while 0 will lead to division by zero)
#define BE_ST_EMU_JOYSTICK_OVERRANGEMAX 16384

#define BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS 500
#define BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS 40

typedef enum {
	BE_ST_MOUSEMODE_ABS_WITH_CURSOR, BE_ST_MOUSEMODE_ABS_WITHOUT_CURSOR, BE_ST_MOUSEMODE_REL
} BESDLMouseModeEnum;

typedef struct {
	bool isSpecial; // Scancode of 0xE0 sent?
	uint8_t dosScanCode;
} emulatedDOSKeyEvent;

/*** Last BE_ST_PollEvents time ***/
extern uint32_t g_sdlLastPollEventsTime;

/*** Exact values might be implementation-defined ***/
extern const int g_sdlJoystickAxisBinaryThreshold, g_sdlJoystickAxisDeadZone, g_sdlJoystickAxisMax, g_sdlJoystickAxisMaxMinusDeadZone;

typedef struct {
	// These represent button states, although a call
	// to BEL_ST_AltControlScheme_CleanUp zeros these out
	bool keys[BE_MAX_KEY_ID];
	bool pbuttons[BE_ST_CTRL_BUT_MAX];
	// We may opstionally use analog axes as buttons
	// (e.g., using stick as arrow keys, triggers as buttons)
	bool paxes[BE_ST_CTRL_AXIS_MAX][2];
} BEInputBindsStates;

extern BEInputBindsStates g_sdlInputbindStates;

/*** These are similar states for a few mouse buttons, required as relative mouse mode is toggled on or off in the middle ***/
extern bool g_sdlMouseButtonsStates[3];

/*** Another internal state, used for default mapping action ***/
extern bool g_sdlDefaultMappingBinaryState;

/*** Emulated mouse and joysticks states (mouse motion state is split for technical reasons) ***/
extern int g_sdlEmuMouseButtonsState;
extern int16_t g_sdlEmuMouseMotionAccumulatedState[2];
extern int16_t g_sdlEmuMouseCursorPos[2];
extern int16_t g_sdlEmuMouseMotionFromJoystick[2];
extern int16_t g_sdlEmuMouseMotionFromJoystick_Accu[2];
extern uint32_t g_sdlEmuMouseMotionFromJoystick_LastTicksMS;
extern int16_t g_sdlVirtualMouseCursorState[2]; // Used e.g., for touch input handling
extern int g_sdlEmuJoyButtonsState;
extern int16_t g_sdlEmuJoyMotionState[4];

/*** Key repeat emulation ***/
extern int g_sdlEmuKeyboardLastPressedScanCode; // 0 on release
extern bool g_sdlEmuKeyboardLastPressedIsSpecial;
extern uint32_t g_sdlEmuKeyboardLastScanCodePressTime;
extern uint32_t g_sdlEmuKeyboardLastScanCodePressTimeDelay;

/*** Similar repeat emulation for arrow movement in on-screen keyboard (key press repeats are emulated separately)   ***/
/*** HACK: With debug keys, once a key is selected, key repeat is in effect! (but that's the case with actual keys.) ***/
extern int g_sdlOnScreenKeyboardLastPressedDirButton;
extern uint32_t g_sdlOnScreenKeyboardLastDirButtonPressTime;
extern uint32_t g_sdlOnScreenKeyboardLastDirButtonPressTimeDelay;

extern bool g_sdlEmuKeyboardStateByScanCode[BE_ST_SC_MAX];

void BEL_ST_HandleEmuKeyboardEvent(bool isPressed, bool isRepeated, emulatedDOSKeyEvent keyEvent);
void BEL_ST_SetMouseMode(BESDLMouseModeEnum mode);

#endif
