#ifndef BE_INPUT_SDL_H
#define BE_INPUT_SDL_H

#include "SDL_gamecontroller.h"

#define BE_ST_MAXJOYSTICKS 8

extern SDL_Joystick *g_sdlJoysticks[BE_ST_MAXJOYSTICKS];
extern SDL_GameController *g_sdlControllers[BE_ST_MAXJOYSTICKS];
extern SDL_JoystickID g_sdlJoysticksInstanceIds[BE_ST_MAXJOYSTICKS];

#endif
