/* Copyright (C) 2015-2016 NY00123
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

#ifdef REFKEEN_ENABLE_LAUNCHER

#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "refkeen.h"
#include "be_st_sdl_private.h"
#include "be_title_and_version.h"

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

// Unused as of SDL v2.0.2
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

static SDL_Rect g_sdlControllerLauncherTextSearchRect, g_sdlControllerLauncherTextInputRect;
static SDL_Texture *g_sdlLauncherTextSearchTexture, *g_sdlLauncherTextInputTexture;
static bool g_sdlLauncherTextSearchUIIsShown, g_sdlLauncherTextInputUIIsShown;

// NOTE: More-or-less duplicated from be_st_sdl_graphics.c, but there may be
// some differences. For one, these are not limited just to a specific kind
// of keyboard UI (partially unused for in-game debug keys UI).
static int g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY;
static bool g_sdlKeyboardUISelectedKeyIsMarked;

static bool g_sdlKeyboardUIIsKeyPressed, g_sdlKeyboardUIIsShifted;

/* Tracked fingers definitions (multi-touch input) */

#define MAX_NUM_OF_TRACKED_FINGERS 10

typedef struct {
	SDL_TouchID touchId;
	SDL_FingerID fingerId;
	int lastX, lastY;
	struct {
		int x, y;
	} key;
} BESDLLauncherTrackedFinger;

static BESDLLauncherTrackedFinger g_sdlTrackedFingers[MAX_NUM_OF_TRACKED_FINGERS];
static int g_nOfTrackedFingers = 0;

// Borrowed from other files
extern int g_sdlDebugFingerRectSideLen;
extern const uint32_t g_sdlEGABGRAScreenColors[];
extern SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS];
extern SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
extern SDL_Rect g_sdlAspectCorrectionBorderedRect;
extern const int g_sdlJoystickAxisBinaryThreshold, g_sdlJoystickAxisDeadZone, g_sdlJoystickAxisMax, g_sdlJoystickAxisMaxMinusDeadZone;
extern int g_sdlLastReportedWindowWidth, g_sdlLastReportedWindowHeight;
// These two are shared ON PURPOSE (for seamless transition from launcher to game, if an SDL_WINDOW_FULLSCREEN_DESKTOP window is used with same features)
extern SDL_Window *g_sdlWindow;
extern SDL_Renderer *g_sdlRenderer;
// Similarly, this is also shared *on purpose* - for a HACK (guess if touch controls should be shown when game is started)
extern bool g_sdlShowTouchUI;

static int g_sdlKeyboardLastKeyPressed;
static bool g_sdlKeyboardLastKeyPressedIsShifted;
static int g_sdlControllerLastButtonPressed;

static uint32_t g_sdlInputLastBinaryPressTime;
static uint32_t g_sdlInputLastBinaryPressTimeDelay;

// Need these for triggers while letting the user selection buttons for gameplay
static bool g_sdlLauncherTriggerBinaryStates[2];

static uint8_t g_sdlLauncherGfxCache[BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT]; // Launcher gets pointer to this for drawing
static bool g_sdlLauncherGfxCacheMarked = false;

// Let's disable this for now since it may lead to problems on certain platforms, but keep the cfg field
//#define BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM

#define BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN 10

/*** Convenience macros - Note that the label *must* be a C string literal ***/
#define BEMENUITEM_DEF_TARGETMENU(menuItemName, label, menuPtr) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, NULL, menuPtr, label, BE_MENUITEM_TYPE_TARGETMENU)
#define BEMENUITEM_DEF_SELECTION(menuItemName, label, choices) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, choices, NULL, label, BE_MENUITEM_TYPE_SELECTION)
#define BEMENUITEM_DEF_HANDLER(menuItemName, label, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, NULL, NULL, label, BE_MENUITEM_TYPE_HANDLER)
#define BEMENUITEM_DEF_HANDLER_LABELVAR(menuItemName, labelSize, handlerPtr) BEMENUITEM_DEF_LABELVAR(menuItemName, handlerPtr, NULL, NULL, labelSize, BE_MENUITEM_TYPE_HANDLER)
#define BEMENUITEM_DEF_SELECTION_WITH_HANDLER(menuItemName, label, choices, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, NULL, label, BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER)
#define BEMENUITEM_DEF_DYNAMIC_SELECTION(menuItemName, label, choices, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, NULL, label, BE_MENUITEM_TYPE_DYNAMIC_SELECTION)
#define BEMENUITEM_DEF_STATIC(menuItemName, label) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, NULL, NULL, label, BE_MENUITEM_TYPE_STATIC)

// A little hack - Store a copy of the label string literal that can be modified
#define BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, menuPtr, label, type) \
	static char menuItemName ## _label[] = label; \
	static BEMenuItem menuItemName = {handlerPtr, choices, menuPtr, menuItemName ## _label, 0, 0, 0, 0, 0, 0, type};

// Same as above, but label is variable and we fill some room for it
#define BEMENUITEM_DEF_LABELVAR(menuItemName, handlerPtr, choices, menuPtr, labelSize, type) \
	static char menuItemName ## _label[labelSize] = {0}; \
	static BEMenuItem menuItemName = {handlerPtr, choices, menuPtr, menuItemName ## _label, 0, 0, 0, 0, 0, 0, type};

static const char *g_be_settingsChoices_boolean[] = {"No","Yes",NULL};

/*** Main menu ***/

BEMENUITEM_DEF_HANDLER_LABELVAR(g_beMainMenuItem_PlayLastChosenGameVer,	40/* HACK to have enough room for string*/, &BE_Launcher_Handler_LastGameVerLaunch)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_SelectGame, "Select game", &g_beSelectGameMenu)
BEMENUITEM_DEF_HANDLER(g_beMainMenuItem_SetArguments, "Set arguments for game *CURRENTLY SET*", &BE_Launcher_Handler_SetArgumentsForGame)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_Settings, "Settings", &g_beSettingsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_ShowVersion, "Show version", &g_beShowVersionMenu)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_Quit, "Quit", &g_beQuitConfirmMenu)

BEMenu g_beMainMenu = {
	REFKEEN_TITLE_STRING,
	&g_beQuitConfirmMenu,
	(BEMenuItem *[])
	{
		&g_beMainMenuItem_PlayLastChosenGameVer, // Either this item's label is filled later, or the menu items array is shifted
		&g_beMainMenuItem_SelectGame,
		&g_beMainMenuItem_SetArguments,
		&g_beMainMenuItem_Settings,
		&g_beMainMenuItem_ShowVersion,
		&g_beMainMenuItem_Quit,
		NULL
	},
	// Ignore the rest
};

/*** Select game menu ***/

/*static*/ BEMenuItem g_beSelectGameMenuItems[BE_GAMEVER_LAST];
/*static*/ char g_beSelectGameMenuItemsStrs[BE_GAMEVER_LAST][78]; // Should be MUTABLE strings for layout preparation
/*static*/ BEMenuItem *g_beSelectGameMenuItemsPtrs[BE_GAMEVER_LAST+4];

BEMENUITEM_DEF_TARGETMENU(g_beSelectGameMenuItem_DisappearedGameHelp, "Help! An installed game disappeared from the list!", &g_beDisappearedGameHelpMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSelectGameMenuItem_ShowSupportedGameVersions, "Show supported game versions", &g_beSupportedGameVersionsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSelectGameMenuItem_AddMissingGameVersion, "Add missing game version", &g_beSelectInitialPathMenu)

BEMenu g_beSelectGameMenu = {
	"Select game",
	&g_beMainMenu,
	g_beSelectGameMenuItemsPtrs, // Array of menu items
	// Ignore the rest
};

BEMENUITEM_DEF_STATIC(g_beDisappearedGameHelpMenuItem_Explanation,
"Reflection Keen can detect compatible DOS game versions from certain installations, including the Catacombs games from GOG.com. Once such a game installation is updated in any minor way, Reflection Keen may fail to locate it. These are the expected behaviors.\nAs an alternative, you can manually add a compatible game installation (if not yet listed)."
);

/*** Disappeared game menu ***/

