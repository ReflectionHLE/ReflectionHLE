#include "SDL.h"

#include "refkeen.h"

#define BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN 10

/*** Convenience macros - Note that the label *must* be a C string literal ***/
#define BEMENUITEM_DEF_TARGETMENU(menuItemName, label, menuPtr) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, NULL, menuPtr, label, BE_MENUITEM_TYPE_TARGETMENU)
#define BEMENUITEM_DEF_SELECTION(menuItemName, label, choices) BEMENUITEM_DEF_GENERIC(menuItemName, NULL, choices, NULL, label, BE_MENUITEM_TYPE_SELECTION)
#define BEMENUITEM_DEF_HANDLER(menuItemName, label, handlerPtr) BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, NULL, NULL, label, BE_MENUITEM_TYPE_HANDLER)

// A little hack - Store a copy of the label string literal that can be modified
#define BEMENUITEM_DEF_GENERIC(menuItemName, handlerPtr, choices, menuPtr, label, type) \
	static char menuItemName ## _label[] = label; \
	static BEMenuItem menuItemName = {handlerPtr, choices, menuPtr, menuItemName ## _label, 0, 0, 0, 0, 0, 0, type};

static const char *g_be_settingsChoices_boolean[] = {"No","Yes",NULL};

/*** Main menu ***/

BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_PlayGame, "Play game", &g_beSelectGameMenu)
BEMENUITEM_DEF_TARGETMENU(g_beMainMenuItem_Settings, "Settings", &g_beSettingsMenu)
BEMENUITEM_DEF_HANDLER(g_beMainMenuItem_Quit, "Quit", &BE_Launcher_Handler_MenuQuit)

BEMenu g_beMainMenu = {
	"Reflection Keen", // TODO rename this based on EXE
	&g_beMainMenu,
	(BEMenuItem *[])
	{
		&g_beMainMenuItem_PlayGame,
		&g_beMainMenuItem_Settings,
		&g_beMainMenuItem_Quit,
		NULL
	},
	// Ignore the rest
};

/*** Select game menu ***/

static BEMenuItem g_beSelectGameMenuItems[BE_CROSS_MAX_GAME_INSTALLATIONS];
static char g_beSelectGameMenuItemsStrs[BE_CROSS_MAX_GAME_INSTALLATIONS][78]; // Should be MUTABLE strings for layout preparation
static BEMenuItem *g_beSelectGameMenuItemsPtrs[BE_CROSS_MAX_GAME_INSTALLATIONS+1];

BEMenu g_beSelectGameMenu = {
	"Select game",
	&g_beMainMenu,
	g_beSelectGameMenuItemsPtrs, // Array of menu items
	// Ignore the rest
};

/*** Settings menu ***/

BEMENUITEM_DEF_TARGETMENU(g_beSettingsMenuItem_VideoSettings, "Video settings", &g_beVideoSettingsMenu)
BEMENUITEM_DEF_TARGETMENU(g_beSettingsMenuItem_MoreSettings, "More settings", &g_beMoreSettingsMenu)

BEMenu g_beSettingsMenu = {
	"Settings",
	&g_beMainMenu,
	(BEMenuItem *[])
	{
		&g_beSettingsMenuItem_VideoSettings,
		&g_beSettingsMenuItem_MoreSettings,
		NULL
	},
	// Ignore the rest
};

/** Video settings menu ***/

#define BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS 10

typedef struct {
	int width, height;
} BEMenuItemScreenResPair;

static BEMenuItemScreenResPair g_be_videoSettingsChoices_fullResolutionsVals[BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS];

static char g_be_videoSettingsChoices_fullResolutionsStrs[BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS][BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN];
// Need to add an additional desktop/default entry, plus NULL terminator
static const char *g_be_videoSettingsChoices_fullResolutions[BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS+2];

static const char *g_be_videoSettingsChoices_displayNums[] = {"0","1","2","3","4","5","6","7",NULL};

#define BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS 10

