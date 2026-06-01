#ifndef BE_INPUT_CONTROLLER_MAPPINGS_H
#define BE_INPUT_CONTROLLER_MAPPINGS_H

#include "be_st.h"

extern const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr;

bool BEL_ST_AltControlScheme_HandleEntry(const BE_ST_ControllerSingleMap *map, int32_t value, int32_t *lastDeviceValuePtr);
bool BEL_ST_AltControlScheme_HandleAnyEntry(const BE_ST_ControllerSingleMap *map, int32_t value, int32_t *lastDeviceValuePtr, bool isAccum);
void BEL_ST_AltControlScheme_ClearBinaryStates(void);
void BEL_ST_AltControlScheme_CleanUp(void);
void BEL_ST_AltControlScheme_HandleTextInputEvent(int but, bool isPressed);
void BEL_ST_AltControlScheme_HandleDebugKeysEvent(int but, bool isPressed);

#endif
