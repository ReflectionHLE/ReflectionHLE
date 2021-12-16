#ifndef BE_EVENTS_SDL_H
#define BE_EVENTS_SDL_H

#include "refkeen_config.h"

#ifdef REFKEEN_CONFIG_EVENTS_CALLBACK

#include "SDL.h"

extern SDL_sem *g_sdlEventsCallbackToMainSem, *g_sdlMainToEventsCallbackSem;

void BEL_ST_CheckForExitFromEventsCallback(void);
int BEL_ST_EventsCallback(void *userdata, SDL_Event *event);
#endif

#endif
