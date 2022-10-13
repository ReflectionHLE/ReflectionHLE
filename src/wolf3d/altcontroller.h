#ifndef ALTCONTROLLER_H
#define ALTCONTROLLER_H

#include <stdbool.h>

REFKEEN_NS_B

extern int g_binding_value_button[],
           g_binding_value_axisx, g_binding_value_axisy,
           g_binding_value_up, g_binding_value_down, g_binding_value_left, g_binding_value_right,
           g_binding_value_map;

// Used with keyboard overrides
extern bool g_keybind_used_button[],
            g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right,
            g_keybind_used_map;

void PrepareGamePlayControllerMapping(void);
void UpdateAltControllerMappingsByMousePresence(bool withmouse);

REFKEEN_NS_E

#endif
