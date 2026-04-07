#include <SDL3/SDL.h>
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
SDL_Gamepad *g_sdlControllers[BE_ST_MAXJOYSTICKS];
SDL_JoystickID g_sdlJoysticksInstanceIds[BE_ST_MAXJOYSTICKS];

static BESDLMouseModeEnum g_sdlMouseMode = BE_ST_MOUSEMODE_ABS_WITH_CURSOR;

void BEL_ST_SetMouseMode(BESDLMouseModeEnum mode)
{
	if (g_sdlMouseMode == mode)
		return;

	g_sdlMouseMode = mode;
	if (mode == BE_ST_MOUSEMODE_ABS_WITHOUT_CURSOR)
		SDL_ShowCursor(false);
	else if (mode == BE_ST_MOUSEMODE_ABS_WITH_CURSOR)
		SDL_ShowCursor(true);

	SDL_SetRelativeMouseMode((mode == BE_ST_MOUSEMODE_REL) ? true : false);
}

void BEL_ST_FillJoysticksList(void)
{
	int n_joysticks;
	SDL_JoystickID *joysticks = SDL_GetJoysticks(&n_joysticks);
	if (!joysticks)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_FillJoysticksList: SDL_GetJoysticks failed,\n%s\n", SDL_GetError());
		return;
	}

	if (!g_refKeenCfg.altControlScheme)
	{
		for (int dev_index = 0, i = 0;
		     dev_index < n_joysticks && i < BE_ST_MAXJOYSTICKS; ++dev_index)
			if (!g_sdlJoysticks[i])
			{
				g_sdlJoysticks[i] = SDL_OpenJoystick(joysticks[dev_index]);
				g_sdlJoysticksInstanceIds[i] = SDL_GetJoystickID(g_sdlJoysticks[i]);
				BEL_ST_CheckForHidingTouchUI();
				++i;
			}
	}
	else
	{
		for (int dev_index = 0, i = 0;
		     dev_index < n_joysticks && i < BE_ST_MAXJOYSTICKS; ++dev_index)
			if (!g_sdlControllers[i] && SDL_IsGamepad(joysticks[dev_index]))
			{
				g_sdlControllers[i] = SDL_OpenGamepad(joysticks[dev_index]);
				g_sdlJoysticksInstanceIds[i] = SDL_GetJoystickID(SDL_GetGamepadJoystick(g_sdlControllers[i]));
				++i;

				g_sdlShowControllerUI = true;
				g_sdlForceGfxControlUiRefresh = true;
				BEL_ST_ConditionallyShowAltInputPointer();
				BEL_ST_CheckForHidingTouchUI();
			}
	}
}

void BEL_ST_ConditionallyAddJoystick(SDL_JoystickID dev_id)
{
	if (!g_refKeenCfg.altControlScheme)
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			if (!g_sdlJoysticks[i])
			{
				g_sdlJoysticks[i] = SDL_OpenJoystick(dev_id);
				g_sdlJoysticksInstanceIds[i] = SDL_GetJoystickID(g_sdlJoysticks[i]);
				BEL_ST_CheckForHidingTouchUI();
				break;
			}
	}
	else if (SDL_IsGamepad(dev_id))
	{
		for (int i = 0; i < BE_ST_MAXJOYSTICKS; ++i)
			if (!g_sdlControllers[i])
			{
				g_sdlControllers[i] = SDL_OpenGamepad(dev_id);
				g_sdlJoysticksInstanceIds[i] = SDL_GetJoystickID(SDL_GetGamepadJoystick(g_sdlControllers[i]));

				g_sdlShowControllerUI = true;
				g_sdlForceGfxControlUiRefresh = true;
				BEL_ST_ConditionallyShowAltInputPointer();
				BEL_ST_CheckForHidingTouchUI();
				break;
			}
	}
}