BEMenu g_beDisappearedGameHelpMenu = {
	"Where may a game disappear",
	&g_beSelectGameMenu,
	(BEMenuItem *[])
	{
		&g_beDisappearedGameHelpMenuItem_Explanation,
		NULL
	},
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

BEMenu g_beSelectDirectoryErrorMenu = {
	"Failed to select directory",
	NULL, // SPECIAL (using back button handler)
	(BEMenuItem *[])
	{
		&g_beSelectDirectoryErrorMenuItem_DisappearedGameHelp,
		NULL
	},
	&BE_Launcher_Handler_DirectorySelectionGoPrev, // SPECIAL (back button handler)
	// Ignore the rest
};

/*** Compatible game found menu ***/

// At this point, all directory-selection related resources are freed
BEMENUITEM_DEF_TARGETMENU(g_beSelectDirectoryFoundGameMenuItem_ShowGamesList, "Show games list", &g_beSelectGameMenu)

BEMenu g_beSelectDirectoryFoundGameMenu = {
	"New compatible game found!",
	&g_beSelectGameMenu,
	(BEMenuItem *[])
	{
		&g_beSelectDirectoryFoundGameMenuItem_ShowGamesList,
		NULL
	},
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

BEMenu g_beSettingsMenu = {
	"Settings",
	&g_beMainMenu,
	(BEMenuItem *[])
	{
		&g_beSettingsMenuItem_VideoSettings,
		&g_beSettingsMenuItem_SoundSettings,
		&g_beSettingsMenuItem_InputSettings,
		NULL
	},
	// Ignore the rest
};

/** Video settings menu ***/

#define BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS 10

typedef struct {
	int width, height;
} BEMenuItemScreenResPair;

#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
static BEMenuItemScreenResPair g_be_videoSettingsChoices_fullResolutionsVals[BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS];

static char g_be_videoSettingsChoices_fullResolutionsStrs[BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS][BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN];
// Need to add an additional desktop/default entry, plus NULL terminator
static const char *g_be_videoSettingsChoices_fullResolutions[BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS+2];
#endif

static const char *g_be_videoSettingsChoices_displayNums[] = {"0","1","2","3","4","5","6","7",NULL};

#define BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS 10

static char g_be_videoSettingsChoices_sdlRendererDriversStrs[BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS][BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN];
// Need to add additional auto entry, plus NULL terminator
static const char *g_be_videoSettingsChoices_sdlRendererDrivers[BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS+2];

static const char *g_be_videoSettingsChoices_scaleType[] = {"4:3","Fill",NULL};
static const char *g_be_videoSettingsChoices_scaleFactor[] = {"1","2","3","4",NULL};
static const char *g_be_videoSettingsChoices_vSync[] = {"Auto","Off","On",NULL};
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
static const char *g_be_videoSettingsChoices_launcherWindowType[] = {"Default","Fullscreen","Software",NULL};
#endif

#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
static void BEL_ST_Launcher_Handler_DisplayNum(BEMenuItem **menuItemP);
#endif

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_Fullscreen, "Fullscreen", g_be_settingsChoices_boolean)
#endif
#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_FullscreenRes, "Fullscreen resolution", g_be_videoSettingsChoices_fullResolutions)
BEMENUITEM_DEF_SELECTION_WITH_HANDLER(g_beVideoSettingsMenuItem_DisplayNum, "Display number", g_be_videoSettingsChoices_displayNums, &BEL_ST_Launcher_Handler_DisplayNum)
#else
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_DisplayNum, "Display number", g_be_videoSettingsChoices_displayNums)
#endif
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_SDLRenderer, "SDL renderer", g_be_videoSettingsChoices_sdlRendererDrivers)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_Bilinear, "Bilinear interpolation", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ScaleType, "Scale type*", g_be_videoSettingsChoices_scaleType)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ScaleFactor, "Scale factor", g_be_videoSettingsChoices_scaleFactor)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_VSync, "VSync", g_be_videoSettingsChoices_vSync)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ForceFullSoftScaling, "Force full software scaling*", g_be_settingsChoices_boolean)
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_LauncherWindowType, "Launcher window type", g_be_videoSettingsChoices_launcherWindowType)
#endif
BEMENUITEM_DEF_STATIC(g_beVideoSettingsMenuItem_SoftScalingComment,
"* Full software scaling should be manually toggled. Note that it can lead to great slowdowns with high-resolution windows."
);

BEMenu g_beVideoSettingsMenu = {
	"Video settings",
	&g_beSettingsMenu,
	(BEMenuItem *[])
	{
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
		&g_beVideoSettingsMenuItem_Fullscreen,
#endif
#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
		&g_beVideoSettingsMenuItem_FullscreenRes,
#endif
		&g_beVideoSettingsMenuItem_DisplayNum,
		&g_beVideoSettingsMenuItem_SDLRenderer,
		&g_beVideoSettingsMenuItem_Bilinear,
		&g_beVideoSettingsMenuItem_ScaleType,
		&g_beVideoSettingsMenuItem_ScaleFactor,
		&g_beVideoSettingsMenuItem_VSync,
		&g_beVideoSettingsMenuItem_ForceFullSoftScaling,
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
		&g_beVideoSettingsMenuItem_LauncherWindowType,
#endif
		&g_beVideoSettingsMenuItem_SoftScalingComment,
		NULL
	},
	// Ignore the rest
};

/*** Sounds settings menu ***/

static const int g_be_soundsSettingsChoices_sndSampleRateVals[] = {8000, 11025, 12000, 16000, 22050, 32000, 44100, 48000, 49716, 96000, 192000};
static const char *g_be_soundsSettingsChoices_sndSampleRate[] = {"8000","11025","12000","16000","22050","32000","44100","48000","49716","96000","192000",NULL};

BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_SndSampleRate, "Sound sample rate\n(in Hz)", g_be_soundsSettingsChoices_sndSampleRate)
BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_SndSubSystem, "Enable sound subsystem", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_OPLEmulation, "Enable OPL emulation", g_be_settingsChoices_boolean)

#ifndef REFKEEN_RESAMPLER_NONE
BEMENUITEM_DEF_SELECTION(g_beSoundSettingsMenuItem_UseResampler, "Use resampler", g_be_settingsChoices_boolean)
#endif

BEMenu g_beSoundSettingsMenu = {
	"Sounds settings",
	&g_beSettingsMenu,
	(BEMenuItem *[])
	{
		&g_beSoundSettingsMenuItem_SndSampleRate,
		&g_beSoundSettingsMenuItem_SndSubSystem,
		&g_beSoundSettingsMenuItem_OPLEmulation,
#ifndef REFKEEN_RESAMPLER_NONE
		&g_beSoundSettingsMenuItem_UseResampler,
#endif
		NULL
	},
	// Ignore the rest
};

/*** Input settings menu ***/

static const char *g_be_inputSettingsChoices_controllerScheme[] = {"Classic", "Modern", NULL};
static const char *g_be_inputSettingsChoices_touchControls[] = {"Auto", "Off", "Forced", NULL};
static const char *g_be_inputSettingsChoices_mouseGrab[] = {"Auto", "Off", "Commonly", NULL};

static void BEL_ST_Launcher_Handler_TouchInputDebugging(BEMenuItem **menuItemP);

BEMENUITEM_DEF_TARGETMENU(g_beInputSettingsMenuItem_ControllerSettings, "Modern controller settings", &g_beControllerSettingsMenu)
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_ControllerScheme, "Game controller scheme", g_be_inputSettingsChoices_controllerScheme)
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_TouchControls, "Enable touch controls", g_be_inputSettingsChoices_touchControls);
BEMENUITEM_DEF_SELECTION_WITH_HANDLER(g_beInputSettingsMenuItem_TouchInputDebugging, "Touch input debugging", g_be_settingsChoices_boolean, &BEL_ST_Launcher_Handler_TouchInputDebugging);
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_MouseGrab, "Mouse grab*\n(windowed mode specific)", g_be_inputSettingsChoices_mouseGrab)
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
BEMENUITEM_DEF_SELECTION(g_beInputSettingsMenuItem_AbsMouseMotion, "Absolute mouse motion**", g_be_settingsChoices_boolean)
#endif
BEMENUITEM_DEF_STATIC(g_beInputSettingsMenuItem_MouseGrabComment,
"* It's possible for mouse to be ungrabbed even if \"Commonly\" is chosen."
);
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
BEMENUITEM_DEF_STATIC(g_beInputSettingsMenuItem_AbsMouseMotionComment,
"** If toggled off, mouse cursor motion behaviors are similar to original DOS versions. Otherwise it may move at a different rate, but be more convenient to use in windowed mode."
);
#endif

