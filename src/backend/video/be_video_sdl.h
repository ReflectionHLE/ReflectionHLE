#ifndef BE_VIDEO_SDL_H
#define BE_VIDEO_SDL_H

#include "SDL_touch.h"

/* All that should be assumed about the values is that they can be compared
   as equal or different.
   No specific value should represent an uninitialized id. */
typedef SDL_TouchID BE_ST_TouchID;
typedef SDL_FingerID BE_ST_FingerID;

// One exception: It's assumed the following touch id may represent the mouse
enum { BE_ST_MouseTouchID = 0 };

#endif
