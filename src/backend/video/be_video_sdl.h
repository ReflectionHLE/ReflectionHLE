#ifndef BE_VIDEO_SDL
#define BE_VIDEO_SDL

#include "SDL.h"

typedef struct BE_ST_Rect {
	int x, y, w, h;
} BE_ST_Rect;

typedef SDL_Texture BE_ST_Texture;

void BEL_ST_RecreateWindowAndRenderer(
	int displayNo,
	int windowWidth, int windowHeight,
	int fullWidth, int fullHeight,
	Uint32 windowFlags, int driverIndex, Uint32 rendererFlags);

static void BEL_ST_CalcWindowDimsFromCfg(int *outWidth, int *outHeight);

BE_ST_Texture *BEL_ST_CreateARGBTexture(int w, int h, bool isTarget, bool isLinear);
void BEL_ST_DestroyTexture(BE_ST_Texture *texture);
int BEL_ST_RenderFromTexture(BE_ST_Texture *texture, const BE_ST_Rect *dst);
void *BEL_ST_LockTexture(BE_ST_Texture *texture);
void BEL_ST_UnlockTexture(BE_ST_Texture *texture);
void BEL_ST_UpdateTexture(BE_ST_Texture *texture, const BE_ST_Rect *rect, const void *pixels, int pitch);
int BEL_ST_SetRenderTarget(BE_ST_Texture *texture);

void BEL_ST_RenderClear(void);
void BEL_ST_SetDrawColor(uint32_t color);
void BEL_ST_RenderFill(const BE_ST_Rect *rect);
void BEL_ST_UpdateWindow(void);

#endif