static char g_be_videoSettingsChoices_sdlRendererDriversStrs[BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS][BE_LAUNCHER_MAX_CHOICE_STRBUFFLEN];
// Need to add additional auto entry, plus NULL terminator
static const char *g_be_videoSettingsChoices_sdlRendererDrivers[BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS+2];

static const char *g_be_settingsChoices_vSync[] = {"Auto","Off","On",NULL};
static const char *g_be_settingsChoices_scaleType[] = {"4:3","Fill",NULL};
static const char *g_be_settingsChoices_scaleFactor[] = {"1","2","3","4",NULL};

BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_Fullscreen, "Fullscreen", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_FullscreenRes, "Fullscreen resolution", g_be_videoSettingsChoices_fullResolutions)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_DisplayNum, "Display number", g_be_videoSettingsChoices_displayNums)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_SDLRenderer, "SDL renderer", g_be_videoSettingsChoices_sdlRendererDrivers)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_VSync, "VSync", g_be_settingsChoices_vSync)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_Bilinear, "Bilinear interpolation", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ScaleType, "Scale type", g_be_settingsChoices_scaleType)
BEMENUITEM_DEF_SELECTION(g_beVideoSettingsMenuItem_ScaleFactor, "Scale factor", g_be_settingsChoices_scaleFactor)

BEMenu g_beVideoSettingsMenu = {
	"Video settings",
	&g_beSettingsMenu,
	(BEMenuItem *[])
	{
		&g_beVideoSettingsMenuItem_Fullscreen,
		&g_beVideoSettingsMenuItem_FullscreenRes,
		&g_beVideoSettingsMenuItem_DisplayNum,
		&g_beVideoSettingsMenuItem_SDLRenderer,
		&g_beVideoSettingsMenuItem_VSync,
		&g_beVideoSettingsMenuItem_Bilinear,
		&g_beVideoSettingsMenuItem_ScaleType,
		&g_beVideoSettingsMenuItem_ScaleFactor,
		NULL
	},
	// Ignore the rest
};

/** More settings menu ***/

// TODO (REFKEEN) Add more settings for game controller

static const int g_be_settingsChoices_sndSampleRateVals[] = {8000, 11025, 12000, 16000, 22050, 32000, 44100, 48000, 49716, 96000};
static const char *g_be_settingsChoices_sndSampleRate[] = {"8000","11025","12000","16000","22050","32000","44100","48000","49716","96000",NULL};

static const char *g_be_settingsChoices_controllerScheme[] = {"Classic", "Modern", NULL};

BEMENUITEM_DEF_SELECTION(g_beMoreSettingsMenuItem_Autolock, "Autolock mouse cursor in fullscreen", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beMoreSettingsMenuItem_SndSampleRate, "Sound sample rate (in Hz)", g_be_settingsChoices_sndSampleRate)
BEMENUITEM_DEF_SELECTION(g_beMoreSettingsMenuItem_SndSubSystem, "Enable sound subsystem", g_be_settingsChoices_boolean)
BEMENUITEM_DEF_SELECTION(g_beMoreSettingsMenuItem_ControllerScheme, "Game controller scheme", g_be_settingsChoices_controllerScheme)

BEMenu g_beMoreSettingsMenu = {
	"More settings",
	&g_beSettingsMenu,
	(BEMenuItem *[])
	{
		&g_beMoreSettingsMenuItem_Autolock,
		&g_beMoreSettingsMenuItem_SndSampleRate,
		&g_beMoreSettingsMenuItem_SndSubSystem,
		&g_beMoreSettingsMenuItem_ControllerScheme,
		NULL
	},
	// Ignore the rest
};


