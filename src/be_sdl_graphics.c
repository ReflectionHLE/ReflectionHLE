#include "SDL.h"
#include "be_cross.h"
#include "be_sdl.h"

/*static*/ SDL_Window *g_sdlWindow;
static SDL_Renderer *g_sdlRenderer;
static SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
static SDL_Rect g_sdlAspectCorrectionRect, g_sdlAspectCorrectionBorderedRect;

static bool g_sdlDoRefreshGfxOutput;
static bool g_sdlDoPendingRefreshGfxOutput;

void BE_SDL_MarkGfxForUpdate(void)
{
	if (g_sdlDoPendingRefreshGfxOutput)
	{
		g_sdlDoPendingRefreshGfxOutput = false;
		g_sdlDoRefreshGfxOutput = true;
	}
}

void BE_SDL_MarkGfxForPendingUpdate(void)
{
	g_sdlDoPendingRefreshGfxOutput = true;
}
#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

#define GFX_TEX_WIDTH 320
#define GFX_TEX_HEIGHT 200
#define VGA_TXT_TEX_WIDTH 720
#define VGA_TXT_TEX_HEIGHT 400
//#define EGACGA_TXT_TEX_WIDTH 640
//#define EGACGA_TXT_TEX_HEIGHT 200

// Overscan border dimensions (for each side of the screen)
#define ENGINE_VGA_GFX_OVERSCAN_LEFT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_GFX_OVERSCAN_RIGHT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING 7    // 200-line doubling
#define ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING 7 // 200-line doubling
#define ENGINE_VGA_TXT_OVERSCAN_LEFT 9
#define ENGINE_VGA_TXT_OVERSCAN_RIGHT 9
#define ENGINE_VGA_TXT_OVERSCAN_TOP 7
#define ENGINE_VGA_TXT_OVERSCAN_BOTTOM 7

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16


extern uint8_t g_vga_8x16TextFont[256*VGA_TXT_CHAR_PIX_WIDTH*VGA_TXT_CHAR_PIX_HEIGHT];
// We can use a union because the memory contents are refreshed on screen mode change
static union {
	uint8_t egaGfx[4][0x10000];
	uint8_t cgaGfx[0x2000];
	uint8_t text[TXT_COLS_NUM*TXT_ROWS_NUM*2];
} g_sdlVidMem, g_sdlVidMemCache;

static uint16_t g_sdlScreenStartAddress = 0, g_sdlScreenStartAddressCache;
static int g_sdlScreenMode = 3;
static int g_sdlTexWidth, g_sdlTexHeight;
static uint8_t g_sdlPelPanning = 0, g_sdlPelPanningCache;
static uint8_t g_sdlLineWidth = 40, g_sdlLineWidthCache;
static int16_t g_sdlSplitScreenLine = -1, g_sdlSplitScreenLineCache;
static int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
static bool g_sdlTxtCursorEnabled = true;
static int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

void BE_SDL_SetGfxOutputRects(void);

void BE_SDL_InitGfx(void)
{
#ifdef CHOCO_KEEN_VER_KDREAMS
	const char *windowTitle = "Chocolate Keen Dreams";
#elif defined CHOCO_KEEN_VER_CATABYSS
	const char *windowTitle = "Chocolate Catacomb Abyss";
#else
#error "FATAL ERROR: No Chocolate port game macro is defined!"
#endif
	if (g_chocolateKeenCfg.isFullscreen)
	{
		if (g_chocolateKeenCfg.fullWidth && g_chocolateKeenCfg.fullHeight)
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), g_chocolateKeenCfg.fullWidth, g_chocolateKeenCfg.fullHeight, SDL_WINDOW_FULLSCREEN);
		}
		else
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}
	else
	{
		int actualWinWidth = g_chocolateKeenCfg.winWidth, actualWinHeight = g_chocolateKeenCfg.winHeight;
		if (!actualWinWidth || !actualWinHeight)
		{
			bool doSoftwareRendering;
			if (g_chocolateKeenCfg.sdlRendererDriver >= 0)
			{
				SDL_RendererInfo info;
				SDL_GetRenderDriverInfo(g_chocolateKeenCfg.sdlRendererDriver, &info);
				doSoftwareRendering = (info.flags & SDL_RENDERER_SOFTWARE);
			}
			else
			{
				doSoftwareRendering = false;
			}
			if (doSoftwareRendering)
			{
				actualWinWidth = 640;
				actualWinHeight = 480;
			}
			else
			{
				SDL_DisplayMode mode;
				SDL_GetDesktopDisplayMode(g_chocolateKeenCfg.displayNum, &mode);
				// In the 200-lines modes on the VGA, where line doubling is in effect,
				// and after adding the overscan borders, the aspect ratio for the whole output
				// (after aspect correction i.e., multiplying height by 1.2) is 280:207.
				if (207*mode.w < 280*mode.h) // Thinner than 280:207
				{
					mode.h = mode.w*207/280;
				}
				else  // As wide as 280:207 at the least
				{
					mode.w = mode.h*280/207;
				}
				// Just for the sake of it, using the golden ratio...
				actualWinWidth = mode.w*500/809;
				actualWinHeight = mode.h*500/809;
			}
		}
		g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), actualWinWidth, actualWinHeight, SDL_WINDOW_RESIZABLE);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}
