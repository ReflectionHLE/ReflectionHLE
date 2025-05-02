/* This header must be included *only* from be_gamedefs_bmenace.h */

void RefKeen_Patch_id_rf(void);
void RefKeen_Patch_id_us(void);

// Assumed to be a part of a namespace which depends on the game version
static void (*g_be_patcherfuncs[])(void) = {
	RefKeen_Patch_id_rf, RefKeen_Patch_id_us, 0
};

void bmenace_exe_main(void);
void RefKeen_Load_Embedded_Resources_From_bmenace_exe(void);