void BE_ST_PrepareLauncher(void)
{
	// Set fullscreen value
	g_beVideoSettingsMenuItem_Fullscreen.choice = g_refKeenCfg.isFullscreen;
	/*** Prepare displays list ***/
	int nOfDisplays = SDL_GetNumVideoDisplays();
	if (nOfDisplays >= sizeof(g_be_videoSettingsChoices_displayNums)/sizeof(*g_be_videoSettingsChoices_displayNums))
		nOfDisplays = sizeof(g_be_videoSettingsChoices_displayNums)/sizeof(*g_be_videoSettingsChoices_displayNums) - 1;
	g_be_videoSettingsChoices_displayNums[nOfDisplays] = NULL;
	g_beVideoSettingsMenuItem_DisplayNum.choice = g_refKeenCfg.displayNum;
	if (g_beVideoSettingsMenuItem_DisplayNum.choice >= nOfDisplays)
		g_beVideoSettingsMenuItem_DisplayNum.choice = 0;
	/*** Prepare fullscreen resolutions list ***/
	// FIXME: Resolutions list should be reset after choosing another display number
	int nOfDisplayModes = SDL_GetNumDisplayModes(g_refKeenCfg.displayNum);
	if (nOfDisplayModes > BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS)
		nOfDisplayModes = BE_LAUNCHER_MAX_NUM_OF_RESOLUTIONS;
	g_beVideoSettingsMenuItem_FullscreenRes.choices[0] = "Desktop";
	g_beVideoSettingsMenuItem_FullscreenRes.choice = 0;
	int actualCounter = 0;
	for (int i = 0; i < nOfDisplayModes; ++i)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(g_refKeenCfg.displayNum, i, &mode);
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
	/*** Prepare SDL renderer drivers list ***/
	int nOfSDLRendererDrivers = SDL_GetNumRenderDrivers();
	if (nOfSDLRendererDrivers > BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS)
		nOfSDLRendererDrivers = BE_LAUNCHER_MAX_NUM_OF_SDL_RENDERER_DRIVERS;
	g_beVideoSettingsMenuItem_SDLRenderer.choices[0] = "auto";
	g_beVideoSettingsMenuItem_SDLRenderer.choices[nOfSDLRendererDrivers+1] = NULL;
	for (int i = 0; i < nOfSDLRendererDrivers; ++i)
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
	// Set VSync value
	g_beVideoSettingsMenuItem_VSync.choice = g_refKeenCfg.vSync;
	// Set Bilinear value
	g_beVideoSettingsMenuItem_Bilinear.choice = g_refKeenCfg.isBilinear;
	// Set ScaleType value
	g_beVideoSettingsMenuItem_ScaleType.choice = g_refKeenCfg.scaleType;
	// Set ScaleFactor value
	if ((g_refKeenCfg.scaleFactor <= 0) || (g_refKeenCfg.scaleFactor >= sizeof(g_be_settingsChoices_scaleFactor)/sizeof(*g_be_settingsChoices_scaleFactor)))
		g_beVideoSettingsMenuItem_ScaleFactor.choice = 0;
	else
		g_beVideoSettingsMenuItem_ScaleFactor.choice = g_refKeenCfg.scaleFactor-1;
	// Set Autolock value
	g_beMoreSettingsMenuItem_Autolock.choice = g_refKeenCfg.autolockCursor;
	// Set SndSampleRate value
	g_beMoreSettingsMenuItem_SndSampleRate.choice = 8; // FIXME - Better way to grab default off cfg
	for (int i = 0; i < sizeof(g_be_settingsChoices_sndSampleRateVals)/sizeof(*g_be_settingsChoices_sndSampleRateVals); ++i)
		if (g_be_settingsChoices_sndSampleRateVals[i] == g_refKeenCfg.sndSampleRate)
		{
			g_beMoreSettingsMenuItem_SndSampleRate.choice = i;
			break;
		}
	// Set SndSubSystem value
	g_beMoreSettingsMenuItem_SndSubSystem.choice = g_refKeenCfg.sndSubSystem;
	// Set ControllerScheme value
	g_beMoreSettingsMenuItem_ControllerScheme.choice = g_refKeenCfg.altControlScheme.isEnabled;

	/*** Prepare installed game versions menu ***/
	for (int i = 0; i < g_be_gameinstallations_num; ++i)
	{
		g_beSelectGameMenuItemsPtrs[i] = &g_beSelectGameMenuItems[i];
		g_beSelectGameMenuItems[i].handler = &BE_Launcher_Handler_GameLaunch;
		snprintf(g_beSelectGameMenuItemsStrs[i], sizeof(g_beSelectGameMenuItemsStrs[i]), "%s", BE_Cross_GetGameInstallationDescription(i));
		g_beSelectGameMenuItems[i].label = g_beSelectGameMenuItemsStrs[i];
		g_beSelectGameMenuItems[i].type = BE_MENUITEM_TYPE_HANDLER;
	}
	g_beSelectGameMenuItemsPtrs[g_be_gameinstallations_num] = NULL;

	/*** Finally prepare window for launcher ***/
	BE_ST_SetScreenMode(-1);
}


