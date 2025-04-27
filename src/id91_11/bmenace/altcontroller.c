// TODO: This should be implemented later

#include "id_heads.h"

REFKEEN_NS_B

int g_binding_value_button[2],
    g_binding_value_axisx, g_binding_value_axisy,
    g_binding_value_up, g_binding_value_down, g_binding_value_left, g_binding_value_right,
    g_binding_value_drink, g_binding_value_bolt, g_binding_value_nuke, g_binding_value_fastturn;

bool g_keybind_used_button[2],
     g_keybind_used_up, g_keybind_used_down, g_keybind_used_left, g_keybind_used_right,
     g_keybind_used_drink, g_keybind_used_bolt, g_keybind_used_nuke, g_keybind_used_fastturn;

void UpdateAltControllerMappingsByMousePresence(bool withmouse) {}

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu = {
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_menu_confirm = {
};

BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback = {
};

REFKEEN_NS_E
