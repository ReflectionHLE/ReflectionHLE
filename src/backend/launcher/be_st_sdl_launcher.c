/* Copyright (C) 2015-2021 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "refkeen.h"

#ifdef REFKEEN_ENABLE_LAUNCHER

#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "backend/input/be_input.h"
#include "backend/input/be_input_sdl.h"
#include "backend/video/be_video_ui.h"
#include "backend/video/be_video_sdl.h"
#include "backend/video/be_video_textures.h"
#include "be_features.h"
#include "be_launcher_cfg.h"
#include "be_launcher_cfg_sdl.h"
#include "be_st_launcher.h"
#include "be_st_sdl_private.h"
#include "be_title_and_version.h"

// In C++, const implies static, so we need to specify extern
#ifdef __cplusplus
#define BUFFLINKAGE extern
#else
#define BUFFLINKAGE
#endif

// HACK - Duplicated from be_st_sdl_graphics.c
#define ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH 22
#define ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT 12


// Measured in keys
#define ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH 10
#define ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT 5

#define ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH (ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
#define ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_HEIGHT (ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT)

// Again measured in keys
#define ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH 14
#define ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT 4

#define ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH (ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
#define ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_HEIGHT (ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT)

// Internally we use SDL2 scancodes for on-screen keyboards here,
// but there are a few exceptions we make for convenience

// Unused as of SDL v2.0.5
#define ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_LEFT 1
#define ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_RIGHT 2
#define ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT 3

// Used instead of SDL_SCANCODE_TAB
#define ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_DELETE SDL_SCANCODE_TAB
// Used instead of SDL_SCANCODE_NONUSHASH
#define ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_HOME SDL_SCANCODE_NONUSHASH
// Used instead of SDL_SCANCODE_CAPSLOCK
#define ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_END SDL_SCANCODE_CAPSLOCK

// HACK - Remember last event type, so when game is started, we can guess
// if touch controls should be shown or not (if "auto" is set for these)
static Uint32 g_sdlLauncherLastEventType;

static BE_ST_Rect g_sdlControllerLauncherTextSearchRect, g_sdlControllerLauncherTextInputRect;
static BE_ST_Texture *g_sdlLauncherTextSearchTexture, *g_sdlLauncherTextInputTexture;
static bool g_sdlLauncherTextSearchUIIsShown, g_sdlLauncherTextInputUIIsShown;

// NOTE: More-or-less duplicated from be_st_sdl_graphics.c, but there may be
// some differences. For one, these are not limited just to a specific kind
// of keyboard UI (partially unused for in-game debug keys UI).
static int g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY;
static bool g_sdlKeyboardUISelectedKeyIsMarked;

static bool g_sdlKeyboardUIIsKeyPressed, g_sdlKeyboardUIIsShifted;

// Borrowed from other files
extern int g_sdlDebugFingerRectSideLen;
extern const uint32_t g_sdlEGABGRAScreenColors[];
extern SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS];
extern BE_ST_Texture *g_sdlTexture, *g_sdlTargetTexture;
extern BE_ST_Rect g_sdlAspectCorrectionBorderedRect;
extern const int g_sdlJoystickAxisBinaryThreshold, g_sdlJoystickAxisDeadZone, g_sdlJoystickAxisMax, g_sdlJoystickAxisMaxMinusDeadZone;
extern int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;

// The window and renderer are shared ON PURPOSE:
// - For seamless transition from launcher to game, if a borderless
// fullscreen window is used with same features.
// - Additionally, the renderer is shared, as texture pointers are

// Similarly, g_sdlShowTouchUI is also shared *on purpose* - for a HACK (guess if touch controls should be shown when game is started)

static int g_sdlKeyboardLastKeyPressed;
static bool g_sdlKeyboardLastKeyPressedIsShifted;
static int g_sdlControllerLastButtonPressed;

static uint32_t g_sdlInputLastBinaryPressTime;
static uint32_t g_sdlInputLastBinaryPressTimeDelay;

// Need these for triggers while letting the user selection buttons for gameplay
static bool g_sdlLauncherTriggerBinaryStates[2];

static uint8_t g_sdlLauncherGfxCache[BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT]; // Launcher gets pointer to this for drawing
static bool g_sdlLauncherGfxCacheMarked = false;

/*** Convenience macros - Note that the label *must* be a C string literal ***/
#define BEMENUITEM_DEF_TARGETMENU(menuItemName, label, menuPtr) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, NULL, menuPtr, label, BE_MENUITEM_TYPE_TARGETMENU)
#define BEMENUITEM_DEF_SELECTION(menuItemName, label, choices) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, choices, NULL, label, BE_MENUITEM_TYPE_SELECTION)
#define BEMENUITEM_DEF_HANDLER(menuItemName, label, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, NULL, NULL, label, BE_MENUITEM_TYPE_HANDLER)
#define BEMENUITEM_DEF_HANDLER_LABELVAR(menuItemName, labelSize, handlerPtr) BEMENUITEM_DEF_LABELVAR(menuItemName, handlerPtr, NULL, NULL, labelSize, BE_MENUITEM_TYPE_HANDLER)
#define BEMENUITEM_DEF_SELECTION_WITH_HANDLER(menuItemName, label, choices, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, NULL, label, BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER)
#define BEMENUITEM_DEF_SLIDER(menuItemName, label, choices) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, choices, NULL, label, BE_MENUITEM_TYPE_SLIDER)
#define BEMENUITEM_DEF_RANGE_SLIDER(menuItemName, label, nOfChoices) BEMENUITEM_DEF_GENERIC2(menuItemName, NULL, nOfChoices, NULL, label, BE_MENUITEM_TYPE_RANGE_SLIDER)
#define BEMENUITEM_DEF_DYNAMIC_SELECTION(menuItemName, label, choices, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, NULL, label, BE_MENUITEM_TYPE_DYNAMIC_SELECTION)
#define BEMENUITEM_DEF_STATIC(menuItemName, label) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, NULL, NULL, label, BE_MENUITEM_TYPE_STATIC)

// A little hack - Store a copy of the label string literal that can be modified
#define BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, menuPtr, label, type) \
	static char menuItemName ## _label[] = label; \
	BEMenuItem menuItemName = {handlerPtr, choices, menuPtr, menuItemName ## _label, 0, 0, 0, 0, 0, 0, type};

// Similar macro, but for BEMENUITEM_DEF_RANGE_SLIDER
#define BEMENUITEM_DEF_GENERIC2(menuItemName, handlerPtr, nOfChoices, menuPtr, label, type) \
	static char menuItemName ## _label[] = label; \
	BEMenuItem menuItemName = {handlerPtr, 0, menuPtr, menuItemName ## _label, 0, nOfChoices, 0, 0, 0, 0, type};

// Same as above, but label is variable and we fill some room for it
#define BEMENUITEM_DEF_LABELVAR(menuItemName, handlerPtr, choices, menuPtr, labelSize, type) \
	static char menuItemName ## _label[labelSize] = {0}; \
	BEMenuItem menuItemName = {handlerPtr, choices, menuPtr, menuItemName ## _label, 0, 0, 0, 0, 0, 0, type};

static const char *g_be_settingsChoices_boolean[] = {"No","Yes",NULL};

/*** Main menu ***/

BEMENUITEM_DEF_HANDLER_LABELVAR(g_beMainMenuItem_PlayLastChosenGameVer, 92/* HACK to have enough room for string*/, &BE_Launcher_Handler_LastGameVerLaunch)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_SelectGame, "Select game", &g_beSelectGameMenu)
BEMENUITEM_DEF_HANDLER(g_beMainMenuItem_SetArguments, "Set arguments for game *CURRENTLY SET*", &BE_Launcher_Handler_SetArgumentsForGame)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_Settings, "Settings", &g_beSettingsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_ShowVersion, "Show version", &g_beShowVersionMenu)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_Quit, "Quit", &g_beQuitConfirmMenu)

static BEMenuItem *g_beMainMenuItems[] = {
	&g_beMainMenuItem_PlayLastChosenGameVer, // Either this item's label is filled later, or the menu items array is shifted
	&g_beMainMenuItem_SelectGame,
	&g_beMainMenuItem_SetArguments,
	&g_beMainMenuItem_Settings,
	&g_beMainMenuItem_ShowVersion,
	&g_beMainMenuItem_Quit,
	NULL
};

BEMenu g_beMainMenu = {
	REFKEEN_TITLE_STRING,
	&g_beQuitConfirmMenu,
	g_beMainMenuItems,
	// Ignore the rest
};

/*** Select game menu ***/

static BEMenuItem g_beSelectGameMenuItems[BE_GAMEVER_LAST];
static char g_beSelectGameMenuItemsStrs[BE_GAMEVER_LAST][78]; // Should be MUTABLE strings for layout preparation
static BEMenuItem *g_beSelectGameMenuItemsPtrs[BE_GAMEVER_LAST+4];

BEMENUITEM_DEF_TARGETMENU(g_beSelectGameMenuItem_DisappearedGameHelp, "Help! An installed game disappeared from the list!", &g_beDisappearedGameHelpMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSelectGameMenuItem_ShowSupportedGameVersions, "Show supported game versions", &g_beSupportedGameVersionsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSelectGameMenuItem_AddMissingGameVersion, "Add missing game version", &g_beSelectInitialPathMenu)

BEMenu g_beSelectGameMenu = {
	"Select game",
	&g_beMainMenu,
	g_beSelectGameMenuItemsPtrs, // Array of menu items
	// Ignore the rest
};

/*** Select game EXE menu ***/

#define MAX_NUM_OF_DISPLAYED_GAME_EXES_PER_VER 4

static BEMenuItem g_beSelectGameExeMenuItems[MAX_NUM_OF_DISPLAYED_GAME_EXES_PER_VER];
static char g_beSelectGameExeMenuItemsStrs[MAX_NUM_OF_DISPLAYED_GAME_EXES_PER_VER][78]; // Should be MUTABLE strings for layout preparation
static BEMenuItem *g_beSelectGameExeMenuItemsPtrs[MAX_NUM_OF_DISPLAYED_GAME_EXES_PER_VER];

BEMenu g_beSelectGameExeMenu = {
	"Choose what to start",
	&g_beSelectGameMenu,
	g_beSelectGameExeMenuItemsPtrs, // Array of menu items
	// Ignore the rest
};

/*** Disappeared game menu ***/

BEMENUITEM_DEF_STATIC(g_beDisappearedGameHelpMenuItem_Explanation,
"Reflection Keen can detect compatible DOS game versions from certain installations, including the Catacombs games from GOG.com. Once such a game installation is updated in any minor way, Reflection Keen may fail to locate it. These are the expected behaviors.\nAs an alternative, you can manually add a compatible game installation (if not yet listed)."
);

static BEMenuItem *g_beDisappearedGameHelpMenuItems[] = {
	&g_beDisappearedGameHelpMenuItem_Explanation,
	NULL
};

BEMenu g_beDisappearedGameHelpMenu = {
	"Where may a game disappear",
	&g_beSelectGameMenu,
	g_beDisappearedGameHelpMenuItems,
	// Ignore the rest
};

/*** Supported game versions menu ***/

// Statically allocated, but filled later
static BEMenuItem *g_beSupportedGameVersionsMenu_MenuItemsPtrs[BE_GAMEVER_LAST+1] = {
	// All pointers are initialized to NULL, and the last entry shall always be NULL
};

BEMenu g_beSupportedGameVersionsMenu = {
	"Select game version for details",
	&g_beSelectGameMenu,
	g_beSupportedGameVersionsMenu_MenuItemsPtrs, // Filled later
	// Ignore the rest
};

/*** Game version details menu (details vary by specific game version) ***/

BEMenu g_beGameVersionDetailsMenu = {
	NULL, // Dynamically chosen
	&g_beSupportedGameVersionsMenu,
	NULL, // Dynamically allocated, filled later
	&BE_Launcher_Handler_ReturnToSupportedGameVersionsMenu, // SPECIAL (back button handler)
	// Ignore the rest
};

/*** Select initial path menu ***/

static BEMenuItem *g_beSelectInitialPathMenuItems;
static BEMenuItem **g_beSelectInitialPathMenuItemsPtrs;
static char *g_beSelectInitialPathMenuItemsStrsBuffer;

BEMenu g_beSelectInitialPathMenu = {
	"Select initial path",
	&g_beSelectGameMenu,
	NULL, // Dynamically allocated, filled later
	// Ignore the rest
};

/*** Select directory menu (basically a dialog that may change often) ***/

BEMenu g_beSelectDirectoryMenu = {
	"Select directory",
	NULL, // SPECIAL (using back button handler)
	NULL, // Dynamically allocated, filled later
	&BE_Launcher_Handler_DirectorySelectionGoPrev, // SPECIAL (back button handler)
	// Ignore the rest
};

/*** Select directory error menu ***/

BEMENUITEM_DEF_HANDLER(g_beSelectDirectoryErrorMenuItem_DisappearedGameHelp, "Try to go up in directories hierachy", &BE_Launcher_Handler_DirectorySelectionGoPrev)

static BEMenuItem *g_beSelectDirectoryErrorMenuItems[] = {
	&g_beSelectDirectoryErrorMenuItem_DisappearedGameHelp,
	NULL
};

BEMenu g_beSelectDirectoryErrorMenu = {
	"Failed to select directory",
	NULL, // SPECIAL (using back button handler)
	g_beSelectDirectoryErrorMenuItems,
	&BE_Launcher_Handler_DirectorySelectionGoPrev, // SPECIAL (back button handler)
	// Ignore the rest
};

/*** Compatible game found menu ***/

// At this point, all directory-selection related resources are freed
BEMENUITEM_DEF_TARGETMENU(g_beSelectDirectoryFoundGameMenuItem_ShowGamesList, "Show games list", &g_beSelectGameMenu)

static BEMenuItem *g_beSelectDirectoryFoundGameMenuItems[] = {
	&g_beSelectDirectoryFoundGameMenuItem_ShowGamesList,
	NULL
};

BEMenu g_beSelectDirectoryFoundGameMenu = {
	"New compatible game found!",
	&g_beSelectGameMenu,
	g_beSelectDirectoryFoundGameMenuItems,
	// Ignore the rest
};

/*** No additional compatible game found menu ***/

