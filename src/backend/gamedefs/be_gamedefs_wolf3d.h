/* This header must be included *only* from be_gamedefs.h */

#ifdef REFKEEN_HAS_VER_WL920312
#include "be_gamedefs_wl920312.h"
#endif
#ifdef REFKEEN_HAS_VER_WL1AP10
#include "be_gamedefs_wl1ap10.h"
#endif
#ifdef REFKEEN_HAS_VER_WL1AP11
#include "be_gamedefs_wl1ap11.h"
#endif
#ifdef REFKEEN_HAS_VER_WL6AP11
#include "be_gamedefs_wl6ap11.h"
#endif
#ifdef REFKEEN_HAS_VER_WL1AP14
#include "be_gamedefs_wl1ap14.h"
#endif
#ifdef REFKEEN_HAS_VER_WL6GT14A
#include "be_gamedefs_wl6gt14a_id14.h"
#endif
#ifdef REFKEEN_HAS_VER_WJ6IM14
#include "be_gamedefs_wj6im14.h"
#endif
#ifdef REFKEEN_HAS_VER_WL6GT14B
#include "be_gamedefs_wl6gt14b.h"
#endif
#ifdef REFKEEN_HAS_VER_WL6AC14
#include "be_gamedefs_wl6ac14.h"
#endif
#ifdef REFKEEN_HAS_VER_SDMFG10
#include "be_gamedefs_sdmfg10.h"
#endif
#ifdef REFKEEN_HAS_VER_SODFG10
#include "be_gamedefs_sodfg10.h"
#endif
#ifdef REFKEEN_HAS_VER_SODFG14
#include "be_gamedefs_sodfg14.h"
#endif
#ifdef REFKEEN_HAS_VER_SODAC14
#include "be_gamedefs_sodac14.h"
#endif
#ifdef REFKEEN_HAS_VER_N3DWT10
#include "be_gamedefs_n3dwt10.h"
#endif

static const BE_GameVerDetails_T *g_be_gamever_ptrs_wolf3d_all[] = {
#ifdef REFKEEN_HAS_VER_WL920312
	&g_be_gamever_wl920312,
#endif
#ifdef REFKEEN_HAS_VER_WL1AP10
	&g_be_gamever_wl1ap10,
#endif
#ifdef REFKEEN_HAS_VER_WL1AP11
	&g_be_gamever_wl1ap11,
#endif
#ifdef REFKEEN_HAS_VER_WL6AP11
	&g_be_gamever_wl6ap11,
	&g_be_gamever_wl1ap12,
	&g_be_gamever_wl3ap12,
	&g_be_gamever_wl6ap12,
#endif
#ifdef REFKEEN_HAS_VER_WL1AP14
	&g_be_gamever_wl1ap14,
	&g_be_gamever_wl3ap14,
	&g_be_gamever_wl6ap14,
#endif
#ifdef REFKEEN_HAS_VER_WL6GT14A
	&g_be_gamever_wl6gt14a,
	&g_be_gamever_wl6id14,
#endif
#ifdef REFKEEN_HAS_VER_WJ6IM14
	&g_be_gamever_wj6im14,
#endif
#ifdef REFKEEN_HAS_VER_WL6GT14B
	&g_be_gamever_wl6gt14b,
#endif
#ifdef REFKEEN_HAS_VER_WL6AC14
	&g_be_gamever_wl6ac14,
#endif
#ifdef REFKEEN_HAS_VER_SDMFG10
	&g_be_gamever_sdmfg10,
#endif
#ifdef REFKEEN_HAS_VER_SODFG10
	&g_be_gamever_sodfg10,
	&g_be_gamever_sd2fg10,
	&g_be_gamever_sd3fg10,
#endif
#ifdef REFKEEN_HAS_VER_SODFG14
	&g_be_gamever_sodfg14,
	&g_be_gamever_sd2fg14,
	&g_be_gamever_sd3fg14,
#endif
#ifdef REFKEEN_HAS_VER_SODAC14
	&g_be_gamever_sodac14,
	&g_be_gamever_sd2ac14,
	&g_be_gamever_sd3ac14,
#endif
#ifdef REFKEEN_HAS_VER_N3DWT10
	&g_be_gamever_n3dwt10,
#endif
};

static const BE_GameDetails_T g_be_game_wolf3d_all = {
	"Wolf3D/SOD/S3DNA",
	g_be_gamever_ptrs_wolf3d_all,
	BE_Cross_ArrayLen(g_be_gamever_ptrs_wolf3d_all),
	BE_GAME_WOLF3D_ALL
};
