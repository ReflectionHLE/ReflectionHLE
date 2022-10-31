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

#ifdef REFKEEN_VER_WL920312
REFKEEN_NS_B

// List of all states
extern statetype s_player;
extern statetype s_attack;
extern statetype s_grdstand;
extern statetype s_grdpath1;
extern statetype s_grdpath1s;
extern statetype s_grdpath2;
extern statetype s_grdpath3;
extern statetype s_grdpath3s;
extern statetype s_grdpath4;
extern statetype s_grdpain;
extern statetype s_grdshoot1;
extern statetype s_grdshoot2;
extern statetype s_grdshoot3;
extern statetype s_grdchase1;
extern statetype s_grdchase1s;
extern statetype s_grdchase2;
extern statetype s_grdchase3;
extern statetype s_grdchase3s;
extern statetype s_grdchase4;
extern statetype s_grddie1;
extern statetype s_grddie2;
extern statetype s_grddie3;
extern statetype s_grddie4;
extern statetype s_dogpath1;
extern statetype s_dogpath1s;
extern statetype s_dogpath2;
extern statetype s_dogpath3;
extern statetype s_dogpath3s;
extern statetype s_dogpath4;
extern statetype s_dogjump1;
extern statetype s_dogjump2;
extern statetype s_dogjump3;
extern statetype s_dogjump4;
extern statetype s_dogjump5;
extern statetype s_dogchase1;
extern statetype s_dogchase1s;
extern statetype s_dogchase2;
extern statetype s_dogchase3;
extern statetype s_dogchase3s;
extern statetype s_dogchase4;
extern statetype s_dogdie1;
extern statetype s_dogdie2;
extern statetype s_dogdie3;
extern statetype s_dogdead;
extern statetype s_ssstand;
extern statetype s_sspath1;
extern statetype s_sspath1s;
extern statetype s_sspath2;
extern statetype s_sspath3;
extern statetype s_sspath3s;
extern statetype s_sspath4;
extern statetype s_sspain;
extern statetype s_ssshoot1;
extern statetype s_ssshoot2;
extern statetype s_ssshoot3;
extern statetype s_ssshoot4;
extern statetype s_ssshoot5;
extern statetype s_ssshoot6;
extern statetype s_ssshoot7;
extern statetype s_ssshoot8;
extern statetype s_ssshoot9;
extern statetype s_sschase1;
extern statetype s_sschase1s;
extern statetype s_sschase2;
extern statetype s_sschase3;
extern statetype s_sschase3s;
extern statetype s_sschase4;
extern statetype s_ssdie1;
extern statetype s_ssdie2;
extern statetype s_ssdie3;
extern statetype s_ssdie4;
extern statetype s_bossstand;

void RefKeen_FillObjStatesWithDOSPointers_WL1(void)
{
	s_player.compatdospointer = 0x1546;
	s_attack.compatdospointer = 0x1556;
	s_grdstand.compatdospointer = 0x178C;
	s_grdpath1.compatdospointer = 0x179C;
	s_grdpath1s.compatdospointer = 0x17AC;
	s_grdpath2.compatdospointer = 0x17BC;
	s_grdpath3.compatdospointer = 0x17CC;
	s_grdpath3s.compatdospointer = 0x17DC;
	s_grdpath4.compatdospointer = 0x17EC;
	s_grdpain.compatdospointer = 0x17FC;
	s_grdshoot1.compatdospointer = 0x180C;
	s_grdshoot2.compatdospointer = 0x181C;
	s_grdshoot3.compatdospointer = 0x182C;
	s_grdchase1.compatdospointer = 0x183C;
	s_grdchase1s.compatdospointer = 0x184C;
	s_grdchase2.compatdospointer = 0x185C;
	s_grdchase3.compatdospointer = 0x186C;
	s_grdchase3s.compatdospointer = 0x187C;
	s_grdchase4.compatdospointer = 0x188C;
	s_grddie1.compatdospointer = 0x189C;
	s_grddie2.compatdospointer = 0x18AC;
	s_grddie3.compatdospointer = 0x18BC;
	s_grddie4.compatdospointer = 0x18CC;
	s_dogpath1.compatdospointer = 0x18DC;
	s_dogpath1s.compatdospointer = 0x18EC;
	s_dogpath2.compatdospointer = 0x18FC;
	s_dogpath3.compatdospointer = 0x190C;
	s_dogpath3s.compatdospointer = 0x191C;
	s_dogpath4.compatdospointer = 0x192C;
	s_dogjump1.compatdospointer = 0x193C;
	s_dogjump2.compatdospointer = 0x194C;
	s_dogjump3.compatdospointer = 0x195C;
	s_dogjump4.compatdospointer = 0x196C;
	s_dogjump5.compatdospointer = 0x197C;
	s_dogchase1.compatdospointer = 0x198C;
	s_dogchase1s.compatdospointer = 0x199C;
	s_dogchase2.compatdospointer = 0x19AC;
	s_dogchase3.compatdospointer = 0x19BC;
	s_dogchase3s.compatdospointer = 0x19CC;
	s_dogchase4.compatdospointer = 0x19DC;
	s_dogdie1.compatdospointer = 0x19EC;
	s_dogdie2.compatdospointer = 0x19FC;
	s_dogdie3.compatdospointer = 0x1A0C;
	s_dogdead.compatdospointer = 0x1A1C;
	s_ssstand.compatdospointer = 0x1A2C;
	s_sspath1.compatdospointer = 0x1A3C;
	s_sspath1s.compatdospointer = 0x1A4C;
	s_sspath2.compatdospointer = 0x1A5C;
	s_sspath3.compatdospointer = 0x1A6C;
	s_sspath3s.compatdospointer = 0x1A7C;
	s_sspath4.compatdospointer = 0x1A8C;
	s_sspain.compatdospointer = 0x1A9C;
	s_ssshoot1.compatdospointer = 0x1AAC;
	s_ssshoot2.compatdospointer = 0x1ABC;
	s_ssshoot3.compatdospointer = 0x1ACC;
	s_ssshoot4.compatdospointer = 0x1ADC;
	s_ssshoot5.compatdospointer = 0x1AEC;
	s_ssshoot6.compatdospointer = 0x1AFC;
	s_ssshoot7.compatdospointer = 0x1B0C;
	s_ssshoot8.compatdospointer = 0x1B1C;
	s_ssshoot9.compatdospointer = 0x1B2C;
	s_sschase1.compatdospointer = 0x1B3C;
	s_sschase1s.compatdospointer = 0x1B4C;
	s_sschase2.compatdospointer = 0x1B5C;
	s_sschase3.compatdospointer = 0x1B6C;
	s_sschase3s.compatdospointer = 0x1B7C;
	s_sschase4.compatdospointer = 0x1B8C;
	s_ssdie1.compatdospointer = 0x1B9C;
	s_ssdie2.compatdospointer = 0x1BAC;
	s_ssdie3.compatdospointer = 0x1BBC;
	s_ssdie4.compatdospointer = 0x1BCC;
	s_bossstand.compatdospointer = 0x1BDC;
}

