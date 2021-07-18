#ifndef BE_INPUT_UI_H
#define BE_INPUT_UI_H

#include "../video/be_video_ui.h"

void BEL_ST_DoHideTouchUI(void);
void BEL_ST_CheckForHidingTouchUI(void);

bool BEL_ST_CheckCommonPointerPressCases(
	BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);

bool BEL_ST_CheckCommonPointerReleaseCases(
	BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);
	
bool BEL_ST_CheckCommonPointerMoveCases(
	BE_ST_TouchID touchId, BE_ST_FingerID fingerId, int x, int y);

#endif
