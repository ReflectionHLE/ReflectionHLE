#ifndef BE_ST_LAUNCHER_H
#define BE_ST_LAUNCHER_H

#ifdef REFKEEN_ENABLE_LAUNCHER

#include "be_launcher.h"

void BE_ST_Launcher_Prepare(void);
void BE_ST_Launcher_Shutdown(void);
void BE_ST_Launcher_RefreshSelectGameMenuContents(void);
void BE_ST_Launcher_RefreshAndShowSelectGameExeMenuContents(int verId, int nOfExes);

// Used by launcher for user input binds
void BE_ST_Launcher_WaitForUserBind(BEMenuItem *menuItem, BEMenuBind menuBind);
// Launcher loop
void BE_ST_Launcher_RunEventLoop(void);

void BE_ST_Launcher_MarkGfxCache(void);
// ***WARNING***: Ensure BE_ST_Launcher_MarkGfxCache is called after drawing.
uint8_t *BE_ST_Launcher_GetGfxPtr(void);

#endif // REFKEEN_ENABLE_LAUNCHER

#endif // BE_ST_LAUNCHER_H