statetype *RefKeen_GetObjStatePtrFromDOSPointer_WL1(uint_fast32_t dosptr)
{
	switch (dosptr)
	{
	case 0x1546: return &s_player;
	case 0x1556: return &s_attack;
	case 0x178C: return &s_grdstand;
	case 0x179C: return &s_grdpath1;
	case 0x17AC: return &s_grdpath1s;
	case 0x17BC: return &s_grdpath2;
	case 0x17CC: return &s_grdpath3;
	case 0x17DC: return &s_grdpath3s;
	case 0x17EC: return &s_grdpath4;
	case 0x17FC: return &s_grdpain;
	case 0x180C: return &s_grdshoot1;
	case 0x181C: return &s_grdshoot2;
	case 0x182C: return &s_grdshoot3;
	case 0x183C: return &s_grdchase1;
	case 0x184C: return &s_grdchase1s;
	case 0x185C: return &s_grdchase2;
	case 0x186C: return &s_grdchase3;
	case 0x187C: return &s_grdchase3s;
	case 0x188C: return &s_grdchase4;
	case 0x189C: return &s_grddie1;
	case 0x18AC: return &s_grddie2;
	case 0x18BC: return &s_grddie3;
	case 0x18CC: return &s_grddie4;
	case 0x18DC: return &s_dogpath1;
	case 0x18EC: return &s_dogpath1s;
	case 0x18FC: return &s_dogpath2;
	case 0x190C: return &s_dogpath3;
	case 0x191C: return &s_dogpath3s;
	case 0x192C: return &s_dogpath4;
	case 0x193C: return &s_dogjump1;
	case 0x194C: return &s_dogjump2;
	case 0x195C: return &s_dogjump3;
	case 0x196C: return &s_dogjump4;
	case 0x197C: return &s_dogjump5;
	case 0x198C: return &s_dogchase1;
	case 0x199C: return &s_dogchase1s;
	case 0x19AC: return &s_dogchase2;
	case 0x19BC: return &s_dogchase3;
	case 0x19CC: return &s_dogchase3s;
	case 0x19DC: return &s_dogchase4;
	case 0x19EC: return &s_dogdie1;
	case 0x19FC: return &s_dogdie2;
	case 0x1A0C: return &s_dogdie3;
	case 0x1A1C: return &s_dogdead;
	case 0x1A2C: return &s_ssstand;
	case 0x1A3C: return &s_sspath1;
	case 0x1A4C: return &s_sspath1s;
	case 0x1A5C: return &s_sspath2;
	case 0x1A6C: return &s_sspath3;
	case 0x1A7C: return &s_sspath3s;
	case 0x1A8C: return &s_sspath4;
	case 0x1A9C: return &s_sspain;
	case 0x1AAC: return &s_ssshoot1;
	case 0x1ABC: return &s_ssshoot2;
	case 0x1ACC: return &s_ssshoot3;
	case 0x1ADC: return &s_ssshoot4;
	case 0x1AEC: return &s_ssshoot5;
	case 0x1AFC: return &s_ssshoot6;
	case 0x1B0C: return &s_ssshoot7;
	case 0x1B1C: return &s_ssshoot8;
	case 0x1B2C: return &s_ssshoot9;
	case 0x1B3C: return &s_sschase1;
	case 0x1B4C: return &s_sschase1s;
	case 0x1B5C: return &s_sschase2;
	case 0x1B6C: return &s_sschase3;
	case 0x1B7C: return &s_sschase3s;
	case 0x1B8C: return &s_sschase4;
	case 0x1B9C: return &s_ssdie1;
	case 0x1BAC: return &s_ssdie2;
	case 0x1BBC: return &s_ssdie3;
	case 0x1BCC: return &s_ssdie4;
	case 0x1BDC: return &s_bossstand;
	}
	return NULL;
}

REFKEEN_NS_E
#endif
