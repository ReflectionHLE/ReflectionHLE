/* Copyright (C) 2015-2021 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BE_LAUNCHER_H
#define BE_LAUNCHER_H

#ifdef REFKEEN_ENABLE_LAUNCHER

#define BE_LAUNCHER_PIX_WIDTH 320
#define BE_LAUNCHER_PIX_HEIGHT 240
// Some upper bound for menu item label buffer length
#define BE_LAUNCHER_MENUITEM_STRBUFFER_LEN_BOUND 40

typedef enum {
	BE_MENUITEM_TYPE_STATIC,
	BE_MENUITEM_TYPE_HANDLER,
	BE_MENUITEM_TYPE_SELECTION,
	BE_MENUITEM_TYPE_SELECTION_WITH_HANDLER,
	BE_MENUITEM_TYPE_DYNAMIC_SELECTION,
	BE_MENUITEM_TYPE_SLIDER,
	BE_MENUITEM_TYPE_RANGE_SLIDER,
	BE_MENUITEM_TYPE_TARGETMENU
} BEMenuItemType;

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
              g_beSelectGameMenu, g_beSelectGameExeMenu, g_beDisappearedGameHelpMenu,
              g_beSupportedGameVersionsMenu, g_beGameVersionDetailsMenu,
              g_beSelectInitialPathMenu, g_beSelectDirectoryMenu, g_beSelectDirectoryErrorMenu,
              g_beSelectDirectoryFoundGameMenu, g_beSelectDirectoryNoGameFoundMenu,
              g_beSettingsMenu, g_beVideoSettingsMenu, g_beSoundSettingsMenu,
              g_beInputSettingsMenu, g_beMiscSettingsMenu,
              g_beControllerSettingsMenu, g_beDeviceVolumesMenu,
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

void BE_Launcher_HandleInput_PointerSelect(int xpos, int ypos, uint32_t ticksinms, bool isMouse);
void BE_Launcher_HandleInput_PointerRelease(int xpos, int ypos, uint32_t ticksinms);
void BE_Launcher_HandleInput_PointerMotion(int xpos, int ypos, uint32_t ticksinms);
void BE_Launcher_HandleInput_PointerVScroll(int ydiff, uint32_t ticksinms);

void BE_Launcher_RefreshVerticalScrolling(uint32_t ticksinms);

void BE_Launcher_Handler_LastGameVerLaunch(BEMenuItem **menuItemP);
void BE_Launcher_Handler_SetArgumentsForGame(BEMenuItem **menuItemP);
void BE_Launcher_Handler_GameLaunch(BEMenuItem **menuItemP);
void BE_Launcher_Handler_GameLaunchWithChosenExe(BEMenuItem **menuItemP);
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

void BE_Launcher_ClearDirSelectionMenu(void);

void BE_Launcher_PrepareMenu(BEMenu *menu);

#endif // REFKEEN_ENABLE_LAUNCHER

#endif // BE_LAUNCHER_H
