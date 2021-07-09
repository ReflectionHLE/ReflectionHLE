#include "be_video_sdl.h"

// A set of texture management wrappers, used to support recreation of textures
// when a relevant event arrives (like a device reset).
//
// ***WARNING*** - You MUST pass a pTexture value which is also used
// to manage the texture later, and NOT store a copy of *pTexture in
// another variable.
//
// The thing is, that the BE_ST_Texture pointer values may
// be CHANGED at ANY MOMENT (due to an SDL_RENDER event).

void BEL_ST_SDLCreateTextureWrapper(BE_ST_Texture **pTexture, int w, int h, bool isTarget, bool isLinear);
void BEL_ST_SDLDestroyTextureWrapper(BE_ST_Texture **pTexture);
void BEL_ST_RecreateAllTextures(void);
