#ifndef ALTCONTROLLER_H
#define ALTCONTROLLER_H

#include <stdbool.h>

// Used with BE_ST_CTRL_MAP_VALUESET mappings
extern int g_binding_value_button[2], g_binding_value_stats,
           g_binding_value_motionx, g_binding_value_motiony;

// Used with keyboard overrides
extern bool g_keybind_used_button[2], g_keybind_used_stats,
            g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right;

void PrepareGamePlayControllerMapping(void);
void FinalizeControlPanelMappingsByMousePresence(bool withmouse);

#endif
