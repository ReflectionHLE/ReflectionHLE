#include "SDL.h"
#include "be_input.h"
#include "be_input_sdl.h"
#include "be_input_ui.h"
#include "../video/be_video_ui.h"

// extern is required for C++
#ifdef __cplusplus
extern
#endif
const int g_sdlJoystickAxisBinaryThreshold = 16384, g_sdlJoystickAxisDeadZone = 3200, g_sdlJoystickAxisMax = 32767, g_sdlJoystickAxisMaxMinusDeadZone = 32767-3200;

SDL_Joystick *g_sdlJoysticks[BE_ST_MAXJOYSTICKS];
SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS];
SDL_JoystickID g_sdlJoysticksInstanceIds[BE_ST_MAXJOYSTICKS];

static BESDLMouseModeEnum g_sdlMouseMode = BE_ST_MOUSEMODE_ABS_WITH_CURSOR;

void BEL_ST_SetMouseMode(BESDLMouseModeEnum mode)
{
	if (g_sdlMouseMode == mode)
		return;

	g_sdlMouseMode = mode;
	if (mode == BE_ST_MOUSEMODE_ABS_WITHOUT_CURSOR)
		SDL_ShowCursor(SDL_FALSE);
	else if (mode == BE_ST_MOUSEMODE_ABS_WITH_CURSOR)
		SDL_ShowCursor(SDL_TRUE);

	SDL_SetRelativeMouseMode((mode == BE_ST_MOUSEMODE_REL) ? SDL_TRUE : SDL_FALSE);
}

void BEL_ST_ConditionallyAddJoystick(int device_index)
{
	if (!g_refKeenCfg.altControlScheme.isEnabled)
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			if (!g_sdlJoysticks[i])
			{
				g_sdlJoysticks[i] = SDL_JoystickOpen(device_index);
				g_sdlJoysticksInstanceIds[i] = SDL_JoystickInstanceID(g_sdlJoysticks[i]);
				BEL_ST_CheckForHidingTouchUI();
				break;
			}
	}
	else if (SDL_IsGameController(device_index))
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			if (!g_sdlControllers[i])
			{
				g_sdlControllers[i] = SDL_GameControllerOpen(device_index);
				g_sdlJoysticksInstanceIds[i] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_sdlControllers[i]));

				g_sdlShowControllerUI = true;
				g_sdlForceGfxControlUiRefresh = true;
				BEL_ST_ConditionallyShowAltInputPointer();
				BEL_ST_CheckForHidingTouchUI();
				break;
			}
	}
}