// Here, directory-selection related resources are STILL allocated
BEMENUITEM_DEF_TARGETMENU(g_beSelectDirectoryNoGameFoundMenu_ReturnToDirectory, "Return to directory", &g_beSelectDirectoryMenu)
BEMENUITEM_DEF_STATIC(g_beSelectDirectoryNoGameFoundMenu_ReportedIssuesByGameVer, "    Reported issues by game version    ") // HACK - Proper spacing for text centering

// Statically allocated, but filled later
static BEMenuItem *g_beSelectDirectoryNoGameFoundMenu_MenuItemsPtrs[BE_GAMEVER_LAST+3] = {
	&g_beSelectDirectoryNoGameFoundMenu_ReturnToDirectory,
	&g_beSelectDirectoryNoGameFoundMenu_ReportedIssuesByGameVer,
	// All the rest are initialized to NULL, and the last entry shall always be NULL
};

BEMenu g_beSelectDirectoryNoGameFoundMenu = {
	"No new compatible game found",
	&g_beSelectDirectoryMenu,
	g_beSelectDirectoryNoGameFoundMenu_MenuItemsPtrs, // Filled later
	// Ignore the rest
};

/*** Settings menu ***/

BEMENUITEM_DEF_TARGETMENU(g_beSettingsMenuItem_VideoSettings, "Video settings", &g_beVideoSettingsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSettingsMenuItem_SoundSettings, "Sound settings", &g_beSoundSettingsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSettingsMenuItem_InputSettings, "Input settings", &g_beInputSettingsMenu)
#ifdef BE_ST_ENABLE_SETTING_LOWFPS
BEMENUITEM_DEF_TARGETMENU(g_beSettingsMenuItem_MiscSettings, "Miscellaneous", &g_beMiscSettingsMenu)
#endif

static BEMenuItem *g_beSettingsMenuItems[] = {
	&g_beSettingsMenuItem_VideoSettings,
	&g_beSettingsMenuItem_SoundSettings,
	&g_beSettingsMenuItem_InputSettings,
#ifdef BE_ST_ENABLE_SETTING_LOWFPS
	&g_beSettingsMenuItem_MiscSettings,
#endif
	NULL
};

BEMenu g_beSettingsMenu = {
	"Settings",
	&g_beMainMenu,
	g_beSettingsMenuItems,
	// Ignore the rest
};

/** Video settings menu ***/

#define BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS 10

typedef struct {
	int width, height;
} BEMenuItemScreenResPair;

const char *g_be_videoSettingsChoices_displayNums[] = {"0","1","2","3","4","5","6","7",NULL};

char g_be_videoSettingsChoices_sdlRendererDriversStrs[BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS][BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN];
// Need to add additional auto entry, plus NULL terminator
static const char *g_be_videoSettingsChoices_sdlRendererDrivers[BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS+2];

static const char *g_be_videoSettingsChoices_scaleType[] = {"4:3 + Porch","Fill",NULL};
static const char *g_be_videoSettingsChoices_scaleFactor[] = {"1","2","3","4",NULL};
static const char *g_be_videoSettingsChoices_vSync[] = {"Auto","Off","On",NULL};
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
static const char *g_be_videoSettingsChoices_launcherWindowType[] = {"Default","Fullscreen","Software",NULL};
#endif

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_Fullscreen, "Fullscreen", g_be_settingsChoices_boolean)
#endif
BEMENUITEM_DEF_SLIDER(g_beVideoSettingsMenuItem_DisplayNum, "Display number", g_be_videoSettingsChoices_displayNums)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_RememberDisplayNum, "Remember last used display number", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_SDLRenderer, "SDL renderer", g_be_videoSettingsChoices_sdlRendererDrivers)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_Bilinear, "Bilinear interpolation", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ScaleType, "Scale type*", g_be_videoSettingsChoices_scaleType)
BEMENUITEM_DEF_SLIDER(g_beVideoSettingsMenuItem_ScaleFactor, "Scale factor", g_be_videoSettingsChoices_scaleFactor)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_VSync, "VSync", g_be_videoSettingsChoices_vSync)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ForceFullSoftScaling, "Force full software scaling**", g_be_settingsChoices_boolean)
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_LauncherWindowType, "Launcher window type", g_be_videoSettingsChoices_launcherWindowType)
#endif
BEMENUITEM_DEF_STATIC(g_beVideoSettingsMenuItem_ScaleTypeComment,
"* Border porches aren't separately shown without aspect correction."
);
BEMENUITEM_DEF_STATIC(g_beVideoSettingsMenuItem_SoftScalingComment,
"** Full software scaling should be manually toggled. Note that it can lead to great slowdowns with high-resolution windows."
);

static BEMenuItem *g_beVideoSettingsMenuItems[] = {
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	&g_beVideoSettingsMenuItem_Fullscreen,
#endif
	&g_beVideoSettingsMenuItem_DisplayNum,
	&g_beVideoSettingsMenuItem_RememberDisplayNum,
	&g_beVideoSettingsMenuItem_SDLRenderer,
	&g_beVideoSettingsMenuItem_Bilinear,
	&g_beVideoSettingsMenuItem_ScaleType,
	&g_beVideoSettingsMenuItem_ScaleFactor,
	&g_beVideoSettingsMenuItem_VSync,
	&g_beVideoSettingsMenuItem_ForceFullSoftScaling,
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
	&g_beVideoSettingsMenuItem_LauncherWindowType,
#endif
	&g_beVideoSettingsMenuItem_ScaleTypeComment,
	&g_beVideoSettingsMenuItem_SoftScalingComment,
	NULL
};

BEMenu g_beVideoSettingsMenu = {
	"Video settings",
	&g_beSettingsMenu,
	g_beVideoSettingsMenuItems,
	// Ignore the rest
};

/*** Sounds settings menu ***/

BUFFLINKAGE const int g_be_soundsSettingsChoices_sndSampleRateVals[] = {8000, 11025, 12000, 16000, 22050, 32000, 44100, 48000, 49716, 96000, 192000};
static const char *g_be_soundsSettingsChoices_sndSampleRate[12] = {"8000","11025","12000","16000","22050","32000","44100","48000","49716","96000","192000",NULL};

BEMENUITEM_DEF_TARGETMENU(g_beSoundSettingsMenuItem_DeviceVolumes, "Emulated device volumes", &g_beDeviceVolumesMenu)
BEMENUITEM_DEF_SLIDER(g_beSoundSettingsMenuItem_SndSampleRate, "Sound sample rate\n(in Hz)", g_be_soundsSettingsChoices_sndSampleRate)
BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_SndSubSystem, "Enable sound subsystem", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_OPLEmulation, "OPL emulation", g_be_settingsChoices_boolean)

#ifdef BE_ST_ENABLE_SETTING_SB
static const char *g_be_soundSettingsChoices_sbType[] = {"None", "SB", "SB Pro", "SB 16", NULL};

BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_SB, "Sound Blaster emulation", g_be_soundSettingsChoices_sbType)
#endif

#ifndef REFKEEN_RESAMPLER_NONE
BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_UseResampler, "Use resampler", g_be_settingsChoices_boolean)
#endif

static BEMenuItem *g_beSoundSettingsMenuItems[] = {
	&g_beSoundSettingsMenuItem_DeviceVolumes,
	&g_beSoundSettingsMenuItem_SndSampleRate,
	&g_beSoundSettingsMenuItem_SndSubSystem,
	&g_beSoundSettingsMenuItem_OPLEmulation,
#ifdef BE_ST_ENABLE_SETTING_SB
	&g_beSoundSettingsMenuItem_SB,
#endif
#ifndef REFKEEN_RESAMPLER_NONE
	&g_beSoundSettingsMenuItem_UseResampler,
#endif
	NULL
};

BEMenu g_beSoundSettingsMenu = {
	"Sounds settings",
	&g_beSettingsMenu,
	g_beSoundSettingsMenuItems,
	// Ignore the rest
};

/*** Device volumes menu ***/

BEMENUITEM_DEF_RANGE_SLIDER(g_beDeviceVolumesMenuItem_PCSpkVol, "PC speaker volume", BE_AUDIO_VOL_MAX - BE_AUDIO_VOL_MIN + 1)
BEMENUITEM_DEF_RANGE_SLIDER(g_beDeviceVolumesMenuItem_OPLVol, "OPL volume", BE_AUDIO_VOL_MAX - BE_AUDIO_VOL_MIN + 1)
#ifdef BE_ST_ENABLE_SETTING_DIGIVOL
BEMENUITEM_DEF_RANGE_SLIDER(g_beDeviceVolumesMenuItem_DigiVol, "Digitized sound volume", BE_AUDIO_VOL_MAX - BE_AUDIO_VOL_MIN + 1)
#endif

static BEMenuItem *g_beDeviceVolumesMenuItems[] = {
	&g_beDeviceVolumesMenuItem_PCSpkVol,
	&g_beDeviceVolumesMenuItem_OPLVol,
#ifdef BE_ST_ENABLE_SETTING_DIGIVOL
	&g_beDeviceVolumesMenuItem_DigiVol,
#endif
	NULL
};

BEMenu g_beDeviceVolumesMenu = {
	"Emulated device volumes",
	&g_beSoundSettingsMenu,
	g_beDeviceVolumesMenuItems,
	// Ignore the rest
};

/*** Input settings menu ***/

static const char *g_be_inputSettingsChoices_controllerScheme[] = {"Classic", "Modern", NULL};
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
static const char *g_be_inputSettingsChoices_touchControls[] = {"Auto", "Off", "Forced", NULL};
#endif
static const char *g_be_inputSettingsChoices_mouseGrab[] = {"Auto", "Off", "Commonly", NULL};

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
static void BEL_ST_Launcher_Handler_TouchInputDebugging(BEMenuItem **menuItemP);
#endif

BEMENUITEM_DEF_TARGETMENU(g_beInputSettingsMenuItem_ControllerSettings, "Modern controller settings", &g_beControllerSettingsMenu)
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_ControllerScheme, "Game controller scheme", g_be_inputSettingsChoices_controllerScheme)
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_TouchControls, "Enable touch controls", g_be_inputSettingsChoices_touchControls);
BEMENUITEM_DEF_SELECTION_WITH_HANDLER(g_beInputSettingsMenuItem_TouchInputDebugging, "Touch input debugging", g_be_settingsChoices_boolean, &BEL_ST_Launcher_Handler_TouchInputDebugging);
#endif
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_MouseGrab, "Mouse grab*\n(windowed mode specific)", g_be_inputSettingsChoices_mouseGrab)
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_AbsMouseMotion, "Absolute mouse motion**", g_be_settingsChoices_boolean)
#endif
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_VertAnalogMotion, "Vertical analog motion toggle", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_STATIC(g_beInputSettingsMenuItem_MouseGrabComment,
"* It's possible for mouse to be ungrabbed even if \"Commonly\" is chosen."
);
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
BEMENUITEM_DEF_STATIC(g_beInputSettingsMenuItem_AbsMouseMotionComment,
"** If toggled off, mouse cursor motion behaviors are similar to original DOS versions. Otherwise it may move at a different rate, but be more convenient to use in windowed mode."
);
#endif

static BEMenuItem *g_beInputSettingsMenuItems[] = {
	&g_beInputSettingsMenuItem_ControllerSettings,
	&g_beInputSettingsMenuItem_ControllerScheme,
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	&g_beInputSettingsMenuItem_TouchControls,
	&g_beInputSettingsMenuItem_TouchInputDebugging,
#endif
	&g_beInputSettingsMenuItem_MouseGrab,
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	&g_beInputSettingsMenuItem_AbsMouseMotion,
#endif
#ifdef BE_ST_ENABLE_SETTING_NOVERT
	&g_beInputSettingsMenuItem_VertAnalogMotion,
#endif
	&g_beInputSettingsMenuItem_MouseGrabComment,
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	&g_beInputSettingsMenuItem_AbsMouseMotionComment,
#endif
	NULL
};

BEMenu g_beInputSettingsMenu = {
	"Input settings",
	&g_beSettingsMenu,
	g_beInputSettingsMenuItems,
	// Ignore the rest
};

/*** Controller settings menu ***/

static const char *g_be_controllerSettingsChoices_actionButton[] = {"A", "B", "X", "Y", NULL, NULL, NULL, "LStick" ,"RStick", "LShoulder", "RShoulder", "DPad Up", "DPad Down", "DPad Left", "DPad Right", NULL, "Paddle 1", "Paddle 2", "Paddle 3", "Paddle 4", "LTrigger", "RTrigger", "N/A"};
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
static const char *g_be_controllerSettingsChoices_analogMotion[] = {"Keyboard", "Mouse", NULL};
#endif

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
static void BEL_ST_Launcher_Handler_ImportControllerMappingsFromSteam(BEMenuItem **menuItemP);
#endif

#ifdef REFKEEN_HAS_VER_KDREAMS // Differing descriptions for same actions
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Up, "Action - Default Up key (Up)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Down, "Action - Default Down key (Down)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#else
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Up, "Action - Default Forward key (Up)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Down, "Action - Default Backward key (Down)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Left, "Action - Default Left key (Left)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Right, "Action - Default Right key (Right)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#ifdef REFKEEN_HAS_VER_KDREAMS
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Jump, "Action - Default Jump key (Ctrl)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Throw, "Action - Default Throw key (Alt)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Stats, "Action - Stats", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Fire, "Action - Default Fire key (Ctrl)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Strafe, "Action - Default Strafe key (Alt)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Drink, "Action - Drink", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Bolt, "Action - Bolt/Zapper", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Nuke, "Action - Nuke/Xterminator", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_FastTurn, "Action - Fast turn", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Scrolls, "Action - Scrolls", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Use, "Action - Default Use key (Space)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Run, "Action - Default Run key (RShift)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Weapon1, "Action - Knife/Hand Feeding", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Weapon2, "Action - Pistol/Small Feed Launcher", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Weapon3, "Action - Machine Gun/Big Feed Launcher", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Weapon4, "Action - Chain Gun/Super Feeder 5000", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Weapon5, "Action - Cantaloupe Feeder", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Weapon6, "Action - Watermelon Feeder", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Map, "Action - Automap (Super 3-D Noah's Ark)", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_FuncKeys, "Action - Function keys", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_DebugKeys, "Action - Debug keys", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)

BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_LeftStick, "Use left stick", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_RightStick, "Use right stick", g_be_settingsChoices_boolean)
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_AnalogMotion, "Motion emulation mode", g_be_controllerSettingsChoices_analogMotion)
#endif
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
BEMENUITEM_DEF_HANDLER(g_beControllerSettingsMenuItem_ImportMappingsFromSteam, "Import controller mappings from Steam\n(shouldn't override existing mappings)", &BEL_ST_Launcher_Handler_ImportControllerMappingsFromSteam)
#endif

static BEMenuItem *g_beControllerSettingsMenuItems[] = {
	&g_beControllerSettingsMenuItem_Action_Up,
	&g_beControllerSettingsMenuItem_Action_Down,
	&g_beControllerSettingsMenuItem_Action_Left,
	&g_beControllerSettingsMenuItem_Action_Right,
#ifdef REFKEEN_HAS_VER_KDREAMS
	&g_beControllerSettingsMenuItem_Action_Jump,
	&g_beControllerSettingsMenuItem_Action_Throw,
	&g_beControllerSettingsMenuItem_Action_Stats,
#endif
#if (defined REFKEEN_HAS_VER_CATACOMB_ALL) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	&g_beControllerSettingsMenuItem_Action_Fire,
	&g_beControllerSettingsMenuItem_Action_Strafe,
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	&g_beControllerSettingsMenuItem_Action_Drink,
	&g_beControllerSettingsMenuItem_Action_Bolt,
	&g_beControllerSettingsMenuItem_Action_Nuke,
	&g_beControllerSettingsMenuItem_Action_FastTurn,
#endif
#if (defined REFKEEN_HAS_VER_CAT3D) || (defined REFKEEN_HAS_VER_CATABYSS)
	&g_beControllerSettingsMenuItem_Action_Scrolls,
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	&g_beControllerSettingsMenuItem_Action_Use,
	&g_beControllerSettingsMenuItem_Action_Run,
	&g_beControllerSettingsMenuItem_Action_Weapon1,
	&g_beControllerSettingsMenuItem_Action_Weapon2,
	&g_beControllerSettingsMenuItem_Action_Weapon3,
	&g_beControllerSettingsMenuItem_Action_Weapon4,
	&g_beControllerSettingsMenuItem_Action_Weapon5,
	&g_beControllerSettingsMenuItem_Action_Weapon6,
	&g_beControllerSettingsMenuItem_Action_Map,
#endif
#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_CATADVENTURES) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
	&g_beControllerSettingsMenuItem_Action_FuncKeys,
#endif
	&g_beControllerSettingsMenuItem_Action_DebugKeys,
	&g_beControllerSettingsMenuItem_LeftStick,
	&g_beControllerSettingsMenuItem_RightStick,
#ifdef BE_ST_ENABLE_SETTING_ANALOGMOTION
	&g_beControllerSettingsMenuItem_AnalogMotion,
#endif
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
	&g_beControllerSettingsMenuItem_ImportMappingsFromSteam,
#endif
	NULL
};

BEMenu g_beControllerSettingsMenu = {
	"Modern controller settings",
	&g_beInputSettingsMenu,
	g_beControllerSettingsMenuItems,
	// Ignore the rest
};

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
/*** Controller mappings from Steam not found menu ***/

BEMENUITEM_DEF_TARGETMENU(g_beControllerMappingsFromSteamNotFoundMenuItem_GoBack, "Go back", &g_beControllerSettingsMenu)

static BEMenuItem *g_beControllerMappingsFromSteamNotFoundMenuItems[] = {
	&g_beControllerMappingsFromSteamNotFoundMenuItem_GoBack,
	NULL
};

static BEMenu g_beControllerMappingsFromSteamNotFoundMenu = {
	"Can't find or open mappings",
	&g_beControllerSettingsMenu,
	g_beControllerMappingsFromSteamNotFoundMenuItems,
	// Ignore the rest
};

/*** Failure to import controller mappings from Steam menu ***/

BEMENUITEM_DEF_TARGETMENU(g_beControllerMappingsFromSteamFailedToImportMenuItem_GoBack, "Go back", &g_beControllerSettingsMenu)

static BEMenuItem *g_beControllerMappingsFromSteamFailedToImportMenuItems[] = {
	&g_beControllerMappingsFromSteamFailedToImportMenuItem_GoBack,
	NULL
};

static BEMenu g_beControllerMappingsFromSteamFailedToImportMenu = {
	"Failed to import mappings!",
	&g_beControllerSettingsMenu,
	g_beControllerMappingsFromSteamFailedToImportMenuItems,
	// Ignore the rest
};

/*** Successfully imported controller mappings from Steam menu ***/

BEMENUITEM_DEF_TARGETMENU(g_beControllerMappingsFromSteamImportedSuccessfullyMenuItem_GoBack, "Go back", &g_beControllerSettingsMenu)

static BEMenuItem *g_beControllerMappingsFromSteamImportedSuccessfullyMenuItems[] = {
	&g_beControllerMappingsFromSteamImportedSuccessfullyMenuItem_GoBack,
	NULL
};

static BEMenu g_beControllerMappingsFromSteamImportedSuccessfullyMenu = {
	"Mappings imported successfully!",
	&g_beControllerSettingsMenu,
	g_beControllerMappingsFromSteamImportedSuccessfullyMenuItems,
	// Ignore the rest
};
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

#ifdef BE_ST_ENABLE_SETTING_LOWFPS
/*** Miscellaneous menu ***/

BEMENUITEM_DEF_SELECTION(g_beMiscSettingsMenuItem_LowFPS, "Low frame rate (compatibility option)", g_be_settingsChoices_boolean)

static BEMenuItem *g_beMiscSettingsMenuItems[] = {
	&g_beMiscSettingsMenuItem_LowFPS,
	NULL
};

BEMenu g_beMiscSettingsMenu = {
	"Miscellaneous",
	&g_beSettingsMenu,
	g_beMiscSettingsMenuItems,
	// Ignore the rest
};
#endif

/*** Show version menu ***/

BEMENUITEM_DEF_STATIC(g_beShowVersionMenuItem_Description, REFKEEN_TITLE_AND_VER_STRING)

static BEMenuItem *g_beShowVersionMenuItems[] = {
	&g_beShowVersionMenuItem_Description,
	NULL
};

BEMenu g_beShowVersionMenu = {
	"Version information",
	&g_beMainMenu,
	g_beShowVersionMenuItems,
	// Ignore the rest
};

/*** Quit confirm menu ***/

BEMENUITEM_DEF_HANDLER(g_beQuitConfirmMenuItem_Yes, "Yes", &BE_Launcher_Handler_MenuQuit)
BEMENUITEM_DEF_TARGETMENU(g_beQuitConfirmMenuItem_No, "No", &g_beMainMenu)

static BEMenuItem *g_beQuitConfirmMenuItems[] = {
	&g_beQuitConfirmMenuItem_Yes,
	&g_beQuitConfirmMenuItem_No,
	NULL
};

BEMenu g_beQuitConfirmMenu = {
	"Are you sure you want to quit?",
	&g_beMainMenu,
	g_beQuitConfirmMenuItems,
	// Ignore the rest
};

static void BEL_ST_Launcher_SetGfxOutputRects(void);

void BEL_ST_Launcher_RefreshSetArgumentsMenuItemLabel(void);

void BE_ST_Launcher_Prepare(void)
{
	int i;
	/*** Prepare ST stuff ***/

	/* Graphics */
	if (!g_refKeenCfg.winWidth || !g_refKeenCfg.winHeight || (g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_SOFTWARE))
	{
		g_refKeenCfg.winWidth = 2*BE_LAUNCHER_PIX_WIDTH;
		g_refKeenCfg.winHeight = 2*BE_LAUNCHER_PIX_HEIGHT;
	}

	bool vsync = BEL_ST_IsVsyncPreferred(true);
	bool fullScreen = (g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_FULL);
	bool resizable = (g_refKeenCfg.launcherWinType != LAUNCHER_WINDOW_SOFTWARE);
	BEL_ST_RecreateWindowAndRenderer(
		g_refKeenCfg.displayNum,
		g_refKeenCfg.winWidth, g_refKeenCfg.winHeight, 0, 0,
		fullScreen, resizable, vsync, -1
	);

	BEL_ST_SDLCreateTextureWrapper(&g_sdlTexture, BE_LAUNCHER_PIX_WIDTH, BE_LAUNCHER_PIX_HEIGHT, false, false);
	if (!g_sdlTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture for launcher,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}

	BEL_ST_SetDrawColor(0xFF000000); // For clears in refreshes
	BEL_ST_Launcher_SetGfxOutputRects();

	// Try, if we fail then simply don't use this
	if (g_refKeenCfg.launcherWinType != LAUNCHER_WINDOW_SOFTWARE)
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTargetTexture, 2*BE_LAUNCHER_PIX_WIDTH, 2*BE_LAUNCHER_PIX_HEIGHT, true, true);

	/* Game controllers */
	int nOfJoysticks = SDL_NumJoysticks();
	if (nOfJoysticks > BE_ST_MAXJOYSTICKS)
		nOfJoysticks = BE_ST_MAXJOYSTICKS;
	for (i = 0; i < nOfJoysticks; ++i)
		if (SDL_IsGameController(i))
			g_sdlControllers[i] = SDL_GameControllerOpen(i);

	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
	g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;

	// Check if there's a last chosen game version to show in main menu
	for (i = 0; i < g_be_gameinstallations_num; ++i)
		if (g_refKeenCfg.lastSelectedGameVer == BE_Cross_GetGameVerFromInstallation(i))
		{
			const char *exeDesc = BE_Cross_GetEXEFileDescriptionStrForGameVer(g_refKeenCfg.lastSelectedGameExe, g_refKeenCfg.lastSelectedGameVer);
			if (exeDesc)
				snprintf(g_beMainMenuItem_PlayLastChosenGameVer_label, sizeof(g_beMainMenuItem_PlayLastChosenGameVer_label), "Start %s: %s", BE_Cross_GetGameInstallationDescription(i), exeDesc);
			else
				snprintf(g_beMainMenuItem_PlayLastChosenGameVer_label, sizeof(g_beMainMenuItem_PlayLastChosenGameVer_label), "Start %s", BE_Cross_GetGameInstallationDescription(i));
			break;
		}
	if (i == g_be_gameinstallations_num)
		g_beMainMenu.menuItems++; // Shift the menu items (effectively removing the item above)

	BEL_ST_Launcher_RefreshSetArgumentsMenuItemLabel(); // Set menu item label based on arguments string

	/*** Prepare most of the launcher-accessible settings menu items ***/
	BE_Launcher_ReadSettings();

	/*** Prepare installed game versions menu ***/
	BE_ST_Launcher_RefreshSelectGameMenuContents();

	/*** Prepare root paths menu ***/
	int nOfRootPaths = BE_Cross_DirSelection_GetNumOfRootPaths();
	const char **rootPathsNames = BE_Cross_DirSelection_GetRootPathsNames();
	g_beSelectInitialPathMenuItems = (BEMenuItem *)malloc(nOfRootPaths*sizeof(BEMenuItem));
	g_beSelectInitialPathMenuItemsPtrs =  (BEMenuItem **)malloc((1+nOfRootPaths)*sizeof(BEMenuItem *));
	g_beSelectInitialPathMenuItemsStrsBuffer = (char *)malloc(nOfRootPaths*BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND);
	if (!g_beSelectInitialPathMenuItems || !g_beSelectInitialPathMenuItemsPtrs || !g_beSelectInitialPathMenuItemsStrsBuffer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BE_ST_Launcher_Prepare: Out of memory!\n");
		// Destroy window, renderer and more?
		exit(0);
	}

	g_beSelectInitialPathMenu.menuItems = g_beSelectInitialPathMenuItemsPtrs;
	char *label = g_beSelectInitialPathMenuItemsStrsBuffer;
	const char **rootPathNamePtr = rootPathsNames;
	for (i = 0; i < nOfRootPaths; ++i, label += BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND, ++rootPathNamePtr)
	{
		g_beSelectInitialPathMenuItemsPtrs[i] = &g_beSelectInitialPathMenuItems[i];
		g_beSelectInitialPathMenuItems[i].handler = &BE_Launcher_Handler_RootPathSelection;
		g_beSelectInitialPathMenuItems[i].choices = NULL;
		g_beSelectInitialPathMenuItems[i].targetMenu = NULL;
		g_beSelectInitialPathMenuItems[i].label = label;
		g_beSelectInitialPathMenuItems[i].type = BE_MENUITEM_TYPE_HANDLER;
		BE_Cross_safeandfastcstringcopy(label, label + BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND, *rootPathNamePtr);
	}
	g_beSelectInitialPathMenuItemsPtrs[nOfRootPaths] = NULL;

	/* Prepare a few menus *not* defined in be_launcher.h */
#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
	BE_Launcher_PrepareMenu(&g_beControllerMappingsFromSteamNotFoundMenu);
	BE_Launcher_PrepareMenu(&g_beControllerMappingsFromSteamFailedToImportMenu);
	BE_Launcher_PrepareMenu(&g_beControllerMappingsFromSteamImportedSuccessfullyMenu);
#endif
}


void BE_ST_Launcher_Shutdown(void)
{
	/*** Free a few launcher-specific resources ***/
	BE_Launcher_ClearDirSelectionMenu();

	/*** Clear ST stuff ***/
	for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
		if (g_sdlControllers[i])
		{
			SDL_GameControllerClose(g_sdlControllers[i]);
			g_sdlControllers[i] = NULL;
		}

	g_nOfTrackedFingers = 0;

	BEL_ST_SDLDestroyTextureWrapper(&g_sdlLauncherTextSearchTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlLauncherTextInputTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTexture);
	BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
	// BEFORE checking if we need to save anything, apply this HACK
	if (g_beInputSettingsMenuItem_TouchControls.choice == TOUCHINPUT_AUTO)
		g_sdlShowTouchUI = ((g_sdlLauncherLastEventType == SDL_FINGERDOWN) || (g_sdlLauncherLastEventType == SDL_FINGERUP));
	else
		g_sdlShowTouchUI = (g_beInputSettingsMenuItem_TouchControls.choice == TOUCHINPUT_FORCED);
#endif

	/*** Save settings if there's any change ***/
	if (!g_be_launcher_wasAnySettingChanged)
		return; // e.g., if there is a cfg value the launcher doesn't cope with (say, out of some range)

	BE_Launcher_UpdateSettings();
}