BEMenu g_beInputSettingsMenu = {
	"Input settings",
	&g_beSettingsMenu,
	(BEMenuItem *[])
	{
		&g_beInputSettingsMenuItem_ControllerSettings,
		&g_beInputSettingsMenuItem_ControllerScheme,
		&g_beInputSettingsMenuItem_TouchControls,
		&g_beInputSettingsMenuItem_TouchInputDebugging,
		&g_beInputSettingsMenuItem_MouseGrab,
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
		&g_beInputSettingsMenuItem_AbsMouseMotion,
#endif
		&g_beInputSettingsMenuItem_MouseGrabComment,
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
		&g_beInputSettingsMenuItem_AbsMouseMotionComment,
#endif
		NULL
	},
	// Ignore the rest
};

/*** Controller settings menu ***/

static const char *g_be_controllerSettingsChoices_actionButton[] = {"A", "B", "X", "Y", NULL, NULL, NULL, "LStick" ,"RStick", "LShoulder", "RShoulder", NULL, NULL, NULL, NULL, "LTrigger", "RTrigger", "N/A"};
#ifdef REFKEEN_VER_CATACOMB_ALL
static const char *g_be_controllerSettingsChoices_analogMotion[] = {"Keyboard", "Mouse", NULL};
#endif


#ifdef REFKEEN_VER_KDREAMS
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Jump, "Action - Jump", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Throw, "Action - Throw", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Stats, "Action - Stats", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#else
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Fire, "Action - Fire", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Strafe, "Action - Strafe", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Drink, "Action - Drink", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Bolt, "Action - Bolt/Zapper", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Nuke, "Action - Nuke/Xterminator", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_FastTurn, "Action - Fast turn", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_Scrolls, "Action - Scrolls", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_FuncKeys, "Action - Function keys", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)
#endif
BEMENUITEM_DEF_DYNAMIC_SELECTION(g_beControllerSettingsMenuItem_Action_DebugKeys, "Action - Debug keys", g_be_controllerSettingsChoices_actionButton, &BE_Launcher_Handler_ControllerAction)

BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_Dpad, "Use d-pad for motion", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_LeftStick, "Use left stick", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_RightStick, "Use right stick", g_be_settingsChoices_boolean)
#ifdef REFKEEN_VER_CATACOMB_ALL
BEMENUITEM_DEF_SELECTION(g_beControllerSettingsMenuItem_AnalogMotion, "Motion emulation mode", g_be_controllerSettingsChoices_analogMotion)
#endif

BEMenu g_beControllerSettingsMenu = {
	"Modern controller settings",
	&g_beInputSettingsMenu,
	(BEMenuItem *[])
	{
#ifdef REFKEEN_VER_KDREAMS
		&g_beControllerSettingsMenuItem_Action_Jump,
		&g_beControllerSettingsMenuItem_Action_Throw,
		&g_beControllerSettingsMenuItem_Action_Stats,
#else
		&g_beControllerSettingsMenuItem_Action_Fire,
		&g_beControllerSettingsMenuItem_Action_Strafe,
		&g_beControllerSettingsMenuItem_Action_Drink,
		&g_beControllerSettingsMenuItem_Action_Bolt,
		&g_beControllerSettingsMenuItem_Action_Nuke,
		&g_beControllerSettingsMenuItem_Action_FastTurn,
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
		&g_beControllerSettingsMenuItem_Action_Scrolls,
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
		&g_beControllerSettingsMenuItem_Action_FuncKeys,
#endif
		&g_beControllerSettingsMenuItem_Action_DebugKeys,
		&g_beControllerSettingsMenuItem_Dpad,
		&g_beControllerSettingsMenuItem_LeftStick,
		&g_beControllerSettingsMenuItem_RightStick,
#ifdef REFKEEN_VER_CATACOMB_ALL
		&g_beControllerSettingsMenuItem_AnalogMotion,
#endif
		NULL
	},
	// Ignore the rest
};

/*** Show version menu ***/

BEMENUITEM_DEF_STATIC(g_beShowVersionMenuItem_Description, REFKEEN_TITLE_AND_VER_STRING ",\na part of Reflection Keen.")

BEMenu g_beShowVersionMenu = {
	"Version information",
	&g_beMainMenu,
	(BEMenuItem *[])
	{
		&g_beShowVersionMenuItem_Description,
		NULL
	},
	// Ignore the rest
};

/*** Quit confirm menu ***/

BEMENUITEM_DEF_HANDLER(g_beQuitConfirmMenuItem_Yes, "Yes", &BE_Launcher_Handler_MenuQuit)
BEMENUITEM_DEF_TARGETMENU(g_beQuitConfirmMenuItem_No, "No", &g_beMainMenu)

BEMenu g_beQuitConfirmMenu = {
	"Are you sure you want to quit?",
	&g_beMainMenu,
	(BEMenuItem *[])
	{
		&g_beQuitConfirmMenuItem_Yes,
		&g_beQuitConfirmMenuItem_No,
		NULL
	},
	// Ignore the rest
};

#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
static void BEL_ST_Launcher_ResetDisplayModes(int displayNum);
#endif
static void BEL_ST_Launcher_SetGfxOutputRects(void);

void BEL_ST_Launcher_RefreshSetArgumentsMenuItemLabel(void);

void BEL_ST_RecreateSDLWindowAndRenderer(int x, int y, int w, int h, Uint32 windowFlags, int driverIndex, Uint32 rendererFlags);

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

	BEL_ST_RecreateSDLWindowAndRenderer(
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_refKeenCfg.winWidth, g_refKeenCfg.winHeight,
		(g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_FULL) ? SDL_WINDOW_FULLSCREEN_DESKTOP : ((g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_DEFAULT) ? SDL_WINDOW_RESIZABLE : 0),
		-1, (g_refKeenCfg.launcherWinType == LAUNCHER_WINDOW_SOFTWARE) ? SDL_RENDERER_SOFTWARE : (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
	);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, BE_LAUNCHER_PIX_WIDTH, BE_LAUNCHER_PIX_HEIGHT);
	if (!g_sdlTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture for launcher,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}

	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE); // For clears in refreshes
	BEL_ST_Launcher_SetGfxOutputRects();

	if (g_refKeenCfg.launcherWinType != LAUNCHER_WINDOW_SOFTWARE)
	{
		// Try, if we fail then simply don't use this
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		g_sdlTargetTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 2*BE_LAUNCHER_PIX_WIDTH, 2*BE_LAUNCHER_PIX_HEIGHT);
	}
	/* Game controllers */
	int nOfJoysticks = SDL_NumJoysticks();
	if (nOfJoysticks > BE_ST_MAXJOYSTICKS)
		nOfJoysticks = BE_ST_MAXJOYSTICKS;
	for (i = 0; i < nOfJoysticks; ++i)
		if (SDL_IsGameController(i))
			g_sdlControllers[i] = SDL_GameControllerOpen(i);

	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
	g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;

	// Check if there's a last chosen game version to show in main menu
	for (i = 0; i < g_be_gameinstallations_num; ++i)
		if (g_refKeenCfg.lastSelectedGameVer == BE_Cross_GetGameVerFromInstallation(i))
		{
			snprintf(g_beMainMenuItem_PlayLastChosenGameVer_label, sizeof(g_beMainMenuItem_PlayLastChosenGameVer_label), "Play %s", BE_Cross_GetGameInstallationDescription(i));
			break;
		}
	if (i == g_be_gameinstallations_num)
		g_beMainMenu.menuItems++; // Shift the menu items (effectively removing the item above)

	BEL_ST_Launcher_RefreshSetArgumentsMenuItemLabel(); // Set menu item label based on arguments string

	// Set fullscreen value
