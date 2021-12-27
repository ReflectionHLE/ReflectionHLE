#ifndef BE_INPUT_KEYTABLES_H
#define BE_INPUT_KEYTABLES_H

#include "be_st.h"

// Offsets are required for default empty strings,
// along with null terminators for the launcher
extern const char *g_be_st_keyIdToNameMap[2+BE_MAX_KEY_ID];
extern const char *g_be_st_mouseFeatureIdToNameMap[2+BE_ST_CTRL_MOUSE_BUT_MAX];

void BEL_ST_InitKeyMap(void);

#endif

