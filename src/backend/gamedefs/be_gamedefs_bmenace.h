/* This header must be included *only* from be_gamedefs.h */

#ifdef REFKEEN_HAS_VER_BM1V11S
#include "be_gamedefs_bm1v11s.h"
#endif
#ifdef REFKEEN_HAS_VER_BM1V11R
#include "be_gamedefs_bm1v11r.h"
#endif
#ifdef REFKEEN_HAS_VER_BM2V11R
#include "be_gamedefs_bm2v11r.h"
#endif
#ifdef REFKEEN_HAS_VER_BM3V11R
#include "be_gamedefs_bm3v11r.h"
#endif
#ifdef REFKEEN_HAS_VER_BM1V11GM
#include "be_gamedefs_bm1v11gm.h"
#endif
#ifdef REFKEEN_HAS_VER_BM1V11F
#include "be_gamedefs_bm1v11f.h"
#endif
#ifdef REFKEEN_HAS_VER_BM2V11F
#include "be_gamedefs_bm2v11f.h"
#endif
#ifdef REFKEEN_HAS_VER_BM3V11F
#include "be_gamedefs_bm3v11f.h"
#endif

static const BE_GameVerDetails_T *g_be_gamever_ptrs_bmenace_all[] = {
#ifdef REFKEEN_HAS_VER_BM1V11S
	&g_be_gamever_bm1v11s,
#endif
#ifdef REFKEEN_HAS_VER_BM1V11R
	&g_be_gamever_bm1v11r,
#endif
#ifdef REFKEEN_HAS_VER_BM2V11R
	&g_be_gamever_bm2v11r,
#endif
#ifdef REFKEEN_HAS_VER_BM3V11R
	&g_be_gamever_bm3v11r,
#endif
#ifdef REFKEEN_HAS_VER_BM1V11GM
	&g_be_gamever_bm1v11gm,
#endif
#ifdef REFKEEN_HAS_VER_BM1V11F
	&g_be_gamever_bm1v11f,
#endif
#ifdef REFKEEN_HAS_VER_BM2V11F
	&g_be_gamever_bm2v11f,
#endif
#ifdef REFKEEN_HAS_VER_BM3V11F
	&g_be_gamever_bm3v11f,
#endif
};

static const BE_GameDetails_T g_be_game_bmenace_all = {
	"BioMenace",
	g_be_gamever_ptrs_bmenace_all,
	BE_Cross_ArrayLen(g_be_gamever_ptrs_bmenace_all),
	BE_GAME_BMENACE_ALL
};