#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	g_beVideoSettingsMenuItem_Fullscreen.choice = g_refKeenCfg.isFullscreen;
#endif
	/*** Prepare displays list ***/
	int nOfDisplays = SDL_GetNumVideoDisplays();
	if (nOfDisplays >= (int)(sizeof(g_be_videoSettingsChoices_displayNums)/sizeof(*g_be_videoSettingsChoices_displayNums)))
		nOfDisplays = sizeof(g_be_videoSettingsChoices_displayNums)/sizeof(*g_be_videoSettingsChoices_displayNums) - 1;
	g_be_videoSettingsChoices_displayNums[nOfDisplays] = NULL;
	g_beVideoSettingsMenuItem_DisplayNum.choice = g_refKeenCfg.displayNum;
	if (g_beVideoSettingsMenuItem_DisplayNum.choice >= nOfDisplays)
		g_beVideoSettingsMenuItem_DisplayNum.choice = 0;
#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
	/*** Prepare fullscreen resolutions list ***/
	BEL_ST_Launcher_ResetDisplayModes(g_refKeenCfg.displayNum);
#endif
	/*** Prepare SDL renderer drivers list ***/
	int nOfSDLRendererDrivers = SDL_GetNumRenderDrivers();
	if (nOfSDLRendererDrivers > BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS)
		nOfSDLRendererDrivers = BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS;
	g_beVideoSettingsMenuItem_SDLRenderer.choices[0] = "auto";
	g_beVideoSettingsMenuItem_SDLRenderer.choices[nOfSDLRendererDrivers+1] = NULL;
	for (i = 0; i < nOfSDLRendererDrivers; ++i)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		snprintf(g_be_videoSettingsChoices_sdlRendererDriversStrs[i], sizeof(g_be_videoSettingsChoices_sdlRendererDriversStrs[i]), "%s", info.name);
		g_beVideoSettingsMenuItem_SDLRenderer.choices[i+1] = g_be_videoSettingsChoices_sdlRendererDriversStrs[i];
	}
	if ((g_refKeenCfg.sdlRendererDriver < 0) || (g_refKeenCfg.sdlRendererDriver >= nOfSDLRendererDrivers))
		g_beVideoSettingsMenuItem_SDLRenderer.choice = 0;
	else
		g_beVideoSettingsMenuItem_SDLRenderer.choice = g_refKeenCfg.sdlRendererDriver + 1;
	// Set Bilinear value
	g_beVideoSettingsMenuItem_Bilinear.choice = g_refKeenCfg.isBilinear;
	// Set ScaleType value
	g_beVideoSettingsMenuItem_ScaleType.choice = g_refKeenCfg.scaleType;
	// Set ScaleFactor value
	if ((g_refKeenCfg.scaleFactor <= 0) || (g_refKeenCfg.scaleFactor >= (int)(sizeof(g_be_videoSettingsChoices_scaleFactor)/sizeof(*g_be_videoSettingsChoices_scaleFactor))))
		g_beVideoSettingsMenuItem_ScaleFactor.choice = 0;
	else
		g_beVideoSettingsMenuItem_ScaleFactor.choice = g_refKeenCfg.scaleFactor-1;
	// Set VSync value
	g_beVideoSettingsMenuItem_VSync.choice = g_refKeenCfg.vSync;
	// Set ForceFullSoftScaling value
	g_beVideoSettingsMenuItem_ForceFullSoftScaling.choice = g_refKeenCfg.forceFullSoftScaling;
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
	// Set LauncherWindowType value
	g_beVideoSettingsMenuItem_LauncherWindowType.choice = g_refKeenCfg.launcherWinType;
#endif
	// Set SndSampleRate value
	g_beSoundSettingsMenuItem_SndSampleRate.choice = 8; // FIXME - Better way to grab default off cfg
	for (i = 0; i < (int)(sizeof(g_be_soundsSettingsChoices_sndSampleRateVals)/sizeof(*g_be_soundsSettingsChoices_sndSampleRateVals)); ++i)
		if (g_be_soundsSettingsChoices_sndSampleRateVals[i] == g_refKeenCfg.sndSampleRate)
		{
			g_beSoundSettingsMenuItem_SndSampleRate.choice = i;
			break;
		}
	// Set SndSubSystem value
	g_beSoundSettingsMenuItem_SndSubSystem.choice = g_refKeenCfg.sndSubSystem;
	// Set OPLEmulation value
	g_beSoundSettingsMenuItem_OPLEmulation.choice = g_refKeenCfg.oplEmulation;
#ifndef REFKEEN_RESAMPLER_NONE
	// Set UseResampler value
	g_beSoundSettingsMenuItem_UseResampler.choice = g_refKeenCfg.useResampler;
#endif
	// Set ControllerScheme value
	g_beInputSettingsMenuItem_ControllerScheme.choice = g_refKeenCfg.altControlScheme.isEnabled;
	// Set TouchControls value
	g_beInputSettingsMenuItem_TouchControls.choice = g_refKeenCfg.touchInputToggle;
	// Set TouchInputDebugging value
	g_beInputSettingsMenuItem_TouchInputDebugging.choice = g_refKeenCfg.touchInputDebugging;
	// Set MouseGrab value
	g_beInputSettingsMenuItem_MouseGrab.choice = g_refKeenCfg.mouseGrab;
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	// Set AbsMouseMotion value
	g_beInputSettingsMenuItem_AbsMouseMotion.choice = g_refKeenCfg.absMouseMotion;
#endif
	// Set Dpad value
	g_beControllerSettingsMenuItem_Dpad.choice = g_refKeenCfg.altControlScheme.useDpad;
	// Set LeftStick value
	g_beControllerSettingsMenuItem_LeftStick.choice = g_refKeenCfg.altControlScheme.useLeftStick;
	// Set RightStick value
	g_beControllerSettingsMenuItem_RightStick.choice = g_refKeenCfg.altControlScheme.useRightStick;
#ifdef REFKEEN_VER_CATACOMB_ALL
	// Set AnalogMotion value
	g_beControllerSettingsMenuItem_AnalogMotion.choice = g_refKeenCfg.altControlScheme.analogMotion;
#endif
	/*** Set controller button bindings ***/
#ifdef REFKEEN_VER_KDREAMS
	g_beControllerSettingsMenuItem_Action_Jump.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_JUMP];
	g_beControllerSettingsMenuItem_Action_Throw.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_THROW];
	g_beControllerSettingsMenuItem_Action_Stats.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STATS];
#else
	g_beControllerSettingsMenuItem_Action_Fire.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE];
	g_beControllerSettingsMenuItem_Action_Strafe.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE];
	g_beControllerSettingsMenuItem_Action_Drink.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DRINK];
	g_beControllerSettingsMenuItem_Action_Bolt.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_BOLT];
	g_beControllerSettingsMenuItem_Action_Nuke.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_NUKE];
	g_beControllerSettingsMenuItem_Action_FastTurn.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FASTTURN];
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	g_beControllerSettingsMenuItem_Action_Scrolls.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_SCROLLS];
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	g_beControllerSettingsMenuItem_Action_FuncKeys.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS];
#endif
	g_beControllerSettingsMenuItem_Action_DebugKeys.choice = g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS];

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

	/* Poll events just in case (e.g., clean up old controller events) */
	// Update (Sep 19 2016) - DON'T, we might miss newly attached game controllers and other events!
#if 0
	SDL_Event event;
	bool doExit = false;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			doExit = true;
	}
#endif

	SDL_DestroyTexture(g_sdlLauncherTextSearchTexture);
	g_sdlLauncherTextSearchTexture = NULL;
	SDL_DestroyTexture(g_sdlLauncherTextInputTexture);
	g_sdlLauncherTextInputTexture = NULL;
	SDL_DestroyTexture(g_sdlTexture);
	g_sdlTexture = NULL;
	SDL_DestroyTexture(g_sdlTargetTexture);
	g_sdlTargetTexture = NULL;
	// Nope - We may re-use window and renderer for game (seamless transition)
