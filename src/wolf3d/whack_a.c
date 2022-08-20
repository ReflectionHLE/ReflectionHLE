/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2022 NY00123
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

#include "wl_def.h"

// Ported from ASM

#ifdef GAMEVER_NOAH3D // REFKEEN: Limit compiled code to S3DNA only for now
REFKEEN_NS_B

id0_char_t planepics[8192]; // 4k of ceiling, 4k of floor

extern id0_int_t mr_rowofs;
extern id0_int_t mr_count;
extern id0_int_t mr_xstep;
extern id0_int_t mr_ystep;
extern id0_int_t mr_xfrac;
extern id0_int_t mr_yfrac;
extern id0_int_t mr_dest;

//============================
//
// MapRow
//
//============================

void MapRow(int plane) // REFKEEN: We need the VGA plane
//void MapRow(void)
{
	id0_word_t dest = mr_dest, src, i;
	id0_longword_t xystep = (mr_xstep & 0xFFFF) | (mr_ystep << 16),
	               xyfrac = (mr_xfrac & 0xFFFF) | (mr_yfrac << 16);

	// REFKEEN: Originally this went for 1 iteration or more.
	// It should still be the case, since MapRow isn't called with mr_count==0.
	for (i = mr_count; i; --i, ++dest)
	{
		src = ((((xyfrac&0xFFFF) >> 3) & (63*64*2)) | ((xyfrac>>16) >> 9)) & (63*65*2);
		xyfrac += xystep;
		BE_ST_VGAUpdateGFXByteInPlane(dest, planepics[src], plane);
		BE_ST_VGAUpdateGFXByteInPlane(dest+mr_rowofs, planepics[src+1], plane);
	}
}

REFKEEN_NS_E
#endif
