/* Copyright (C) 2014-2021 NY00123
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "refkeen_config.h"
#include "../filesystem/be_filesystem_gameinst.h"
#include "../filesystem/be_filesystem_file_ops.h"
#include "be_features.h"
#include "be_gamever.h"
#include "be_startup.h"
#include "be_st.h"

#include "unlzexe/unlzexe.h"
#ifdef ENABLE_PKLITE
#include "depklite/depklite.h"
#endif

#include <setjmp.h>

unsigned char *g_be_current_exeImage;
BE_EXEFileDetails_T const *g_be_current_exeFileDetails;

// gameVer should be BE_GAMEVER_LAST if no specific version is desired
// FIXME: See BE_Cross_InitGame for explanation about setVerOnly
static void BEL_Cross_SelectGameInstallation(int gameVerVal, bool setVerOnly)
{
	if (gameVerVal == BE_GAMEVER_LAST)
	{
		if (!g_be_gameinstallations_num)
		{
			if (setVerOnly)
				return;
			BE_ST_ExitWithErrorMsg("BEL_Cross_SelectGameInstallation: No compatible game installation found!");
		}
		g_be_selectedGameInstallation = &g_be_gameinstallations[0];
	}
	else
	{
		g_be_selectedGameInstallation = g_be_gameinstallationsbyver[gameVerVal];
		if (!g_be_selectedGameInstallation)
		{
			char errorBuffer[80];
			if (setVerOnly)
				return;
			BE_Cross_safeandfastcstringcopy_2strs(errorBuffer, errorBuffer+sizeof(errorBuffer), "BEL_Cross_SelectGameInstallation: Can't find game installation: ",  refkeen_gamever_strs[gameVerVal]);
			BE_ST_ExitWithErrorMsg(errorBuffer);
		}
	}

	if (setVerOnly)
	{
		refkeen_current_gamever = g_be_selectedGameInstallation->verId;
		return;
	}
	g_refKeenCfg.lastSelectedGameVer = refkeen_current_gamever = g_be_selectedGameInstallation->verId;

	for (void (**patcherFuncPtr)(void) = g_be_gamever_ptrs[refkeen_current_gamever]->patcherFuncPtrs;
             *patcherFuncPtr; ++patcherFuncPtr)
		(*patcherFuncPtr)();
}

// Fill g_refKeenDynamicCfg based on the selected game version
static void BEL_Cross_SetDynamicCfg(void)
{
	switch (g_be_gamever_ptrs[refkeen_current_gamever]->gameId)
	{
#ifdef REFKEEN_HAS_VER_KDREAMS
	case BE_GAME_KDREAMS:
		g_refKeenDynamicCfg.absMouseMotion = g_refKeenCfg.absMouseMotion;
		g_refKeenDynamicCfg.novert = false;
		break;
		break;
#endif
#ifdef REFKEEN_HAS_VER_CATACOMB_ALL
	case BE_GAME_CATACOMB_ALL:
		g_refKeenDynamicCfg.absMouseMotion = false;
		g_refKeenDynamicCfg.novert = g_refKeenCfg.novert;
		break;
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
	case BE_GAME_WOLF3D_ALL:
		g_refKeenDynamicCfg.absMouseMotion = false;
		g_refKeenDynamicCfg.novert = g_refKeenCfg.novert;
		break;
#endif
	}
}

// Here the magic happens - used to clear a portion of the stack before
// changing to another "main" function (in case we get a loop).
// In a way, this should be similar to C++ exception handling,
// just C-compatible.
/*static*/ jmp_buf g_be_mainFuncJumpBuffer;

