#ifndef ALTCONTROLLER_H
#define ALTCONTROLLER_H

#include <stdbool.h>

REFKEEN_NS_B

extern int g_binding_value_button[],
       g_binding_value_motionx, g_binding_value_motiony;

void PrepareGamePlayControllerMapping(void);
void UpdateAltControllerMappingsByMousePresence(bool withmouse);

REFKEEN_NS_E

#endif
