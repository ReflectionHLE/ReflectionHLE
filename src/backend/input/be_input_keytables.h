#ifndef BE_INPUT_KEYTABLES_H
#define BE_INPUT_KEYTABLES_H

enum { BE_MAX_KEY_ID = 250 };

extern const char *g_be_st_keyIdToNameMap[BE_MAX_KEY_ID];

void BEL_ST_InitKeyMap(void);

#endif