#if 0
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
#endif

	// BEFORE checking if we need to save anything, apply this HACK
	if (g_beInputSettingsMenuItem_TouchControls.choice == TOUCHINPUT_AUTO)
		g_sdlShowTouchUI = ((g_sdlLauncherLastEventType == SDL_FINGERDOWN) || (g_sdlLauncherLastEventType == SDL_FINGERUP));
	else
		g_sdlShowTouchUI = (g_beInputSettingsMenuItem_TouchControls.choice == TOUCHINPUT_FORCED);

	/*** Save settings if there's any change ***/
	if (!g_be_launcher_wasAnySettingChanged)
		return; // e.g., if there is a cfg value the launcher doesn't cope with (say, out of some range)

#ifdef REFKEEN_CONFIG_USER_FULLSCREEN_TOGGLE
	g_refKeenCfg.isFullscreen = g_beVideoSettingsMenuItem_Fullscreen.choice;
#endif
	g_refKeenCfg.displayNum = g_beVideoSettingsMenuItem_DisplayNum.choice;

#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
	if (g_beVideoSettingsMenuItem_FullscreenRes.choice > 0)
	{
		g_refKeenCfg.fullWidth = g_be_videoSettingsChoices_fullResolutionsVals[g_beVideoSettingsMenuItem_FullscreenRes.choice - 1].width;
		g_refKeenCfg.fullHeight = g_be_videoSettingsChoices_fullResolutionsVals[g_beVideoSettingsMenuItem_FullscreenRes.choice - 1].height;
	}
	else
	{
		g_refKeenCfg.fullWidth = 0;
		g_refKeenCfg.fullHeight = 0;
	}
#endif

	g_refKeenCfg.sdlRendererDriver = g_beVideoSettingsMenuItem_SDLRenderer.choice - 1;
	g_refKeenCfg.isBilinear = g_beVideoSettingsMenuItem_Bilinear.choice;
	g_refKeenCfg.scaleType = (ScaleTypeSettingType)g_beVideoSettingsMenuItem_ScaleType.choice;
	g_refKeenCfg.scaleFactor = g_beVideoSettingsMenuItem_ScaleFactor.choice + 1;
	g_refKeenCfg.vSync = (VSyncSettingType)g_beVideoSettingsMenuItem_VSync.choice;
	g_refKeenCfg.forceFullSoftScaling = g_beVideoSettingsMenuItem_ForceFullSoftScaling.choice;
#ifdef REFKEEN_CONFIG_LAUNCHER_WINDOWTYPE_MENUITEM
	g_refKeenCfg.launcherWinType = (LauncherWindowSettingType)g_beVideoSettingsMenuItem_LauncherWindowType.choice;
#endif

	g_refKeenCfg.sndSampleRate = g_be_soundsSettingsChoices_sndSampleRateVals[g_beSoundSettingsMenuItem_SndSampleRate.choice];
	g_refKeenCfg.sndSubSystem = g_beSoundSettingsMenuItem_SndSubSystem.choice;
	g_refKeenCfg.oplEmulation = g_beSoundSettingsMenuItem_OPLEmulation.choice;
#ifndef REFKEEN_RESAMPLER_NONE
	g_refKeenCfg.useResampler = g_beSoundSettingsMenuItem_UseResampler.choice;
#endif

	g_refKeenCfg.altControlScheme.isEnabled = g_beInputSettingsMenuItem_ControllerScheme.choice;
	g_refKeenCfg.mouseGrab = (MouseGrabSettingType)g_beInputSettingsMenuItem_MouseGrab.choice;
#ifdef BE_ST_SDL_ENABLE_ABSMOUSEMOTION_SETTING
	g_refKeenCfg.absMouseMotion = g_beInputSettingsMenuItem_AbsMouseMotion.choice;
#endif
	g_refKeenCfg.touchInputToggle = (TouchInputSettingType)g_beInputSettingsMenuItem_TouchControls.choice;
	g_refKeenCfg.touchInputDebugging = g_beInputSettingsMenuItem_TouchInputDebugging.choice;

	g_refKeenCfg.altControlScheme.useDpad = g_beControllerSettingsMenuItem_Dpad.choice;
	g_refKeenCfg.altControlScheme.useLeftStick = g_beControllerSettingsMenuItem_LeftStick.choice;
	g_refKeenCfg.altControlScheme.useRightStick = g_beControllerSettingsMenuItem_RightStick.choice;
#ifdef REFKEEN_VER_CATACOMB_ALL
	g_refKeenCfg.altControlScheme.analogMotion = g_beControllerSettingsMenuItem_AnalogMotion.choice;
#endif

#ifdef REFKEEN_VER_KDREAMS
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_JUMP] = g_beControllerSettingsMenuItem_Action_Jump.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_THROW] = g_beControllerSettingsMenuItem_Action_Throw.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STATS] = g_beControllerSettingsMenuItem_Action_Stats.choice;
#else
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FIRE] = g_beControllerSettingsMenuItem_Action_Fire.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_STRAFE] = g_beControllerSettingsMenuItem_Action_Strafe.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DRINK] = g_beControllerSettingsMenuItem_Action_Drink.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_BOLT] = g_beControllerSettingsMenuItem_Action_Bolt.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_NUKE] = g_beControllerSettingsMenuItem_Action_Nuke.choice;
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FASTTURN] = g_beControllerSettingsMenuItem_Action_FastTurn.choice;
#endif
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATABYSS)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_SCROLLS] = g_beControllerSettingsMenuItem_Action_Scrolls.choice;
#endif
#if (defined REFKEEN_VER_KDREAMS) || (defined REFKEEN_VER_CATADVENTURES)
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_FUNCKEYS] = g_beControllerSettingsMenuItem_Action_FuncKeys.choice;
#endif
	g_refKeenCfg.altControlScheme.actionMappings[BE_ST_CTRL_CFG_BUTMAP_DEBUGKEYS] = g_beControllerSettingsMenuItem_Action_DebugKeys.choice;

	// Update (Sep 19 2016) - Not doing this
#if 0
	if (doExit)
		BE_ST_QuickExit();
#endif
}


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


#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
static void BEL_ST_Launcher_ResetDisplayModes(int displayNum)
{
	int nOfDisplayModes = SDL_GetNumDisplayModes(displayNum);
	if (nOfDisplayModes > BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS)
		nOfDisplayModes = BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS;
	g_beVideoSettingsMenuItem_FullscreenRes.choices[0] = "Desktop";
	g_beVideoSettingsMenuItem_FullscreenRes.choice = 0;
	int actualCounter = 0;
	for (int i = 0; i < nOfDisplayModes; ++i)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(displayNum, i, &mode);
		// Make sure there's no duplication (this is a possibility, at least when limiting ourselves just to width,height pairs)
		int j;
		for (j = 0; j < actualCounter; ++j)
			if ((g_be_videoSettingsChoices_fullResolutionsVals[j].width == mode.w) && (g_be_videoSettingsChoices_fullResolutionsVals[j].height == mode.h))
				break;
		if (j < actualCounter)
			continue; // Skip duplicated entry

		g_be_videoSettingsChoices_fullResolutionsVals[actualCounter].width = mode.w;
		g_be_videoSettingsChoices_fullResolutionsVals[actualCounter].height = mode.h;
		snprintf(g_be_videoSettingsChoices_fullResolutionsStrs[actualCounter], sizeof(g_be_videoSettingsChoices_fullResolutionsStrs[actualCounter]), "%dx%d", mode.w, mode.h);
		g_beVideoSettingsMenuItem_FullscreenRes.choices[actualCounter+1] =  g_be_videoSettingsChoices_fullResolutionsStrs[actualCounter];
		if ((g_refKeenCfg.fullWidth == mode.w) && (g_refKeenCfg.fullHeight == mode.h))
			g_beVideoSettingsMenuItem_FullscreenRes.choice = actualCounter+1;

		++actualCounter;
	}
	g_beVideoSettingsMenuItem_FullscreenRes.choices[actualCounter+1] = NULL;
}
#endif