#if GRMODE == EGAGR
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_chocolateKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_chocolateKeenCfg.vSync == VSYNC_OFF) ? 0 : SDL_RENDERER_PRESENTVSYNC));
#elif GRMODE == CGAGR
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_chocolateKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_chocolateKeenCfg.vSync == VSYNC_ON) ? SDL_RENDERER_PRESENTVSYNC : 0));
#else
#error "Supported GRMODE not defined!"
#endif
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		//Destroy window?
		exit(0);
	}
	BE_SDL_SetScreenMode(3); // Includes SDL_Texture handling and output rects preparation
}

void BE_SDL_ShutdownGfx(void)
{
	SDL_DestroyTexture(g_sdlTexture);
	g_sdlTexture = NULL;
	SDL_DestroyTexture(g_sdlTargetTexture);
	g_sdlTargetTexture = NULL;
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
}

static void BEL_SDL_RecreateTexture(void)
{
	if (g_sdlTexture)
	{
		SDL_DestroyTexture(g_sdlTexture);
	}
	if (g_sdlTargetTexture)
	{
		SDL_DestroyTexture(g_sdlTargetTexture);
	}
	// Try using render target
	if ((g_chocolateKeenCfg.scaleFactor > 1) && g_chocolateKeenCfg.isBilinear)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
		// Try, if we fail then simply don't use this
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		g_sdlTargetTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, g_sdlTexWidth*g_chocolateKeenCfg.scaleFactor, g_sdlTexHeight*g_chocolateKeenCfg.scaleFactor);
	}
	else
	{
		// Use just a single texture
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_chocolateKeenCfg.isBilinear ? "linear" : "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
	}
}

void BE_SDL_SetGfxOutputRects(void)
{
	int srcWidth = g_sdlTexWidth;
	int srcHeight = g_sdlTexHeight;
	if (g_sdlScreenMode != 3)
	{
		// On the VGA, line doubling is in effect for graphical 200 lines modes
		srcWidth *= 2;
		srcHeight *= 2;
	}
	int srcBorderLeft, srcBorderRight, srcBorderTop, srcBorderBottom;
	if (g_sdlScreenMode == 3)
	{
		srcBorderLeft = ENGINE_VGA_TXT_OVERSCAN_LEFT;
		srcBorderRight = ENGINE_VGA_TXT_OVERSCAN_RIGHT;
		srcBorderTop = ENGINE_VGA_TXT_OVERSCAN_TOP;
		srcBorderBottom = ENGINE_VGA_TXT_OVERSCAN_BOTTOM;
	}
	else
	{
		srcBorderLeft = ENGINE_VGA_GFX_OVERSCAN_LEFT_AFTER_DOUBLING;
		srcBorderRight = ENGINE_VGA_GFX_OVERSCAN_RIGHT_AFTER_DOUBLING;
		srcBorderTop = ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING;
		srcBorderBottom = ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING;
	}
	int srcBorderedWidth = srcBorderLeft+srcWidth+srcBorderRight;
	int srcBorderedHeight = srcBorderTop+srcHeight+srcBorderBottom;
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
	if (g_chocolateKeenCfg.scaleType == SCALE_FILL)
	{
		g_sdlAspectCorrectionBorderedRect.w = winWidth;
		g_sdlAspectCorrectionBorderedRect.h = winHeight;
		g_sdlAspectCorrectionBorderedRect.x = 0;
		g_sdlAspectCorrectionBorderedRect.y = 0;
	}
	else
	{
		// Aspect correct - Includes overscan borders, should multiply
		// the height by 1.2 first (or the width by 5 and height 6)
		int scaledSrcBorderedWidth = 5*srcBorderedWidth, scaledSrcBorderedHeight = 6*srcBorderedHeight;
		if (scaledSrcBorderedHeight*winWidth < scaledSrcBorderedWidth*winHeight) // Thinner than scaledSrcBorderedWidth:scaledSrcBorderedHeight
		{
			g_sdlAspectCorrectionBorderedRect.w = winWidth;
			g_sdlAspectCorrectionBorderedRect.h = winWidth*scaledSrcBorderedHeight/scaledSrcBorderedWidth;
			g_sdlAspectCorrectionBorderedRect.x = 0;
			g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
		}
		else // As wide as scaledSrcBorderedWidth:scaledSrcBorderedHeight at the least
		{
			g_sdlAspectCorrectionBorderedRect.w = winHeight*scaledSrcBorderedWidth/scaledSrcBorderedHeight;
			g_sdlAspectCorrectionBorderedRect.h = winHeight;
			g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
			g_sdlAspectCorrectionBorderedRect.y = 0;
		}
	}
	// Finish with internal (non-bordered) rect
	g_sdlAspectCorrectionRect.x = g_sdlAspectCorrectionBorderedRect.x + g_sdlAspectCorrectionBorderedRect.w*srcBorderLeft/srcBorderedWidth;
	g_sdlAspectCorrectionRect.y = g_sdlAspectCorrectionBorderedRect.y + g_sdlAspectCorrectionBorderedRect.h*srcBorderTop/srcBorderedHeight;
	g_sdlAspectCorrectionRect.w = g_sdlAspectCorrectionBorderedRect.w*srcWidth/srcBorderedWidth;
	g_sdlAspectCorrectionRect.h = g_sdlAspectCorrectionBorderedRect.h*srcHeight/srcBorderedHeight;
}

