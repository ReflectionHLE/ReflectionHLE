/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020 NY00123
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

extern id0_byte_t update[]; // Was in wl_def.h instead of an id header

void VH_UpdateScreen (void)
{
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) // *** SHAREWARE V1.0 APOGEE RESTORATION ***
	id0_int_t tilenum = UPDATEWIDE*(UPDATEHIGH-1)-1;
#else
	id0_int_t tilenum = UPDATEWIDE*UPDATEHIGH-1;
#endif

	do
	{
		// see if the tile needs to be copied
		if (update[tilenum] & 1)
		{
			update[tilenum] = 0;
			id0_word_t srcOff = blockstarts[tilenum] + bufferofs;
			id0_word_t destOff = blockstarts[tilenum] + displayofs;
			for (int i = 15; i; --i)
			{
				BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(destOff, srcOff, 4);
				destOff += linewidth;
				srcOff += linewidth;
			}
		}
	}
	while (--tilenum >= 0);
}

// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
// A copy-paste of VH_UpdateScreen for most

#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
void VH_UpdateSplitScreen (void)
{
	id0_int_t tilenum = 218-1;

	do
	{
		// see if the tile needs to be copied
		if (update[tilenum] & 1)
		{
			update[tilenum] = 0;
			id0_word_t srcOff = blockstarts[tilenum] + bufferofs;
			id0_word_t destOff = blockstarts[tilenum] + displayofs;
			for (int i = 15; i; --i)
			{
				BE_ST_VGAUpdateGFXBufferInAllPlanesScrToScr(destOff, srcOff, 4);
				destOff += linewidth;
				srcOff += linewidth;
			}
		}
	}
	while (--tilenum >= 0);
}
#endif // GAMEVER_WOLFREV

REFKEEN_NS_E