void BEL_ST_Launcher_RefreshSetArgumentsMenuItemLabel(void)
{
	// HACK
	if (*g_refKeenCfg.launcherExeArgs != '\0')
		strcpy(g_beMainMenuItem_SetArguments.label + 23, "*CURRENTLY SET*");
	else
		strcpy(g_beMainMenuItem_SetArguments.label + 23, "               ");
}

void BEL_Launcher_DrawMenuItem(BEMenuItem *menuItem);

#ifdef BE_LAUNCHER_ENABLE_FULLSCREEN_RES_MENUITEM
static void BEL_ST_Launcher_Handler_DisplayNum(BEMenuItem **menuItemP)
{
	BEL_ST_Launcher_ResetDisplayModes((*menuItemP)->choice);
	BEL_Launcher_DrawMenuItem(&g_beVideoSettingsMenuItem_FullscreenRes);
}
#endif

static void BEL_ST_Launcher_Handler_TouchInputDebugging(BEMenuItem **menuItemP)
{
	// Apply this immediately, so the effect is observed in the launcher itself
	g_refKeenCfg.touchInputDebugging = (*menuItemP)->choice;
}

static void BEL_ST_Launcher_SetGfxOutputRects(void)
{
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);

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
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	g_sdlLauncherTextSearchTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH, ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_HEIGHT);
	if (!g_sdlLauncherTextSearchTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 launcher text search texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(g_sdlLauncherTextSearchTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_Launcher_CreateTextInputTextureIfNeeded(void)
{
	if (g_sdlLauncherTextInputTexture)
	{
		return;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	g_sdlLauncherTextInputTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH, ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_HEIGHT);
	if (!g_sdlLauncherTextInputTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 launcher text input texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(g_sdlLauncherTextInputTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
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

	SDL_UpdateTexture(g_sdlLauncherTextSearchTexture, NULL, pixels, 4*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_PIX_WIDTH);
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

	SDL_UpdateTexture(g_sdlLauncherTextInputTexture, NULL, pixels, 4*ALTCONTROLLER_LAUNCHER_TEXTINPUT_PIX_WIDTH);
}


void BEL_ST_Launcher_ToggleTextSearch(void)
{
	g_sdlLauncherTextSearchUIIsShown = !g_sdlLauncherTextSearchUIIsShown;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
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

	g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
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

	g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;

	g_nOfTrackedFingers = 0;
}

void BEL_ST_Launcher_TurnTextInputOff(void)
{
	g_sdlLauncherTextInputUIIsShown = false;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();

	g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
	g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;

	g_nOfTrackedFingers = 0;
}

static void BEL_ST_Launcher_ToggleTextSearchUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextSearchLayout[y][x]], isMarked, isPressed);

	SDL_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	SDL_UpdateTexture(g_sdlLauncherTextSearchTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}

static void BEL_ST_Launcher_ToggleTextInputUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlIntScanCodeKeyboardUIStrs_Ptr[g_sdlIntScanCodeTextInputLayout[y][x]], isMarked, isPressed);

	SDL_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	SDL_UpdateTexture(g_sdlLauncherTextInputTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}


static void BEL_ST_Launcher_ToggleOffAllTextSearchUIKeysTrackedbyFingers(void)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		BEL_ST_Launcher_ToggleTextSearchUIKey(g_sdlTrackedFingers[i].key.x, g_sdlTrackedFingers[i].key.y, false, false);

	g_nOfTrackedFingers = 0;
	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_ToggleOffAllTextInputUIKeysTrackedbyFingers(void)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		BEL_ST_Launcher_ToggleTextInputUIKey(g_sdlTrackedFingers[i].key.x, g_sdlTrackedFingers[i].key.y, false, false);

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

static BESDLLauncherTrackedFinger *BEL_ST_Launcher_ProcessAndGetPressedTrackedFinger(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
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

	BESDLLauncherTrackedFinger *trackedFinger = &g_sdlTrackedFingers[g_nOfTrackedFingers++];
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

static BESDLLauncherTrackedFinger *BEL_ST_Launcher_ProcessAndGetMovedTrackedFinger(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
			break;

	if (i == g_nOfTrackedFingers)
		return NULL;

	BESDLLauncherTrackedFinger *trackedFinger = &g_sdlTrackedFingers[i];
	if (g_refKeenCfg.touchInputDebugging)
	{
		trackedFinger->lastX = x;
		trackedFinger->lastY = y;
		//g_sdlForceGfxControlUiRefresh = true;
		BE_ST_Launcher_MarkGfxCache();
	}
	return trackedFinger;
}

static BESDLLauncherTrackedFinger *BEL_ST_Launcher_GetReleasedTrackedFinger(SDL_TouchID touchId, SDL_FingerID fingerId)
{
	int i;
	for (i = 0; i < g_nOfTrackedFingers; ++i)
		if ((g_sdlTrackedFingers[i].touchId == touchId) && (g_sdlTrackedFingers[i].fingerId == fingerId))
			return &g_sdlTrackedFingers[i];

	return NULL;
}

static void BEL_ST_Launcher_RemoveTrackedFinger(BESDLLauncherTrackedFinger *trackedFinger)
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


static void BEL_ST_Launcher_CheckMovedPointerInTextSearchUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLLauncherTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerLauncherTextSearchRect.x) || (x >= g_sdlControllerLauncherTextSearchRect.x+g_sdlControllerLauncherTextSearchRect.w)
	    || (y < g_sdlControllerLauncherTextSearchRect.y) || (y >= g_sdlControllerLauncherTextSearchRect.y+g_sdlControllerLauncherTextSearchRect.h))
		return;

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextSearchUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerLauncherTextSearchRect.x)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH/g_sdlControllerLauncherTextSearchRect.w;
	int keyY = (y-g_sdlControllerLauncherTextSearchRect.y)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT/g_sdlControllerLauncherTextSearchRect.h;

	if ((trackedFinger->key.x != keyX) || (trackedFinger->key.y != keyY))
	{
		BEL_ST_Launcher_ToggleTextSearchUIKey(trackedFinger->key.x, trackedFinger->key.y, false, false);
		BEL_ST_Launcher_ToggleTextSearchUIKey(keyX, keyY, true, false);
		trackedFinger->key.x = keyX;
		trackedFinger->key.y = keyY;
	}

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_CheckPressedPointerInTextSearchUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLLauncherTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
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
	trackedFinger->key.x = (x-g_sdlControllerLauncherTextSearchRect.x)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_WIDTH/g_sdlControllerLauncherTextSearchRect.w;
	trackedFinger->key.y = (y-g_sdlControllerLauncherTextSearchRect.y)*ALTCONTROLLER_LAUNCHER_TEXTSEARCH_KEYS_HEIGHT/g_sdlControllerLauncherTextSearchRect.h;

	BEL_ST_Launcher_ToggleTextSearchUIKey(trackedFinger->key.x, trackedFinger->key.y, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_CheckReleasedPointerInTextSearchUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLLauncherTrackedFinger *trackedFinger = BEL_ST_Launcher_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return;

	int prevKeyX = trackedFinger->key.x;
	int prevKeyY = trackedFinger->key.y;

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


static void BEL_ST_Launcher_CheckMovedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLLauncherTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetMovedTrackedFinger(touchId, fingerId, x, y);
	if (!trackedFinger)
		return;

	if ((x < g_sdlControllerLauncherTextInputRect.x) || (x >= g_sdlControllerLauncherTextInputRect.x+g_sdlControllerLauncherTextInputRect.w)
	    || (y < g_sdlControllerLauncherTextInputRect.y) || (y >= g_sdlControllerLauncherTextInputRect.y+g_sdlControllerLauncherTextInputRect.h))
		return;

	BEL_ST_Launcher_UnmarkAndReleaseSelectedKeyInTextInputUI();

	// Normalize coordinates to keys
	int keyX = (x-g_sdlControllerLauncherTextInputRect.x)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerLauncherTextInputRect.w;
	int keyY = (y-g_sdlControllerLauncherTextInputRect.y)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerLauncherTextInputRect.h;

	if ((trackedFinger->key.x != keyX) || (trackedFinger->key.y != keyY))
	{
		BEL_ST_Launcher_ToggleTextInputUIKey(trackedFinger->key.x, trackedFinger->key.y, false, false);
		BEL_ST_Launcher_ToggleTextInputUIKey(keyX, keyY, true, false);
		trackedFinger->key.x = keyX;
		trackedFinger->key.y = keyY;
	}

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_ST_Launcher_CheckPressedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	BESDLLauncherTrackedFinger *trackedFinger = BEL_ST_Launcher_ProcessAndGetPressedTrackedFinger(touchId, fingerId, x, y);
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
	trackedFinger->key.x = (x-g_sdlControllerLauncherTextInputRect.x)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerLauncherTextInputRect.w;
	trackedFinger->key.y = (y-g_sdlControllerLauncherTextInputRect.y)*ALTCONTROLLER_LAUNCHER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerLauncherTextInputRect.h;

	BEL_ST_Launcher_ToggleTextInputUIKey(trackedFinger->key.x, trackedFinger->key.y, true, false);

	//g_sdlForceGfxControlUiRefresh = true;
	BE_ST_Launcher_MarkGfxCache();
}

