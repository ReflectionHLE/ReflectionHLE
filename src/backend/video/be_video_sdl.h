#ifndef BE_VIDEO_SDL
#define BE_VIDEO_SDL

#include "SDL.h"

typedef SDL_Rect BE_ST_Rect; // TODO: This is a transitional definition
typedef SDL_Texture BE_ST_Texture;

BE_ST_Texture *BEL_ST_CreateARGBTexture(int w, int h, bool isTarget, bool isLinear);
static inline void BEL_ST_DestroyTexture(BE_ST_Texture *texture) { SDL_DestroyTexture(texture); }
int BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst);
void *BEL_ST_LockTexture(BE_ST_Texture *texture);
static inline void BEL_ST_UnlockTexture(BE_ST_Texture *texture) { SDL_UnlockTexture(texture); }
int BEL_ST_SetRenderTarget(BE_ST_Texture *texture);

void BEL_ST_RenderClear(void);
void BEL_ST_SetDrawColor(uint32_t color);
void BEL_ST_RenderFill(const BE_ST_Rect *rect);
void BEL_ST_UpdateWindow(void);

#endif
