// This header MUST be included early, so macros can be properly used

#ifndef __REFKEEN_CONFIG_H__
#define __REFKEEN_CONFIG_H__

// BIG ***FIXME*** - Should probably be externally generated instead
#include "SDL_endian.h"
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define REFKEEN_ARCH_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#define REFKEEN_ARCH_BIG_ENDIAN
#endif

#endif // REFKEEN_CONFIG_H