static void BEL_Cross_LoadEXEImageToMem(void)
{
	char errorMsg[256];
	BE_FILE_T exeFp =
		BEL_Cross_OpenMatchingGameFileForReading(
			g_be_current_exeFileDetails->exeNames,
			g_be_selectedGameInstallation->instPath);
	if (!exeFp)
	{
		snprintf(
			errorMsg, sizeof(errorMsg),
			"BEL_Cross_LoadEXEImageToMem: Can't open EXE after checking it!\n"
			"Possible filenames: %s", g_be_current_exeFileDetails->exeNames);
		BE_ST_ExitWithErrorMsg(errorMsg);
	}

	// FIXME - Use BE_Cross_Bmalloc/farmalloc and shuffle things around
	g_be_current_exeImage = (unsigned char *)malloc(g_be_current_exeFileDetails->decompExeImageSize);
	if (!g_be_current_exeImage)
	{
		fclose(exeFp);
		snprintf(
			errorMsg, sizeof(errorMsg),
			"BEL_Cross_LoadEXEImageToMem: Can't allocate memory for unpacked EXE copy!\n"
			"Possible filename: %s", g_be_current_exeFileDetails->exeNames);
		BE_ST_ExitWithErrorMsg(errorMsg);
	}

	bool success;
	switch (g_be_current_exeFileDetails->compressionType)
	{
	case BE_EXECOMPRESSION_NONE:
	{
		uint16_t offsetInPages;
		fseek(exeFp, 8, SEEK_SET);
		BE_Cross_readInt16LE(exeFp, &offsetInPages);
		fseek(exeFp, 16*offsetInPages, SEEK_SET);
		success = (fread(g_be_current_exeImage, g_be_current_exeFileDetails->decompExeImageSize, 1, exeFp) == 1);
		break;
	}
	case BE_EXECOMPRESSION_LZEXE9X:
		success = Unlzexe_unpack(exeFp, g_be_current_exeImage, g_be_current_exeFileDetails->decompExeImageSize);
		break;
#ifdef ENABLE_PKLITE
	case BE_EXECOMPRESSION_PKLITE105:
		success = depklite_unpack(exeFp, g_be_current_exeImage, g_be_current_exeFileDetails->decompExeImageSize);
		break;
#endif
	}

	fclose(exeFp);
	if (!success)
	{
		free(g_be_current_exeImage);
		snprintf(
			errorMsg, sizeof(errorMsg),
			"decompExeImage: Failed to copy EXE in unpacked form!\n"
			"Filename: %s", g_be_current_exeFileDetails->exeNames);
		BE_ST_ExitWithErrorMsg(errorMsg);
	}
}

static void BEL_Cross_FreeEXEImageFromMem(void)
{
	free(g_be_current_exeImage);
}

static void BEL_Cross_DoCallMainFunc(void)
{
	setjmp(g_be_mainFuncJumpBuffer); // Ignore returned value, always doing the same thing

	// Do start!
	BE_ST_AltControlScheme_Reset();

	if (g_be_current_exeFileDetails->embeddedFiles)
	{
		BEL_Cross_LoadEXEImageToMem();
		g_be_current_exeFileDetails->embeddedFilesLoaderFuncPtr();
		BEL_Cross_FreeEXEImageFromMem();
	}

	be_lastSetMainFuncPtr = g_be_current_exeFileDetails->mainFuncPtr;
	if (g_be_current_exeFileDetails->passArgsToMainFunc)
		((void (*)(int, const char **))be_lastSetMainFuncPtr)(g_be_argc, g_be_argv); // HACK
	else
		be_lastSetMainFuncPtr();
}

// When a new main function is called in the middle (BE_Cross_Bexecv),
// by default the current (non-new) main function is stored as
// a "one time" function that shall *never* be called again.
//
// To prevent this, BE_Cross_Bexecv should get a finalizer function pointer,
// used to reset the sub-program as represented by current main function
// to its original state. (Emphasis on global and static variables.)
#define MAX_NUM_OF_ONE_TIME_MAIN_FUNCS 4

static int g_be_numOfOneTimeMainFuncs = 0;
static void (*g_be_oneTimeMainFuncs[MAX_NUM_OF_ONE_TIME_MAIN_FUNCS])(void);

