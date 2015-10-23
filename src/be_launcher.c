/* Copyright (C) 2015 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>

#include "refkeen.h"

#define BE_MENU_CHAR_WIDTH 8
#define BE_MENU_CHAR_HEIGHT 8

#define BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING 2
#define BE_MENU_ITEM_MIN_TEXT_LINE_PIX_SPACING 4
#define BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT 32

#define BE_MENU_TITLE_PIX_YPOS (1+(BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT-BE_MENU_CHAR_HEIGHT)/2)

#define BE_MENU_FIRST_ITEM_PIX_YPOS (1+BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT)

#define BE_MENU_BACKBUTTON_PIX_WIDTH BE_MENU_FIRST_ITEM_PIX_YPOS

#define BE_MENU_STATIC_TEXT_MAX_ROW_STRLEN ((BE_LAUNCHER_PIX_WIDTH-2*(BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING+1))/BE_MENU_CHAR_WIDTH)

#define BE_LAUNCHER_SELECTION_LABEL_PIX_XPOS_UPPERBOUND (2*(BE_LAUNCHER_PIX_WIDTH-1)/3-2*BE_MENU_CHAR_WIDTH)
#define BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS (BE_LAUNCHER_SELECTION_LABEL_PIX_XPOS_UPPERBOUND+BE_MENU_CHAR_WIDTH)
#define BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS (BE_LAUNCHER_PIX_WIDTH-1-BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING-BE_MENU_CHAR_WIDTH)

extern const uint8_t g_cga_8x8TextFont[256*8*8];

static uint8_t *g_be_launcher_screenPtr;

static BEMenu *g_be_launcher_currMenu;
static BEMenuItem **g_be_launcher_selectedMenuItemPtr;

bool g_be_launcher_wasAnySettingChanged;

static int BEL_Launcher_PrepareMenuItem(BEMenuItem *menuItem, int yPos)
{
	char error[160];

	const int xPosStart = BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING + 1;
	int lastYPos;
	int xPosUpperBound = ((menuItem->type == BE_MENUITEM_TYPE_SELECTION) || (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER) || (menuItem->type == BE_MENUITEM_TYPE_DYNAMIC_SELECTION)) ? BE_LAUNCHER_SELECTION_LABEL_PIX_XPOS_UPPERBOUND : (BE_LAUNCHER_PIX_WIDTH-1-BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING);
	int xPos = xPosStart;
	int noOfLabelLines = 1;

	// A minor calculation and verification
	if ((menuItem->type == BE_MENUITEM_TYPE_SELECTION) || (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER) || (menuItem->type == BE_MENUITEM_TYPE_DYNAMIC_SELECTION))
	{
		const char **choicePtr;
		for (choicePtr = menuItem->choices; *choicePtr; ++choicePtr)
		{
			if (strlen(*choicePtr) >= (BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS-BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS)/BE_MENU_CHAR_WIDTH-1)
			{
				snprintf(error, sizeof(error), "BE_Launcher_PrepareMenuItem: Too long selection choice!\n%s", *choicePtr);
				BE_ST_ExitWithErrorMsg(error);
			}
		}

		menuItem->nOfChoices = choicePtr-menuItem->choices;
	}

	menuItem->yPosStart = yPos;
	yPos += BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING + 1;
	// HACK - Label points to a mutable char array
	for (char *lastWordPtr = menuItem->label, *chPtr = lastWordPtr; *chPtr;)
	{
		lastYPos = yPos;
		if (*chPtr == '\n')
		{
			xPos = xPosStart;
			yPos += BE_MENU_CHAR_HEIGHT + BE_MENU_ITEM_MIN_TEXT_LINE_PIX_SPACING;
			++noOfLabelLines;

			lastWordPtr = ++chPtr;
		}
		else if (*chPtr == ' ')
		{
			xPos += BE_MENU_CHAR_WIDTH;
			if (xPos >= xPosUpperBound)
			{
				xPos = xPosStart;
				yPos += BE_MENU_CHAR_HEIGHT + BE_MENU_ITEM_MIN_TEXT_LINE_PIX_SPACING;
				// HACK - Label points to a mutable char array cause of this reason
				*chPtr = '\n';
				++noOfLabelLines;
			}
			lastWordPtr = ++chPtr;
		}
		else
		{
			xPos += BE_MENU_CHAR_WIDTH;
			if (xPos >= xPosUpperBound)
			{
				// If lastWordPtr points to the very beginning of a word, it's a too long word
				if ((lastWordPtr == menuItem->label) || (*(lastWordPtr-1) == '\n'))
				{
					snprintf(error, sizeof(error), "BE_Launcher_PrepareMenuItem: Too long word!\n%s", lastWordPtr);
					BE_ST_ExitWithErrorMsg(error);
				}
				xPos = xPosStart;
				yPos += BE_MENU_CHAR_HEIGHT + BE_MENU_ITEM_MIN_TEXT_LINE_PIX_SPACING;
				// HACK - Label points to a mutable char array cause of this reason
				*(lastWordPtr-1) = '\n';
				chPtr = lastWordPtr; // Word relocated to next line
				++noOfLabelLines;
			}
			else
			{
				++chPtr;
			}	
		}
	}
	menuItem->yPosPastEnd = lastYPos + BE_MENU_CHAR_HEIGHT + BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING;
	// Can happen if there's just one row of text
	if (menuItem->yPosPastEnd - menuItem->yPosStart < BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT + 1)
		menuItem->yPosPastEnd = menuItem->yPosStart + BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT + 1;

	menuItem->labelYPos = menuItem->yPosStart + 1 + (menuItem->yPosPastEnd - (menuItem->yPosStart + 1) - (BE_MENU_CHAR_HEIGHT+BE_MENU_ITEM_MIN_TEXT_LINE_PIX_SPACING)*(noOfLabelLines-1) - BE_MENU_CHAR_HEIGHT) / 2;
	menuItem->selectionYPos = menuItem->yPosStart + 1 + (menuItem->yPosPastEnd - (menuItem->yPosStart + 1) - BE_MENU_CHAR_HEIGHT) / 2;

	return menuItem->yPosPastEnd;
}

void BE_Launcher_PrepareMenu(BEMenu *menu)
{
	char error[160];

	if (strlen(menu->title) > BE_MENU_STATIC_TEXT_MAX_ROW_STRLEN)
	{
		snprintf(error, sizeof(error), "BE_Launcher_PrepareMenu: Too long menu title!\n%s", menu->title);
		BE_ST_ExitWithErrorMsg(error);
	}

	menu->titleXPos = (BE_LAUNCHER_PIX_WIDTH-BE_MENU_CHAR_WIDTH*strlen(menu->title))/2;

	BEMenuItem **menuItemPP;
	int yPos = BE_MENU_FIRST_ITEM_PIX_YPOS;
	for (menuItemPP = menu->menuItems; *menuItemPP; ++menuItemPP)
		yPos = BEL_Launcher_PrepareMenuItem(*menuItemPP, yPos);

	menu->nOfItems = menuItemPP - menu->menuItems;
	menu->pixYScrollUpperBound = yPos - (BE_LAUNCHER_PIX_HEIGHT - 2);
	if (menu->pixYScrollUpperBound < 1)
		menu->pixYScrollUpperBound = 1;

	menu->currPixYScroll = 0;
}


// NOTE: Border doesn't include bottom-most row
static void BEL_Launcher_DrawTopRect(int x, int y, int width, int height, int frameColor, int innerColor, int yFirstBound, int yEndBound)
{
	// Should use g_be_launcher_screenPtr wisely so we don't go out of buffer's bounds
	// (just cause this can lead to undefined behaviors even without dereferencing)

	if (y >= yEndBound)
		return;

	if (y >= yFirstBound)
		memset(g_be_launcher_screenPtr + x + y*BE_LAUNCHER_PIX_WIDTH, frameColor, width);

	if (++y >= yEndBound)
		return;

	int actualEndY = y+height-1;
	if (yEndBound < actualEndY)
		actualEndY = yEndBound;

	if (actualEndY < yFirstBound) // == is OK
		return;

	if (y < yFirstBound)
		y = yFirstBound;

	uint8_t *pixPtr;
	for (pixPtr = g_be_launcher_screenPtr + x + y*BE_LAUNCHER_PIX_WIDTH; y < actualEndY; ++y, pixPtr += BE_LAUNCHER_PIX_WIDTH)
	{
		*pixPtr = frameColor;
		memset(pixPtr+1, innerColor, width-2);
		*(pixPtr+width-1) = frameColor;
	}

}

static void BEL_Launcher_DrawString(const char *str, int x, int y, int color, int yFirstBound, int yEndBound)
{
	// Should use g_be_launcher_screenPtr wisely so we don't go out of buffer's bounds
	// (just cause this can lead to undefined behaviors even without dereferencing)

	if (y >= yEndBound)
		return;

	const char *currLine = str;

	uint8_t *pixPtr;
	while (*currLine)
	{
		const char *ch;
		for (int chRow = 0; chRow < BE_MENU_CHAR_HEIGHT; ++chRow, ++y)
		{
			if (y >= yEndBound)
				return;

			pixPtr = g_be_launcher_screenPtr + x + y*BE_LAUNCHER_PIX_WIDTH;
			for (ch = currLine; *ch && (*ch != '\n'); ++ch)
			{
				if (y < yFirstBound)
					continue; // We still want to track the current line in string

				for (int chCol = 0; chCol < BE_MENU_CHAR_WIDTH; ++chCol, ++pixPtr)
					if (g_cga_8x8TextFont[BE_MENU_CHAR_WIDTH*BE_MENU_CHAR_HEIGHT*(unsigned char)(*ch) + BE_MENU_CHAR_HEIGHT*chRow + chCol])
						*pixPtr = color; // Draw pixel
			}
		}
		currLine = ch;
		if (*currLine == '\n')
			++currLine;
		y += BE_MENU_ITEM_MIN_TEXT_LINE_PIX_SPACING;
	}
}

static void BEL_Launcher_DrawBackButtonLabel(bool isPressed)
{
	BEL_Launcher_DrawString("\xAE\xAE", (BE_MENU_BACKBUTTON_PIX_WIDTH-BE_MENU_CHAR_WIDTH*strlen("\xAE\xAE"))/2, 1 + (BE_MENU_FIRST_ITEM_PIX_YPOS - BE_MENU_CHAR_HEIGHT - 1)/2, isPressed ? 15 : 7, 0, BE_LAUNCHER_PIX_HEIGHT);

	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_Launcher_DrawMenuTitleItem(BEMenu *menu)
{
	BEL_Launcher_DrawTopRect(0, 0, BE_LAUNCHER_PIX_WIDTH, BE_MENU_FIRST_ITEM_PIX_YPOS, 2, 0, 0, BE_LAUNCHER_PIX_HEIGHT);
	BEL_Launcher_DrawString(menu->title, menu->titleXPos, BE_MENU_TITLE_PIX_YPOS, 11, 0, BE_LAUNCHER_PIX_HEIGHT);
	// Back button
	BEL_Launcher_DrawTopRect(0, 0, BE_MENU_BACKBUTTON_PIX_WIDTH, BE_MENU_FIRST_ITEM_PIX_YPOS, 2, 0, 0, BE_LAUNCHER_PIX_HEIGHT);
	BEL_Launcher_DrawBackButtonLabel(false);

	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_Launcher_DrawMenuItemString(const char *str, int selectionYPos, int color)
{
	// HACK - Adding 1 to offset since some characters (lowercase letters) have a bit of spacing on the right
	BEL_Launcher_DrawString(str, 1 + (BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS+BE_MENU_CHAR_WIDTH) + (BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS - (BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS+BE_MENU_CHAR_WIDTH) - BE_MENU_CHAR_WIDTH*strlen(str)) / 2, selectionYPos - g_be_launcher_currMenu->currPixYScroll, color, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);
}

/*static*/ void BEL_Launcher_DrawMenuItem(BEMenuItem *menuItem)
{
	int labelColor;
	switch (menuItem->type)
	{
	case BE_MENUITEM_TYPE_STATIC:
		labelColor = 13;
		break;
	case BE_MENUITEM_TYPE_SELECTION:
	case BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER:
	case BE_MENUITEM_TYPE_DYNAMIC_SELECTION:
		labelColor = (g_be_launcher_selectedMenuItemPtr && (menuItem == *g_be_launcher_selectedMenuItemPtr)) ? 15 : 12;
		break;
	default:
		labelColor = (g_be_launcher_selectedMenuItemPtr && (menuItem == *g_be_launcher_selectedMenuItemPtr)) ? 15 : 7;
	}
	BEL_Launcher_DrawTopRect(0, menuItem->yPosStart - g_be_launcher_currMenu->currPixYScroll, BE_LAUNCHER_PIX_WIDTH, menuItem->yPosPastEnd - menuItem->yPosStart, 2, 0, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);
	BEL_Launcher_DrawString(menuItem->label, BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING + 1, menuItem->labelYPos - g_be_launcher_currMenu->currPixYScroll, labelColor, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);

	if ((menuItem->type == BE_MENUITEM_TYPE_SELECTION) || (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER))
	{
		BEL_Launcher_DrawString("\xAE", BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS, menuItem->selectionYPos - g_be_launcher_currMenu->currPixYScroll, 14, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);
		BEL_Launcher_DrawString("\xAF", BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS, menuItem->selectionYPos - g_be_launcher_currMenu->currPixYScroll, 14, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);
	}

	if ((menuItem->type == BE_MENUITEM_TYPE_SELECTION) || (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER) || (menuItem->type == BE_MENUITEM_TYPE_DYNAMIC_SELECTION))
		BEL_Launcher_DrawMenuItemString(menuItem->choices[menuItem->choice], menuItem->selectionYPos, 14);

	// HACK - Don't forget this!! (bottom of menu title)
	if (menuItem->yPosStart - g_be_launcher_currMenu->currPixYScroll < BE_MENU_FIRST_ITEM_PIX_YPOS)
		BEL_Launcher_DrawTopRect(0, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_WIDTH, 1, 2, 0, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);

	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_Launcher_DrawMenuItems(BEMenu *menu)
{
	BEMenuItem **menuItemPP, *menuItemP;
	for (menuItemPP = menu->menuItems, menuItemP = *menuItemPP; menuItemP; ++menuItemPP, menuItemP = *menuItemPP)
	{
		if (menuItemP->yPosPastEnd - g_be_launcher_currMenu->currPixYScroll <= BE_MENU_FIRST_ITEM_PIX_YPOS)
			continue;
		if (menuItemP->yPosStart - g_be_launcher_currMenu->currPixYScroll >= BE_LAUNCHER_PIX_HEIGHT)
			break;

		BEL_Launcher_DrawMenuItem(menuItemP);
	}
	// HACK - Don't forget this!! (bottom of last menu item)
	menuItemP = (*(--menuItemPP));
	BEL_Launcher_DrawTopRect(0, menuItemP->yPosPastEnd - g_be_launcher_currMenu->currPixYScroll, BE_LAUNCHER_PIX_WIDTH, 1, 2, 0, BE_MENU_FIRST_ITEM_PIX_YPOS, BE_LAUNCHER_PIX_HEIGHT);

	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_Launcher_SetCurrentMenu(BEMenu *menu)
{
	g_be_launcher_currMenu = menu;
	g_be_launcher_selectedMenuItemPtr = menu->menuItems;
	memset(g_be_launcher_screenPtr, 0, BE_LAUNCHER_PIX_WIDTH*BE_LAUNCHER_PIX_HEIGHT); // Clear screen
	BEL_Launcher_DrawMenuTitleItem(menu);
	BEL_Launcher_DrawMenuItems(menu);
}

static void BEL_Launcher_HandleCurrentMenuItem(void)
{
	if (!g_be_launcher_selectedMenuItemPtr || !(*g_be_launcher_selectedMenuItemPtr))
		return;

	BEMenuItem *menuItem = *g_be_launcher_selectedMenuItemPtr;
	switch (menuItem->type)
	{
	case BE_MENUITEM_TYPE_SELECTION:
	case BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER:
		// FIXME Move this to another place? (don't verify ptr is ok twice)
		BE_Launcher_HandleInput_ButtonRight();
		break;
	case BE_MENUITEM_TYPE_TARGETMENU:
		BEL_Launcher_SetCurrentMenu(menuItem->targetMenu);
		break;
	case BE_MENUITEM_TYPE_DYNAMIC_SELECTION:
		g_be_launcher_wasAnySettingChanged = true; // Safer to do this here
		// Fall-through
	case BE_MENUITEM_TYPE_HANDLER:
		menuItem->handler(g_be_launcher_selectedMenuItemPtr);
		break;
	}
}


// User input handlers

void BE_Launcher_HandleInput_ButtonLeft(void)
{
	if (!g_be_launcher_selectedMenuItemPtr || !(*g_be_launcher_selectedMenuItemPtr))
		return;

	BEMenuItem *menuItem = *g_be_launcher_selectedMenuItemPtr;
	if ((menuItem->type == BE_MENUITEM_TYPE_SELECTION) || (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER))
	{
		g_be_launcher_wasAnySettingChanged = true;

		if (--(menuItem->choice) < 0)
			menuItem->choice = menuItem->nOfChoices-1;

		if (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER)
			menuItem->handler(g_be_launcher_selectedMenuItemPtr);

		BEL_Launcher_DrawMenuItem(menuItem);
	}
}

void BE_Launcher_HandleInput_ButtonRight(void)
{
	if (!g_be_launcher_selectedMenuItemPtr || !(*g_be_launcher_selectedMenuItemPtr))
		return;

	BEMenuItem *menuItem = *g_be_launcher_selectedMenuItemPtr;
	if ((menuItem->type == BE_MENUITEM_TYPE_SELECTION) || (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER))
	{
		g_be_launcher_wasAnySettingChanged = true;

		if (++(menuItem->choice) >= menuItem->nOfChoices)
			menuItem->choice = 0;

		if (menuItem->type == BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER)
			menuItem->handler(g_be_launcher_selectedMenuItemPtr);

		BEL_Launcher_DrawMenuItem(menuItem);
	}
}

void BE_Launcher_HandleInput_ButtonUp(void)
{
	if (g_be_launcher_selectedMenuItemPtr)
	{
		if (g_be_launcher_selectedMenuItemPtr == g_be_launcher_currMenu->menuItems)
			return;

		--g_be_launcher_selectedMenuItemPtr;
	}
	else
		g_be_launcher_selectedMenuItemPtr = g_be_launcher_currMenu->menuItems;

	if (g_be_launcher_currMenu->currPixYScroll + BE_MENU_FIRST_ITEM_PIX_YPOS > (*g_be_launcher_selectedMenuItemPtr)->yPosStart)
		g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosStart - BE_MENU_FIRST_ITEM_PIX_YPOS;

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_HandleInput_ButtonDown(void)
{
	if (g_be_launcher_selectedMenuItemPtr)
	{
		if (!(*(g_be_launcher_selectedMenuItemPtr+1)))
			return;

		++g_be_launcher_selectedMenuItemPtr;
	}
	else
		g_be_launcher_selectedMenuItemPtr = g_be_launcher_currMenu->menuItems + g_be_launcher_currMenu->nOfItems-1;

	if (g_be_launcher_currMenu->currPixYScroll + (BE_LAUNCHER_PIX_HEIGHT - 1) < (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd)
		g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd - (BE_LAUNCHER_PIX_HEIGHT - 1);

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_HandleInput_ButtonActivate(void)
{
	BEL_Launcher_HandleCurrentMenuItem();
}

void BE_Launcher_HandleInput_ButtonBack(void)
{
	// HACK
	if (g_be_launcher_currMenu->backButtonHandler)
		g_be_launcher_currMenu->backButtonHandler(NULL);
	else
		BEL_Launcher_SetCurrentMenu(g_be_launcher_currMenu->backMenu);
}


static int g_be_launcher_vscroll_acceleration = 0;
static bool g_be_launcher_pointer_in_use = false;
static bool g_be_launcher_back_button_pressed = false;
static int g_be_launcher_lastpointery;

void BE_Launcher_HandleInput_PointerSelect(int xpos, int ypos)
{
	g_be_launcher_pointer_in_use = true;
	g_be_launcher_back_button_pressed = (xpos < BE_MENU_BACKBUTTON_PIX_WIDTH) && (ypos < BE_MENU_FIRST_ITEM_PIX_YPOS);
	BEL_Launcher_DrawBackButtonLabel(g_be_launcher_back_button_pressed);
	g_be_launcher_lastpointery = ypos;

	if (!g_be_launcher_back_button_pressed)
	{
		ypos += g_be_launcher_currMenu->currPixYScroll;
		for (BEMenuItem **menuItemPP = g_be_launcher_currMenu->menuItems; *menuItemPP; ++menuItemPP)
			if ((ypos >= (*menuItemPP)->yPosStart) && (ypos < (*menuItemPP)->yPosPastEnd))
			{
				BEMenuItem **prevMenuItemPP = g_be_launcher_selectedMenuItemPtr;
				g_be_launcher_selectedMenuItemPtr = menuItemPP;

				if (prevMenuItemPP)
					BEL_Launcher_DrawMenuItem(*prevMenuItemPP);
				BEL_Launcher_DrawMenuItem(*menuItemPP);
				return;
			}
	}
	BEMenuItem **prevMenuItemPP = g_be_launcher_selectedMenuItemPtr;
	g_be_launcher_selectedMenuItemPtr = NULL;
	if (prevMenuItemPP)
		BEL_Launcher_DrawMenuItem(*prevMenuItemPP);
}

void BE_Launcher_HandleInput_PointerRelease(int xpos, int ypos)
{
	g_be_launcher_pointer_in_use = false;
	//g_be_launcher_lastpointery = ypos;
	if (g_be_launcher_back_button_pressed)
	{
		g_be_launcher_back_button_pressed = false;
		//BEL_Launcher_DrawBackButtonLabel(g_be_launcher_back_button_pressed);
		BE_Launcher_HandleInput_ButtonBack();
		return;
	}

	if (g_be_launcher_selectedMenuItemPtr && (ypos+g_be_launcher_currMenu->currPixYScroll >= (*g_be_launcher_selectedMenuItemPtr)->yPosStart) && (ypos+g_be_launcher_currMenu->currPixYScroll < (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd))
	{
		// Should work like confirm key except for some exception
		BEMenuItem *menuItem = *g_be_launcher_selectedMenuItemPtr;
		switch (menuItem->type)
		{
		case BE_MENUITEM_TYPE_SELECTION:
		case BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER:
			// FIXME Move these to another places? (don't verify ptr is ok twice)
			if ((xpos >= BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS) && (xpos < BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS +  BE_MENU_CHAR_WIDTH))
				BE_Launcher_HandleInput_ButtonLeft();
			else if ((xpos >= BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS) && (xpos < BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS +  BE_MENU_CHAR_WIDTH))
				BE_Launcher_HandleInput_ButtonRight();
			else
		default:
			BEL_Launcher_HandleCurrentMenuItem();
		}
	}

	g_be_launcher_vscroll_acceleration = (g_be_launcher_lastpointery-ypos); // Invert direction
}

void BE_Launcher_HandleInput_PointerMotion(int xpos, int ypos)
{
	if (!g_be_launcher_pointer_in_use)
		return;

	g_be_launcher_back_button_pressed = false;
	BEL_Launcher_DrawBackButtonLabel(g_be_launcher_back_button_pressed);

	g_be_launcher_selectedMenuItemPtr = NULL;
	g_be_launcher_vscroll_acceleration = 0;

	g_be_launcher_currMenu->currPixYScroll += (g_be_launcher_lastpointery-ypos); // Invert direction
	if (g_be_launcher_currMenu->currPixYScroll >= g_be_launcher_currMenu->pixYScrollUpperBound)
		g_be_launcher_currMenu->currPixYScroll = g_be_launcher_currMenu->pixYScrollUpperBound-1;
	else if (g_be_launcher_currMenu->currPixYScroll < 0)
		g_be_launcher_currMenu->currPixYScroll = 0;

	g_be_launcher_lastpointery = ypos;

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_HandleInput_PointerVScroll(int ydiff)
{
	g_be_launcher_vscroll_acceleration += ydiff;
}


void BE_Launcher_RefreshVerticalScrolling(void)
{
	if (g_be_launcher_vscroll_acceleration == 0)
		return;

	if (g_be_launcher_vscroll_acceleration > 0)
		g_be_launcher_currMenu->currPixYScroll += g_be_launcher_vscroll_acceleration--;
	else
		g_be_launcher_currMenu->currPixYScroll += g_be_launcher_vscroll_acceleration++;

	if (g_be_launcher_currMenu->currPixYScroll >= g_be_launcher_currMenu->pixYScrollUpperBound)
	{
		g_be_launcher_currMenu->currPixYScroll = g_be_launcher_currMenu->pixYScrollUpperBound-1;
		g_be_launcher_vscroll_acceleration = 0;
	}
	else if (g_be_launcher_currMenu->currPixYScroll < 0)
	{
		g_be_launcher_currMenu->currPixYScroll = 0;
		g_be_launcher_vscroll_acceleration = 0;
	}

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_Start(void)
{
	BE_ST_Launcher_Prepare();

	g_be_launcher_screenPtr = BE_ST_Launcher_GetGfxPtr();

	BE_Launcher_PrepareMenu(&g_beMainMenu);
	BE_Launcher_PrepareMenu(&g_beSelectGameMenu);
	BE_Launcher_PrepareMenu(&g_beDisappearedGameHelpMenu);
	BE_Launcher_PrepareMenu(&g_beSelectInitialPathMenu);
	//BE_Launcher_PrepareMenu(&g_beSelectDirectoryMenu); // Dynamically adjusted
	BE_Launcher_PrepareMenu(&g_beSelectDirectoryErrorMenu);
	BE_Launcher_PrepareMenu(&g_beSelectDirectoryFoundGameMenu);
	BE_Launcher_PrepareMenu(&g_beSelectDirectoryNoGameFoundMenu);
	BE_Launcher_PrepareMenu(&g_beSettingsMenu);
	BE_Launcher_PrepareMenu(&g_beVideoSettingsMenu);
	BE_Launcher_PrepareMenu(&g_beSoundSettingsMenu);
	BE_Launcher_PrepareMenu(&g_beInputSettingsMenu);
	BE_Launcher_PrepareMenu(&g_beControllerSettingsMenu);
	BE_Launcher_PrepareMenu(&g_beQuitConfirmMenu);

	BEL_Launcher_SetCurrentMenu(&g_beMainMenu);

	g_be_launcher_wasAnySettingChanged = false;

	BE_ST_Launcher_RunEventLoop();
}

void BE_Launcher_Handler_GameLaunch(BEMenuItem **menuItemP)
{
	int menuItemNum = menuItemP - g_be_launcher_currMenu->menuItems;
	BE_ST_Launcher_Shutdown();
	int argc = 1;
	static char someEXEName[] = "proxy";
	char *argv[] = {someEXEName};
	BE_Cross_StartGame(BE_Cross_GetGameVerFromInstallation(menuItemNum), argc, argv, 0);
}



static BEMenuItem *g_beSelectDirectoryMenuItems;
static BEMenuItem **g_beSelectDirectoryMenuItemsPtrs;
static char *g_beSelectDirectoryMenuItemsStrsBuffer;

static void BEL_Launcher_FillDirSelectionMenu(const char **dirNames, int numOfSubDirs)
{
	g_beSelectDirectoryMenuItems = (BEMenuItem *)malloc((3+numOfSubDirs)*sizeof(BEMenuItem));
	g_beSelectDirectoryMenuItemsPtrs = (BEMenuItem **)malloc((4+numOfSubDirs)*sizeof(BEMenuItem *));
	const int strBufferLenBound = 40;
	g_beSelectDirectoryMenuItemsStrsBuffer = (char *)malloc((3+numOfSubDirs)*strBufferLenBound);
	if (!g_beSelectDirectoryMenuItems || !g_beSelectDirectoryMenuItemsPtrs || !g_beSelectDirectoryMenuItemsStrsBuffer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BE_ST_Launcher_Prepare: Out of memory!\n");
		BE_ST_Launcher_Shutdown();
		BE_ST_QuickExit();
	}

	g_beSelectDirectoryMenu.menuItems = g_beSelectDirectoryMenuItemsPtrs;
	char *label = g_beSelectDirectoryMenuItemsStrsBuffer;
	const char **dirNamePtr = dirNames;
	for (int i = 0; i < 3+numOfSubDirs; ++i, label += strBufferLenBound)
	{
		g_beSelectDirectoryMenuItemsPtrs[i] = &g_beSelectDirectoryMenuItems[i];
		g_beSelectDirectoryMenuItems[i].choices = NULL;
		g_beSelectDirectoryMenuItems[i].targetMenu = NULL;
		g_beSelectDirectoryMenuItems[i].label = label;
		g_beSelectDirectoryMenuItems[i].type = BE_MENUITEM_TYPE_HANDLER;
		switch (i)
		{
		case 0:
			g_beSelectDirectoryMenuItems[i].handler = &BE_Launcher_Handler_DirectorySelectionConfirm;
			BE_Cross_safeandfastcstringcopy(label, label + strBufferLenBound, "Confirm");
			break;
		case 1:
			g_beSelectDirectoryMenuItems[i].handler = &BE_Launcher_Handler_DirectorySelectionGoPrev;
			BE_Cross_safeandfastcstringcopy(label, label + strBufferLenBound, "Go to previous location");
			break;
		case 2:
			g_beSelectDirectoryMenuItems[i].handler = NULL;
			BE_Cross_safeandfastcstringcopy(label, label + strBufferLenBound, "");
			g_beSelectDirectoryMenuItems[i].type = BE_MENUITEM_TYPE_STATIC; // SPECIAL
			break;
		default:
		{
			size_t dirNameLen = strlen(*dirNamePtr);
			g_beSelectDirectoryMenuItems[i].handler = &BE_Launcher_Handler_DirectorySelection;
			BE_Cross_safeandfastcstringcopy(label, label + strBufferLenBound, *dirNamePtr++);
			if (dirNameLen >= strBufferLenBound)
				memset(label+strBufferLenBound-4, '.', 3);
		}
		}
	}
	g_beSelectDirectoryMenuItemsPtrs[3+numOfSubDirs] = NULL;

	BE_Launcher_PrepareMenu(&g_beSelectDirectoryMenu);
	BEL_Launcher_SetCurrentMenu(&g_beSelectDirectoryMenu);
}

void BE_Launcher_ClearDirSelectionMenu(void)
{
	free(g_beSelectDirectoryMenuItems);
	free(g_beSelectDirectoryMenuItemsPtrs);
	free(g_beSelectDirectoryMenuItemsStrsBuffer);
	g_beSelectDirectoryMenuItems = NULL;
	g_beSelectDirectoryMenuItemsPtrs = NULL;
	g_beSelectDirectoryMenuItemsStrsBuffer = NULL;
}

void BE_Launcher_Handler_RootPathSelection(BEMenuItem **menuItemP)
{
	int menuItemNum = menuItemP - g_be_launcher_currMenu->menuItems;
	int numOfSubDirs;
	const char **dirs = BE_Cross_DirSelection_Start(menuItemNum, &numOfSubDirs);
	if (dirs)
		BEL_Launcher_FillDirSelectionMenu(dirs, numOfSubDirs);
	else
		BEL_Launcher_SetCurrentMenu(&g_beSelectDirectoryErrorMenu);
}

void BE_Launcher_Handler_DirectorySelection(BEMenuItem **menuItemP)
{
	int menuItemNum = menuItemP - g_be_launcher_currMenu->menuItems;
	int numOfSubDirs;
	BE_Launcher_ClearDirSelectionMenu();
	const char **dirs = BE_Cross_DirSelection_GetNext(menuItemNum - 3, &numOfSubDirs);
	if (dirs)
		BEL_Launcher_FillDirSelectionMenu(dirs, numOfSubDirs);
	else
		BEL_Launcher_SetCurrentMenu(&g_beSelectDirectoryErrorMenu);
}

void BE_Launcher_Handler_DirectorySelectionConfirm(BEMenuItem **menuItemP)
{
	int gameVer = BE_Cross_DirSelection_TryAddGameInstallation();
	if (gameVer != BE_GAMEVER_LAST)
	{
		BE_ST_Launcher_RefreshSelectGameMenuContents();

		// Clear resources
		BE_Launcher_ClearDirSelectionMenu();
		BE_Cross_DirSelection_Finish();

		BE_Launcher_PrepareMenu(&g_beSelectGameMenu); // Refresh this menu

		BEL_Launcher_SetCurrentMenu(&g_beSelectDirectoryFoundGameMenu); // Show this "menu" (actually a dialog)
	}
	else
		BEL_Launcher_SetCurrentMenu(&g_beSelectDirectoryNoGameFoundMenu); // Do NOT clear resources, we stay in current directory
}

void BE_Launcher_Handler_DirectorySelectionGoPrev(BEMenuItem **menuItemP)
{
	int numOfSubDirs;
	BE_Launcher_ClearDirSelectionMenu();
	const char **dirs = BE_Cross_DirSelection_GetPrev(&numOfSubDirs);
	if (dirs)
		BEL_Launcher_FillDirSelectionMenu(dirs, numOfSubDirs);
	else
		BEL_Launcher_SetCurrentMenu(&g_beSelectInitialPathMenu);
}

void BE_Launcher_Handler_MenuQuit(BEMenuItem **menuItemP)
{
	BE_ST_Launcher_Shutdown();
	BE_ST_QuickExit();
}

void BE_Launcher_Handler_ControllerAction(BEMenuItem **menuItemP)
{
	BEMenuItem *menuItem = *menuItemP;
	BEL_Launcher_DrawMenuItemString(menuItem->choices[menuItem->choice], menuItem->selectionYPos, 0); // Erase string
	BEL_Launcher_DrawMenuItemString("Press...", menuItem->selectionYPos, 9); // Draw this one instead
	BE_ST_Launcher_WaitForControllerButton(menuItem);
	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}
