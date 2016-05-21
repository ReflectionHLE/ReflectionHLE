/* Copyright (C) 2015-2016 NY00123
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

#ifdef REFKEEN_ENABLE_LAUNCHER

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

#define BE_MENU_SEARCHBUTTON_PIX_WIDTH BE_MENU_FIRST_ITEM_PIX_YPOS

#define BE_MENU_STATIC_TEXT_MAX_ROW_STRLEN ((BE_LAUNCHER_PIX_WIDTH-2*(BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING+1))/BE_MENU_CHAR_WIDTH)

#define BE_LAUNCHER_SELECTION_LABEL_PIX_XPOS_UPPERBOUND (2*(BE_LAUNCHER_PIX_WIDTH-1)/3-2*BE_MENU_CHAR_WIDTH)
#define BE_LAUNCHER_SELECTION_LARROW_PIX_XPOS (BE_LAUNCHER_SELECTION_LABEL_PIX_XPOS_UPPERBOUND+BE_MENU_CHAR_WIDTH)
#define BE_LAUNCHER_SELECTION_RARROW_PIX_XPOS (BE_LAUNCHER_PIX_WIDTH-1-BE_MENU_ITEM_MIN_TEXT_BORDER_PIX_SPACING-BE_MENU_CHAR_WIDTH)

#define BE_LAUNCHER_POINTER_MOTION_PIX_THRESHOLD 8 // Moving the pointer less than that leads to no effect


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

static void BEL_Launcher_DrawSearchButtonLabel(bool isPressed)
{
	BEL_Launcher_DrawString("\xF9\xF9\xF9", BE_LAUNCHER_PIX_WIDTH-(BE_MENU_SEARCHBUTTON_PIX_WIDTH+BE_MENU_CHAR_WIDTH*strlen("\xF9\xF9\xF9"))/2, 1 + (BE_MENU_FIRST_ITEM_PIX_YPOS - BE_MENU_CHAR_HEIGHT - 1)/2, isPressed ? 15 : 7, 0, BE_LAUNCHER_PIX_HEIGHT);

	BE_ST_Launcher_MarkGfxCache();
}

static void BEL_Launcher_DrawMenuTitleItem(BEMenu *menu)
{
	BEL_Launcher_DrawTopRect(0, 0, BE_LAUNCHER_PIX_WIDTH, BE_MENU_FIRST_ITEM_PIX_YPOS, 2, 0, 0, BE_LAUNCHER_PIX_HEIGHT);
	BEL_Launcher_DrawString(menu->title, menu->titleXPos, BE_MENU_TITLE_PIX_YPOS, 11, 0, BE_LAUNCHER_PIX_HEIGHT);
	// Back button
	BEL_Launcher_DrawTopRect(0, 0, BE_MENU_BACKBUTTON_PIX_WIDTH, BE_MENU_FIRST_ITEM_PIX_YPOS, 2, 0, 0, BE_LAUNCHER_PIX_HEIGHT);
	BEL_Launcher_DrawBackButtonLabel(false);
	// Search button
	BEL_Launcher_DrawTopRect(BE_LAUNCHER_PIX_WIDTH-BE_MENU_SEARCHBUTTON_PIX_WIDTH, 0, BE_MENU_SEARCHBUTTON_PIX_WIDTH, BE_MENU_FIRST_ITEM_PIX_YPOS, 2, 0, 0, BE_LAUNCHER_PIX_HEIGHT);
	BEL_Launcher_DrawSearchButtonLabel(false);

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

static bool BEL_Launcher_IsMenuItemAboveViewPort(BEMenuItem *menuItem)
{
	return (menuItem->yPosPastEnd - g_be_launcher_currMenu->currPixYScroll <= BE_MENU_FIRST_ITEM_PIX_YPOS);
}

static bool BEL_Launcher_IsMenuItemBelowViewPort(BEMenuItem *menuItem)
{
	return (menuItem->yPosStart - g_be_launcher_currMenu->currPixYScroll >= BE_LAUNCHER_PIX_HEIGHT);
}

static void BEL_Launcher_DrawMenuItems(BEMenu *menu)
{
	BEMenuItem **menuItemPP, *menuItemP;
	for (menuItemPP = menu->menuItems, menuItemP = *menuItemPP; menuItemP; ++menuItemPP, menuItemP = *menuItemPP)
	{
		if (BEL_Launcher_IsMenuItemAboveViewPort(menuItemP))
			continue;
		if (BEL_Launcher_IsMenuItemBelowViewPort(menuItemP))
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

	void BEL_ST_Launcher_TurnTextSearchOff(void);
	BEL_ST_Launcher_TurnTextSearchOff();
}



static char g_be_launcher_currInputStrSearch[BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND];
static char *g_be_launcher_currInputStrSearchPtr;
static char * const g_be_launcher_currInputStrSearchLastCharPtr = g_be_launcher_currInputStrSearch + BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND - 1;


static void BEL_Launcher_HandleCurrentMenuItem(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
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

static int g_be_launcher_vscroll_currrateper100ms = 0;

void BE_Launcher_HandleInput_ButtonLeft(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

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
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

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
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

	BEMenuItem **menuItemRPP; // Technically used as as "reverse iterator" (this is done since we don't want to mistakenly dereference g_be_launcher_currMenu->menuItems-1)
	if (!g_be_launcher_selectedMenuItemPtr || BEL_Launcher_IsMenuItemAboveViewPort(*g_be_launcher_selectedMenuItemPtr) || BEL_Launcher_IsMenuItemBelowViewPort(*g_be_launcher_selectedMenuItemPtr))
		menuItemRPP = g_be_launcher_currMenu->menuItems + g_be_launcher_currMenu->nOfItems;
	else
		menuItemRPP = g_be_launcher_selectedMenuItemPtr;

	while ((menuItemRPP != g_be_launcher_currMenu->menuItems) && BEL_Launcher_IsMenuItemBelowViewPort(*(menuItemRPP-1)))
		--menuItemRPP;
	while ((menuItemRPP != g_be_launcher_currMenu->menuItems) && !BEL_Launcher_IsMenuItemAboveViewPort(*(menuItemRPP-1)) && ((*(menuItemRPP-1))->type == BE_MENUITEM_TYPE_STATIC))
		--menuItemRPP;

	if ((menuItemRPP != g_be_launcher_currMenu->menuItems) && !BEL_Launcher_IsMenuItemAboveViewPort(*(menuItemRPP-1))) // Match found
	{
		g_be_launcher_selectedMenuItemPtr = menuItemRPP-1;
		if (g_be_launcher_currMenu->currPixYScroll + (BE_LAUNCHER_PIX_HEIGHT - 1) < (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd)
			g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd - (BE_LAUNCHER_PIX_HEIGHT - 1);
		else if (g_be_launcher_currMenu->currPixYScroll + BE_MENU_FIRST_ITEM_PIX_YPOS > (*g_be_launcher_selectedMenuItemPtr)->yPosStart)
			g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosStart - BE_MENU_FIRST_ITEM_PIX_YPOS;
	}
	else
	{
		g_be_launcher_currMenu->currPixYScroll -= BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT;
		if (g_be_launcher_currMenu->currPixYScroll < 0)
			g_be_launcher_currMenu->currPixYScroll = 0;
	}

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_HandleInput_ButtonDown(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

	BEMenuItem **menuItemPP;
	if (!g_be_launcher_selectedMenuItemPtr || BEL_Launcher_IsMenuItemAboveViewPort(*g_be_launcher_selectedMenuItemPtr) || BEL_Launcher_IsMenuItemBelowViewPort(*g_be_launcher_selectedMenuItemPtr))
		menuItemPP = g_be_launcher_currMenu->menuItems;
	else
		menuItemPP = g_be_launcher_selectedMenuItemPtr+1;

	while ((*menuItemPP) && BEL_Launcher_IsMenuItemAboveViewPort(*menuItemPP))
		++menuItemPP;
	while ((*menuItemPP) && !BEL_Launcher_IsMenuItemBelowViewPort(*menuItemPP) && ((*menuItemPP)->type == BE_MENUITEM_TYPE_STATIC))
		++menuItemPP;

	if ((*menuItemPP) && !BEL_Launcher_IsMenuItemBelowViewPort(*menuItemPP)) // Match found
	{
		g_be_launcher_selectedMenuItemPtr = menuItemPP;
		if (g_be_launcher_currMenu->currPixYScroll + (BE_LAUNCHER_PIX_HEIGHT - 1) < (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd)
			g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd - (BE_LAUNCHER_PIX_HEIGHT - 1);
		else if (g_be_launcher_currMenu->currPixYScroll + BE_MENU_FIRST_ITEM_PIX_YPOS > (*g_be_launcher_selectedMenuItemPtr)->yPosStart)
			g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosStart - BE_MENU_FIRST_ITEM_PIX_YPOS;
	}
	else
	{
		g_be_launcher_currMenu->currPixYScroll += BE_MENU_ITEM_MIN_INTERNAL_PIX_HEIGHT;
		if (g_be_launcher_currMenu->currPixYScroll >= g_be_launcher_currMenu->pixYScrollUpperBound)
			g_be_launcher_currMenu->currPixYScroll = g_be_launcher_currMenu->pixYScrollUpperBound-1;
	}

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_HandleInput_ButtonPageUp(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

	g_be_launcher_currMenu->currPixYScroll -= BE_LAUNCHER_PIX_HEIGHT-BE_MENU_FIRST_ITEM_PIX_YPOS;
	if (g_be_launcher_currMenu->currPixYScroll < 0)
		g_be_launcher_currMenu->currPixYScroll = 0;
	// HACK
	g_be_launcher_selectedMenuItemPtr = NULL;
	BE_Launcher_HandleInput_ButtonDown();
}

void BE_Launcher_HandleInput_ButtonPageDown(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

	g_be_launcher_currMenu->currPixYScroll += BE_LAUNCHER_PIX_HEIGHT-BE_MENU_FIRST_ITEM_PIX_YPOS;
	if (g_be_launcher_currMenu->currPixYScroll >= g_be_launcher_currMenu->pixYScrollUpperBound)
		g_be_launcher_currMenu->currPixYScroll = g_be_launcher_currMenu->pixYScrollUpperBound-1;
	// HACK
	g_be_launcher_selectedMenuItemPtr = NULL;
	BE_Launcher_HandleInput_ButtonUp();
}

void BE_Launcher_HandleInput_ButtonActivate(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

	BEL_Launcher_HandleCurrentMenuItem();
}

void BE_Launcher_HandleInput_ButtonBack(void)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;
	// HACK
	if (g_be_launcher_currMenu->backButtonHandler)
		g_be_launcher_currMenu->backButtonHandler(NULL);
	else
		BEL_Launcher_SetCurrentMenu(g_be_launcher_currMenu->backMenu);
}

void BE_Launcher_HandleInput_ButtonSearch(void)
{
	void BEL_ST_Launcher_ToggleTextSearch(void);
	BEL_ST_Launcher_ToggleTextSearch();
}

void BE_Launcher_HandleInput_ASCIIChar(char ch)
{
	g_be_launcher_vscroll_currrateper100ms = 0;

	if (g_be_launcher_currInputStrSearchPtr == g_be_launcher_currInputStrSearchLastCharPtr)
		return;

	*g_be_launcher_currInputStrSearchPtr++ = ch;
	BEMenuItem **startMenuItemPP = g_be_launcher_selectedMenuItemPtr;
	if (!startMenuItemPP)
		startMenuItemPP = g_be_launcher_currMenu->menuItems;

	BEMenuItem **menuItemPP = startMenuItemPP;
	do
	{
		if (((*menuItemPP)->type != BE_MENUITEM_TYPE_STATIC) && !BE_Cross_strncasecmp((*menuItemPP)->label, g_be_launcher_currInputStrSearch, g_be_launcher_currInputStrSearchPtr-g_be_launcher_currInputStrSearch))
		{
			g_be_launcher_selectedMenuItemPtr = menuItemPP;

			if (g_be_launcher_currMenu->currPixYScroll + (BE_LAUNCHER_PIX_HEIGHT - 1) < (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd)
				g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosPastEnd - (BE_LAUNCHER_PIX_HEIGHT - 1);
			else if (g_be_launcher_currMenu->currPixYScroll + BE_MENU_FIRST_ITEM_PIX_YPOS > (*g_be_launcher_selectedMenuItemPtr)->yPosStart)
				g_be_launcher_currMenu->currPixYScroll = (*g_be_launcher_selectedMenuItemPtr)->yPosStart - BE_MENU_FIRST_ITEM_PIX_YPOS;

			BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
			return;
		}

		++menuItemPP;
		if (!(*menuItemPP))
			menuItemPP = g_be_launcher_currMenu->menuItems;
	} while (menuItemPP != startMenuItemPP);
	// Repeat from the beginning if it's not already the first search char
	if (g_be_launcher_currInputStrSearchPtr != g_be_launcher_currInputStrSearch+1)
	{
		g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
		BE_Launcher_HandleInput_ASCIIChar(ch);
	}
}



static bool g_be_launcher_pointer_in_use = false;
static bool g_be_launcher_back_button_pressed = false;
static bool g_be_launcher_search_button_pressed = false;
static int g_be_launcher_startpointerx;
static int g_be_launcher_startpointery;
static bool g_be_launcher_pointermotionactuallystarted;
static int g_be_launcher_lastpointery;
static int g_be_launcher_lastpointeryforcurrentratemeasurement;
static uint32_t g_be_launcher_lastpointerymeasurementticksinms;
static int g_be_launcher_pointerymotionrateper100ms;
static uint32_t g_be_launcher_lastaccelarationupdateticks;
static uint32_t g_be_launcher_lastrefreshvscrollticks;

void BE_Launcher_HandleInput_PointerSelect(int xpos, int ypos, uint32_t ticksinms)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;
	g_be_launcher_vscroll_currrateper100ms = 0;

	g_be_launcher_pointer_in_use = true;
	g_be_launcher_back_button_pressed = (xpos < BE_MENU_BACKBUTTON_PIX_WIDTH) && (ypos < BE_MENU_FIRST_ITEM_PIX_YPOS);
	BEL_Launcher_DrawBackButtonLabel(g_be_launcher_back_button_pressed);
	g_be_launcher_search_button_pressed = (xpos >= BE_LAUNCHER_PIX_WIDTH-BE_MENU_SEARCHBUTTON_PIX_WIDTH) && (ypos < BE_MENU_FIRST_ITEM_PIX_YPOS);
	BEL_Launcher_DrawSearchButtonLabel(g_be_launcher_search_button_pressed);
	g_be_launcher_startpointerx = xpos;
	g_be_launcher_startpointery = ypos;
	g_be_launcher_pointermotionactuallystarted = false;
	g_be_launcher_lastpointery = ypos;
	g_be_launcher_lastpointeryforcurrentratemeasurement = ypos;
	g_be_launcher_lastpointerymeasurementticksinms = ticksinms;
	g_be_launcher_pointerymotionrateper100ms = 0;

	if (!g_be_launcher_back_button_pressed && !g_be_launcher_search_button_pressed && (ypos >= BE_MENU_FIRST_ITEM_PIX_YPOS))
	{
		ypos += g_be_launcher_currMenu->currPixYScroll;
		for (BEMenuItem **menuItemPP = g_be_launcher_currMenu->menuItems; *menuItemPP; ++menuItemPP)
			if ((ypos >= (*menuItemPP)->yPosStart) && (ypos < (*menuItemPP)->yPosPastEnd) && ((*menuItemPP)->type != BE_MENUITEM_TYPE_STATIC))
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

void BE_Launcher_HandleInput_PointerRelease(int xpos, int ypos, uint32_t ticksinms)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;

	g_be_launcher_pointer_in_use = false;
	//g_be_launcher_lastpointery = ypos;

	if (g_be_launcher_back_button_pressed)
	{
		g_be_launcher_back_button_pressed = false;
		//BEL_Launcher_DrawBackButtonLabel(g_be_launcher_back_button_pressed);
		BE_Launcher_HandleInput_ButtonBack();
		return;
	}

	if (g_be_launcher_search_button_pressed)
	{
		g_be_launcher_search_button_pressed = false;
		BEL_Launcher_DrawSearchButtonLabel(g_be_launcher_search_button_pressed);
		BE_Launcher_HandleInput_ButtonSearch();
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

	if (ticksinms != g_be_launcher_lastpointerymeasurementticksinms) // Theoretically this shouldn't ever occur (being handled by a preceding motion event instead), but checking just in case
		g_be_launcher_pointerymotionrateper100ms = (ypos-g_be_launcher_lastpointeryforcurrentratemeasurement)*100/(int32_t)(ticksinms - g_be_launcher_lastpointerymeasurementticksinms);

	g_be_launcher_vscroll_currrateper100ms = -g_be_launcher_pointerymotionrateper100ms; // Invert direction
	g_be_launcher_lastaccelarationupdateticks = ticksinms;
	g_be_launcher_lastrefreshvscrollticks = ticksinms;
}

void BE_Launcher_HandleInput_PointerMotion(int xpos, int ypos, uint32_t ticksinms)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;

	if (!g_be_launcher_pointer_in_use)
		return;

	if (!g_be_launcher_pointermotionactuallystarted)
	{
		if ((ypos >= g_be_launcher_startpointery-BE_LAUNCHER_POINTER_MOTION_PIX_THRESHOLD) && (ypos <= g_be_launcher_startpointery+BE_LAUNCHER_POINTER_MOTION_PIX_THRESHOLD))
			return;
		g_be_launcher_pointermotionactuallystarted = true;
	}

	g_be_launcher_vscroll_currrateper100ms = 0;

	g_be_launcher_back_button_pressed = false;
	BEL_Launcher_DrawBackButtonLabel(g_be_launcher_back_button_pressed);

	g_be_launcher_search_button_pressed = false;
	BEL_Launcher_DrawSearchButtonLabel(g_be_launcher_search_button_pressed);

	g_be_launcher_selectedMenuItemPtr = NULL;
	g_be_launcher_vscroll_currrateper100ms = 0;

	g_be_launcher_currMenu->currPixYScroll += (g_be_launcher_lastpointery-ypos); // Invert direction
	if (g_be_launcher_currMenu->currPixYScroll >= g_be_launcher_currMenu->pixYScrollUpperBound)
		g_be_launcher_currMenu->currPixYScroll = g_be_launcher_currMenu->pixYScrollUpperBound-1;
	else if (g_be_launcher_currMenu->currPixYScroll < 0)
		g_be_launcher_currMenu->currPixYScroll = 0;

	if (ticksinms - g_be_launcher_lastpointerymeasurementticksinms >= 100)
	{
		g_be_launcher_pointerymotionrateper100ms = (ypos-g_be_launcher_lastpointeryforcurrentratemeasurement)*100/(int32_t)(ticksinms - g_be_launcher_lastpointerymeasurementticksinms);
		g_be_launcher_lastpointerymeasurementticksinms += 100*((ticksinms-g_be_launcher_lastpointerymeasurementticksinms)/100);
		g_be_launcher_lastpointeryforcurrentratemeasurement = ypos;
	}

	g_be_launcher_lastpointery = ypos;

	BEL_Launcher_DrawMenuItems(g_be_launcher_currMenu);
}

void BE_Launcher_HandleInput_PointerVScroll(int ydiff, uint32_t ticksinms)
{
	g_be_launcher_currInputStrSearchPtr = g_be_launcher_currInputStrSearch;

	if (!g_be_launcher_vscroll_currrateper100ms)
	{
		g_be_launcher_lastaccelarationupdateticks = ticksinms;
		g_be_launcher_lastrefreshvscrollticks = ticksinms;
	}
	g_be_launcher_vscroll_currrateper100ms += ydiff;
}


void BE_Launcher_RefreshVerticalScrolling(uint32_t ticksinms)
{
	if (g_be_launcher_vscroll_currrateper100ms == 0)
		return;

	g_be_launcher_currMenu->currPixYScroll += (int32_t)(ticksinms-g_be_launcher_lastrefreshvscrollticks)*g_be_launcher_vscroll_currrateper100ms/100;
	g_be_launcher_lastrefreshvscrollticks = ticksinms;

	if (ticksinms - g_be_launcher_lastaccelarationupdateticks >= 10)
	{
		int stepsdiff = (ticksinms-g_be_launcher_lastaccelarationupdateticks)/10;
		g_be_launcher_lastaccelarationupdateticks += 10*stepsdiff;
		if (g_be_launcher_vscroll_currrateper100ms > 0)
		{
			g_be_launcher_vscroll_currrateper100ms -= stepsdiff;
			if (g_be_launcher_vscroll_currrateper100ms < 0)
				g_be_launcher_vscroll_currrateper100ms = 0;
		}
		else
		{
			g_be_launcher_vscroll_currrateper100ms += stepsdiff;
			if (g_be_launcher_vscroll_currrateper100ms > 0)
				g_be_launcher_vscroll_currrateper100ms = 0;
		}
	}
#if 0
	if (g_be_launcher_vscroll_acceleration > 0)
		g_be_launcher_currMenu->currPixYScroll += g_be_launcher_vscroll_acceleration--;
	else
		g_be_launcher_currMenu->currPixYScroll += g_be_launcher_vscroll_acceleration++;
#endif

	if (g_be_launcher_currMenu->currPixYScroll >= g_be_launcher_currMenu->pixYScrollUpperBound)
	{
		g_be_launcher_currMenu->currPixYScroll = g_be_launcher_currMenu->pixYScrollUpperBound-1;
		g_be_launcher_vscroll_currrateper100ms = 0;
	}
	else if (g_be_launcher_currMenu->currPixYScroll < 0)
	{
		g_be_launcher_currMenu->currPixYScroll = 0;
		g_be_launcher_vscroll_currrateper100ms = 0;
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
	BE_Launcher_PrepareMenu(&g_beShowVersionMenu);
	BE_Launcher_PrepareMenu(&g_beQuitConfirmMenu);

	BEL_Launcher_SetCurrentMenu(&g_beMainMenu);

	g_be_launcher_wasAnySettingChanged = false;

	BE_ST_Launcher_RunEventLoop();
}

static void BEL_Launcher_DoLaunchGame(int gameVer)
{
	BE_ST_Launcher_Shutdown();
	int argc = 0;
	// Making a copy since we modify this buffer (separating the arguments)
	static char argsCopy[sizeof(g_refKeenCfg.launcherExeArgs)];
	memcpy(argsCopy, g_refKeenCfg.launcherExeArgs, sizeof(argsCopy));
	char *argv[sizeof(argsCopy)/2+1];

	static char someEXEName[] = "proxy";
	argv[argc++] = someEXEName;

	char *srcPtr = argsCopy;
	while (*srcPtr)	
	{
		while (*srcPtr && (*srcPtr == ' '))
			++srcPtr;
		if (*srcPtr)
		{
			argv[argc++] = srcPtr;
			do
				++srcPtr;
			while (*srcPtr && (*srcPtr != ' '));
			if (*srcPtr == ' ')
				*srcPtr++ = '\0'; // Separate the arguments
		}
	}
	BE_Cross_StartGame(gameVer, argc, argv, 0);
}

void BE_Launcher_Handler_LastGameVerLaunch(BEMenuItem **menuItemP)
{
	BEL_Launcher_DoLaunchGame(g_refKeenCfg.lastSelectedGameVer);
}

void BE_Launcher_Handler_GameLaunch(BEMenuItem **menuItemP)
{
	BEL_Launcher_DoLaunchGame(BE_Cross_GetGameVerFromInstallation(menuItemP - g_be_launcher_currMenu->menuItems));
}



static BEMenuItem *g_beSelectDirectoryMenuItems;
static BEMenuItem **g_beSelectDirectoryMenuItemsPtrs;
static char *g_beSelectDirectoryMenuItemsStrsBuffer;

static void BEL_Launcher_FillDirSelectionMenu(const char **dirNames, int numOfSubDirs)
{
	g_beSelectDirectoryMenuItems = (BEMenuItem *)malloc((3+numOfSubDirs)*sizeof(BEMenuItem));
	g_beSelectDirectoryMenuItemsPtrs = (BEMenuItem **)malloc((4+numOfSubDirs)*sizeof(BEMenuItem *));
	g_beSelectDirectoryMenuItemsStrsBuffer = (char *)malloc((3+numOfSubDirs)*BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND);
	if (!g_beSelectDirectoryMenuItems || !g_beSelectDirectoryMenuItemsPtrs || !g_beSelectDirectoryMenuItemsStrsBuffer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BE_ST_Launcher_Prepare: Out of memory!\n");
		BE_ST_Launcher_Shutdown();
		BE_ST_QuickExit();
	}

	g_beSelectDirectoryMenu.menuItems = g_beSelectDirectoryMenuItemsPtrs;
	char *label = g_beSelectDirectoryMenuItemsStrsBuffer;
	const char **dirNamePtr = dirNames;
	for (int i = 0; i < 3+numOfSubDirs; ++i, label += BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND)
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
			BE_Cross_safeandfastcstringcopy(label, label + BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND, "Confirm");
			break;
		case 1:
			g_beSelectDirectoryMenuItems[i].handler = &BE_Launcher_Handler_DirectorySelectionGoPrev;
			BE_Cross_safeandfastcstringcopy(label, label + BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND, "Go to previous location");
			break;
		case 2:
			g_beSelectDirectoryMenuItems[i].handler = NULL;
			BE_Cross_safeandfastcstringcopy(label, label + BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND, "");
			g_beSelectDirectoryMenuItems[i].type = BE_MENUITEM_TYPE_STATIC; // SPECIAL
			break;
		default:
		{
			size_t dirNameLen = strlen(*dirNamePtr);
			g_beSelectDirectoryMenuItems[i].handler = &BE_Launcher_Handler_DirectorySelection;
			BE_Cross_safeandfastcstringcopy(label, label + BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND, *dirNamePtr++);
			if (dirNameLen >= BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND)
				memset(label+BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND-4, '.', 3);
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

#endif // REFKEEN_ENABLE_LAUNCHER