void BE_ST_UpdateCfgFromLauncher(void)
{
	if (!g_be_launcher_wasAnySettingChanged)
		return; // e.g., if there is a cfg value the launcher doesn't cope with (say, out of some range)

	g_refKeenCfg.isFullscreen = g_beVideoSettingsMenuItem_Fullscreen.choice;
	g_refKeenCfg.displayNum = g_beVideoSettingsMenuItem_DisplayNum.choice;

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

	g_refKeenCfg.sdlRendererDriver = g_beVideoSettingsMenuItem_SDLRenderer.choice - 1;
	g_refKeenCfg.vSync = g_beVideoSettingsMenuItem_VSync.choice;
	g_refKeenCfg.isBilinear = g_beVideoSettingsMenuItem_Bilinear.choice;
	g_refKeenCfg.scaleType = g_beVideoSettingsMenuItem_ScaleType.choice;
	g_refKeenCfg.scaleFactor = g_beVideoSettingsMenuItem_ScaleFactor.choice + 1;

	g_refKeenCfg.autolockCursor = g_beMoreSettingsMenuItem_Autolock.choice;
	g_refKeenCfg.sndSampleRate = g_be_settingsChoices_sndSampleRateVals[g_beMoreSettingsMenuItem_SndSampleRate.choice];
	g_refKeenCfg.sndSubSystem = g_beMoreSettingsMenuItem_SndSubSystem.choice;
	g_refKeenCfg.altControlScheme.isEnabled = g_beMoreSettingsMenuItem_ControllerScheme.choice;
}


void BE_ST_Launcher_RunEventLoop(void)
{
	SDL_Event event;
	while (1)
	{
		while (SDL_PollEvent(&event))
		{
			void BEL_ST_NormalizeBorderedPos(int *px, int *py);

			switch (event.type)
			{
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode)
				{
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
					BE_Launcher_HandleInput_ButtonBack();
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				BEL_ST_NormalizeBorderedPos(&event.button.x, &event.button.y);
				BE_Launcher_HandleInput_PointerSelect(event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				BEL_ST_NormalizeBorderedPos(&event.button.x, &event.button.y);
				BE_Launcher_HandleInput_PointerRelease(event.button.x, event.button.y);
				break;
			case SDL_MOUSEMOTION:
				BEL_ST_NormalizeBorderedPos(&event.button.x, &event.button.y);
				BE_Launcher_HandleInput_PointerMotion(event.button.x, event.button.y);
				break;
			case SDL_MOUSEWHEEL:
				BE_Launcher_HandleInput_PointerVScroll(-6*event.wheel.y);
				break;
			// TODO - Mouse events (including wheel and cursor drags)
			// TODO - Game controller events (similar to keyboard)
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				case  SDL_WINDOWEVENT_RESIZED:
				{
					void BE_ST_SetGfxOutputRects(void);
					BE_ST_SetGfxOutputRects();
					//BE_ST_MarkGfxForPendingUpdate();
					BE_ST_MarkGfxForUpdate();
					break;
				}
				case SDL_WINDOWEVENT_EXPOSED:
					//BE_ST_MarkGfxForPendingUpdate();
					BE_ST_MarkGfxForUpdate();
					break;
				break;
			case SDL_QUIT:
				BE_ST_UpdateCfgFromLauncher();
				BE_ST_QuickExit();
				break;
			}
		}
		SDL_Delay(1);
		BE_Launcher_RefreshVerticalScrolling();
		void BEL_ST_UpdateHostDisplay(void);
		BEL_ST_UpdateHostDisplay();
	}
}