static bool BEL_ST_Launcher_CheckReleasedPointerInTextInputUI(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, bool *pConfirmed)
{
	bool ret = false;
	BESDLLauncherTrackedFinger *trackedFinger = BEL_ST_Launcher_GetReleasedTrackedFinger(touchId, fingerId);
	if (!trackedFinger)
		return ret;

	int prevKeyX = trackedFinger->key.x;
	int prevKeyY = trackedFinger->key.y;

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
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		if (isPressed)
			BEL_ST_Launcher_MoveUpInTextSearchUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		if (isPressed)
			BEL_ST_Launcher_MoveDownInTextSearchUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		if (isPressed)
			BEL_ST_Launcher_MoveLeftInTextSearchUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_Launcher_MoveRightInTextSearchUI();
		break;
	// A few other special cases
	case SDL_CONTROLLER_BUTTON_B:
	case SDL_CONTROLLER_BUTTON_BACK:
		if (isPressed)
			BEL_ST_Launcher_HandleKeyPressEvent(SDL_SCANCODE_ESCAPE, g_sdlKeyboardUIIsShifted);
		break;
	case SDL_CONTROLLER_BUTTON_X:
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
	if ((but == SDL_CONTROLLER_BUTTON_Y) && isPressed)
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
	case SDL_CONTROLLER_BUTTON_A:
		BE_Launcher_HandleInput_ButtonActivate();
		break;
	case SDL_CONTROLLER_BUTTON_B:
	case SDL_CONTROLLER_BUTTON_BACK:
		BE_Launcher_HandleInput_ButtonBack();
		break;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		BE_Launcher_HandleInput_ButtonPageUp();
		break;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		BE_Launcher_HandleInput_ButtonPageDown();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		BE_Launcher_HandleInput_ButtonUp();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		BE_Launcher_HandleInput_ButtonDown();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		BE_Launcher_HandleInput_ButtonLeft();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		BE_Launcher_HandleInput_ButtonRight();
		break;
	}
}


static void BEL_ST_Launcher_FinishHostDisplayUpdate(void)
{
	//g_sdlForceGfxControlUiRefresh = false;

	if (g_refKeenCfg.touchInputDebugging)
	{
		SDL_SetRenderDrawColor(g_sdlRenderer, 0xFF, 0x00, 0x00, 0xBF); // Includes some alpha
		SDL_SetRenderDrawBlendMode(g_sdlRenderer, SDL_BLENDMODE_BLEND);
		BESDLLauncherTrackedFinger *trackedFinger = g_sdlTrackedFingers;
		for (int i = 0; i < g_nOfTrackedFingers; ++i, ++trackedFinger)
		{
			SDL_Rect rect = {trackedFinger->lastX-g_sdlDebugFingerRectSideLen/2, trackedFinger->lastY-g_sdlDebugFingerRectSideLen/2, g_sdlDebugFingerRectSideLen, g_sdlDebugFingerRectSideLen};
			SDL_RenderFillRect(g_sdlRenderer, &rect);
		}
		SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_SetRenderDrawBlendMode(g_sdlRenderer, SDL_BLENDMODE_NONE);
	}

	if (g_sdlLauncherTextSearchUIIsShown)
		SDL_RenderCopy(g_sdlRenderer, g_sdlLauncherTextSearchTexture, NULL, &g_sdlControllerLauncherTextSearchRect);
	else if (g_sdlLauncherTextInputUIIsShown)
		SDL_RenderCopy(g_sdlRenderer, g_sdlLauncherTextInputTexture, NULL, &g_sdlControllerLauncherTextInputRect);


        SDL_RenderPresent(g_sdlRenderer);
}


static uint32_t g_be_sdlLastRefreshTicks = 0;

static void BEL_ST_Launcher_UpdateHostDisplay(void)
{
	if (g_sdlLauncherGfxCacheMarked)
	{
		BEL_ST_SleepMS(1);
		g_sdlLauncherGfxCacheMarked = false;
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		uint8_t *currPalPixPtr = g_sdlLauncherGfxCache;
		for (int pixnum = 0; pixnum < BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlEGABGRAScreenColors[*currPalPixPtr];
		}

		SDL_UnlockTexture(g_sdlTexture);
		SDL_RenderClear(g_sdlRenderer);
		if (g_sdlTargetTexture)
		{
			if (SDL_SetRenderTarget(g_sdlRenderer, g_sdlTargetTexture) != 0)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_Launcher_UpdateHostDisplay: Failed to set target texture as render target (disabling),\n%s\n", SDL_GetError());
				SDL_DestroyTexture(g_sdlTargetTexture);
				g_sdlTargetTexture = NULL;
				goto refreshwithnorendertarget;
			}
			SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, NULL);
			if (SDL_SetRenderTarget(g_sdlRenderer, NULL) != 0)
				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_Launcher_UpdateHostDisplay: Failed to set default render target!\n%s\n", SDL_GetError());
			SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionBorderedRect);
		}
		else
		{
refreshwithnorendertarget:
			SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionBorderedRect);
		}
		BEL_ST_Launcher_FinishHostDisplayUpdate();
		g_be_sdlLastRefreshTicks = SDL_GetTicks();
	}
	else
	{
		// Refresh graphics from time to time in case a part of the window is overridden by anything,
		// like the Steam Overlay. Sleep for less time so the application is somewhat responsive, though.
		BEL_ST_SleepMS(10);
		uint32_t currRefreshTicks = SDL_GetTicks();
		if (currRefreshTicks - g_be_sdlLastRefreshTicks >= 100)
		{
			SDL_RenderClear(g_sdlRenderer);

			if (g_sdlTargetTexture)
				SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionBorderedRect);
			else
				SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionBorderedRect);

			BEL_ST_Launcher_FinishHostDisplayUpdate();
			g_be_sdlLastRefreshTicks = currRefreshTicks;
		}
	}
}


static void BEL_ST_Launcher_CheckCommonPointerPressCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, uint32_t ticksBeforePoll)
{
	if (g_sdlLauncherTextSearchUIIsShown)
	{
		g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
		BEL_ST_Launcher_CheckPressedPointerInTextSearchUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_HandleInput_PointerSelect(x, y, ticksBeforePoll);
	}
}

static void BEL_ST_Launcher_CheckCommonPointerReleaseCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, uint32_t ticksBeforePoll)
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

