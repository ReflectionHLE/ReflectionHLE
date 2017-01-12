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

#ifndef	_BE_LAUNCHER_
#define _BE_LAUNCHER_

#ifdef REFKEEN_ENABLE_LAUNCHER

#define BE_LAUNCHER_PIX_WIDTH 320
#define BE_LAUNCHER_PIX_HEIGHT 240
// Some upper bound for menu item label buffer length
#define BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND 40

typedef enum { BE_MENUITEM_TYPE_STATIC, BE_MENUITEM_TYPE_HANDLER, BE_MENUITEM_TYPE_SELECTION, BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER, BE_MENUITEM_TYPE_DYNAMIC_SELECTION, BE_MENUITEM_TYPE_TARGETMENU } BEMenuItemType;

struct BEMenuItem;
struct BEMenu;

typedef void (*BEMenuItemHandler)(struct BEMenuItem **menuItemP);

typedef struct BEMenuItem
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
	BEMenuItemHandler backButtonHandler; // Usually unused
	int nOfItems;
	int titleXPos;
	int currPixYScroll;
	int pixYScrollUpperBound;
} BEMenu;

extern BEMenu g_beMainMenu,
              g_beSelectGameMenu, g_beDisappearedGameHelpMenu,
              g_beSupportedGameVersionsMenu, g_beGameVersionDetailsMenu,
              g_beSelectInitialPathMenu, g_beSelectDirectoryMenu, g_beSelectDirectoryErrorMenu,
              g_beSelectDirectoryFoundGameMenu, g_beSelectDirectoryNoGameFoundMenu,
              g_beSettingsMenu, g_beVideoSettingsMenu, g_beSoundSettingsMenu,
              g_beInputSettingsMenu, g_beControllerSettingsMenu,
              g_beShowVersionMenu, g_beQuitConfirmMenu;

extern bool g_be_launcher_wasAnySettingChanged;

void BE_Launcher_HandleInput_ButtonLeft(void);
void BE_Launcher_HandleInput_ButtonRight(void);
void BE_Launcher_HandleInput_ButtonUp(void);
void BE_Launcher_HandleInput_ButtonDown(void);
void BE_Launcher_HandleInput_ButtonPageUp(void);
void BE_Launcher_HandleInput_ButtonPageDown(void);
void BE_Launcher_HandleInput_ButtonActivate(void);
void BE_Launcher_HandleInput_ButtonBack(void);
void BE_Launcher_HandleInput_ButtonSearch(void);
void BE_Launcher_HandleInput_ASCIIChar(char ch);

void BE_Launcher_HandleInput_PointerSelect(int xpos, int ypos, uint32_t ticksinms);
void BE_Launcher_HandleInput_PointerRelease(int xpos, int ypos, uint32_t ticksinms);
void BE_Launcher_HandleInput_PointerMotion(int xpos, int ypos, uint32_t ticksinms);
void BE_Launcher_HandleInput_PointerVScroll(int ydiff, uint32_t ticksinms);

void BE_Launcher_RefreshVerticalScrolling(uint32_t ticksinms);

void BE_Launcher_Handler_LastGameVerLaunch(BEMenuItem **menuItemP);
void BE_Launcher_Handler_SetArgumentsForGame(BEMenuItem **menuItemP);
void BE_Launcher_Handler_GameLaunch(BEMenuItem **menuItemP);
void BE_Launcher_Handler_SupportedGameVersionSelection(BEMenuItem **menuItemP);
void BE_Launcher_Handler_ReturnToSupportedGameVersionsMenu(BEMenuItem **menuItemP);
void BE_Launcher_Handler_RootPathSelection(BEMenuItem **menuItemP);
void BE_Launcher_Handler_DirectorySelection(BEMenuItem **menuItemP);
void BE_Launcher_Handler_DirectorySelectionConfirm(BEMenuItem **menuItemP);
void BE_Launcher_Handler_DirectorySelectionGoPrev(BEMenuItem **menuItemP);
void BE_Launcher_Handler_MenuQuit(BEMenuItem **menuItemP);
void BE_Launcher_Handler_ControllerAction(BEMenuItem **menuItemP);

void BE_Launcher_ArgumentsEditing_MoveCursorToEdge(bool moveForward);
void BE_Launcher_ArgumentsEditing_MoveCursorOnePos(bool moveForward);
void BE_Launcher_ArgumentsEditing_InsertChar(char ch);
void BE_Launcher_ArgumentsEditing_DeleteChar(bool deleteAt);

void BE_Launcher_ArgumentsEditing_HandleInput_PointerSelect(int xpos, int ypos);
bool BE_Launcher_ArgumentsEditing_HandleInput_PointerRelease(int xpos, int ypos);
void BE_Launcher_ArgumentsEditing_HandleInput_PointerMotion(int xpos, int ypos);

void BE_Launcher_Start(void);
void BE_Launcher_ClearDirSelectionMenu(void);

void BE_Launcher_PrepareMenu(BEMenu *menu);

#endif // REFKEEN_ENABLE_LAUNCHER

#endif // _BE_LAUNCHER_