#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK
extern SDL_sem *g_sdlEventsCallbackToMainSem, *g_sdlMainToEventsCallbackSem;

static void BEL_ST_Launcher_CheckForExitFromEventsCallback(void)
{
	if (SDL_SemTryWait(g_sdlEventsCallbackToMainSem) == 0)
	{
		// Let's not call BE_ST_Launcher_Shutdown and/or BEL_ST_SaveConfig here
		SDL_SemPost(g_sdlMainToEventsCallbackSem);
		SDL_SemWait(g_sdlEventsCallbackToMainSem); // Wait here "forever"
	}
}
#else
#define BEL_ST_Launcher_CheckForExitFromEventsCallback()
#endif


void BE_ST_Launcher_RefreshSelectGameMenuContents(void)
{
	int i;
	for (i = 0; i < g_be_gameinstallations_num; ++i)
	{
		g_beSelectGameMenuItemsPtrs[i] = &g_beSelectGameMenuItems[i];
		g_beSelectGameMenuItems[i].handler = &BE_Launcher_Handler_GameLaunch;
		snprintf(g_beSelectGameMenuItemsStrs[i], sizeof(g_beSelectGameMenuItemsStrs[i]), "%s", BE_Cross_GetGameInstallationDescription(i));
		g_beSelectGameMenuItems[i].label = g_beSelectGameMenuItemsStrs[i];
		g_beSelectGameMenuItems[i].type = BE_MENUITEM_TYPE_HANDLER;
	}
	g_beSelectGameMenuItemsPtrs[i++] = &g_beSelectGameMenuItem_DisappearedGameHelp;
	g_beSelectGameMenuItemsPtrs[i++] = &g_beSelectGameMenuItem_ShowSupportedGameVersions;
	if (g_be_gameinstallations_num < BE_GAMEVER_LAST)
		g_beSelectGameMenuItemsPtrs[i++] = &g_beSelectGameMenuItem_AddMissingGameVersion;
	g_beSelectGameMenuItemsPtrs[i] = NULL;
}

void BEL_Launcher_SetCurrentMenu(BEMenu *menu);

void BE_ST_Launcher_RefreshAndShowSelectGameExeMenuContents(int verId, int nOfExes)
{
	if (nOfExes > MAX_NUM_OF_DISPLAYED_GAME_EXES_PER_VER)
	{
		char error[88];
		snprintf(error, sizeof(error), "BE_Launcher_RefreshAndShowSelectGameExeMenuContents: Too many EXEs!\n%d", nOfExes);
		BE_ST_ExitWithErrorMsg(error);
	}

	const char *exesPtrs[MAX_NUM_OF_DISPLAYED_GAME_EXES_PER_VER];
	BE_Cross_FillAccessibleEXEFileNamesForGameVer(verId, exesPtrs);

	int i;
	for (i = 0; i < nOfExes; ++i)
	{
		g_beSelectGameExeMenuItemsPtrs[i] = &g_beSelectGameExeMenuItems[i];
		g_beSelectGameExeMenuItems[i].handler = &BE_Launcher_Handler_GameLaunchWithChosenExe;
		snprintf(g_beSelectGameExeMenuItemsStrs[i], sizeof(g_beSelectGameExeMenuItemsStrs[i]), "%s", exesPtrs[i]);
		g_beSelectGameExeMenuItems[i].label = g_beSelectGameExeMenuItemsStrs[i];
		g_beSelectGameExeMenuItems[i].type = BE_MENUITEM_TYPE_HANDLER;
	}
	g_beSelectGameExeMenuItemsPtrs[i] = NULL;

	BE_Launcher_PrepareMenu(&g_beSelectGameExeMenu);
	BEL_Launcher_SetCurrentMenu(&g_beSelectGameExeMenu);
}


void BEL_ST_Launcher_RefreshSetArgumentsMenuItemLabel(void)
{
	// HACK
	if (*g_refKeenCfg.launcherExeArgs != '\0')
		strcpy(g_beMainMenuItem_SetArguments.label + 23, "*CURRENTLY SET*");
	else
		strcpy(g_beMainMenuItem_SetArguments.label + 23, "               ");
}


/*** SPECIAL - An extra SDL(2)-specific handler not defined in be_launcher.c ***/

#ifdef REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION
/* FIXME - This is incomplete! Go over mappings
 * from Steam config *and* refkeen mapping file,
 * and ask the user if it overwrite anything.
 * Also, avoid from needing an "append" wrapper over _tfopen.
 */
static void BEL_ST_Launcher_Handler_ImportControllerMappingsFromSteam(BEMenuItem **menuItemP)
{
	// WARNING: This opens file in BINARY mode, but that's ok
	FILE *cfgfp = BE_Cross_open_steamcfg_for_reading();
	if (!cfgfp)
	{
		BEL_Launcher_SetCurrentMenu(&g_beControllerMappingsFromSteamNotFoundMenu);
		return;
	}

	// WARNING: This file is also opened in BINARY mode
	// FIXME: Again, this is incomplete, for now we just
	// read the file-as-is, then rewrite the contents and add
	// more lines (we we don't need to add an "append" wrapper).
	void *mappingfpinmem = NULL;
	int32_t mappingfpsize = 0;
	FILE *mappingfp = BE_Cross_open_additionalfile_for_reading("gamecontrollerdb.txt");
	if (mappingfp)
	{
		mappingfpsize = BE_Cross_FileLengthFromHandle(mappingfp);
		mappingfpinmem = malloc(mappingfpsize);
		if (!mappingfpinmem)
		{
			fclose(mappingfp);
			fclose(cfgfp);
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BE_ST_Launcher_Handler_ImportControllerMappingsFromSteam: Out of memory!\n");
			// Destroy window, renderer and more?
			exit(0);
		}

		fread(mappingfpinmem, mappingfpsize, 1, mappingfp);
		fclose(mappingfp);
	}

	// Let's overwrite file with old contents (if there are any),
	// and then add new mappings (i.e., emulate file append)
	mappingfp = BE_Cross_open_additionalfile_for_overwriting("gamecontrollerdb.txt");
	if (!mappingfp)
	{
		fclose(cfgfp);
		free(mappingfpinmem); // Possibly NULL
		BEL_Launcher_SetCurrentMenu(&g_beControllerMappingsFromSteamFailedToImportMenu);
		// Destroy window, renderer and more?
		exit(0);
	}

	if (mappingfpinmem)
	{
		fwrite(mappingfpinmem, mappingfpsize, 1, mappingfp);
		free(mappingfpinmem);
	}

	char buffer[512];

	while (fgets(buffer, sizeof(buffer), cfgfp))
	{
		// Example for relevant config line:
		// 	"SDL_GamepadBind"		"03000000100800000100000010010000,Twin USB Joystick,a:b2,b:b1,y:b0,x:b3,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b4,righttrigger:b5,"

		const char * const key = "\"SDL_GamepadBind\"";
		/*const*/ char *substr = strstr(buffer, key);
		if (!substr)
			continue;
		substr = strchr(substr + strlen(key), '"');
		if (!substr)
			continue;
		++substr;
		/*const*/  char *guidend = strchr(substr, ',');
		if (!guidend || (guidend - substr != 32))
			continue;
		const char *nameend = strchr(guidend+1, ',');
		if (!nameend)
			continue;
		const char *substrend = strchr(nameend+1, '"');
		if (!substrend)
			continue;

		// HACK
		*guidend = '\0';
		SDL_JoystickGUID sdlJoyGuid = SDL_JoystickGetGUIDFromString(substr);

		char *mappingStr = SDL_GameControllerMappingForGUID(sdlJoyGuid);
		if (mappingStr)
		{
			SDL_free(mappingStr);
			// FIXME let's skip this for now, simpler to not add mapping to file, etc.
			continue;
		}

		// HACK again
		*guidend = ',';

		// And another HACK - Shift the contents...
		int movedChunkSize = substrend - substr;
		memmove(buffer, substr, movedChunkSize);
		// ...so we can make the room for the additional platform field
		snprintf(buffer + movedChunkSize, sizeof(buffer) - movedChunkSize, "platform:%s,\n", SDL_GetPlatform());
		// Finally write new mapping to file, and internally add it, too
		fwrite(buffer, strlen(buffer), 1, mappingfp);
		SDL_GameControllerAddMappingsFromRW(SDL_RWFromConstMem(buffer, strlen(buffer)), 1);
	}
	// Adding a mapping doesn't imply we'll get a "joystick/controller added" event,
	// so manually add what's missing
	int nJoysticksToScan = BE_Cross_TypedMin(int, SDL_NumJoysticks(), BE_ST_MAXJOYSTICKS);
	for (int i = 0; i < nJoysticksToScan; ++i)
		if (!g_sdlControllers[i] && SDL_IsGameController(i))
			g_sdlControllers[i] = SDL_GameControllerOpen(i);

	BEL_Launcher_SetCurrentMenu(&g_beControllerMappingsFromSteamImportedSuccessfullyMenu);
}
#endif // REFKEEN_CONFIG_CHECK_FOR_STEAM_INSTALLATION

/******/


void BEL_Launcher_DrawMenuItem(BEMenuItem *menuItem);

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
static void BEL_ST_Launcher_Handler_TouchInputDebugging(BEMenuItem **menuItemP)
{
	// Apply this immediately, so the effect is observed in the launcher itself
	g_refKeenCfg.touchInputDebugging = (*menuItemP)->choice;
}
#endif

static void BEL_ST_Launcher_SetGfxOutputRects(void)
{
	int winWidth, winHeight;
	BEL_ST_GetWindowSize(&winWidth, &winHeight);

	if (g_refKeenCfg.rememberDisplayNum)
	{
		int displayNum = BEL_ST_GetWindowDisplayNum();
		// HUGE FIXME - Bad idea!!!
		if (displayNum < (int)BE_Cross_ArrayLen(g_be_videoSettingsChoices_displayNums)) // Ignore last NULL entry
			if (1/*g_beVideoSettingsMenuItem_DisplayNum.choice != displayNum*/)
			{
				extern BEMenu *g_be_launcher_currMenu;

				g_beVideoSettingsMenuItem_DisplayNum.choice = displayNum;
				if (g_be_launcher_currMenu == &g_beVideoSettingsMenu)
					BEL_Launcher_DrawMenuItem(&g_beVideoSettingsMenuItem_DisplayNum);
			}
	}


	g_sdlLastReportedWindowWidth = winWidth;
	g_sdlLastReportedWindowHeight = winHeight;

	if (g_refKeenCfg.launcherWinType != LAUNCHER_WINDOW_FULL)
	{
		g_refKeenCfg.winWidth = winWidth;
		g_refKeenCfg.winHeight = winHeight;
	}

	if (BE_LAUNCHER_PIX_HEIGHT*winWidth < BE_LAUNCHER_PIX_WIDTH*winHeight) // Thinner than BE_LAUNCHER_PIX_WIDTH:BE_LAUNCHER_PIX_HEIGHT
	{
		g_sdlAspectCorrectionBorderedRect.w = winWidth;
		g_sdlAspectCorrectionBorderedRect.h = winWidth*BE_LAUNCHER_PIX_HEIGHT/BE_LAUNCHER_PIX_WIDTH;
		g_sdlAspectCorrectionBorderedRect.x = 0;
		g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
	}
	else // As wide as BE_LAUNCHER_PIX_WIDTH:BE_LAUNCHER_PIX_HEIGHT at the least
	{
		g_sdlAspectCorrectionBorderedRect.w = winHeight*BE_LAUNCHER_PIX_WIDTH/BE_LAUNCHER_PIX_HEIGHT;
		g_sdlAspectCorrectionBorderedRect.h = winHeight;
		g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
		g_sdlAspectCorrectionBorderedRect.y = 0;
	}

	// This is similar to code in be_st_sdl_graphics.c (possibly with minor differences)

	g_sdlControllerLauncherTextSearchRect.w = winWidth;
	g_sdlControllerLauncherTextSearchRect.h = winHeight/2;
	if (g_sdlControllerLauncherTextSearchRect.w * ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT > g_sdlControllerLauncherTextSearchRect.h * ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH)
		g_sdlControllerLauncherTextSearchRect.w = g_sdlControllerLauncherTextSearchRect.h * ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH / ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT;
	else
		g_sdlControllerLauncherTextSearchRect.h = g_sdlControllerLauncherTextSearchRect.w * ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT / ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH;
	g_sdlControllerLauncherTextSearchRect.x = (winWidth-g_sdlControllerLauncherTextSearchRect.w)/2;
	g_sdlControllerLauncherTextSearchRect.y = winHeight-g_sdlControllerLauncherTextSearchRect.h;

	g_sdlControllerLauncherTextInputRect.w = winWidth;
	g_sdlControllerLauncherTextInputRect.h = winHeight*3/8;
	if (g_sdlControllerLauncherTextInputRect.w * ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT > g_sdlControllerLauncherTextInputRect.h * ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH)
		g_sdlControllerLauncherTextInputRect.w = g_sdlControllerLauncherTextInputRect.h * ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH / ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT;
	else
		g_sdlControllerLauncherTextInputRect.h = g_sdlControllerLauncherTextInputRect.w * ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT / ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH;
	g_sdlControllerLauncherTextInputRect.x = (winWidth-g_sdlControllerLauncherTextInputRect.w)/2;
	g_sdlControllerLauncherTextInputRect.y = winHeight-g_sdlControllerLauncherTextInputRect.h;

	int minWinDim = (winWidth >= winHeight) ? winHeight : winWidth;
	g_sdlDebugFingerRectSideLen = minWinDim/4;
}

void BE_ST_Launcher_MarkGfxCache(void)
{
	g_sdlLauncherGfxCacheMarked = true;
}

uint8_t *BE_ST_Launcher_GetGfxPtr(void)
{
	return g_sdlLauncherGfxCache;
}