static void BEL_ST_Launcher_CheckCommonPointerMoveCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, uint32_t ticksBeforePoll)
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
		uint32_t ticksBeforePoll = SDL_GetTicks();
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

				BEL_ST_Launcher_CheckCommonPointerPressCases(0, 0, event.button.x, event.button.y, ticksBeforePoll);
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_CheckCommonPointerReleaseCases(0, 0, event.button.x, event.button.y, ticksBeforePoll);
				break;
			case SDL_MOUSEMOTION:
				if (event.button.which == SDL_TOUCH_MOUSEID)
					break;

				BEL_ST_Launcher_CheckCommonPointerMoveCases(0, 0, event.motion.x, event.motion.y, ticksBeforePoll);
				break;
			case SDL_MOUSEWHEEL:
				BE_Launcher_HandleInput_PointerVScroll(-10*event.wheel.y, ticksBeforePoll);
				break;

			case SDL_FINGERDOWN:
				BEL_ST_Launcher_CheckCommonPointerPressCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, ticksBeforePoll);
				break;
			case SDL_FINGERUP:
				BEL_ST_Launcher_CheckCommonPointerReleaseCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, ticksBeforePoll);
				break;
			case SDL_FINGERMOTION:
				BEL_ST_Launcher_CheckCommonPointerMoveCases(event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x * g_sdlLastReportedWindowWidth, event.tfinger.y * g_sdlLastReportedWindowHeight, ticksBeforePoll);
				break;

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
				if ((event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) || (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
					g_sdlLauncherTriggerBinaryStates[event.caxis.axis - SDL_CONTROLLER_AXIS_TRIGGERLEFT] = (event.caxis.value >= g_sdlJoystickAxisBinaryThreshold);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				g_sdlControllerLastButtonPressed = event.cbutton.button;
				g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
				g_sdlInputLastBinaryPressTime = ticksBeforePoll;
				g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
				BEL_ST_Launcher_HandleControllerButtonEvent(event.cbutton.button, true);
				break;
			case SDL_CONTROLLERBUTTONUP:
				if (g_sdlControllerLastButtonPressed == event.cbutton.button)
					g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
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
			case SDL_QUIT:
				BE_ST_Launcher_Shutdown();
				BE_ST_QuickExit();
				break;
			}
		}

		// Emulate "key repeat" for keyboard/controller buttons
		if (((g_sdlKeyboardLastKeyPressed != SDL_SCANCODE_UNKNOWN) || (g_sdlControllerLastButtonPressed != SDL_CONTROLLER_BUTTON_INVALID)) && (ticksBeforePoll - g_sdlInputLastBinaryPressTime >= g_sdlInputLastBinaryPressTimeDelay))
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
	void *pixels;
	int pitch;
	SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
	uint32_t *currPixPtr = (uint32_t *)pixels;
	uint8_t *currPalPixPtr = g_sdlLauncherGfxCache;
	for (int pixnum = 0; pixnum < BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
	{
		*currPixPtr = g_sdlEGABGRAScreenColors[*currPalPixPtr];
	}

	SDL_UnlockTexture(g_sdlTexture);

	if (g_sdlTargetTexture)
	{
		SDL_SetRenderTarget(g_sdlRenderer, g_sdlTargetTexture);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, NULL);
		SDL_SetRenderTarget(g_sdlRenderer, NULL);
	}

	SDL_Event event;
	bool keepRunning = true;
	const int defaultChoice = SDL_CONTROLLER_BUTTON_MAX + 2/*triggers*/;
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
			case SDL_FINGERDOWN:
				keepRunning = false;
				break;

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
				if ((event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) || (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
				{
					int triggerNum = event.caxis.axis - SDL_CONTROLLER_AXIS_TRIGGERLEFT;
					bool prevBinaryState = g_sdlLauncherTriggerBinaryStates[triggerNum];
					g_sdlLauncherTriggerBinaryStates[triggerNum] = (event.caxis.value >= g_sdlJoystickAxisBinaryThreshold);
					if (!prevBinaryState && g_sdlLauncherTriggerBinaryStates[triggerNum])
					{
						choice = SDL_CONTROLLER_BUTTON_MAX + triggerNum; // HACK
						keepRunning = false;
					}
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				switch (event.cbutton.button)
				{
					case SDL_CONTROLLER_BUTTON_A:
					case SDL_CONTROLLER_BUTTON_B:
					case SDL_CONTROLLER_BUTTON_X:
					case SDL_CONTROLLER_BUTTON_Y:
					case SDL_CONTROLLER_BUTTON_LEFTSTICK:
					case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
					case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
					case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
						choice = event.cbutton.button;
						// Fall-through
					default:
						keepRunning = false;
						break;
				}
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
			case SDL_QUIT:
				BE_ST_Launcher_Shutdown();
				BE_ST_QuickExit();
				break;
			}
		}

		BEL_ST_SleepMS(10);
		SDL_RenderClear(g_sdlRenderer);

		if (g_sdlTargetTexture)
			SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionBorderedRect);
		else
			SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionBorderedRect);

		SDL_RenderPresent(g_sdlRenderer);
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
	g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
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
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		if (isPressed)
			BEL_ST_Launcher_MoveUpInTextInputUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		if (isPressed)
			BEL_ST_Launcher_MoveDownInTextInputUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		if (isPressed)
			BEL_ST_Launcher_MoveLeftInTextInputUI();
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		if (isPressed)
			BEL_ST_Launcher_MoveRightInTextInputUI();
		break;
	// A few other special cases
	case SDL_CONTROLLER_BUTTON_B:
	case SDL_CONTROLLER_BUTTON_BACK:
		if (isPressed)
			return BEL_ST_Launcher_ArgumentsEditing_HandleKeyPressEvent(SDL_SCANCODE_ESCAPE, g_sdlKeyboardUIIsShifted, pConfirmed);
		break;
	case SDL_CONTROLLER_BUTTON_X:
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
	if ((but == SDL_CONTROLLER_BUTTON_Y) && isPressed)
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
	case SDL_CONTROLLER_BUTTON_A:
		BEL_ST_Launcher_ToggleTextInput();
		break;
	case SDL_CONTROLLER_BUTTON_B:
	case SDL_CONTROLLER_BUTTON_BACK:
		*pConfirmed = false;
		return true;
	}
	return false;
}

static void BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerPressCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
{
	if (g_sdlLauncherTextInputUIIsShown)
	{
		g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
		BEL_ST_Launcher_CheckPressedPointerInTextInputUI(touchId, fingerId, x, y);
	}
	else
	{
		BEL_ST_Launcher_NormalizePos(&x, &y);
		BE_Launcher_ArgumentsEditing_HandleInput_PointerSelect(x, y);
	}
}

static bool BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerReleaseCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y, bool *pConfirmed)
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

static void BEL_ST_Launcher_ArgumentsEditing_CheckCommonPointerMoveCases(SDL_TouchID touchId, SDL_FingerID fingerId, int x, int y)
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
		uint32_t ticksBeforePoll = SDL_GetTicks();
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
				if ((event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) || (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
					g_sdlLauncherTriggerBinaryStates[event.caxis.axis - SDL_CONTROLLER_AXIS_TRIGGERLEFT] = (event.caxis.value >= g_sdlJoystickAxisBinaryThreshold);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				g_sdlControllerLastButtonPressed = event.cbutton.button;
				g_sdlKeyboardLastKeyPressed = SDL_SCANCODE_UNKNOWN;
				g_sdlInputLastBinaryPressTime = ticksBeforePoll;
				g_sdlInputLastBinaryPressTimeDelay = BE_ST_SDL_CONTROLLER_DELAY_BEFORE_DIGIACTION_REPEAT_MS;
				if (BEL_ST_Launcher_ArgumentsEditing_HandleControllerButtonEvent(event.cbutton.button, true, &confirmed))
					return confirmed;
				break;
			case SDL_CONTROLLERBUTTONUP:
				if (g_sdlControllerLastButtonPressed == event.cbutton.button)
					g_sdlControllerLastButtonPressed = SDL_CONTROLLER_BUTTON_INVALID;
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
			case SDL_QUIT:
				BE_ST_Launcher_Shutdown();
				BE_ST_QuickExit();
				break;
			}
		}
		// Emulate "key repeat" for keyboard/controller buttons
		if (((g_sdlKeyboardLastKeyPressed != SDL_SCANCODE_UNKNOWN) || (g_sdlControllerLastButtonPressed != SDL_CONTROLLER_BUTTON_INVALID)) && (ticksBeforePoll - g_sdlInputLastBinaryPressTime >= g_sdlInputLastBinaryPressTimeDelay))
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