void BE_Cross_Bexecv(void (*mainFunc)(void), const char **argv, void (*finalizer)(void), bool passArgsToMainFunc)
{
	for (int i = 0; i < g_be_numOfOneTimeMainFuncs; ++i)
		if (g_be_oneTimeMainFuncs[i] == mainFunc)
			BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - One-time function unexpectedly called again!");

	if (finalizer)
		finalizer();
	else
	{
		if (g_be_numOfOneTimeMainFuncs == MAX_NUM_OF_ONE_TIME_MAIN_FUNCS)
			BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - Too many one-time functions called!");
		g_be_oneTimeMainFuncs[g_be_numOfOneTimeMainFuncs++] = be_lastSetMainFuncPtr;
	}

	// Note this does NOT work for memory not managed by us (e.g., simple calls to malloc)
	void BEL_Cross_ClearMemory(void);
	BEL_Cross_ClearMemory();

	g_be_argv = argv;
	for (g_be_argc = 0; *argv; ++g_be_argc, ++argv)
		;

	// Locate the right EXE
	for (g_be_current_exeFileDetails = g_be_gamever_ptrs[refkeen_current_gamever]->exeFiles; g_be_current_exeFileDetails->mainFuncPtr && (g_be_current_exeFileDetails->mainFuncPtr != mainFunc); ++g_be_current_exeFileDetails)
		;
	if (!g_be_current_exeFileDetails->mainFuncPtr)
		BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - Unrecognized main function!");

	longjmp(g_be_mainFuncJumpBuffer, 0); // A little bit of magic
}

void BE_Cross_InitGame(int gameVerVal)
{
	// FIXME: We have a problem:
	// 1. BE_ST_PrepareForGameStartupWithoutAudio should theoretically be
	// called before BEL_Cross_SelectGameInstallation, so an error message
	// can be displayed if no matching game version is found.
	// 2. On the other hand, as of writing this, we first need to set
	// refkeen_current_gamever via BEL_Cross_SelectGameInstallation.
	// Reason is that before the call to BE_ST_PrepareForGameStartupWithoutAudio
	// is completed, this version may currently determine if VSync is enabled
	// when AUTO is chosen (it's disabled for Keen Dreams with CGA graphics).
	BEL_Cross_SelectGameInstallation(gameVerVal, true);
	BE_ST_PrepareForGameStartupWithoutAudio(); // Some additional preparation required
	BEL_Cross_SelectGameInstallation(gameVerVal, false);
	BEL_Cross_SetDynamicCfg();
	BE_ST_InitAudio(); // Do this now, since we can tell if we want digi audio out or not
}

void BE_Cross_StartGame(int argc, char **argv, void (*mainFuncPtr)(void))
{
	// Prepare arguments for ported game code
	g_be_argc = argc;
	// HACK: In Keen Dreams CGA v1.05, even if argc == 1, argv[1] is accessed...
	// Furthermore, in Keen Dreams Shareware v1.13, argc, argv[1], argv[2] and argv[3] are all modified...
	// And then in Catacomb Abyss, argv[3] is compared to "1". In its INTROSCN.EXE argv[4] is compared...

	// REFKEEN - As long as argv[0] isn't used, use a placeholder
	const char *our_workaround_argv[] = { "PROG.EXE", "", "", "", "", "", "", "", "", NULL };
	if (argc < 10)
	{
		for (int currarg = 1; currarg < argc; ++currarg)
		{
			our_workaround_argv[currarg] = argv[currarg];
		}
		g_be_argv = our_workaround_argv;
	}
	else
	{
		// REFKEEN - Hack, but we don't access argv directly anyway...
		g_be_argv = (const char **)argv;
	}

	// Locate the right EXE
	if (!mainFuncPtr)
		mainFuncPtr = g_be_gamever_ptrs[refkeen_current_gamever]->exeFiles->mainFuncPtr;

	for (g_be_current_exeFileDetails = g_be_gamever_ptrs[refkeen_current_gamever]->exeFiles; g_be_current_exeFileDetails->mainFuncPtr && (g_be_current_exeFileDetails->mainFuncPtr != mainFuncPtr); ++g_be_current_exeFileDetails)
		;
	if (!g_be_current_exeFileDetails->mainFuncPtr)
		BE_ST_ExitWithErrorMsg("BE_Cross_StartGame - Unrecognized main function!");

	snprintf(
		g_refKeenCfg.lastSelectedGameExe, sizeof(g_refKeenCfg.lastSelectedGameExe), "%s",
		g_be_current_exeFileDetails->exeNames ? g_be_current_exeFileDetails->exeNames : "");

	BEL_Cross_DoCallMainFunc(); // Do a bit more preparation and then begin
}