static void BEL_ST_Launcher_NormalizePos(int *px, int *py)
{
	*px = BE_LAUNCHER_PIX_WIDTH*(*px-g_sdlAspectCorrectionBorderedRect.x)/g_sdlAspectCorrectionBorderedRect.w;
	*py = BE_LAUNCHER_PIX_HEIGHT*(*py-g_sdlAspectCorrectionBorderedRect.y)/g_sdlAspectCorrectionBorderedRect.h;
}

// Translates our internal key codes (usually the same as SDL2 scancodes) to SDL2 scancodes
static int BEL_ST_Launcher_TranslateInternalKeyCode(int code)
{
	switch (code)
	{
		// NOTE: No need for shift code for now
		case ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_LEFT:
			return SDL_SCANCODE_LEFT;
		case ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_RIGHT:
			return SDL_SCANCODE_RIGHT;
		case ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_DELETE:
			return SDL_SCANCODE_DELETE;
		case ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_HOME:
			return SDL_SCANCODE_HOME;
		case ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_END:
			return SDL_SCANCODE_END;
	}
	return code;
}

// Internal codes names for on-screen keyboards in non-shifted state,
// based on SDL2 scancodes, with a few exceptions (where our internal codes are used)

static const char *g_sdlIntCodeKeyboardUINonShiftedStrs[] = {
	NULL, "\x1B", "\x1A", "\x1E",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"Ent", NULL, "\x11", "Del", "[_]", // Note: SDL2 scancode of Tab is reused for Del
	"-", "=", "[", "]", "\\", "\x1B\x1B", ";", "'", "`", ",", ".", "/", // Note: SDL scancode of Non-US Hash is reused for Home
	"\x1A\x1A", // Note: SDL scancode of Caps Lock is reused for End
};

static const char *g_sdlIntCodeKeyboardUIShiftedStrs[] = {
	NULL, "\x1B", "\x1A", "\x1E",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
	"Ent", NULL, "\x11", "Del", "[_]", // Note: SDL2 scancode of Tab is reused for Del
	"_", "+", "{", "}", "|", "\x1B\x1B", ":", "\"", "~", "<", ">", "?", // Note: SDL scancode of Non-US Hash is reused for Home
	"\x1A\x1A", // Note: SDL scancode of Caps Lock is reused for End
};

// Keyboard UI: One of the shifted and non-shifted variants
static const char **g_sdlIntScanCodeKeyboardUIStrs_Ptr;

// Text search keyboard layout definition

static const int g_sdlIntScanCodeTextSearchLayout[ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT][ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH] = {
	{SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0},
	{SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_T, SDL_SCANCODE_Y, SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_P},
	{SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_SEMICOLON},
	{SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V, SDL_SCANCODE_B, SDL_SCANCODE_N, SDL_SCANCODE_M, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH},
	{ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT, SDL_SCANCODE_RETURN, SDL_SCANCODE_SPACE, SDL_SCANCODE_MINUS, SDL_SCANCODE_EQUALS, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET, SDL_SCANCODE_BACKSLASH, SDL_SCANCODE_APOSTROPHE, SDL_SCANCODE_GRAVE},
};

// Text input keyboard layout definition

static const int g_sdlIntScanCodeTextInputLayout[ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT][ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH] = {
	{SDL_SCANCODE_GRAVE, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0, SDL_SCANCODE_MINUS, SDL_SCANCODE_EQUALS, SDL_SCANCODE_BACKSPACE},
	{ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_LEFT, SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_T, SDL_SCANCODE_Y, SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_P, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET, ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_HOME},
	{ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_RIGHT, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_APOSTROPHE, SDL_SCANCODE_BACKSLASH, ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_END},
	{ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT, SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V, SDL_SCANCODE_B, SDL_SCANCODE_N, SDL_SCANCODE_M, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH, SDL_SCANCODE_SPACE, ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_DELETE, SDL_SCANCODE_RETURN},
};


static void BEL_ST_Launcher_CreateTextSearchTextureIfNeeded(void)
{
	if (g_sdlLauncherTextSearchTexture)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(&g_sdlLauncherTextSearchTexture, ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH, ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_HEIGHT, false, false);
	if (!g_sdlLauncherTextSearchTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 launcher text search texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	BEL_ST_SetTextureBlendMode(g_sdlLauncherTextSearchTexture, true);
}

static void BEL_ST_Launcher_CreateTextInputTextureIfNeeded(void)
{
	if (g_sdlLauncherTextInputTexture)
	{
		return;
	}
	BEL_ST_SDLCreateTextureWrapper(&g_sdlLauncherTextInputTexture, ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH, ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_HEIGHT, false, false);
	if (!g_sdlLauncherTextInputTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 launcher text input texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	BEL_ST_SetTextureBlendMode(g_sdlLauncherTextInputTexture, true);
}

// borrowed from be_st_sdl_graphics.c
void BEL_ST_RedrawKeyToBuffer(uint32_t *picPtr, int picWidth, const char *text, bool isMarked, bool isPressed);

static void BEL_ST_Launcher_RedrawWholeTextSearchUI(void)
{
	uint32_t pixels[ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_HEIGHT];
	uint32_t *currPtr = pixels;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT; ++currKeyRow, currPtr += ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1))
	{
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH; ++currKeyCol, currPtr += ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
		{
			BEL_ST_RedrawKeyToBuffer(currPtr, ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextSearchLayout[currKeyRow][currKeyCol]], false, false);
		}
	}
	// Simpler to do so outside the loop
	BEL_ST_RedrawKeyToBuffer(pixels + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*g_sdlKeyboardUISelectedKeyX) + ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT*g_sdlKeyboardUISelectedKeyY), ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextSearchLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]], true, g_sdlKeyboardUIIsKeyPressed);

	BEL_ST_UpdateTexture(g_sdlLauncherTextSearchTexture, NULL, pixels, 4*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH);
}

static void BEL_ST_Launcher_RedrawWholeTextInputUI(void)
{
	uint32_t pixels[ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH*ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_HEIGHT];
	uint32_t *currPtr = pixels;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT; ++currKeyRow, currPtr += ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1))
	{
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH; ++currKeyCol, currPtr += ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
		{
			BEL_ST_RedrawKeyToBuffer(currPtr, ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextInputLayout[currKeyRow][currKeyCol]], false, false);
		}
	}
	// Simpler to do so outside the loop
	BEL_ST_RedrawKeyToBuffer(pixels + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*g_sdlKeyboardUISelectedKeyX) + ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT*g_sdlKeyboardUISelectedKeyY), ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]], true, g_sdlKeyboardUIIsKeyPressed);

	BEL_ST_UpdateTexture(g_sdlLauncherTextInputTexture, NULL, pixels, 4*ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH);
}


void BEL_ST_Launcher_ToggleTextSearch(void)
{
	g_sdlLauncherTextSearchUIIsShown = !g_sdlLauncherTextSearchUIIsShown;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;

	g_nOfTrackedFingers = 0;

	if (!g_sdlLauncherTextSearchUIIsShown)
		return;

	BEL_ST_Launcher_CreateTextSearchTextureIfNeeded();

	g_sdlKeyboardUISelectedKeyX = 0;
	g_sdlKeyboardUISelectedKeyY = 0;
	g_sdlKeyboardUISelectedKeyIsMarked = true;
	g_sdlKeyboardUIIsKeyPressed = false;
	g_sdlKeyboardUIIsShifted = false;
	g_sdlIntScanCodeKeyboardUIStrs_Ptr = g_sdlIntCodeKeyboardUINonShiftedStrs;

	BEL_ST_Launcher_RedrawWholeTextSearchUI();
	//g_sdlTextInputUIIsShown = true;

	//BEL_ST_ConditionallyShowAltInputPointer();
}

void BEL_ST_Launcher_ToggleTextInput(void)
{
	g_sdlLauncherTextInputUIIsShown = !g_sdlLauncherTextInputUIIsShown;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;

	g_nOfTrackedFingers = 0;

	if (!g_sdlLauncherTextInputUIIsShown)
		return;

	BEL_ST_Launcher_CreateTextInputTextureIfNeeded();

	g_sdlKeyboardUISelectedKeyX = 0;
	g_sdlKeyboardUISelectedKeyY = 0;
	g_sdlKeyboardUISelectedKeyIsMarked = true;
	g_sdlKeyboardUIIsKeyPressed = false;
	g_sdlKeyboardUIIsShifted = false;
	g_sdlIntScanCodeKeyboardUIStrs_Ptr = g_sdlIntCodeKeyboardUINonShiftedStrs;

	BEL_ST_Launcher_RedrawWholeTextInputUI();
	//g_sdlTextInputUIIsShown = true;

	//BEL_ST_ConditionallyShowAltInputPointer();
}

void BEL_ST_Launcher_TurnTextSearchOff(void)
{
	g_sdlLauncherTextSearchUIIsShown = false;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;

	g_nOfTrackedFingers = 0;
}

void BEL_ST_Launcher_TurnTextInputOff(void)
{
	g_sdlLauncherTextInputUIIsShown = false;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;

	g_nOfTrackedFingers = 0;
}

static void BEL_ST_Launcher_ToggleTextSearchUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextSearchLayout[y][x]], isMarked, isPressed);

	BE_ST_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	BEL_ST_UpdateTexture(g_sdlLauncherTextSearchTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}

static void BEL_ST_Launcher_ToggleTextInputUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextInputLayout[y][x]], isMarked, isPressed);

	BE_ST_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	BEL_ST_UpdateTexture(g_sdlLauncherTextInputTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}


static void BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers(void)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlTrackedFingers[i].miscData.key.x, g_sdlTrackedFingers[i].miscData.key.y, false, false);

	g_nOfTrackedFingers = 0;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers(void)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlTrackedFingers[i].miscData.key.x, g_sdlTrackedFingers[i].miscData.key.y, false, false);

	g_nOfTrackedFingers = 0;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_ToggleShiftStateInTextSearchUI(void);
static void BEL_ST_Launcher_ToggleShiftStateInTextInputUI(void);
static void BEL_ST_Launcher_HandleKeyPressEvent(int scancode, bool isShifted);
static bool BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(int scancode, bool isShifted, bool *pConfirmed);

static void BEL_ST_Launcher_DoKeyPressInTextSearchUI(int scanCode)
{
	if (scanCode == ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT)
		BEL_ST_Launcher_ToggleShiftStateInTextSearchUI();
	else
		BEL_ST_Launcher_HandleKeyPressEvent(BEL_ST_Launcher_TranslateInternalKeyCode(scanCode), g_sdlKeyboardUIIsShifted);
}

static bool BEL_ST_Launcher_DoKeyPressInTextInputUI(int scanCode, bool *pConfirmed)
{
	if (scanCode == ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT)
	{
		BEL_ST_Launcher_ToggleShiftStateInTextInputUI();
		return false;
	}
	else
		return BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(BEL_ST_Launcher_TranslateInternalKeyCode(scanCode), g_sdlKeyboardUIIsShifted, pConfirmed);
}


static void BEL_ST_Launcher_MoveUpInTextSearchUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		--g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY < 0)
			g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT-1;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_MoveDownInTextSearchUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		++g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY >= ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT)
			g_sdlKeyboardUISelectedKeyY = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_MoveLeftInTextSearchUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		--g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX < 0)
			g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH-1;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_MoveRightInTextSearchUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		++g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX >= ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH)
			g_sdlKeyboardUISelectedKeyX = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}


static void BEL_ST_Launcher_MoveUpInTextInputUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		--g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY < 0)
			g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT-1;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_MoveDownInTextInputUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		++g_sdlKeyboardUISelectedKeyY;
		if (g_sdlKeyboardUISelectedKeyY >= ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT)
			g_sdlKeyboardUISelectedKeyY = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_MoveLeftInTextInputUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		--g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX < 0)
			g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH-1;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH-1;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_MoveRightInTextInputUI(void)
{
	BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUIIsKeyPressed = false;
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

		++g_sdlKeyboardUISelectedKeyX;
		if (g_sdlKeyboardUISelectedKeyX >= ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH)
			g_sdlKeyboardUISelectedKeyX = 0;
	}
	else
	{
		g_sdlKeyboardUISelectedKeyX = 0;
		g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
	}
	BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}


static void BEL_ST_Launcher_ToggleShiftStateInTextSearchUI(void)
{
	g_sdlKeyboardUIIsShifted = !g_sdlKeyboardUIIsShifted;
	g_sdlIntScanCodeKeyboardUIStrs_Ptr = g_sdlKeyboardUIIsShifted ? g_sdlIntCodeKeyboardUIShiftedStrs : g_sdlIntCodeKeyboardUINonShiftedStrs;

	BEL_ST_Launcher_RedrawWholeTextSearchUI();

	//BEL_ST_Launcher_HandleKeyPressEvent(BEL_ST_Launcher_TranslateInternalKeyCode(ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT), g_sdlKeyboardUIIsShifted);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_ToggleShiftStateInTextInputUI(void)
{
	g_sdlKeyboardUIIsShifted = !g_sdlKeyboardUIIsShifted;
	g_sdlIntScanCodeKeyboardUIStrs_Ptr = g_sdlKeyboardUIIsShifted ? g_sdlIntCodeKeyboardUIShiftedStrs : g_sdlIntCodeKeyboardUINonShiftedStrs;

	BEL_ST_Launcher_RedrawWholeTextInputUI();

	//BEL_ST_Launcher_HandleKeyPressEvent(BEL_ST_Launcher_TranslateInternalKeyCode(ALTCONTROLLER_LAUNCHER_KEYBOARD_INTERNALCODE_SHIFT), g_sdlKeyboardUIIsShifted);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_ToggleKeyPressInTextSearchUI(bool toggle)
{
	BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers();

	if (!g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUISelectedKeyX = g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);
		//g_sdlForceGfxControlUiRefresh = true;
		BE_ST_Launcher_MarkGfxCache();
		return;
	}

	// Don't check - support gamepad button press repeat
	//if (g_sdlKeyboardUIIsKeyPressed == toggle)
	//	return;
	g_sdlKeyboardUIIsKeyPressed = toggle;

	BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, toggle);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	if (toggle)
		BEL_ST_Launcher_DoKeyPressInTextSearchUI(g_sdlIntScanCodeTextSearchLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);
}