void BE_SDL_SetScreenStartAddress(uint16_t crtc)
{
	g_sdlScreenStartAddress = crtc;
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t *BE_SDL_GetTextModeMemoryPtr(void)
{
	return g_sdlVidMem.text;
}

uint8_t *BE_SDL_GetCGAMemoryPtr(void)
{
	return g_sdlVidMem.cgaGfx;
}



// Colors in BGRA format/order (on certain platforms)

static const uint32_t g_sdlCGAGfxBGRAScreenColors[] = {
	0xff000000/*black*/,
	0xff00ffff/*light cyan*/,
	0xffff00ff/*light magenta*/,
	0xffffffff/*white*/
};

// Same but for the EGA/VGA (and colored text modes on CGA/EGA/VGA)

static const uint32_t g_sdlEGABGRAScreenColors[] = {
	0xff000000/*black*/, 0xff0000aa/*blue*/, 0xff00aa00/*green*/, 0xff00aaaa/*cyan*/,
	0xffaa0000/*red*/, 0xffaa00aa/*magenta*/, 0xffaa5500/*brown*/, 0xffaaaaaa/*light gray*/,
	0xff555555/*gray*/, 0xff5555ff/*light blue*/, 0xff55ff55/*light green*/, 0xff55ffff/*light cyan*/,
	0xffff5555/*light red*/, 0xffff55ff/*light magenta*/, 0xffffff55/*yellow*/, 0xffffffff/*white*/
};

static uint32_t g_sdlEGACurrBGRAPaletteAndBorder[17], g_sdlEGACurrBGRAPaletteAndBorderCache[17];

/* Gets a value represeting 6 EGA signals determining a color number and
 * returns it in a "Blue Green Red Intensity" 4-bit format.
 * Usually, the 6 signals represented by the given input mean:
 * "Blue Green Red Secondary-Blue Secondary-Green Secondary-Red". However, for
 * the historical reason of compatibility with CGA monitors, on the 200-lines
 * modes used by Keen the Secondary-Green signal is treated as an Intensity
 * one and the two other intensity signals are ignored.
 */
static int BEL_SDL_ConvertEGASignalToEGAEntry(int color)
{
	return (color & 7) | ((color & 16) >> 1);
}


void BE_SDL_SetBorderColor(uint8_t color)
{
	g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[BEL_SDL_ConvertEGASignalToEGAEntry(color)];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetPaletteAndBorder(const uint8_t *palette)
{
	for (int entry = 0; entry < 16; ++entry)
	{
		g_sdlEGACurrBGRAPaletteAndBorder[entry] =  g_sdlEGABGRAScreenColors[BEL_SDL_ConvertEGASignalToEGAEntry(palette[entry])];
	}
	g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[BEL_SDL_ConvertEGASignalToEGAEntry(palette[16])];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_SetPelPanning(uint8_t panning)
{
	g_sdlPelPanning = panning;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetLineWidth(uint8_t widthInBytes)
{
	g_sdlLineWidth = widthInBytes;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetSplitScreen(int16_t linenum)
{
	// TODO (CHOCO KEEN) No idea why, required for VGA (not EGA)
	g_sdlSplitScreenLine = (linenum+1)/2;
	//g_sdlSplitScreenLine = linenum;
}

void BE_SDL_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask)
{
	if (mask & 1)
		g_sdlVidMem.egaGfx[0][destOff] = srcVal;
	if (mask & 2)
		g_sdlVidMem.egaGfx[1][destOff] = srcVal;
	if (mask & 4)
		g_sdlVidMem.egaGfx[2][destOff] = srcVal;
	if (mask & 8)
		g_sdlVidMem.egaGfx[3][destOff] = srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_SDL_EGAUpdateGFXByte but picking specific bits out of each byte, and WITHOUT planes mask
void BE_SDL_EGAUpdateGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_LinearToWrapped_MemCopy
static void BEL_SDL_LinearToEGAPlane_MemCopy(uint8_t *planeDstPtr, uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		memcpy(planeDstPtr+planeDstOff, linearSrc, num);
	}
	else
	{
		memcpy(planeDstPtr+planeDstOff, linearSrc, bytesToEnd);
		memcpy(planeDstPtr, linearSrc+bytesToEnd, num-bytesToEnd);
	}
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToLinear_MemCopy
static void BEL_SDL_EGAPlaneToLinear_MemCopy(uint8_t *linearDst, const uint8_t *planeSrcPtr, uint16_t planeSrcOff, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeSrcOff;
	if (num <= bytesToEnd)
	{
		memcpy(linearDst, planeSrcPtr+planeSrcOff, num);
	}
	else
	{
		memcpy(linearDst, planeSrcPtr+planeSrcOff, bytesToEnd);
		memcpy(linearDst+bytesToEnd, planeSrcPtr, num-bytesToEnd);
	}
	//No need to since we just read screen data
	//g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToWrapped_MemCopy
static void BEL_SDL_EGAPlaneToEGAPlane_MemCopy(uint8_t *planeCommonPtr, uint16_t planeDstOff, uint16_t planeSrcOff, uint16_t num)
{
	uint16_t srcBytesToEnd = 0x10000-planeSrcOff;
	uint16_t dstBytesToEnd = 0x10000-planeDstOff;
	if (num <= srcBytesToEnd)
	{
		// Source is linear: Same as BE_Cross_LinearToWrapped_MemCopy here
		if (num <= dstBytesToEnd)
		{
			memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, num);
		}
		else
		{
			memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, dstBytesToEnd);
			memcpy(planeCommonPtr, planeCommonPtr+planeSrcOff+dstBytesToEnd, num-dstBytesToEnd);
		}
	}
	// Otherwise, check if at least the destination is linear
	else if (num <= dstBytesToEnd)
	{
		// Destination is linear: Same as BE_Cross_WrappedToLinear_MemCopy, non-linear source
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, srcBytesToEnd);
		memcpy(planeCommonPtr+planeDstOff+srcBytesToEnd, planeCommonPtr, num-srcBytesToEnd);
	}
	// BOTH buffers have wrapping. We don't check separately if
	// srcBytesToEnd==dstBytesToEnd (in such a case planeDstOff==planeSrcOff...)
	else if (srcBytesToEnd <= dstBytesToEnd)
	{
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, srcBytesToEnd);
		memcpy(planeCommonPtr+planeDstOff+srcBytesToEnd, planeCommonPtr, dstBytesToEnd-srcBytesToEnd);
		memcpy(planeCommonPtr, planeCommonPtr+(dstBytesToEnd-srcBytesToEnd), num-dstBytesToEnd);
	}
	else // srcBytesToEnd > dstBytesToEnd
	{
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, dstBytesToEnd);
		memcpy(planeCommonPtr, planeCommonPtr+planeSrcOff+dstBytesToEnd, srcBytesToEnd-dstBytesToEnd);
		memcpy(planeCommonPtr+(srcBytesToEnd-dstBytesToEnd), planeCommonPtr, num-srcBytesToEnd);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXBuffer(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t mask)
{
	if (mask & 1)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[0], destOff, srcPtr, num);
	if (mask & 2)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[1], destOff, srcPtr, num);
	if (mask & 4)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[2], destOff, srcPtr, num);
	if (mask & 8)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[3], destOff, srcPtr, num);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff)
{
	g_sdlVidMem.egaGfx[0][destOff] = g_sdlVidMem.egaGfx[0][srcOff];
	g_sdlVidMem.egaGfx[1][destOff] = g_sdlVidMem.egaGfx[1][srcOff];
	g_sdlVidMem.egaGfx[2][destOff] = g_sdlVidMem.egaGfx[2][srcOff];
	g_sdlVidMem.egaGfx[3][destOff] = g_sdlVidMem.egaGfx[3][srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_SDL_EGAUpdateGFXByteScrToScr but picking specific bits out of each byte
void BE_SDL_EGAUpdateGFXBitsScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[0][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[1][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[2][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[3][srcOff] & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[0], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[1], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[2], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[3], destOff, srcOff, num);
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t BE_SDL_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum)
{
	return g_sdlVidMem.egaGfx[planenum][destOff];
}

void BE_SDL_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum)
{
	BEL_SDL_EGAPlaneToLinear_MemCopy(destPtr, g_sdlVidMem.egaGfx[planenum], srcOff, num);
}

void BE_SDL_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint16_t mask)
{
	for (int currBitNum = 0, currBitMask = 1; currBitNum < 8; ++currBitNum, currBitMask <<= 1)
	{
		if (mask & currBitMask)
		{
			g_sdlVidMem.egaGfx[0][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[0][destOff] |= ((color & 1) << currBitNum);
			g_sdlVidMem.egaGfx[1][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[1][destOff] |= (((color & 2) >> 1) << currBitNum);
			g_sdlVidMem.egaGfx[2][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[2][destOff] |= (((color & 4) >> 2) << currBitNum);
			g_sdlVidMem.egaGfx[3][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[3][destOff] |= (((color & 8) >> 3) << currBitNum);
		}
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXPixel4bppRepeatedly(uint16_t destOff, uint8_t color, uint16_t count, uint16_t mask)
{
	for (uint16_t loopVar = 0; loopVar < count; ++loopVar, ++destOff)
	{
		BE_SDL_EGAUpdateGFXPixel4bpp(destOff, color, mask);
	}
}

void BE_SDL_EGAXorGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask)
{
	if (mask & 1)
		g_sdlVidMem.egaGfx[0][destOff] ^= srcVal;
	if (mask & 2)
		g_sdlVidMem.egaGfx[1][destOff] ^= srcVal;
	if (mask & 4)
		g_sdlVidMem.egaGfx[2][destOff] ^= srcVal;
	if (mask & 8)
		g_sdlVidMem.egaGfx[3][destOff] ^= srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Like BE_SDL_EGAXorGFXByte, but:
// - OR instead of XOR.
// - Only specific bits are updated in each plane's byte.
void BE_SDL_EGAOrGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] |= (srcVal & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}


void BE_SDL_SetScreenMode(int mode)
{
	g_sdlScreenMode = mode;
	g_sdlDoRefreshGfxOutput = true;
	switch (mode)
	{
	case 3:
		g_sdlTexWidth = VGA_TXT_TEX_WIDTH;
		g_sdlTexHeight = VGA_TXT_TEX_HEIGHT;
		g_sdlTxtColor = 7;
		g_sdlTxtBackground = 0;
		g_sdlTxtCursorPosX = g_sdlTxtCursorPosY = 0;
		BE_SDL_clrscr();
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlVidMemCache.text[0] = g_sdlVidMem.text[0]^0xFF; // Force refresh
		break;
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlVidMem.cgaGfx,  0, sizeof(g_sdlVidMem.cgaGfx));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlVidMemCache.cgaGfx[0] = g_sdlVidMem.cgaGfx[0]^0xFF; // Force refresh
		break;
	case 0xD:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPaletteAndBorder, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlPelPanning = 0;
		g_sdlLineWidth = 40;
		g_sdlSplitScreenLine = -1;
		memset(g_sdlVidMem.egaGfx,  0, sizeof(g_sdlVidMem.egaGfx));
		g_sdlVidMemCache.egaGfx[0][0] = g_sdlVidMem.egaGfx[0][0]^0xFF; // Force refresh
		break;
	}
	BE_SDL_SetGfxOutputRects();
	BEL_SDL_RecreateTexture();
}

void BE_SDL_textcolor(int color)
{
	g_sdlTxtColor = color;
}

void BE_SDL_textbackground(int color)
{
	g_sdlTxtBackground = color;
}

void BE_SDL_clrscr(void)
{
	uint8_t *currMemByte = g_sdlVidMem.text;
	for (int i = 0; i < 2*TXT_COLS_NUM*TXT_ROWS_NUM; ++i)
	{
		*(currMemByte++) = ' ';
		*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
	}
}

void BE_SDL_MoveTextCursorTo(int x, int y)
{
	g_sdlTxtCursorPosX = x;
	g_sdlTxtCursorPosY = y;
}

void BE_SDL_ToggleTextCursor(bool isEnabled)
{
	g_sdlTxtCursorEnabled = isEnabled;
}

static void BEL_SDL_Simplified_putsorprintf(const char *str, bool isprintfcall)
{
	// TODO (CHOCO KEEN): Tabs?
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		if (*str == '\n')
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
			++g_sdlTxtCursorPosY; // Line feed
			currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
		}
		else
		{
			if ((*str == '%') && isprintfcall)
			{
				++str; // This is still a SIMPLIFIED printf...
			}
			*(currMemByte++) = *str;
			*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			if (g_sdlTxtCursorPosX == TXT_COLS_NUM - 1)
			{
				g_sdlTxtCursorPosX = 0; // Carriage return
				++g_sdlTxtCursorPosY; // Line feed
				currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
			}
			else
			{
				++g_sdlTxtCursorPosX;
			}
		}
		if (g_sdlTxtCursorPosY == TXT_ROWS_NUM)
		{
			--g_sdlTxtCursorPosY;
			// Scroll one line down
			memmove(g_sdlVidMem.text, g_sdlVidMem.text+2*TXT_COLS_NUM, sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM);
			currMemByte = g_sdlVidMem.text+sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM;
			// New empty line
			for (int i = 0; i < TXT_COLS_NUM; ++i)
			{
				*(currMemByte++) = ' ';
				*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			}
			currMemByte -= 2*TXT_COLS_NUM; // Go back to beginning of line
		}
	}
}

void BE_SDL_Simplified_printf(const char *str)
{
	BEL_SDL_Simplified_putsorprintf(str, true);
}

void BE_SDL_puts(const char *str)
{
	BEL_SDL_Simplified_putsorprintf(str, false);
	BEL_SDL_Simplified_putsorprintf("\n", false);
}

void BE_SDL_UpdateHostDisplay(void);

void BE_SDL_WaitVBL(int16_t number)
{
	// TODO (CHOCO KEEN) Make a difference based on HW?
	Uint32 startTicks = SDL_GetTicks();
	if (SDL_GetTicks() - startTicks < (int)number*1000000/70086)
	{
		SDL_Delay(1);
		BE_SDL_UpdateHostDisplay();
		BE_SDL_PollEvents();
	}
	while (SDL_GetTicks() - startTicks < (int)number*1000000/70086)
	{
		SDL_Delay(1);
		BE_SDL_PollEvents();
	}
}

// Call during a busy loop of some unknown duration (e.g., waiting for key press/release)
void BE_SDL_ShortSleep(void)
{
	SDL_Delay(1);
	// TODO: Make this more efficient
	BE_SDL_UpdateHostDisplay();
	BE_SDL_PollEvents();
}

// Use this ONLY in Catacombs' CalcTics (from ThreeDRefresh)
BE_SDL_ThreeDRefreshSleep(void)
{
	SDL_Delay(15);
	// TODO: Make this more efficient?
	BE_SDL_UpdateHostDisplay();
	BE_SDL_PollEvents();
}

void BE_SDL_Delay(uint16_t msec) // Replacement for delay from dos.h
{
	uint32_t endTime = SDL_GetTicks() + msec;
	while ((int32_t)(SDL_GetTicks() - endTime) < 0)
	{
		BE_SDL_ShortSleep();
	}
}




void BE_SDL_UpdateHostDisplay(void)
{
	if (g_sdlScreenMode == 3) // Text mode
	{
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		// Maybe we don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		for (int i = 0; i < sizeof(g_sdlVidMem.text); ++i)
		{
			if (g_sdlVidMem.text[i] != g_sdlVidMemCache.text[i])
			{
				g_sdlVidMemCache.text[i] = g_sdlVidMem.text[i];
				doUpdate = true;
			}
		}
		// But there are still blinking characters and cursor
		bool areBlinkingCharsShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = g_sdlTxtCursorEnabled && (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
		// We check g_sdlDoRefreshGfxOutput since this is currently set to "true" only after setting video mode (for text mode)
		if (!g_sdlDoRefreshGfxOutput && !doUpdate && (wereBlinkingCharsShown == areBlinkingCharsShown) && (wasBlinkingCursorShown == isBlinkingCursorShown))
		{
			return;
		}
		/****** Do update ******/
		wereBlinkingCharsShown = areBlinkingCharsShown;
		wasBlinkingCursorShown = isBlinkingCursorShown;
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *screenPixelPtr = (uint32_t *)pixels;
		uint8_t currChar;
		const uint8_t *currCharFontPtr;
		uint32_t *currScrPixelPtr, currBackgroundColor, currCharColor;
		int txtByteCounter = 0;
		int currCharPixX, currCharPixY;
		for (int currCharY = 0, currCharX; currCharY < TXT_ROWS_NUM; ++currCharY)
		{
			// Draw striped lines
			for (currCharX = 0; currCharX < TXT_COLS_NUM; ++currCharX)
			{
				currChar = g_sdlVidMem.text[txtByteCounter];
				// Luckily, the width*height product is always divisible by 8...
				// Note that the actual width is always 8,
				// even in VGA mode. We convert to 9 while drawing.
				currCharFontPtr = g_vga_8x16TextFont + currChar*16*8;
				++txtByteCounter;
				currBackgroundColor = g_sdlEGABGRAScreenColors[(g_sdlVidMem.text[txtByteCounter] >> 4) & 7];
				// Should the character blink?
				if (!(g_sdlVidMem.text[txtByteCounter] & 0x80) || areBlinkingCharsShown)
					currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[txtByteCounter] & 15];
				else
					currCharColor = currBackgroundColor;
				++txtByteCounter;
				currScrPixelPtr = screenPixelPtr;
				for (currCharPixY = 0; currCharPixY < VGA_TXT_CHAR_PIX_HEIGHT; ++currCharPixY)
				{
					/* NOTE: The char width is actually 8
					in both of the EGA and VGA fonts. On the
					VGA case, the 9th pixel is determined
					according to the 8th and char number. */
					for (currCharPixX = 0; currCharPixX < 8; ++currCharPixX, ++currCharFontPtr, ++currScrPixelPtr)
					{
						*currScrPixelPtr = (*currCharFontPtr) ? currCharColor : currBackgroundColor;
					}
					// Add an extra 9th column on VGA
					*currScrPixelPtr = ((currChar < 192) || (currChar > 223)) ? currBackgroundColor : *(currScrPixelPtr-1);
					currScrPixelPtr += (g_sdlTexWidth-VGA_TXT_CHAR_PIX_WIDTH+1);
				}
				screenPixelPtr += VGA_TXT_CHAR_PIX_WIDTH;
			}
			// Go to the character right below current one
			screenPixelPtr += g_sdlTexWidth*(VGA_TXT_CHAR_PIX_HEIGHT-1);
		}
		// Finish with outputting the cursor if required
		currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[1+((TXT_COLS_NUM*g_sdlTxtCursorPosY+g_sdlTxtCursorPosX)<<1)] & 15];
		if (isBlinkingCursorShown)
		{
			screenPixelPtr = (uint32_t *)pixels+g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosY*VGA_TXT_CHAR_PIX_HEIGHT*g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosX*VGA_TXT_CHAR_PIX_WIDTH;
			// Out of 3 last scanlines of char, draw to the first 2.
			screenPixelPtr += (VGA_TXT_CHAR_PIX_HEIGHT-3)*g_sdlTexWidth;
			for (currCharPixY = 0; currCharPixY < 2; currCharPixY++)
			{
				for (currCharPixX = 0; currCharPixX < VGA_TXT_CHAR_PIX_WIDTH; currCharPixX++, screenPixelPtr++)
					*screenPixelPtr = currCharColor;
				screenPixelPtr += g_sdlTexWidth - VGA_TXT_CHAR_PIX_WIDTH;
			}
		}
	}
	else if (g_sdlScreenMode == 4) // CGA graphics
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			return;
		}
		// Maybe we still don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		// This is never modified in the CGA code of Keen Dreams, even though there is a CGA implementation of VW_SetScreen modifying this
#if 0
		if (g_sdlScreenStartAddress != g_sdlScreenStartAddressCache)
		{
			g_sdlScreenStartAddressCache = g_sdlScreenStartAddress;
			doUpdate = true;
		}
#endif
		if (g_sdlEGACurrBGRAPaletteAndBorder[16] != g_sdlEGACurrBGRAPaletteAndBorderCache[16])
		{
			g_sdlEGACurrBGRAPaletteAndBorderCache[16] = g_sdlEGACurrBGRAPaletteAndBorder[16];
			doUpdate = true;
		}

		if (doUpdate) // We already know we should refresh, so just copy
		{
			memcpy(g_sdlVidMemCache.cgaGfx, g_sdlVidMem.cgaGfx, sizeof(g_sdlVidMem.cgaGfx));
		}
		else // Otherwise we should still check
		{
			for (int i = 0; i < sizeof(g_sdlVidMem.cgaGfx); ++i)
			{
				if (g_sdlVidMem.cgaGfx[i] != g_sdlVidMemCache.cgaGfx[i])
				{
					g_sdlVidMemCache.cgaGfx[i] = g_sdlVidMem.cgaGfx[i];
					doUpdate = true;
				}
			}
		}

		if (!doUpdate)
		{
			return;
		}
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr;
		uint8_t *srcCGAPtr;
		uint8_t currBitLoc; // Location of bits to check inside byte (two bits)
		// First fill the "even" rows, then the "odd" ones
		for (int i = 0; i < 2; ++i)
		{
			currPixPtr = (uint32_t *)pixels + i*GFX_TEX_WIDTH;
			// g_sdlScreenStartAddress is never modified in the CGA code of Keen Dreams, even though there is a CGA implementation of VW_SetScreen modifying this
			srcCGAPtr = &g_sdlVidMem.cgaGfx[i*0x2000];
			//srcCGAPtr = &g_sdlVidMem.cgaGfx[g_sdlScreenStartAddress+i*0x2000];
			currBitLoc = 6; // Location of bits to check (two bits)
			for (int line = 0; line < (GFX_TEX_HEIGHT/2); ++line)
			{
				for (int col = 0; col < GFX_TEX_WIDTH; ++col, ++currPixPtr)
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
						if ((++srcCGAPtr) == (g_sdlVidMem.cgaGfx+sizeof(g_sdlVidMem.cgaGfx)))
						{
							srcCGAPtr = g_sdlVidMem.cgaGfx; // Wrap around
						}
					}
				}
				currPixPtr += GFX_TEX_WIDTH; // Skip "odd" row
			}
		}
	}
	else // EGA graphics mode 0xD
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			return;
		}
		// Caching of graphics seems to be a bit expensive here
		// (more than the CGA case), so not doing that for now
