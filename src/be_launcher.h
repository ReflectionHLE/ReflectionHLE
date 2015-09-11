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

extern BEMenu g_beMainMenu, g_beSelectGameMenu,
              g_beSettingsMenu, g_beVideoSettingsMenu, g_beMoreSettingsMenu;

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