static bool BEL_ST_Launcher_ToggleKeyPressInTextInputUI(bool toggle, bool *pConfirmed)
{
	BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers();

	if (!g_sdlKeyboardUISelectedKeyIsMarked)
	{
		g_sdlKeyboardUISelectedKeyX = g_sdlKeyboardUISelectedKeyY = 0;
		g_sdlKeyboardUISelectedKeyIsMarked = true;
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);
		//g_sdlForceGfxControlUiRefresh = true;
		BE_ST_Launcher_MarkGfxCache();
		return false;
	}

	// Don't check - support gamepad button press repeat
	//if (g_sdlKeyboardUIIsKeyPressed == toggle)
	//	return false;
	g_sdlKeyboardUIIsKeyPressed = toggle;

	BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, toggle);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	if (toggle)
		return BEL_ST_Launcher_DoKeyPressInTextInputUI(g_sdlIntScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], pConfirmed);

	return false;
}

/*** Pointer stuff common to all kinds of controller / touch input UI ***/

static BESDLTrackedFinger *BEL_ST_Launcher_ProcessAndGetPressedTrackedFinger(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
		{
			if (g_refKeenCfg.touchInputDebugging)
			{
				g_sdlTrackedFingers[i].lastX = x;
				g_sdlTrackedFingers[i].lastY = y;
				//g_sdlForceGfxControlUiRefresh = true;
				BE_ST_Launcher_MarkGfxCache();
			}
			return NULL; // In case of some mistaken double-tap of same finger
		}

	if (g_nOfTrackedFingers == MAX_NUM_OF_TRACKED_FINGERS)
		return NULL;

	BESDLTrackedFinger *trackedFinger = &g_sdlTrackedFingers[g_nOfTrackedFingers++];
	trackedFinger->touchId = touchId;
	trackedFinger->fingerId = fingerId;
	if (g_refKeenCfg.touchInputDebugging)
	{
		trackedFinger->lastX = x;
		trackedFinger->lastY = y;
		//g_sdlForceGfxControlUiRefresh = true;
		BE_ST_Launcher_MarkGfxCache();
	}
	return trackedFinger;
}

static BESDLTrackedFinger *BEL_ST_Launcher_ProcessAndGetMovedTrackedFinger(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
			break;

	if (i == g_nOfTrackedFingers)
		return NULL;

	BESDLTrackedFinger *trackedFinger = &g_sdlTrackedFingers[i];
	if (g_refKeenCfg.touchInputDebugging)
	{
		trackedFinger->lastX = x;
		trackedFinger->lastY = y;
		//g_sdlForceGfxControlUiRefresh = true;
		BE_ST_Launcher_MarkGfxCache();
	}
	return trackedFinger;
}

static BESDLTrackedFinger *BEL_ST_Launcher_GetReleasedTrackedFinger(BE_ST_TouchID touchId, BE_ST_FingerID fingerId)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
			return &g_sdlTrackedFingers[i];

	return NULL;
}

static void BEL_ST_Launcher_RemoveTrackedFinger(BESDLTrackedFinger *trackedFinger)
{
	*trackedFinger = g_sdlTrackedFingers[--g_nOfTrackedFingers]; // Remove finger entry without moving the rest, except for maybe the last
	if (g_refKeenCfg.touchInputDebugging)
		//g_sdlForceGfxControlUiRefresh = true;
		BE_ST_Launcher_MarkGfxCache(); // Remove debugging finger mark from screen
}

static void BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextSearchUI(void)
{
	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlKeyboardUIIsKeyPressed)
		{
			//BEL_ST_Launcher_ChangeKeyStateInTextSearchUI(g_sdlDOSScanCodeTextSearchLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlKeyboardUIIsKeyPressed = false;
		}
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
		g_sdlKeyboardUISelectedKeyIsMarked = false;
	}
}

static void BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextInputUI(void)
{
	if (g_sdlKeyboardUISelectedKeyIsMarked)
	{
		if (g_sdlKeyboardUIIsKeyPressed)
		{
			//BEL_ST_Launcher_ChangeKeyStateInTextInputUI(g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX], false);
			g_sdlKeyboardUIIsKeyPressed = false;
		}
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
		g_sdlKeyboardUISelectedKeyIsMarked = false;
	}
}


static void BEL_ST_Launcher_CheckMovedPointerInTextSearchUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerLauncherTextSearchRect.x) || (x >= g_sdlControllerLauncherTextSearchRect.x+g_sdlControllerLauncherTextSearchRect.w)
	    || (y < g_sdlControllerLauncherTextSearchRect.y) || (y >= g_sdlControllerLauncherTextSearchRect.y+g_sdlControllerLauncherTextSearchRect.h))
		return;

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextSearchUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerLauncherTextSearchRect.x)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH/g_sdlControllerLauncherTextSearchRect.w;
	int keyY = (y-g_sdlControllerLauncherTextSearchRect.y)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT/g_sdlControllerLauncherTextSearchRect.h;

	if ((trackedFinger->miscData.key.x != keyX) || (trackedFinger->miscData.key.y != keyY))
	{
		BEL_ST_Launcher_ToggleTextSearchUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, false, false);
		BEL_ST_Launcher_ToggleTextSearchUIKey(keyX, keyY, true, false);
		trackedFinger->miscData.key.x = keyX;
		trackedFinger->miscData.key.y = keyY;
	}

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_CheckPressedPointerInTextSearchUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerLauncherTextSearchRect.x) || (x >= g_sdlControllerLauncherTextSearchRect.x+g_sdlControllerLauncherTextSearchRect.w)
	    || (y < g_sdlControllerLauncherTextSearchRect.y) || (y >= g_sdlControllerLauncherTextSearchRect.y+g_sdlControllerLauncherTextSearchRect.h))
	{
		//BEL_ST_Launcher_RemoveTrackedFinger(trackedFinger);
		BEL_ST_Launcher_TurnTextSearchOff();
		return;
	}

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextSearchUI();

	// Normalize coordinates to keys
	trackedFinger->miscData.key.x = (x-g_sdlControllerLauncherTextSearchRect.x)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH/g_sdlControllerLauncherTextSearchRect.w;
	trackedFinger->miscData.key.y = (y-g_sdlControllerLauncherTextSearchRect.y)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT/g_sdlControllerLauncherTextSearchRect.h;

	BEL_ST_Launcher_ToggleTextSearchUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_CheckReleasedPointerInTextSearchUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_Launcher_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return;

	int prevKeyX = trackedFinger->miscData.key.x;
	int prevKeyY = trackedFinger->miscData.key.y;

	BEL_ST_Launcher_RemoveTrackedFinger(trackedFinger);

	if ((x >= g_sdlControllerLauncherTextSearchRect.x) && (x < g_sdlControllerLauncherTextSearchRect.x+g_sdlControllerLauncherTextSearchRect.w)
	    && (y >= g_sdlControllerLauncherTextSearchRect.y) && (y < g_sdlControllerLauncherTextSearchRect.y+g_sdlControllerLauncherTextSearchRect.h))
		// Do key press (including any special handling possibly required for shift key)
		BEL_ST_Launcher_DoKeyPressInTextSearchUI(g_sdlIntScanCodeTextSearchLayout[prevKeyY][prevKeyX]);

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextSearchUI();
	BEL_ST_Launcher_ToggleTextSearchUIKey(prevKeyX, prevKeyY, false, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	return;
}