#if 0
		// Maybe we still don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		if (g_sdlScreenStartAddress != g_sdlScreenStartAddressCache)
		{
			g_sdlScreenStartAddressCache = g_sdlScreenStartAddress;
			doUpdate = true;
		}
		if (g_sdlPelPanning != g_sdlPelPanningCache)
		{
			g_sdlPelPanningCache = g_sdlPelPanning;
			doUpdate = true;
		}
		if (g_sdlLineWidth != g_sdlLineWidthCache)
		{
			g_sdlLineWidthCache = g_sdlLineWidth;
			doUpdate = true;
		}
		if (g_sdlSplitScreenLine != g_sdlSplitScreenLineCache)
		{
			g_sdlSplitScreenLineCache = g_sdlSplitScreenLine;
		}

		if (doUpdate) // We already know we should refresh, so just copy
		{
			memcpy(g_sdlEGACurrBGRAPaletteAndBorderCache, g_sdlEGACurrBGRAPaletteAndBorder, sizeof(g_sdlEGACurrBGRAPaletteAndBorder));
		}
		else // Otherwise we should still check
		{
			for (int paletteAndBorderEntry = 0; paletteAndBorderEntry < 17; ++paletteAndBorderEntry)
			{
				if (g_sdlEGACurrBGRAPaletteAndBorder[paletteAndBorderEntry] != g_sdlEGACurrBGRAPaletteAndBorderCache[paletteAndBorderEntry])
				{
					g_sdlEGACurrBGRAPaletteAndBorderCache[paletteAndBorderEntry] = g_sdlEGACurrBGRAPaletteAndBorder[paletteAndBorderEntry];
					doUpdate = true;
				}
			}
		}

		if (doUpdate) // We already know we should refresh, so just copy
		{
			memcpy(g_sdlVidMemCache.egaGfx, g_sdlVidMem.egaGfx, sizeof(g_sdlVidMem.egaGfx));
		}
		else // Otherwise we should still check
		{

			for (int planeNum = 0; planeNum < 4; ++planeNum)
			{
				for (int i = 0; i < sizeof(g_sdlVidMem.egaGfx[planeNum]); ++i)
				{
					if (g_sdlVidMem.egaGfx[planeNum][i] != g_sdlVidMemCache.egaGfx[planeNum][i])
					{
						g_sdlVidMemCache.egaGfx[planeNum][i] = g_sdlVidMem.egaGfx[planeNum][i];
						doUpdate = true;
					}
				}
			}
		}

		if (!doUpdate)
		{
			return;
		}
