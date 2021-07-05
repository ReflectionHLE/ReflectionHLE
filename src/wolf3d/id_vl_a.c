/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2021 NY00123
 *
 * This file is part of Wolfenstein 3D.
 *
 * Wolfenstein 3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Wolfenstein 3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an addendum, you can further use the Wolfenstein 3D Source Code under
 * the terms of the limited use software license agreement for Wolfenstein 3D.
 * See id-wolf3d.txt for these terms.
 */

#include "id_heads.h"

// Ported from ASM
REFKEEN_NS_B

//==============
//
// VL_WaitVBL
//
// Wait for the vertical retrace (returns before the actual vertical sync)
//
//==============

// TODO (REFKEEN): Use this for somewhat faster rates in the revision
// of VW_WaitVBL from Wolf3D v1.1 and earlier, albeit this isn't
// exactly accurate. Same about the wait in VL_SetScreen.
static unsigned g_waitVblCounter;

void VL_WaitVBL (id0_int_t vbls)
{
	// TODO (REFKEEN): Emulate behaviors for number < 0?
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	int i = 0;
	while (i < vbls)
		if ((++g_waitVblCounter) & 3)
			--vbls;
		else
			++i;
#endif
	BE_ST_WaitForNewVerticalRetraces(vbls);
}

//==============
//
// VL_SetCRTC
//
//==============

void VL_SetCRTC (id0_int_t crtc)
{
	// REFKEEN (TODO): We skip wait for display signal here
	BE_ST_SetScreenStartAddress(crtc);
}

//==============
//
// VL_SetScreen
//
//==============

void VL_SetScreen (id0_int_t crtc, id0_int_t pel)
{
	if (!((++g_waitVblCounter) & 3))
		BE_ST_WaitForNewVerticalRetraces(1);
	BE_ST_SetScreenStartAddress(crtc);
	BE_ST_EGASetPelPanning(pelpan);
}

//============================================================================
//
// VL_ScreenToScreen
//
// Basic block copy routine.  Copies one block of screen memory to another,
// using write mode 1 (sets it and returns with write mode 0).  bufferofs is
// NOT accounted for.
//
//============================================================================

void VL_ScreenToScreen(id0_word_t source, id0_word_t dest,
	id0_int_t wide, id0_int_t height)
{
	for (id0_word_t lineCounter = height; lineCounter; --lineCounter, source += linewidth, dest += linewidth)
		BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(dest, source, wide);
}

//=========================================================
//
// Name:	VL_VideoID
//
// Function:	Detects the presence of various video subsystems
//
// int VideoID;
//
// Subsystem ID values:
// 	 0  = (none)
// 	 1  = MDA
// 	 2  = CGA
// 	 3  = EGA
// 	 4  = MCGA
// 	 5  = VGA
// 	80h = HGC
// 	81h = HGC+
// 	82h = Hercules InColor
//
//=========================================================

id0_int_t VL_VideoID (void)
{
	return 5/*VGA*/;
}

REFKEEN_NS_E
