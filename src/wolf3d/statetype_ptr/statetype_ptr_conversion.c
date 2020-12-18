/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
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

// New file used for preparing and converting pointers to states

#include "wl_def.h"

REFKEEN_NS_B

statetype *RefKeen_GetObjStatePtrFromDOSPointer_WL1(uint_fast32_t dosptr);
statetype *RefKeen_GetObjStatePtrFromDOSPointer_NonWL1(uint_fast32_t dosptr);

statetype * (*RefKeen_GetObjStatePtrFromDOSPointer)(uint_fast32_t dosptr);

void RefKeen_FillObjStatesWithDOSPointers_WL1(void);
void RefKeen_FillObjStatesWithDOSPointers_NonWL1(void);

void RefKeen_FillObjStatesWithDOSPointers(void)
{
#if (defined REFKEEN_VER_WL920312) || \
    (defined REFKEEN_VER_WL1AP10) || \
    (defined REFKEEN_VER_WL1AP11)
	// WL1 only
	RefKeen_GetObjStatePtrFromDOSPointer = RefKeen_GetObjStatePtrFromDOSPointer_WL1;
	RefKeen_FillObjStatesWithDOSPointers_WL1();
#elif (defined REFKEEN_VER_WL6AP11) || (defined REFKEEN_VER_WL1AP14)
	// Multiple WL1/WL3/WL6 variants under common macros
	if (UPLOAD)
	{
		RefKeen_GetObjStatePtrFromDOSPointer = RefKeen_GetObjStatePtrFromDOSPointer_WL1;
		RefKeen_FillObjStatesWithDOSPointers_WL1();
	}
	else
	{
		RefKeen_GetObjStatePtrFromDOSPointer = RefKeen_GetObjStatePtrFromDOSPointer_NonWL1;
		RefKeen_FillObjStatesWithDOSPointers_NonWL1();
	}
#else
	// Non-WL1 only
	RefKeen_GetObjStatePtrFromDOSPointer = RefKeen_GetObjStatePtrFromDOSPointer_NonWL1;
	RefKeen_FillObjStatesWithDOSPointers_NonWL1();
#endif
}

REFKEEN_NS_E