#endif
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint16_t currLineFirstByte = (g_sdlScreenStartAddress + g_sdlPelPanning/8) % 0x10000;
		uint8_t panningWithinInByte = g_sdlPelPanning%8;
		for (int line = 0, col; line < GFX_TEX_HEIGHT; ++line)
		{
			uint8_t currBitNum = 7-panningWithinInByte, currBitMask = 1<<currBitNum;
			uint16_t currByte = currLineFirstByte;
			uint32_t *currPixPtr = (uint32_t *)pixels + line*GFX_TEX_WIDTH;
			for (col = 0; col < GFX_TEX_WIDTH; ++col, ++currPixPtr)
			{
				*currPixPtr = g_sdlEGACurrBGRAPaletteAndBorder[
					((g_sdlVidMem.egaGfx[0][currByte]&currBitMask)>>currBitNum) |
					(((g_sdlVidMem.egaGfx[1][currByte]&currBitMask)>>currBitNum)<<1) |
					(((g_sdlVidMem.egaGfx[2][currByte]&currBitMask)>>currBitNum)<<2) |
					(((g_sdlVidMem.egaGfx[3][currByte]&currBitMask)>>currBitNum)<<3)
				];
				if (currBitNum == 0)
				{
					++currByte;
					currByte %= 0x10000;
					currBitNum = 7;
					currBitMask = 0x80;
				}
				else
				{
					--currBitNum;
					currBitMask >>= 1;
				}
				if (col == 8*g_sdlLineWidth)
				{
					++col;
					++currPixPtr;
					break;
				}
			}
			// Just if this makes sense... (FIXME: Check!)
			for (; col < GFX_TEX_WIDTH; ++col, ++currPixPtr)
			{
				*currPixPtr = g_sdlEGACurrBGRAPaletteAndBorder[0];
			}
			if (g_sdlSplitScreenLine == line)
			{
				currLineFirstByte = 0; // NEXT line begins split screen, NOT g_sdlSplitScreenLine
			}
			else
			{
				currLineFirstByte += g_sdlLineWidth;
				currLineFirstByte %= 0x10000;
			}
		}
	}
	g_sdlDoRefreshGfxOutput = false;
	SDL_UnlockTexture(g_sdlTexture);
	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_sdlRenderer);
	SDL_SetRenderDrawColor(g_sdlRenderer, (g_sdlEGACurrBGRAPaletteAndBorder[16]>>16)&0xFF, (g_sdlEGACurrBGRAPaletteAndBorder[16]>>8)&0xFF, g_sdlEGACurrBGRAPaletteAndBorder[16]&0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(g_sdlRenderer, &g_sdlAspectCorrectionBorderedRect);
	if (g_sdlTargetTexture)
	{
		SDL_SetRenderTarget(g_sdlRenderer, g_sdlTargetTexture);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, NULL);
		SDL_SetRenderTarget(g_sdlRenderer, NULL);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionRect);
	}
	else
	{
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionRect);
	}
        SDL_RenderPresent(g_sdlRenderer);
}