static void BEL_ST_Launcher_CheckMovedPointerInTextInputUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerLauncherTextInputRect.x) || (x >= g_sdlControllerLauncherTextInputRect.x+g_sdlControllerLauncherTextInputRect.w)
	    || (y < g_sdlControllerLauncherTextInputRect.y) || (y >= g_sdlControllerLauncherTextInputRect.y+g_sdlControllerLauncherTextInputRect.h))
		return;

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextInputUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerLauncherTextInputRect.x)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerLauncherTextInputRect.w;
	int keyY = (y-g_sdlControllerLauncherTextInputRect.y)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerLauncherTextInputRect.h;

	if ((trackedFinger->miscData.key.x != keyX) || (trackedFinger->miscData.key.y != keyY))
	{
		BEL_ST_Launcher_ToggleTextInputUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, false, false);
		BEL_ST_Launcher_ToggleTextInputUIKey(keyX, keyY, true, false);
		trackedFinger->miscData.key.x = keyX;
		trackedFinger->miscData.key.y = keyY;
	}

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_CheckPressedPointerInTextInputUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	BESDLTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerLauncherTextInputRect.x) || (x >= g_sdlControllerLauncherTextInputRect.x+g_sdlControllerLauncherTextInputRect.w)
	    || (y < g_sdlControllerLauncherTextInputRect.y) || (y >= g_sdlControllerLauncherTextInputRect.y+g_sdlControllerLauncherTextInputRect.h))
	{
		//BEL_ST_Launcher_RemoveTrackedFinger(trackedFinger);
		BEL_ST_Launcher_TurnTextInputOff();
		return;
	}

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextInputUI();

	// Normalize coordinates to keys
	trackedFinger->miscData.key.x = (x-g_sdlControllerLauncherTextInputRect.x)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerLauncherTextInputRect.w;
	trackedFinger->miscData.key.y = (y-g_sdlControllerLauncherTextInputRect.y)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerLauncherTextInputRect.h;

	BEL_ST_Launcher_ToggleTextInputUIKey(trackedFinger->miscData.key.x, trackedFinger->miscData.key.y, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static bool BEL_ST_Launcher_CheckReleasedPointerInTextInputUI(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y, bool *pConfirmed)
{
	bool ret = false;
	BESDLTrackedFinger *trackedFinger = BEL_ST_Launcher_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return ret;

	int prevKeyX = trackedFinger->miscData.key.x;
	int prevKeyY = trackedFinger->miscData.key.y;

	BEL_ST_Launcher_RemoveTrackedFinger(trackedFinger);

	if ((x >= g_sdlControllerLauncherTextInputRect.x) && (x < g_sdlControllerLauncherTextInputRect.x+g_sdlControllerLauncherTextInputRect.w)
	    && (y >= g_sdlControllerLauncherTextInputRect.y) && (y < g_sdlControllerLauncherTextInputRect.y+g_sdlControllerLauncherTextInputRect.h))
		// Do key press (including any special handling possibly required for shift key)
		ret = BEL_ST_Launcher_DoKeyPressInTextInputUI(g_sdlIntScanCodeTextInputLayout[prevKeyY][prevKeyX], pConfirmed);

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextInputUI();
	BEL_ST_Launcher_ToggleTextInputUIKey(prevKeyX, prevKeyY, false, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	return ret;
}



// Maps SDL scancodes to corresponding ASCII chars (including shifted state)
static const char g_sdlScancodeToNonShiftedCharMap[] = "\0\0\0\0abcdefghijklmnopqrstuvwxyz1234567890\0\0\0\0 -=[]\\\0;'`,./";
static const char g_sdlScancodeToShiftedCharMap[] = "\0\0\0\0ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()\0\0\0\0 _+{}|\0:\"~<>?";

static void BEL_ST_Launcher_HandleKeyPressEvent(int scancode, bool isShifted)
{
	switch (scancode)
	{
	case SDL_SCANCODE_PAGEUP:
		BE_Launcher_HandleInput_ButtonPageUp();
		break;
	case SDL_SCANCODE_PAGEDOWN:
		BE_Launcher_HandleInput_ButtonPageDown();
		break;
	case SDL_SCANCODE_RIGHT:
		BE_Launcher_HandleInput_ButtonRight();
		break;
	case SDL_SCANCODE_LEFT:
		BE_Launcher_HandleInput_ButtonLeft();
		break;
	case SDL_SCANCODE_DOWN:
		BE_Launcher_HandleInput_ButtonDown();
		break;
	case SDL_SCANCODE_UP:
		BE_Launcher_HandleInput_ButtonUp();
		break;
	case SDL_SCANCODE_RETURN:
		BE_Launcher_HandleInput_ButtonActivate();
		break;
	case SDL_SCANCODE_ESCAPE:
	case SDL_SCANCODE_AC_BACK:
		BE_Launcher_HandleInput_ButtonBack();
		break;
	case SDL_SCANCODE_AC_SEARCH:
		BE_Launcher_HandleInput_ButtonSearch();
		break;
	default:
		if (((scancode >= SDL_SCANCODE_A) && (scancode <= SDL_SCANCODE_Z)) || // Letters
		    ((scancode >= SDL_SCANCODE_1) && (scancode <= SDL_SCANCODE_0)) || // Digits
		    ((scancode >= SDL_SCANCODE_SPACE) && (scancode <= SDL_SCANCODE_SLASH) && (scancode != SDL_SCANCODE_NONUSHASH)) // A few other keys
		)
			BE_Launcher_HandleInput_ASCIIChar((isShifted ? g_sdlScancodeToShiftedCharMap : g_sdlScancodeToNonShiftedCharMap)[scancode]);
		break;
	}
}


static void BEL_ST_Launcher_HandleTextSearchEvent(Uint8 but, bool isPressed)
{
	switch (but)
	{
	case BE_ST_CTRL_BUT_DPAD_UP:
		if (isPressed)
			BEL_ST_Launcher_MoveUpInTextSearchUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_DOWN:
		if (isPressed)
			BEL_ST_Launcher_MoveDownInTextSearchUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_LEFT:
		if (isPressed)
			BEL_ST_Launcher_MoveLeftInTextSearchUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_Launcher_MoveRightInTextSearchUI();
		break;
	// A few other special cases
	case BE_ST_CTRL_BUT_B:
	case BE_ST_CTRL_BUT_BACK:
		if (isPressed)
			BEL_ST_Launcher_HandleKeyPressEvent(SDL_SCANCODE_ESCAPE, g_sdlKeyboardUIIsShifted);
		break;
	case BE_ST_CTRL_BUT_X:
		if (isPressed)
			BEL_ST_Launcher_ToggleShiftStateInTextSearchUI();
		break;
	default:
	{
		// Select key from UI.
		BEL_ST_Launcher_ToggleKeyPressInTextSearchUI(isPressed);
	}
	}
}

static void BEL_ST_Launcher_HandleControllerButtonEvent(Uint8 but, bool isPressed)
{
	// Special case (applies with and without on-screen keyboard)
	if ((but == BE_ST_CTRL_BUT_Y) && isPressed)
	{
		BEL_ST_Launcher_ToggleTextSearch();
		return;
	}

	if (g_sdlLauncherTextSearchUIIsShown)
	{
		BEL_ST_Launcher_HandleTextSearchEvent(but, isPressed);
		return;
	}

	if (!isPressed)
		return;

	switch (but)
	{
	case BE_ST_CTRL_BUT_A:
		BE_Launcher_HandleInput_ButtonActivate();
		break;
	case BE_ST_CTRL_BUT_B:
	case BE_ST_CTRL_BUT_BACK:
		BE_Launcher_HandleInput_ButtonBack();
		break;
	case BE_ST_CTRL_BUT_LSHOULDER:
		BE_Launcher_HandleInput_ButtonPageUp();
		break;
	case BE_ST_CTRL_BUT_RSHOULDER:
		BE_Launcher_HandleInput_ButtonPageDown();
		break;
	case BE_ST_CTRL_BUT_DPAD_UP:
		BE_Launcher_HandleInput_ButtonUp();
		break;
	case BE_ST_CTRL_BUT_DPAD_DOWN:
		BE_Launcher_HandleInput_ButtonDown();
		break;
	case BE_ST_CTRL_BUT_DPAD_LEFT:
		BE_Launcher_HandleInput_ButtonLeft();
		break;
	case BE_ST_CTRL_BUT_DPAD_RIGHT:
		BE_Launcher_HandleInput_ButtonRight();
		break;
	}
}


static void BEL_ST_Launcher_FinishHostDisplayUpdate(void)
{
	//g_sdlForceGfxControlUiRefresh = false;

	if (g_refKeenCfg.touchInputDebugging)
	{
		BEL_ST_SetDrawColor(0xBFFF0000); // Includes some alpha
		BEL_ST_SetDrawBlendMode(true);
		BESDLTrackedFinger *trackedFinger = g_sdlTrackedFingers;
		for (int i = 0; i < g_nOfTrackedFingers; ++i, ++trackedFinger)
		{
			BE_ST_Rect rect = {trackedFinger->lastX-g_sdlDebugFingerRectSideLen/2, trackedFinger->lastY-g_sdlDebugFingerRectSideLen/2, g_sdlDebugFingerRectSideLen, g_sdlDebugFingerRectSideLen};
			BEL_ST_RenderFill(&rect);
		}
		BEL_ST_SetDrawColor(0xFF000000);
		BEL_ST_SetDrawBlendMode(false);
	}

	if (g_sdlLauncherTextSearchUIIsShown)
		BEL_ST_RenderFromTexture(g_sdlLauncherTextSearchTexture, &g_sdlControllerLauncherTextSearchRect);
	else if (g_sdlLauncherTextInputUIIsShown)
		BEL_ST_RenderFromTexture(g_sdlLauncherTextInputTexture, &g_sdlControllerLauncherTextInputRect);


        BEL_ST_UpdateWindow();
}


static uint32_t g_be_sdlLastRefreshTicks = 0;

static void BEL_ST_Launcher_UpdateHostDisplay(void)
{
	if (g_sdlLauncherGfxCacheMarked)
	{
		BEL_ST_SleepMS(1);
		g_sdlLauncherGfxCacheMarked = false;
		uint32_t *currPixPtr = (uint32_t *)BEL_ST_LockTexture(g_sdlTexture);
		uint8_t *currPalPixPtr = g_sdlLauncherGfxCache;
		for (int pixnum = 0; pixnum < BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlEGABGRAScreenColors[*currPalPixPtr];
		}

		BEL_ST_UnlockTexture(g_sdlTexture);
		BEL_ST_RenderClear();
		if (g_sdlTargetTexture)
		{
			if (BEL_ST_SetRenderTarget(g_sdlTargetTexture) != 0)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_Launcher_UpdateHostDisplay: Failed to set target texture as render target (disabling)\n");
				BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
				goto refreshwithnorendertarget;
			}
			BEL_ST_RenderFromTexture(g_sdlTexture, NULL);
			if (BEL_ST_SetRenderTarget(NULL) != 0)
				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_Launcher_UpdateHostDisplay: Failed to set default render target!\n");
			BEL_ST_RenderFromTexture(g_sdlTargetTexture, &g_sdlAspectCorrectionBorderedRect);
		}
		else
		{
refreshwithnorendertarget:
			BEL_ST_RenderFromTexture(g_sdlTexture, &g_sdlAspectCorrectionBorderedRect);
		}
		BEL_ST_Launcher_FinishHostDisplayUpdate();
		g_be_sdlLastRefreshTicks = BEL_ST_GetTicksMS();
	}
	else
	{
		// Refresh graphics from time to time in case a part of the window is overridden by anything,
		// like the Steam Overlay. Sleep for less time so the application is somewhat responsive, though.
		BEL_ST_SleepMS(10);
		uint32_t currRefreshTicks = BEL_ST_GetTicksMS();
		if (currRefreshTicks - g_be_sdlLastRefreshTicks >= 100)
		{
			BEL_ST_RenderClear();

			if (g_sdlTargetTexture)
				BEL_ST_RenderFromTexture(g_sdlTargetTexture, &g_sdlAspectCorrectionBorderedRect);
			else
				BEL_ST_RenderFromTexture(g_sdlTexture, &g_sdlAspectCorrectionBorderedRect);

			BEL_ST_Launcher_FinishHostDisplayUpdate();
			g_be_sdlLastRefreshTicks = currRefreshTicks;
		}
	}
}


static void BEL_ST_Launcher_CheckCommonPointerPressCases(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y, uint32_t ticksBeforePoll)
{
	if (g_sdlLauncherTextSearchUIIsShown)
	{
		g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
		BEL_ST_Launcher_CheckPressedPointerInTextSearchUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_HandleInput_PointerSelect(x, y, ticksBeforePoll, touchId == BE_ST_MouseTouchID);
	}
}

static void BEL_ST_Launcher_CheckCommonPointerReleaseCases(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y, uint32_t ticksBeforePoll)
{
	if (g_sdlLauncherTextSearchUIIsShown)
	{
		BEL_ST_Launcher_CheckReleasedPointerInTextSearchUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_HandleInput_PointerRelease(x, y, ticksBeforePoll);
	}
}

static void BEL_ST_Launcher_CheckCommonPointerMoveCases(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y, uint32_t ticksBeforePoll)
{
	if (g_sdlLauncherTextSearchUIIsShown)
	{
		BEL_ST_Launcher_CheckMovedPointerInTextSearchUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_HandleInput_PointerMotion(x, y, ticksBeforePoll);
	}
}

void BE_ST_Launcher_RunEventLoop(void)
{
	SDL_Event event;
	g_be_sdlLastRefreshTicks = 0;

	while (1)
	{
		uint32_t ticksBeforePoll = BEL_ST_GetTicksMS();
		while (SDL_PollEvent(&event))
		{
			g_sdlLauncherLastEventType = event.type;

			void BEL_ST_NormalizeBorderedPos(int *px, int *py);

			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.repeat)
					break; // Ignore
				g_sdlKeyboardLastKeyPressed = event.key.keysym.scancode;
				g_sdlKeyboardLastKeyPressedIsShifted = (event.key.keysym.mod & KMOD_SHIFT);
				g_sdlInputLastBinaryPressTime = ticksBeforePoll;
				g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
				BEL_ST_Launcher_HandleKeyPressEvent(event.key.keysym.scancode, g_sdlKeyboardLastKeyPressedIsShifted);
				break;
			case SDL_KEYUP:
				if (g_sdlKeyboardLastKeyPressed == event.key.keysym.scancode)
					g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_CheckCommonPointerPressCases(BE_ST_MouseTouchID, 0, event.button.x, event.button.y, ticksBeforePoll);
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_CheckCommonPointerReleaseCases(BE_ST_MouseTouchID, 0, event.button.x, event.button.y, ticksBeforePoll);
				break;
			case SDL_MOUSEMOTION:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_CheckCommonPointerMoveCases(BE_ST_MouseTouchID, 0, event.motion.x, event.motion.y, ticksBeforePoll);
				break;
			case SDL_MOUSEWHEEL:
				BE_Launcher_HandleInput_PointerVScroll(-10*event.wheel.y, ticksBeforePoll);
				break;

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
			case SDL_FINGERDOWN:
				BEL_ST_Launcher_CheckCommonPointerPressCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, ticksBeforePoll);
				break;
			case SDL_FINGERUP:
				BEL_ST_Launcher_CheckCommonPointerReleaseCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, ticksBeforePoll);
				break;
			case SDL_FINGERMOTION:
				BEL_ST_Launcher_CheckCommonPointerMoveCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, ticksBeforePoll);
				break;
#endif

			/* Don't use SDL_CONTROLLERDEVICEADDED with alternative controller schemes, and for the sake of consistency avoid SDL_CONTROLLERDEVICEREMOVED as well.
			 * Reason is that on init, there is a problem handling controller mappings loaded from the database using SDL_CONTROLLERDEVICEADDED
			 * (if loaded before init, the mappings seem to be deleted, otherwise SDL_CONTROLLERDEVICEADDED is just not spawned for these).
			 */
			case SDL_JOYDEVICEADDED:
				if ((event.jdevice.which < BE_ST_MAXJOYSTICKS) && !g_sdlControllers[event.jdevice.which] && SDL_IsGameController(event.jdevice.which))
					g_sdlControllers[event.jdevice.which] = SDL_GameControllerOpen(event.jdevice.which);
				break;
			case SDL_JOYDEVICEREMOVED:
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
					if (g_sdlControllers[i] && (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i])) == event.jdevice.which))
					{
						SDL_GameControllerClose(g_sdlControllers[i]);
						g_sdlControllers[i] = NULL;
					}
				break;

			case SDL_CONTROLLERAXISMOTION: // Need this so a pressed trigger is ignored once user gets to choose a button for in-game action
				if ((event.caxis.axis == BE_ST_CTRL_AXIS_LTRIGGER) || (event.caxis.axis == BE_ST_CTRL_AXIS_RTRIGGER))
					g_sdlLauncherTriggerBinaryStates[event.caxis.axis - BE_ST_CTRL_AXIS_LTRIGGER] = (event.caxis.value >= g_sdlJoystickAxisBinaryThreshold);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				if ((event.cbutton.button < 0) || (event.cbutton.button >= BE_ST_CTRL_BUT_MAX))
					break;
				g_sdlControllerLastButtonPressed = event.cbutton.button;
				g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
				g_sdlInputLastBinaryPressTime = ticksBeforePoll;
				g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
				BEL_ST_Launcher_HandleControllerButtonEvent(event.cbutton.button, true);
				break;
			case SDL_CONTROLLERBUTTONUP:
				if ((event.cbutton.button < 0) || (event.cbutton.button >= BE_ST_CTRL_BUT_MAX))
					break;
				if (g_sdlControllerLastButtonPressed == event.cbutton.button)
					g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
				BEL_ST_Launcher_HandleControllerButtonEvent(event.cbutton.button, false);
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				case  SDL_WINDOWEVENT_RESIZED:
				{
					BEL_ST_Launcher_SetGfxOutputRects();
					BE_ST_Launcher_MarkGfxCache();
					break;
				}
				case SDL_WINDOWEVENT_EXPOSED:
					BE_ST_Launcher_MarkGfxCache();
					break;
				break;

			case SDL_RENDER_TARGETS_RESET:
			case SDL_RENDER_DEVICE_RESET:
				BEL_ST_RecreateAllTextures();
				break;

			case SDL_QUIT:
				BE_ST_Launcher_Shutdown();
				BE_ST_QuickExit();
				break;
			}
		}

		BEL_ST_Launcher_CheckForExitFromEventsCallback();

		// Emulate "key repeat" for keyboard/controller buttons
		if (((g_sdlKeyboardLastKeyPressed != SDL_SCANCODE_UNKNOWN) || (g_sdlControllerLastButtonPressed != BE_ST_CTRL_BUT_INVALID)) && (ticksBeforePoll - g_sdlInputLastBinaryPressTime >= g_sdlInputLastBinaryPressTimeDelay))
		{
			g_sdlInputLastBinaryPressTime += g_sdlInputLastBinaryPressTimeDelay;
			g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;

			if (g_sdlKeyboardLastKeyPressed != SDL_SCANCODE_UNKNOWN)
				BEL_ST_Launcher_HandleKeyPressEvent(g_sdlKeyboardLastKeyPressed, g_sdlKeyboardLastKeyPressedIsShifted);
			else
				BEL_ST_Launcher_HandleControllerButtonEvent(g_sdlControllerLastButtonPressed, true);
		}

		BE_Launcher_RefreshVerticalScrolling(ticksBeforePoll);

		BEL_ST_Launcher_UpdateHostDisplay();
	}
}

