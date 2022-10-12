#ifndef ALTCONTROLLER_H
#define ALTCONTROLLER_H

#include <stdbool.h>

REFKEEN_NS_B

extern int g_binding_value_button[2],
           g_binding_value_motionx, g_binding_value_motiony,
           g_binding_value_drink, g_binding_value_bolt, g_binding_value_nuke, g_binding_value_fastturn;

// Used with keyboard overrides
extern bool g_keybind_used_button[2],
            g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right,
            g_keybind_used_drink, g_keybind_used_bolt, g_keybind_used_nuke, g_keybind_used_fastturn;

void PrepareGamePlayControllerMapping(void);
void UpdateAltControllerMappingsByMousePresence(bool withmouse);

REFKEEN_NS_E

#endif
