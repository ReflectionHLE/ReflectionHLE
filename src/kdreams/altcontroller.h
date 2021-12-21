#ifndef ALTCONTROLLER_H
#define ALTCONTROLLER_H

#include <stdbool.h>

// Used with BE_ST_CTRL_MAP_VALUESET mappings
extern int g_binding_value_button[2],
           g_binding_value_motionx, g_binding_value_motiony;

void PrepareGamePlayControllerMapping(void);
void FinalizeControlPanelMappingsByMousePresence(bool withmouse);

#endif
