#include "SDL.h"
#include "id_heads.h"
#include "be_cross.h"

static SDL_Window *g_sdlWindow;
static SDL_Renderer *g_sdlRenderer;
static SDL_Texture *g_sdlTexture;
static SDL_Rect g_sdlAspectCorrectionRect;

#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

#define TEX_WIDTH 320
#define TEX_HEIGHT 200

static uint8_t g_sdlCGAGfxMemory[8192*2];
static uint8_t g_sdlTextModeMemory[80*25*2];
static uint16_t g_sdlScreenStartAddress = 0;
static int g_sdlScreenMode = 3;
static uint8_t g_sdlBorderColor = 0;

void BE_SDL_InitGfx(void)
{
	g_sdlWindow = SDL_CreateWindow("Chocolate Cat3D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!g_sdlWindow)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, -1, 0);
	if (!g_sdlRenderer)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		//Destroy window?
		exit(0);
	}
	g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TEX_WIDTH, TEX_HEIGHT);
	if (!g_sdlTexture)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "Failed to create SDL2 texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
	// Aspect correct
	if (3*winWidth < 4*winHeight) // Thinner than 4:3
	{
		g_sdlAspectCorrectionRect.w = winWidth;
		g_sdlAspectCorrectionRect.h = winWidth*3/4;
		g_sdlAspectCorrectionRect.x = 0;
		g_sdlAspectCorrectionRect.y = (winHeight-g_sdlAspectCorrectionRect.h)/2;
	}
	else // As wide as 4:3 at the least
	{
		g_sdlAspectCorrectionRect.w = winHeight*4/3;
		g_sdlAspectCorrectionRect.h = winHeight;
		g_sdlAspectCorrectionRect.x = (winWidth-g_sdlAspectCorrectionRect.w)/2;
		g_sdlAspectCorrectionRect.y = 0;
	}
}

void BE_SDL_ShutdownGfx(void)
{
	SDL_DestroyTexture(g_sdlTexture);
	g_sdlTexture = NULL;
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
}

void BE_SDL_SetScreenStartAddress(id0_unsigned_t crtc)
{
	g_sdlScreenStartAddress = crtc;
}

uint8_t *BE_SDL_GetTextModeMemoryPtr(void)
{
	return g_sdlTextModeMemory;
}

uint8_t *BE_SDL_GetCGAMemoryPtr(void)
{
	return g_sdlCGAGfxMemory;
}

uint8_t *BE_SDL_GetEGAMemoryPtr(void)
{
	// TODO: IMPLEMENT!
	return 0;
}

void BE_SDL_SetBorderColor(uint8_t color)
{
	g_sdlBorderColor = color;
}
void BE_SDL_SetScreenMode(int mode)
{
	g_sdlScreenMode = mode;
}

// Colors in BGRA format/order (on certain platforms)

static const uint32_t g_sdlCGAGfxBGRAScreenColors[] = {
	0xff000000/*black*/,
	0xff00ffff/*cyan*/,
	0xffff00ff/*magenta*/,
	0xffffffff/*white*/
};

void BE_SDL_UpdateHostDisplay(void)
{
	void *pixels;
	int pitch;
	SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
	uint32_t *currPixPtr;
	uint8_t *srcCGAPtr;
	uint8_t currBitLoc; // Location of bits to check inside byte (two bits)
	// First fill the "even" rows, then the "odd" ones
	for (int i = 0; i < 2; ++i)
	{
		currPixPtr = (uint32_t *)pixels + i*TEX_WIDTH;
		srcCGAPtr = &g_sdlCGAGfxMemory[g_sdlScreenStartAddress+i*0x2000];
		currBitLoc = 6; // Location of bits to check (two bits)
		for (int line = 0; line < (TEX_HEIGHT/2); ++line)
		{
			for (int col = 0; col < TEX_WIDTH; ++col, ++currPixPtr)
			{
				*currPixPtr = g_sdlCGAGfxBGRAScreenColors[((*srcCGAPtr) >> currBitLoc) & 3];
				if (currBitLoc)
				{
					currBitLoc -= 2; // Check following bits in same CGA byte
				}
				else
				{
					currBitLoc = 6;
					// Check next byte
					if ((++srcCGAPtr) == (g_sdlCGAGfxMemory+sizeof(g_sdlCGAGfxMemory)))
					{
						srcCGAPtr = g_sdlCGAGfxMemory; // Wrap around
					}
				}
			}
			currPixPtr += TEX_WIDTH; // Skip "odd" row
		}
	}
	SDL_UnlockTexture(g_sdlTexture);
	SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionRect);
        SDL_RenderPresent(g_sdlRenderer);
}
