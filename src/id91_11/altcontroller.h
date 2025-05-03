#ifndef ALTCONTROLLER_H
#define ALTCONTROLLER_H

#include <stdbool.h>

REFKEEN_NS_B

extern int g_binding_value_button[2], g_binding_value_stats, g_binding_value_grenade,
           g_binding_value_lookup, g_binding_value_lookdown, g_binding_value_scorebox,
           g_binding_value_axisx, g_binding_value_axisy,
           g_binding_value_up, g_binding_value_down, g_binding_value_left, g_binding_value_right,
           g_binding_value_drink, g_binding_value_bolt, g_binding_value_nuke, g_binding_value_fastturn;

// Used with keyboard overrides
extern bool g_keybind_used_button[2], g_keybind_used_stats, g_keybind_used_grenade,
            g_keybind_used_lookup, g_keybind_used_lookdown, g_keybind_used_scorebox,
            g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right,
            g_keybind_used_drink, g_keybind_used_bolt, g_keybind_used_nuke, g_keybind_used_fastturn;

extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback;

void PrepareGamePlayControllerMapping(void);
void UpdateAltControllerMappingsByMousePresence(bool withmouse);

REFKEEN_NS_E

#endif
