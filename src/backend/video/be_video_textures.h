#ifndef BE_VIDEO_TEXTURES_H
#define BE_VIDEO_TEXTURES_H

#include "be_video.h"

typedef void BE_ST_Texture;

extern BE_ST_Texture *g_sdlTexture, *g_sdlTargetTexture;

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
void BEL_ST_RecreateMainTextures(void);
void BEL_ST_RecreateAllTextures(void);

// These might be implementation-dependent
int BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst);
void *BEL_ST_LockTexture(BE_ST_Texture *texture);
void BEL_ST_UnlockTexture(BE_ST_Texture *texture);
void BEL_ST_UpdateTexture(BE_ST_Texture *texture, const BE_ST_Rect *rect, const void *pixels, int pitch);
int BEL_ST_SetRenderTarget(BE_ST_Texture *texture);

#endif