void BE_ST_Launcher_WaitForControllerButton(BEMenuItem *menuItem)
{
	BEL_ST_Launcher_TurnTextSearchOff();

	// HACK - Refresh window and make sure none of it is filled with random data while waiting for button press
	uint32_t *currPixPtr = (uint32_t *)BEL_ST_LockTexture(g_sdlTexture);
	uint8_t *currPalPixPtr = g_sdlLauncherGfxCache;
	for (int pixnum = 0; pixnum < BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
	{
		*currPixPtr = g_sdlEGABGRAScreenColors[*currPalPixPtr];
	}

	BEL_ST_UnlockTexture(g_sdlTexture);

	if (g_sdlTargetTexture)
	{
		BEL_ST_SetRenderTarget(g_sdlTargetTexture);
		BEL_ST_RenderFromTexture(g_sdlTexture, NULL);
		BEL_ST_SetRenderTarget(NULL);
	}

	SDL_Event event;
	bool keepRunning = true;
	const int defaultChoice = BE_ST_CTRL_BUT_MAX + 2/*triggers*/;
	int choice = defaultChoice;

	while (keepRunning)
	{
		while (SDL_PollEvent(&event))
		{
			g_sdlLauncherLastEventType = event.type;

			void BEL_ST_NormalizeBorderedPos(int *px, int *py);

			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.repeat)
					break; // Ignore
				// Fall-through
			case SDL_MOUSEBUTTONDOWN:
#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
			case SDL_FINGERDOWN:
				keepRunning = false;
				break;
#endif

			case SDL_JOYDEVICEADDED:
				if ((event.jdevice.which < BE_ST_MAXJOYSTICKS) && SDL_IsGameController(event.jdevice.which))
					g_sdlControllers[event.jdevice.which] = SDL_GameControllerOpen(event.jdevice.which);
				break;
			case SDL_JOYDEVICEREMOVED:
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
					if (g_sdlControllers[i] && (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i])) == event.jdevice.which))
					{
						SDL_GameControllerClose(g_sdlControllers[i]);
						g_sdlControllers[i] = NULL;
					}
				break;

			case SDL_CONTROLLERAXISMOTION:
				if ((event.caxis.axis == BE_ST_CTRL_AXIS_LTRIGGER) || (event.caxis.axis == BE_ST_CTRL_AXIS_RTRIGGER))
				{
					int triggerNum = event.caxis.axis - BE_ST_CTRL_AXIS_LTRIGGER;
					bool prevBinaryState = g_sdlLauncherTriggerBinaryStates[triggerNum];
					g_sdlLauncherTriggerBinaryStates[triggerNum] = (event.caxis.value >= g_sdlJoystickAxisBinaryThreshold);
					if (!prevBinaryState && g_sdlLauncherTriggerBinaryStates[triggerNum])
					{
						choice = BE_ST_CTRL_BUT_MAX + triggerNum; // HACK
						keepRunning = false;
					}
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				if ((event.cbutton.button >= 0) &&
				    (event.cbutton.button < BE_ST_CTRL_BUT_MAX) &&
				    (event.cbutton.button != BE_ST_CTRL_BUT_BACK) &&
				    (event.cbutton.button != BE_ST_CTRL_BUT_GUIDE) &&
				    (event.cbutton.button != BE_ST_CTRL_BUT_START) &&
				    (event.cbutton.button != BE_ST_CTRL_BUT_MISC1))
				{
					choice = event.cbutton.button;
				}
				keepRunning = false;
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				case  SDL_WINDOWEVENT_RESIZED:
				{
					BEL_ST_Launcher_SetGfxOutputRects();
					BE_ST_Launcher_MarkGfxCache();
					break;
				}
				case SDL_WINDOWEVENT_EXPOSED:
					BE_ST_Launcher_MarkGfxCache();
					break;
				break;

			case SDL_RENDER_TARGETS_RESET:
			case SDL_RENDER_DEVICE_RESET:
				BEL_ST_RecreateAllTextures();
				break;

			case SDL_QUIT:
				BE_ST_Launcher_Shutdown();
				BE_ST_QuickExit();
				break;
			}
		}

		BEL_ST_Launcher_CheckForExitFromEventsCallback();
		BEL_ST_SleepMS(10);
		BEL_ST_RenderClear();

		if (g_sdlTargetTexture)
			BEL_ST_RenderFromTexture(g_sdlTargetTexture, &g_sdlAspectCorrectionBorderedRect);
		else
			BEL_ST_RenderFromTexture(g_sdlTexture, &g_sdlAspectCorrectionBorderedRect);

		BEL_ST_UpdateWindow();
	}

	if (choice != defaultChoice)
	{
		for (BEMenuItem **menuItemP = g_beControllerSettingsMenu.menuItems; menuItemP != g_beControllerSettingsMenu.menuItems + BE_ST_CTRL_CFG_BUTMAP_AFTERLAST; ++menuItemP)
			if ((*menuItemP != menuItem) && ((*menuItemP)->choice == choice))
				(*menuItemP)->choice = defaultChoice; // Remove duplications
	}
	menuItem->choice = choice;

	// Reset
	g_sdlLauncherGfxCacheMarked = true;

	memset(g_sdlLauncherTriggerBinaryStates, 0, sizeof(g_sdlLauncherTriggerBinaryStates));
	g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
}

static bool BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(int scancode, bool isShifted, bool *pConfirmed)
{
	switch (scancode)
	{
	case SDL_SCANCODE_HOME:
		BE_Launcher_ArgumentsEditing_MoveCursorToEdge(false);
		break;
	case SDL_SCANCODE_END:
		BE_Launcher_ArgumentsEditing_MoveCursorToEdge(true);
		break;
	case SDL_SCANCODE_RIGHT:
		BE_Launcher_ArgumentsEditing_MoveCursorOnePos(true);
		break;
	case SDL_SCANCODE_LEFT:
		BE_Launcher_ArgumentsEditing_MoveCursorOnePos(false);
		break;
	case SDL_SCANCODE_RETURN:
		*pConfirmed = true;
		return true;
	case SDL_SCANCODE_ESCAPE:
	case SDL_SCANCODE_AC_BACK:
		*pConfirmed = false;
		return true;
	case SDL_SCANCODE_BACKSPACE:
		BE_Launcher_ArgumentsEditing_DeleteChar(false);
		break;
	case SDL_SCANCODE_DELETE:
		BE_Launcher_ArgumentsEditing_DeleteChar(true);
		break;
	default:
		if (((scancode >= SDL_SCANCODE_A) && (scancode <= SDL_SCANCODE_Z)) || // Letters
		    ((scancode >= SDL_SCANCODE_1) && (scancode <= SDL_SCANCODE_0)) || // Digits
		    ((scancode >= SDL_SCANCODE_SPACE) && (scancode <= SDL_SCANCODE_SLASH) && (scancode != SDL_SCANCODE_NONUSHASH)) // A few other keys
		)
			BE_Launcher_ArgumentsEditing_InsertChar((isShifted ? g_sdlScancodeToShiftedCharMap : g_sdlScancodeToNonShiftedCharMap)[scancode]);
		break;
	}
	return false;
}


static bool BEL_ST_Launcher_HandleTextInputEvent(Uint8 but, bool isPressed, bool *pConfirmed)
{
	switch (but)
	{
	case BE_ST_CTRL_BUT_DPAD_UP:
		if (isPressed)
			BEL_ST_Launcher_MoveUpInTextInputUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_DOWN:
		if (isPressed)
			BEL_ST_Launcher_MoveDownInTextInputUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_LEFT:
		if (isPressed)
			BEL_ST_Launcher_MoveLeftInTextInputUI();
		break;
	case BE_ST_CTRL_BUT_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_Launcher_MoveRightInTextInputUI();
		break;
	// A few other special cases
	case BE_ST_CTRL_BUT_B:
	case BE_ST_CTRL_BUT_BACK:
		if (isPressed)
			return BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(SDL_SCANCODE_ESCAPE, g_sdlKeyboardUIIsShifted, pConfirmed);
		break;
	case BE_ST_CTRL_BUT_X:
		if (isPressed)
			BEL_ST_Launcher_ToggleShiftStateInTextInputUI();
		break;
	default:
		return BEL_ST_Launcher_ToggleKeyPressInTextInputUI(isPressed, pConfirmed);
	}

	return false;
}

static bool BEL_ST_Launcher_ArgumentsEditing_HandleControllerButtonEvent(Uint8 but, bool isPressed, bool *pConfirmed)
{
	// Special case (applies with and without on-screen keyboard)
	if ((but == BE_ST_CTRL_BUT_Y) && isPressed)
	{
		BEL_ST_Launcher_ToggleTextInput();
		return false;
	}

	if (g_sdlLauncherTextInputUIIsShown)
	{
		return BEL_ST_Launcher_HandleTextInputEvent(but, isPressed, pConfirmed);
	}

	if (!isPressed)
		return false;

	switch (but)
	{
	case BE_ST_CTRL_BUT_A:
		BEL_ST_Launcher_ToggleTextInput();
		break;
	case BE_ST_CTRL_BUT_B:
	case BE_ST_CTRL_BUT_BACK:
		*pConfirmed = false;
		return true;
	}
	return false;
}

static void BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerPressCases(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	if (g_sdlLauncherTextInputUIIsShown)
	{
		g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
		BEL_ST_Launcher_CheckPressedPointerInTextInputUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_ArgumentsEditing_HandleInput_PointerSelect(x, y);
	}
}

static bool BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerReleaseCases(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y, bool *pConfirmed)
{
	if (g_sdlLauncherTextInputUIIsShown)
	{
		return BEL_ST_Launcher_CheckReleasedPointerInTextInputUI(touchId, fingerId, x, y, pConfirmed);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		*pConfirmed = !BE_Launcher_ArgumentsEditing_HandleInput_PointerRelease(x, y);
		return !(*pConfirmed); // Hack to halt if function returns true, and otherwise remain in the arguments editing loop
	}
}

static void BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerMoveCases(BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y)
{
	if (g_sdlLauncherTextInputUIIsShown)
	{
		BEL_ST_Launcher_CheckMovedPointerInTextInputUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_ArgumentsEditing_HandleInput_PointerMotion(x, y);
	}
}

bool BEL_ST_SDL_Launcher_DoEditArguments(void)
{
	SDL_Event event;
	bool confirmed;
	g_be_sdlLastRefreshTicks = 0;

	while (1)
	{
		uint32_t ticksBeforePoll = BEL_ST_GetTicksMS();
		while (SDL_PollEvent(&event))
		{
			g_sdlLauncherLastEventType = event.type;

			void BEL_ST_NormalizeBorderedPos(int *px, int *py);

			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.repeat)
					break; // Ignore
				// Otherwise some handler may be called
				g_sdlKeyboardLastKeyPressed = event.key.keysym.scancode;
				g_sdlKeyboardLastKeyPressedIsShifted = (event.key.keysym.mod & KMOD_SHIFT);
				g_sdlInputLastBinaryPressTime = ticksBeforePoll;
				g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
				if (BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(event.key.keysym.scancode, g_sdlKeyboardLastKeyPressedIsShifted, &confirmed))
					return confirmed;
				break;
			case SDL_KEYUP:
				if (g_sdlKeyboardLastKeyPressed == event.key.keysym.scancode)
					g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerPressCases(0, 0, event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				if (BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerReleaseCases(0, 0, event.button.x, event.button.y, &confirmed))
					return confirmed;
				break;
			case SDL_MOUSEMOTION:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerMoveCases(0, 0, event.motion.x, event.motion.y);
				break;

#ifdef REFKEEN_CONFIG_ENABLE_TOUCHINPUT
			case SDL_FINGERDOWN:
				BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerPressCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
				break;
			case SDL_FINGERUP:
				if (BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerReleaseCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, &confirmed))
					return confirmed;
				break;
			case SDL_FINGERMOTION:
				BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerMoveCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight);
				break;
#endif

			case SDL_JOYDEVICEADDED:
				if ((event.jdevice.which < BE_ST_MAXJOYSTICKS) && SDL_IsGameController(event.jdevice.which))
					g_sdlControllers[event.jdevice.which] = SDL_GameControllerOpen(event.jdevice.which);
				break;
			case SDL_JOYDEVICEREMOVED:
				for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
					if (g_sdlControllers[i] && (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i])) == event.jdevice.which))
					{
						SDL_GameControllerClose(g_sdlControllers[i]);
						g_sdlControllers[i] = NULL;
					}
				break;

			case SDL_CONTROLLERAXISMOTION: // Need this so a pressed trigger is ignored once user gets to choose a button for in-game action
				if ((event.caxis.axis == BE_ST_CTRL_AXIS_LTRIGGER) || (event.caxis.axis == BE_ST_CTRL_AXIS_RTRIGGER))
					g_sdlLauncherTriggerBinaryStates[event.caxis.axis - BE_ST_CTRL_AXIS_LTRIGGER] = (event.caxis.value >= g_sdlJoystickAxisBinaryThreshold);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				if ((event.cbutton.button < 0) || (event.cbutton.button >= BE_ST_CTRL_BUT_MAX))
					break;
				g_sdlControllerLastButtonPressed = event.cbutton.button;
				g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
				g_sdlInputLastBinaryPressTime = ticksBeforePoll;
				g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
				if (BEL_ST_Launcher_ArgumentsEditing_HandleControllerButtonEvent(event.cbutton.button, true, &confirmed))
					return confirmed;
				break;
			case SDL_CONTROLLERBUTTONUP:
				if ((event.cbutton.button < 0) || (event.cbutton.button >= BE_ST_CTRL_BUT_MAX))
					break;
				if (g_sdlControllerLastButtonPressed == event.cbutton.button)
					g_sdlControllerLastButtonPressed = BE_ST_CTRL_BUT_INVALID;
				if (BEL_ST_Launcher_ArgumentsEditing_HandleControllerButtonEvent(event.cbutton.button, false, &confirmed))
					return confirmed;
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				case  SDL_WINDOWEVENT_RESIZED:
				{
					BEL_ST_Launcher_SetGfxOutputRects();
					BE_ST_Launcher_MarkGfxCache();
					break;
				}
				case SDL_WINDOWEVENT_EXPOSED:
					BE_ST_Launcher_MarkGfxCache();
					break;
				break;

			case SDL_RENDER_TARGETS_RESET:
			case SDL_RENDER_DEVICE_RESET:
				BEL_ST_RecreateAllTextures();
				break;

			case SDL_QUIT:
				BE_ST_Launcher_Shutdown();
				BE_ST_QuickExit();
				break;
			}
		}

		BEL_ST_Launcher_CheckForExitFromEventsCallback();

		// Emulate "key repeat" for keyboard/controller buttons
		if (((g_sdlKeyboardLastKeyPressed != SDL_SCANCODE_UNKNOWN) || (g_sdlControllerLastButtonPressed != BE_ST_CTRL_BUT_INVALID)) && (ticksBeforePoll - g_sdlInputLastBinaryPressTime >= g_sdlInputLastBinaryPressTimeDelay))
		{
			g_sdlInputLastBinaryPressTime += g_sdlInputLastBinaryPressTimeDelay;
			g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DIGIACTION_REPEAT_RATE_MS;

			bool ret;
			if (g_sdlKeyboardLastKeyPressed != SDL_SCANCODE_UNKNOWN)
				ret = BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(g_sdlKeyboardLastKeyPressed, g_sdlKeyboardLastKeyPressedIsShifted, &confirmed);
			else
				ret = BEL_ST_Launcher_ArgumentsEditing_HandleControllerButtonEvent(g_sdlControllerLastButtonPressed, true, &confirmed);

			if (ret)
				return confirmed;
		}

		BEL_ST_Launcher_UpdateHostDisplay();
	}

	return false;
}

#endif // REFKEEN_ENABLE_LAUNCHER
