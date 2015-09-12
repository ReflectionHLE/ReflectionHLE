/* Copyright (C) 2014-2015 NY00123
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

#ifndef	__BE_LAUNCHER__
#define __BE_LAUNCHER__

#define BE_LAUNCHER_PIX_WIDTH 320
#define BE_LAUNCHER_PIX_HEIGHT 240	

typedef enum { BE_MENUITEM_TYPE_STATIC, BE_MENUITEM_TYPE_SELECTION, BE_MENUITEM_TYPE_TARGETMENU, BE_MENUITEM_TYPE_HANDLER } BEMenuItemType;

typedef void (*BEMenuItemHandler)(int menuItemNum);

struct BEMenu;

typedef struct
{
	BEMenuItemHandler handler;
	const char **choices;
	struct BEMenu *targetMenu;
	char *label; // HACK - This points to a mutable char array
	int choice;
	int nOfChoices;
	int yPosStart, yPosPastEnd;
	int labelYPos, selectionYPos;
	BEMenuItemType type;
} BEMenuItem;

typedef struct BEMenu
{
	const char *title;
	struct BEMenu *backMenu;
	BEMenuItem **menuItems;
	int nOfItems;
	int titleXPos;
	int currPixYScroll;
	int pixYScrollUpperBound;
} BEMenu;

extern BEMenu g_beMainMenu, g_beSelectGameMenu, g_beDisappearedGameHelpMenu,
              g_beSettingsMenu, g_beVideoSettingsMenu, g_beMoreSettingsMenu,
              g_beQuitConfirmMenu;

extern bool g_be_launcher_wasAnySettingChanged;

void BE_Launcher_HandleInput_ButtonLeft(void);
void BE_Launcher_HandleInput_ButtonRight(void);
void BE_Launcher_HandleInput_ButtonUp(void);
void BE_Launcher_HandleInput_ButtonDown(void);
void BE_Launcher_HandleInput_ButtonActivate(void);
void BE_Launcher_HandleInput_ButtonBack(void);

void BE_Launcher_HandleInput_PointerSelect(int xpos, int ypos);
void BE_Launcher_HandleInput_PointerRelease(int xpos, int ypos);
void BE_Launcher_HandleInput_PointerMotion(int xpos, int ypos);
void BE_Launcher_HandleInput_PointerVScroll(int ydiff);

void BE_Launcher_RefreshVerticalScrolling(void);

void BE_Launcher_Handler_GameLaunch(int menuItemNum);
void BE_Launcher_Handler_MenuQuit(int menuItemNum);

void BE_Launcher_Start();

#endif // __BE_LAUNCHER__
