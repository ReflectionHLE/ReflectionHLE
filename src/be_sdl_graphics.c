/* Copyright (C) 2014 NY00123
 *
 * This file is part of Chocolate Keen Dreams.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "SDL.h"
#include "id_heads.h"

/*static*/ SDL_Window *g_sdlWindow;
static SDL_Renderer *g_sdlRenderer;
static SDL_Texture *g_sdlTexture;
static SDL_Rect g_sdlAspectCorrectionRect;

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
#define EGACGA_TXT_TEX_WIDTH 640
#define EGACGA_TXT_TEX_HEIGHT 200

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16


extern uint8_t g_vga_8x16TextFont[256*VGA_TXT_CHAR_PIX_WIDTH*VGA_TXT_CHAR_PIX_HEIGHT];

static uint8_t g_sdlCGAGfxMemory[8192*2], g_sdlCGAGfxMemoryCache[8192*2];
static uint8_t g_sdlTextModeMemory[TXT_COLS_NUM*TXT_ROWS_NUM*2], g_sdlTextModeMemoryCache[TXT_COLS_NUM*TXT_ROWS_NUM*2];
static uint16_t g_sdlScreenStartAddress = 0;
static int g_sdlScreenMode = 3;
static int g_sdlTexWidth, g_sdlTexHeight;
static uint8_t g_sdlBorderColor = 0;
static int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
static int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

void BE_SDL_SetAspectCorrectionRect(void);

void BE_SDL_InitGfx(void)
{
	char *envVar = getenv("CHOCOLATE_KEEN_FULLSCREEN");
	bool isFullScreen = (envVar && (*envVar == '1'));
	envVar = getenv("CHOCOLATE_KEEN_DISPLAYNUM");
	int displayNum = (envVar && (*envVar)) ? atoi(envVar) : 0;
	envVar = getenv("CHOCOLATE_KEEN_SOFTWARE_RENDERING");
	bool doSoftwareRendering = (envVar && (*envVar == '1'));
	const char *windowTitle = "Chocolate Keen Dreams";
	if (isFullScreen)
	{
		g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNum), 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{
		if (doSoftwareRendering)
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNum), 640, 480, SDL_WINDOW_RESIZABLE);
		}
		else
		{
			SDL_DisplayMode mode;
			SDL_GetDesktopDisplayMode(displayNum, &mode);
			if (3*mode.w < 4*mode.h) // Thinner than 4:3
			{
				mode.h = mode.w*3/4;
			}
			else  // As wide as 4:3 at the least
			{
				mode.w = mode.h*4/3;
			}
			// Just for the sake of it, using the golden ratio...
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayNum), mode.w*500/809, mode.h*500/809, SDL_WINDOW_RESIZABLE);
		}
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, -1, doSoftwareRendering ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE);
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		//Destroy window?
		exit(0);
	}
	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	BE_SDL_SetScreenMode(3); // Includes SDL_Texture handling
	BE_SDL_SetAspectCorrectionRect();
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

static void BEL_SDL_RecreateTexture(void)
{
	if (g_sdlTexture)
	{
		SDL_DestroyTexture(g_sdlTexture);
	}
	g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
	if (!g_sdlTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
}

void BE_SDL_SetAspectCorrectionRect(void)
{
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

void BE_SDL_SetScreenStartAddress(id0_unsigned_t crtc)
{
	g_sdlScreenStartAddress = crtc;
}

id0_byte_t *BE_SDL_GetTextModeMemoryPtr(void)
{
	return g_sdlTextModeMemory;
}

id0_byte_t *BE_SDL_GetCGAMemoryPtr(void)
{
	return g_sdlCGAGfxMemory;
}

id0_byte_t *BE_SDL_GetEGAMemoryPtr(void)
{
	// TODO: IMPLEMENT!
	return 0;
}

void BE_SDL_SetBorderColor(id0_byte_t color)
{
	g_sdlBorderColor = color;
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
		g_sdlTextModeMemoryCache[0] = g_sdlTextModeMemory[0]^0xFF; // Force refresh
		break;
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlCGAGfxMemory,  0, sizeof(g_sdlCGAGfxMemory));
		g_sdlCGAGfxMemoryCache[0] = g_sdlCGAGfxMemory[0]^0xFF; // Force refresh
		break;
	}
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
	uint8_t *currMemByte = g_sdlTextModeMemory;
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

void BE_SDL_Simplified_printf(const char *str)
{
	// TODO (CHOCO KEEN): Tabs?
	uint8_t *currMemByte = g_sdlTextModeMemory + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		if (*str == '\n')
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
			++g_sdlTxtCursorPosY; // Line feed
			currMemByte = g_sdlTextModeMemory + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
		}
		else
		{
			*(currMemByte++) = *str;
			*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			if (g_sdlTxtCursorPosX == TXT_COLS_NUM - 1)
			{
				g_sdlTxtCursorPosX = 0; // Carriage return
				++g_sdlTxtCursorPosY; // Line feed
				currMemByte = g_sdlTextModeMemory + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
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
			memmove(g_sdlTextModeMemory, g_sdlTextModeMemory+2*TXT_COLS_NUM, sizeof(g_sdlTextModeMemory)-2*TXT_COLS_NUM);
			currMemByte = g_sdlTextModeMemory+sizeof(g_sdlTextModeMemory)-2*TXT_COLS_NUM;
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

void BE_SDL_puts(const char *str)
{
	BE_SDL_Simplified_printf(str);
	BE_SDL_Simplified_printf("\n");
}

void BE_SDL_UpdateHostDisplay(void);

void BE_SDL_WaitVBL(id0_int_t number)
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



void BE_SDL_UpdateHostDisplay(void)
{
	if (g_sdlScreenMode == 3) // Textual
	{
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		// Maybe we don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		for (int i = 0; i < sizeof(g_sdlTextModeMemory); ++i)
		{
			if (g_sdlTextModeMemory[i] != g_sdlTextModeMemoryCache[i])
			{
				g_sdlTextModeMemoryCache[i] = g_sdlTextModeMemory[i];
				doUpdate = true;
			}
		}
		// But there are still blinking characters and cursor
		bool areBlinkingCharsShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
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
				currChar = g_sdlTextModeMemory[txtByteCounter];
				// Luckily, the width*height product is always divisible by 8...
				// Note that the actual width is always 8,
				// even in VGA mode. We convert to 9 while drawing.
				currCharFontPtr = g_vga_8x16TextFont + currChar*16*8;
				++txtByteCounter;
				currBackgroundColor = g_sdlEGABGRAScreenColors[(g_sdlTextModeMemory[txtByteCounter] >> 4) & 7];
				// Should the character blink?
				if (!(g_sdlTextModeMemory[txtByteCounter] & 0x80) || areBlinkingCharsShown)
					currCharColor = g_sdlEGABGRAScreenColors[g_sdlTextModeMemory[txtByteCounter] & 15];
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
		currCharColor = g_sdlEGABGRAScreenColors[g_sdlTextModeMemory[1+((TXT_COLS_NUM*g_sdlTxtCursorPosY+g_sdlTxtCursorPosX)<<1)] & 15];
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
	else // CGA graphics mode 4
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			return;
		}
		// Maybe we still don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		for (int i = 0; i < sizeof(g_sdlCGAGfxMemory); ++i)
		{
			if (g_sdlCGAGfxMemory[i] != g_sdlCGAGfxMemoryCache[i])
			{
				g_sdlCGAGfxMemoryCache[i] = g_sdlCGAGfxMemory[i];
				doUpdate = true;
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
			srcCGAPtr = &g_sdlCGAGfxMemory[g_sdlScreenStartAddress+i*0x2000];
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
						if ((++srcCGAPtr) == (g_sdlCGAGfxMemory+sizeof(g_sdlCGAGfxMemory)))
						{
							srcCGAPtr = g_sdlCGAGfxMemory; // Wrap around
						}
					}
				}
				currPixPtr += GFX_TEX_WIDTH; // Skip "odd" row
			}
		}
	}
	g_sdlDoRefreshGfxOutput = false;
	SDL_UnlockTexture(g_sdlTexture);
	SDL_RenderClear(g_sdlRenderer);
	SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionRect);
        SDL_RenderPresent(g_sdlRenderer);
}
