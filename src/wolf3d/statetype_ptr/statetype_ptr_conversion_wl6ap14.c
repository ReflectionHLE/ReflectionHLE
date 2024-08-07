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

#ifdef REFKEEN_VER_WL1AP14
REFKEEN_NS_B

// List of all states
extern statetype s_player;
extern statetype s_attack;
extern statetype s_rocket;
extern statetype s_smoke1;
extern statetype s_smoke2;
extern statetype s_smoke3;
extern statetype s_smoke4;
extern statetype s_boom1;
extern statetype s_boom2;
extern statetype s_boom3;
extern statetype s_grdstand;
extern statetype s_grdpath1;
extern statetype s_grdpath1s;
extern statetype s_grdpath2;
extern statetype s_grdpath3;
extern statetype s_grdpath3s;
extern statetype s_grdpath4;
extern statetype s_grdpain;
extern statetype s_grdpain1;
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
extern statetype s_blinkychase1;
extern statetype s_blinkychase2;
extern statetype s_inkychase1;
extern statetype s_inkychase2;
extern statetype s_pinkychase1;
extern statetype s_pinkychase2;
extern statetype s_clydechase1;
extern statetype s_clydechase2;
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
extern statetype s_ofcstand;
extern statetype s_ofcpath1;
extern statetype s_ofcpath1s;
extern statetype s_ofcpath2;
extern statetype s_ofcpath3;
extern statetype s_ofcpath3s;
extern statetype s_ofcpath4;
extern statetype s_ofcpain;
extern statetype s_ofcpain1;
extern statetype s_ofcshoot1;
extern statetype s_ofcshoot2;
extern statetype s_ofcshoot3;
extern statetype s_ofcchase1;
extern statetype s_ofcchase1s;
extern statetype s_ofcchase2;
extern statetype s_ofcchase3;
extern statetype s_ofcchase3s;
extern statetype s_ofcchase4;
extern statetype s_ofcdie1;
extern statetype s_ofcdie2;
extern statetype s_ofcdie3;
extern statetype s_ofcdie4;
extern statetype s_ofcdie5;
extern statetype s_mutstand;
extern statetype s_mutpath1;
extern statetype s_mutpath1s;
extern statetype s_mutpath2;
extern statetype s_mutpath3;
extern statetype s_mutpath3s;
extern statetype s_mutpath4;
extern statetype s_mutpain;
extern statetype s_mutpain1;
extern statetype s_mutshoot1;
extern statetype s_mutshoot2;
extern statetype s_mutshoot3;
extern statetype s_mutshoot4;
extern statetype s_mutchase1;
extern statetype s_mutchase1s;
extern statetype s_mutchase2;
extern statetype s_mutchase3;
extern statetype s_mutchase3s;
extern statetype s_mutchase4;
extern statetype s_mutdie1;
extern statetype s_mutdie2;
extern statetype s_mutdie3;
extern statetype s_mutdie4;
extern statetype s_mutdie5;
extern statetype s_ssstand;
extern statetype s_sspath1;
extern statetype s_sspath1s;
extern statetype s_sspath2;
extern statetype s_sspath3;
extern statetype s_sspath3s;
extern statetype s_sspath4;
extern statetype s_sspain;
extern statetype s_sspain1;
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
extern statetype s_bosschase1;
extern statetype s_bosschase1s;
extern statetype s_bosschase2;
extern statetype s_bosschase3;
extern statetype s_bosschase3s;
extern statetype s_bosschase4;
extern statetype s_bossdie1;
extern statetype s_bossdie2;
extern statetype s_bossdie3;
extern statetype s_bossdie4;
extern statetype s_bossshoot1;
extern statetype s_bossshoot2;
extern statetype s_bossshoot3;
extern statetype s_bossshoot4;
extern statetype s_bossshoot5;
extern statetype s_bossshoot6;
extern statetype s_bossshoot7;
extern statetype s_bossshoot8;
extern statetype s_gretelstand;
extern statetype s_gretelchase1;
extern statetype s_gretelchase1s;
extern statetype s_gretelchase2;
extern statetype s_gretelchase3;
extern statetype s_gretelchase3s;
extern statetype s_gretelchase4;
extern statetype s_greteldie1;
extern statetype s_greteldie2;
extern statetype s_greteldie3;
extern statetype s_greteldie4;
extern statetype s_gretelshoot1;
extern statetype s_gretelshoot2;
extern statetype s_gretelshoot3;
extern statetype s_gretelshoot4;
extern statetype s_gretelshoot5;
extern statetype s_gretelshoot6;
extern statetype s_gretelshoot7;
extern statetype s_gretelshoot8;
extern statetype s_schabbstand;
extern statetype s_schabbchase1;
extern statetype s_schabbchase1s;
extern statetype s_schabbchase2;
extern statetype s_schabbchase3;
extern statetype s_schabbchase3s;
extern statetype s_schabbchase4;
extern statetype s_schabbdeathcam;
extern statetype s_schabbdie1;
extern statetype s_schabbdie2;
extern statetype s_schabbdie3;
extern statetype s_schabbdie4;
extern statetype s_schabbdie5;
extern statetype s_schabbdie6;
extern statetype s_schabbshoot1;
extern statetype s_schabbshoot2;
extern statetype s_needle1;
extern statetype s_needle2;
extern statetype s_needle3;
extern statetype s_needle4;
extern statetype s_giftstand;
extern statetype s_giftchase1;
extern statetype s_giftchase1s;
extern statetype s_giftchase2;
extern statetype s_giftchase3;
extern statetype s_giftchase3s;
extern statetype s_giftchase4;
extern statetype s_giftdeathcam;
extern statetype s_giftdie1;
extern statetype s_giftdie2;
extern statetype s_giftdie3;
extern statetype s_giftdie4;
extern statetype s_giftdie5;
extern statetype s_giftdie6;
extern statetype s_giftshoot1;
extern statetype s_giftshoot2;
extern statetype s_fatstand;
extern statetype s_fatchase1;
extern statetype s_fatchase1s;
extern statetype s_fatchase2;
extern statetype s_fatchase3;
extern statetype s_fatchase3s;
extern statetype s_fatchase4;
extern statetype s_fatdeathcam;
extern statetype s_fatdie1;
extern statetype s_fatdie2;
extern statetype s_fatdie3;
extern statetype s_fatdie4;
extern statetype s_fatdie5;
extern statetype s_fatdie6;
extern statetype s_fatshoot1;
extern statetype s_fatshoot2;
extern statetype s_fatshoot3;
extern statetype s_fatshoot4;
extern statetype s_fatshoot5;
extern statetype s_fatshoot6;
extern statetype s_fakestand;
extern statetype s_fakechase1;
extern statetype s_fakechase1s;
extern statetype s_fakechase2;
extern statetype s_fakechase3;
extern statetype s_fakechase3s;
extern statetype s_fakechase4;
extern statetype s_fakedie1;
extern statetype s_fakedie2;
extern statetype s_fakedie3;
extern statetype s_fakedie4;
extern statetype s_fakedie5;
extern statetype s_fakedie6;
extern statetype s_fakeshoot1;
extern statetype s_fakeshoot2;
extern statetype s_fakeshoot3;
extern statetype s_fakeshoot4;
extern statetype s_fakeshoot5;
extern statetype s_fakeshoot6;
extern statetype s_fakeshoot7;
extern statetype s_fakeshoot8;
extern statetype s_fakeshoot9;
extern statetype s_fire1;
extern statetype s_fire2;
extern statetype s_mechastand;
extern statetype s_mechachase1;
extern statetype s_mechachase1s;
extern statetype s_mechachase2;
extern statetype s_mechachase3;
extern statetype s_mechachase3s;
extern statetype s_mechachase4;
extern statetype s_mechadie1;
extern statetype s_mechadie2;
extern statetype s_mechadie3;
extern statetype s_mechadie4;
extern statetype s_mechashoot1;
extern statetype s_mechashoot2;
extern statetype s_mechashoot3;
extern statetype s_mechashoot4;
extern statetype s_mechashoot5;
extern statetype s_mechashoot6;
extern statetype s_hitlerchase1;
extern statetype s_hitlerchase1s;
extern statetype s_hitlerchase2;
extern statetype s_hitlerchase3;
extern statetype s_hitlerchase3s;
extern statetype s_hitlerchase4;
extern statetype s_hitlerdeathcam;
extern statetype s_hitlerdie1;
extern statetype s_hitlerdie2;
extern statetype s_hitlerdie3;
extern statetype s_hitlerdie4;
extern statetype s_hitlerdie5;
extern statetype s_hitlerdie6;
extern statetype s_hitlerdie7;
extern statetype s_hitlerdie8;
extern statetype s_hitlerdie9;
extern statetype s_hitlerdie10;
extern statetype s_hitlershoot1;
extern statetype s_hitlershoot2;
extern statetype s_hitlershoot3;
extern statetype s_hitlershoot4;
extern statetype s_hitlershoot5;
extern statetype s_hitlershoot6;
extern statetype s_bjrun1;
extern statetype s_bjrun1s;
extern statetype s_bjrun2;
extern statetype s_bjrun3;
extern statetype s_bjrun3s;
extern statetype s_bjrun4;
extern statetype s_bjjump1;
extern statetype s_bjjump2;
extern statetype s_bjjump3;
extern statetype s_bjjump4;
extern statetype s_deathcam;

void RefKeen_FillObjStatesWithDOSPointers_NonWL1(void)
{
	s_player.compatdospointer = 0x18F8;
	s_attack.compatdospointer = 0x1908;
	s_rocket.compatdospointer = 0x1BE6;
	s_smoke1.compatdospointer = 0x1BF6;
	s_smoke2.compatdospointer = 0x1C06;
	s_smoke3.compatdospointer = 0x1C16;
	s_smoke4.compatdospointer = 0x1C26;
	s_boom1.compatdospointer = 0x1C36;
	s_boom2.compatdospointer = 0x1C46;
	s_boom3.compatdospointer = 0x1C56;
	s_grdstand.compatdospointer = 0x1C66;
	s_grdpath1.compatdospointer = 0x1C76;
	s_grdpath1s.compatdospointer = 0x1C86;
	s_grdpath2.compatdospointer = 0x1C96;
	s_grdpath3.compatdospointer = 0x1CA6;
	s_grdpath3s.compatdospointer = 0x1CB6;
	s_grdpath4.compatdospointer = 0x1CC6;
	s_grdpain.compatdospointer = 0x1CD6;
	s_grdpain1.compatdospointer = 0x1CE6;
	s_grdshoot1.compatdospointer = 0x1CF6;
	s_grdshoot2.compatdospointer = 0x1D06;
	s_grdshoot3.compatdospointer = 0x1D16;
	s_grdchase1.compatdospointer = 0x1D26;
	s_grdchase1s.compatdospointer = 0x1D36;
	s_grdchase2.compatdospointer = 0x1D46;
	s_grdchase3.compatdospointer = 0x1D56;
	s_grdchase3s.compatdospointer = 0x1D66;
	s_grdchase4.compatdospointer = 0x1D76;
	s_grddie1.compatdospointer = 0x1D86;
	s_grddie2.compatdospointer = 0x1D96;
	s_grddie3.compatdospointer = 0x1DA6;
	s_grddie4.compatdospointer = 0x1DB6;
	s_blinkychase1.compatdospointer = 0x1DC6;
	s_blinkychase2.compatdospointer = 0x1DD6;
	s_inkychase1.compatdospointer = 0x1DE6;
	s_inkychase2.compatdospointer = 0x1DF6;
	s_pinkychase1.compatdospointer = 0x1E06;
	s_pinkychase2.compatdospointer = 0x1E16;
	s_clydechase1.compatdospointer = 0x1E26;
	s_clydechase2.compatdospointer = 0x1E36;
	s_dogpath1.compatdospointer = 0x1E46;
	s_dogpath1s.compatdospointer = 0x1E56;
	s_dogpath2.compatdospointer = 0x1E66;
	s_dogpath3.compatdospointer = 0x1E76;
	s_dogpath3s.compatdospointer = 0x1E86;
	s_dogpath4.compatdospointer = 0x1E96;
	s_dogjump1.compatdospointer = 0x1EA6;
	s_dogjump2.compatdospointer = 0x1EB6;
	s_dogjump3.compatdospointer = 0x1EC6;
	s_dogjump4.compatdospointer = 0x1ED6;
	s_dogjump5.compatdospointer = 0x1EE6;
	s_dogchase1.compatdospointer = 0x1EF6;
	s_dogchase1s.compatdospointer = 0x1F06;
	s_dogchase2.compatdospointer = 0x1F16;
	s_dogchase3.compatdospointer = 0x1F26;
	s_dogchase3s.compatdospointer = 0x1F36;
	s_dogchase4.compatdospointer = 0x1F46;
	s_dogdie1.compatdospointer = 0x1F56;
	s_dogdie2.compatdospointer = 0x1F66;
	s_dogdie3.compatdospointer = 0x1F76;
	s_dogdead.compatdospointer = 0x1F86;
	s_ofcstand.compatdospointer = 0x1F96;
	s_ofcpath1.compatdospointer = 0x1FA6;
	s_ofcpath1s.compatdospointer = 0x1FB6;
	s_ofcpath2.compatdospointer = 0x1FC6;
	s_ofcpath3.compatdospointer = 0x1FD6;
	s_ofcpath3s.compatdospointer = 0x1FE6;
	s_ofcpath4.compatdospointer = 0x1FF6;
	s_ofcpain.compatdospointer = 0x2006;
	s_ofcpain1.compatdospointer = 0x2016;
	s_ofcshoot1.compatdospointer = 0x2026;
	s_ofcshoot2.compatdospointer = 0x2036;
	s_ofcshoot3.compatdospointer = 0x2046;
	s_ofcchase1.compatdospointer = 0x2056;
	s_ofcchase1s.compatdospointer = 0x2066;
	s_ofcchase2.compatdospointer = 0x2076;
	s_ofcchase3.compatdospointer = 0x2086;
	s_ofcchase3s.compatdospointer = 0x2096;
	s_ofcchase4.compatdospointer = 0x20A6;
	s_ofcdie1.compatdospointer = 0x20B6;
	s_ofcdie2.compatdospointer = 0x20C6;
	s_ofcdie3.compatdospointer = 0x20D6;
	s_ofcdie4.compatdospointer = 0x20E6;
	s_ofcdie5.compatdospointer = 0x20F6;
	s_mutstand.compatdospointer = 0x2106;
	s_mutpath1.compatdospointer = 0x2116;
	s_mutpath1s.compatdospointer = 0x2126;
	s_mutpath2.compatdospointer = 0x2136;
	s_mutpath3.compatdospointer = 0x2146;
	s_mutpath3s.compatdospointer = 0x2156;
	s_mutpath4.compatdospointer = 0x2166;
	s_mutpain.compatdospointer = 0x2176;
	s_mutpain1.compatdospointer = 0x2186;
	s_mutshoot1.compatdospointer = 0x2196;
	s_mutshoot2.compatdospointer = 0x21A6;
	s_mutshoot3.compatdospointer = 0x21B6;
	s_mutshoot4.compatdospointer = 0x21C6;
	s_mutchase1.compatdospointer = 0x21D6;
	s_mutchase1s.compatdospointer = 0x21E6;
	s_mutchase2.compatdospointer = 0x21F6;
	s_mutchase3.compatdospointer = 0x2206;
	s_mutchase3s.compatdospointer = 0x2216;
	s_mutchase4.compatdospointer = 0x2226;
	s_mutdie1.compatdospointer = 0x2236;
	s_mutdie2.compatdospointer = 0x2246;
	s_mutdie3.compatdospointer = 0x2256;
	s_mutdie4.compatdospointer = 0x2266;
	s_mutdie5.compatdospointer = 0x2276;
	s_ssstand.compatdospointer = 0x2286;
	s_sspath1.compatdospointer = 0x2296;
	s_sspath1s.compatdospointer = 0x22A6;
	s_sspath2.compatdospointer = 0x22B6;
	s_sspath3.compatdospointer = 0x22C6;
	s_sspath3s.compatdospointer = 0x22D6;
	s_sspath4.compatdospointer = 0x22E6;
	s_sspain.compatdospointer = 0x22F6;
	s_sspain1.compatdospointer = 0x2306;
	s_ssshoot1.compatdospointer = 0x2316;
	s_ssshoot2.compatdospointer = 0x2326;
	s_ssshoot3.compatdospointer = 0x2336;
	s_ssshoot4.compatdospointer = 0x2346;
	s_ssshoot5.compatdospointer = 0x2356;
	s_ssshoot6.compatdospointer = 0x2366;
	s_ssshoot7.compatdospointer = 0x2376;
	s_ssshoot8.compatdospointer = 0x2386;
	s_ssshoot9.compatdospointer = 0x2396;
	s_sschase1.compatdospointer = 0x23A6;
	s_sschase1s.compatdospointer = 0x23B6;
	s_sschase2.compatdospointer = 0x23C6;
	s_sschase3.compatdospointer = 0x23D6;
	s_sschase3s.compatdospointer = 0x23E6;
	s_sschase4.compatdospointer = 0x23F6;
	s_ssdie1.compatdospointer = 0x2406;
	s_ssdie2.compatdospointer = 0x2416;
	s_ssdie3.compatdospointer = 0x2426;
	s_ssdie4.compatdospointer = 0x2436;
	s_bossstand.compatdospointer = 0x2446;
	s_bosschase1.compatdospointer = 0x2456;
	s_bosschase1s.compatdospointer = 0x2466;
	s_bosschase2.compatdospointer = 0x2476;
	s_bosschase3.compatdospointer = 0x2486;
	s_bosschase3s.compatdospointer = 0x2496;
	s_bosschase4.compatdospointer = 0x24A6;
	s_bossdie1.compatdospointer = 0x24B6;
	s_bossdie2.compatdospointer = 0x24C6;
	s_bossdie3.compatdospointer = 0x24D6;
	s_bossdie4.compatdospointer = 0x24E6;
	s_bossshoot1.compatdospointer = 0x24F6;
	s_bossshoot2.compatdospointer = 0x2506;
	s_bossshoot3.compatdospointer = 0x2516;
	s_bossshoot4.compatdospointer = 0x2526;
	s_bossshoot5.compatdospointer = 0x2536;
	s_bossshoot6.compatdospointer = 0x2546;
	s_bossshoot7.compatdospointer = 0x2556;
	s_bossshoot8.compatdospointer = 0x2566;
	s_gretelstand.compatdospointer = 0x2576;
	s_gretelchase1.compatdospointer = 0x2586;
	s_gretelchase1s.compatdospointer = 0x2596;
	s_gretelchase2.compatdospointer = 0x25A6;
	s_gretelchase3.compatdospointer = 0x25B6;
	s_gretelchase3s.compatdospointer = 0x25C6;
	s_gretelchase4.compatdospointer = 0x25D6;
	s_greteldie1.compatdospointer = 0x25E6;
	s_greteldie2.compatdospointer = 0x25F6;
	s_greteldie3.compatdospointer = 0x2606;
	s_greteldie4.compatdospointer = 0x2616;
	s_gretelshoot1.compatdospointer = 0x2626;
	s_gretelshoot2.compatdospointer = 0x2636;
	s_gretelshoot3.compatdospointer = 0x2646;
	s_gretelshoot4.compatdospointer = 0x2656;
	s_gretelshoot5.compatdospointer = 0x2666;
	s_gretelshoot6.compatdospointer = 0x2676;
	s_gretelshoot7.compatdospointer = 0x2686;
	s_gretelshoot8.compatdospointer = 0x2696;
	s_schabbstand.compatdospointer = 0x26B8;
	s_schabbchase1.compatdospointer = 0x26C8;
	s_schabbchase1s.compatdospointer = 0x26D8;
	s_schabbchase2.compatdospointer = 0x26E8;
	s_schabbchase3.compatdospointer = 0x26F8;
	s_schabbchase3s.compatdospointer = 0x2708;
	s_schabbchase4.compatdospointer = 0x2718;
	s_schabbdeathcam.compatdospointer = 0x2728;
	s_schabbdie1.compatdospointer = 0x2738;
	s_schabbdie2.compatdospointer = 0x2748;
	s_schabbdie3.compatdospointer = 0x2758;
	s_schabbdie4.compatdospointer = 0x2768;
	s_schabbdie5.compatdospointer = 0x2778;
	s_schabbdie6.compatdospointer = 0x2788;
	s_schabbshoot1.compatdospointer = 0x2798;
	s_schabbshoot2.compatdospointer = 0x27A8;
	s_needle1.compatdospointer = 0x27B8;
	s_needle2.compatdospointer = 0x27C8;
	s_needle3.compatdospointer = 0x27D8;
	s_needle4.compatdospointer = 0x27E8;
	s_giftstand.compatdospointer = 0x27F8;
	s_giftchase1.compatdospointer = 0x2808;
	s_giftchase1s.compatdospointer = 0x2818;
	s_giftchase2.compatdospointer = 0x2828;
	s_giftchase3.compatdospointer = 0x2838;
	s_giftchase3s.compatdospointer = 0x2848;
	s_giftchase4.compatdospointer = 0x2858;
	s_giftdeathcam.compatdospointer = 0x2868;
	s_giftdie1.compatdospointer = 0x2878;
	s_giftdie2.compatdospointer = 0x2888;
	s_giftdie3.compatdospointer = 0x2898;
	s_giftdie4.compatdospointer = 0x28A8;
	s_giftdie5.compatdospointer = 0x28B8;
	s_giftdie6.compatdospointer = 0x28C8;
	s_giftshoot1.compatdospointer = 0x28D8;
	s_giftshoot2.compatdospointer = 0x28E8;
	s_fatstand.compatdospointer = 0x28F8;
	s_fatchase1.compatdospointer = 0x2908;
	s_fatchase1s.compatdospointer = 0x2918;
	s_fatchase2.compatdospointer = 0x2928;
	s_fatchase3.compatdospointer = 0x2938;
	s_fatchase3s.compatdospointer = 0x2948;
	s_fatchase4.compatdospointer = 0x2958;
	s_fatdeathcam.compatdospointer = 0x2968;
	s_fatdie1.compatdospointer = 0x2978;
	s_fatdie2.compatdospointer = 0x2988;
	s_fatdie3.compatdospointer = 0x2998;
	s_fatdie4.compatdospointer = 0x29A8;
	s_fatdie5.compatdospointer = 0x29B8;
	s_fatdie6.compatdospointer = 0x29C8;
	s_fatshoot1.compatdospointer = 0x29D8;
	s_fatshoot2.compatdospointer = 0x29E8;
	s_fatshoot3.compatdospointer = 0x29F8;
	s_fatshoot4.compatdospointer = 0x2A08;
	s_fatshoot5.compatdospointer = 0x2A18;
	s_fatshoot6.compatdospointer = 0x2A28;
	s_fakestand.compatdospointer = 0x2A38;
	s_fakechase1.compatdospointer = 0x2A48;
	s_fakechase1s.compatdospointer = 0x2A58;
	s_fakechase2.compatdospointer = 0x2A68;
	s_fakechase3.compatdospointer = 0x2A78;
	s_fakechase3s.compatdospointer = 0x2A88;
	s_fakechase4.compatdospointer = 0x2A98;
	s_fakedie1.compatdospointer = 0x2AA8;
	s_fakedie2.compatdospointer = 0x2AB8;
	s_fakedie3.compatdospointer = 0x2AC8;
	s_fakedie4.compatdospointer = 0x2AD8;
	s_fakedie5.compatdospointer = 0x2AE8;
	s_fakedie6.compatdospointer = 0x2AF8;
	s_fakeshoot1.compatdospointer = 0x2B08;
	s_fakeshoot2.compatdospointer = 0x2B18;
	s_fakeshoot3.compatdospointer = 0x2B28;
	s_fakeshoot4.compatdospointer = 0x2B38;
	s_fakeshoot5.compatdospointer = 0x2B48;
	s_fakeshoot6.compatdospointer = 0x2B58;
	s_fakeshoot7.compatdospointer = 0x2B68;
	s_fakeshoot8.compatdospointer = 0x2B78;
	s_fakeshoot9.compatdospointer = 0x2B88;
	s_fire1.compatdospointer = 0x2B98;
	s_fire2.compatdospointer = 0x2BA8;
	s_mechastand.compatdospointer = 0x2BB8;
	s_mechachase1.compatdospointer = 0x2BC8;
	s_mechachase1s.compatdospointer = 0x2BD8;
	s_mechachase2.compatdospointer = 0x2BE8;
	s_mechachase3.compatdospointer = 0x2BF8;
	s_mechachase3s.compatdospointer = 0x2C08;
	s_mechachase4.compatdospointer = 0x2C18;
	s_mechadie1.compatdospointer = 0x2C28;
	s_mechadie2.compatdospointer = 0x2C38;
	s_mechadie3.compatdospointer = 0x2C48;
	s_mechadie4.compatdospointer = 0x2C58;
	s_mechashoot1.compatdospointer = 0x2C68;
	s_mechashoot2.compatdospointer = 0x2C78;
	s_mechashoot3.compatdospointer = 0x2C88;
	s_mechashoot4.compatdospointer = 0x2C98;
	s_mechashoot5.compatdospointer = 0x2CA8;
	s_mechashoot6.compatdospointer = 0x2CB8;
	s_hitlerchase1.compatdospointer = 0x2CC8;
	s_hitlerchase1s.compatdospointer = 0x2CD8;
	s_hitlerchase2.compatdospointer = 0x2CE8;
	s_hitlerchase3.compatdospointer = 0x2CF8;
	s_hitlerchase3s.compatdospointer = 0x2D08;
	s_hitlerchase4.compatdospointer = 0x2D18;
	s_hitlerdeathcam.compatdospointer = 0x2D28;
	s_hitlerdie1.compatdospointer = 0x2D38;
	s_hitlerdie2.compatdospointer = 0x2D48;
	s_hitlerdie3.compatdospointer = 0x2D58;
	s_hitlerdie4.compatdospointer = 0x2D68;
	s_hitlerdie5.compatdospointer = 0x2D78;
	s_hitlerdie6.compatdospointer = 0x2D88;
	s_hitlerdie7.compatdospointer = 0x2D98;
	s_hitlerdie8.compatdospointer = 0x2DA8;
	s_hitlerdie9.compatdospointer = 0x2DB8;
	s_hitlerdie10.compatdospointer = 0x2DC8;
	s_hitlershoot1.compatdospointer = 0x2DD8;
	s_hitlershoot2.compatdospointer = 0x2DE8;
	s_hitlershoot3.compatdospointer = 0x2DF8;
	s_hitlershoot4.compatdospointer = 0x2E08;
	s_hitlershoot5.compatdospointer = 0x2E18;
	s_hitlershoot6.compatdospointer = 0x2E28;
	s_bjrun1.compatdospointer = 0x2E40;
	s_bjrun1s.compatdospointer = 0x2E50;
	s_bjrun2.compatdospointer = 0x2E60;
	s_bjrun3.compatdospointer = 0x2E70;
	s_bjrun3s.compatdospointer = 0x2E80;
	s_bjrun4.compatdospointer = 0x2E90;
	s_bjjump1.compatdospointer = 0x2EA0;
	s_bjjump2.compatdospointer = 0x2EB0;
	s_bjjump3.compatdospointer = 0x2EC0;
	s_bjjump4.compatdospointer = 0x2ED0;
	s_deathcam.compatdospointer = 0x2EE0;
}

statetype *RefKeen_GetObjStatePtrFromDOSPointer_NonWL1(uint_fast32_t dosptr)
{
	switch (dosptr)
	{
	case 0x18F8: return &s_player;
	case 0x1908: return &s_attack;
	case 0x1BE6: return &s_rocket;
	case 0x1BF6: return &s_smoke1;
	case 0x1C06: return &s_smoke2;
	case 0x1C16: return &s_smoke3;
	case 0x1C26: return &s_smoke4;
	case 0x1C36: return &s_boom1;
	case 0x1C46: return &s_boom2;
	case 0x1C56: return &s_boom3;
	case 0x1C66: return &s_grdstand;
	case 0x1C76: return &s_grdpath1;
	case 0x1C86: return &s_grdpath1s;
	case 0x1C96: return &s_grdpath2;
	case 0x1CA6: return &s_grdpath3;
	case 0x1CB6: return &s_grdpath3s;
	case 0x1CC6: return &s_grdpath4;
	case 0x1CD6: return &s_grdpain;
	case 0x1CE6: return &s_grdpain1;
	case 0x1CF6: return &s_grdshoot1;
	case 0x1D06: return &s_grdshoot2;
	case 0x1D16: return &s_grdshoot3;
	case 0x1D26: return &s_grdchase1;
	case 0x1D36: return &s_grdchase1s;
	case 0x1D46: return &s_grdchase2;
	case 0x1D56: return &s_grdchase3;
	case 0x1D66: return &s_grdchase3s;
	case 0x1D76: return &s_grdchase4;
	case 0x1D86: return &s_grddie1;
	case 0x1D96: return &s_grddie2;
	case 0x1DA6: return &s_grddie3;
	case 0x1DB6: return &s_grddie4;
	case 0x1DC6: return &s_blinkychase1;
	case 0x1DD6: return &s_blinkychase2;
	case 0x1DE6: return &s_inkychase1;
	case 0x1DF6: return &s_inkychase2;
	case 0x1E06: return &s_pinkychase1;
	case 0x1E16: return &s_pinkychase2;
	case 0x1E26: return &s_clydechase1;
	case 0x1E36: return &s_clydechase2;
	case 0x1E46: return &s_dogpath1;
	case 0x1E56: return &s_dogpath1s;
	case 0x1E66: return &s_dogpath2;
	case 0x1E76: return &s_dogpath3;
	case 0x1E86: return &s_dogpath3s;
	case 0x1E96: return &s_dogpath4;
	case 0x1EA6: return &s_dogjump1;
	case 0x1EB6: return &s_dogjump2;
	case 0x1EC6: return &s_dogjump3;
	case 0x1ED6: return &s_dogjump4;
	case 0x1EE6: return &s_dogjump5;
	case 0x1EF6: return &s_dogchase1;
	case 0x1F06: return &s_dogchase1s;
	case 0x1F16: return &s_dogchase2;
	case 0x1F26: return &s_dogchase3;
	case 0x1F36: return &s_dogchase3s;
	case 0x1F46: return &s_dogchase4;
	case 0x1F56: return &s_dogdie1;
	case 0x1F66: return &s_dogdie2;
	case 0x1F76: return &s_dogdie3;
	case 0x1F86: return &s_dogdead;
	case 0x1F96: return &s_ofcstand;
	case 0x1FA6: return &s_ofcpath1;
	case 0x1FB6: return &s_ofcpath1s;
	case 0x1FC6: return &s_ofcpath2;
	case 0x1FD6: return &s_ofcpath3;
	case 0x1FE6: return &s_ofcpath3s;
	case 0x1FF6: return &s_ofcpath4;
	case 0x2006: return &s_ofcpain;
	case 0x2016: return &s_ofcpain1;
	case 0x2026: return &s_ofcshoot1;
	case 0x2036: return &s_ofcshoot2;
	case 0x2046: return &s_ofcshoot3;
	case 0x2056: return &s_ofcchase1;
	case 0x2066: return &s_ofcchase1s;
	case 0x2076: return &s_ofcchase2;
	case 0x2086: return &s_ofcchase3;
	case 0x2096: return &s_ofcchase3s;
	case 0x20A6: return &s_ofcchase4;
	case 0x20B6: return &s_ofcdie1;
	case 0x20C6: return &s_ofcdie2;
	case 0x20D6: return &s_ofcdie3;
	case 0x20E6: return &s_ofcdie4;
	case 0x20F6: return &s_ofcdie5;
	case 0x2106: return &s_mutstand;
	case 0x2116: return &s_mutpath1;
	case 0x2126: return &s_mutpath1s;
	case 0x2136: return &s_mutpath2;
	case 0x2146: return &s_mutpath3;
	case 0x2156: return &s_mutpath3s;
	case 0x2166: return &s_mutpath4;
	case 0x2176: return &s_mutpain;
	case 0x2186: return &s_mutpain1;
	case 0x2196: return &s_mutshoot1;
	case 0x21A6: return &s_mutshoot2;
	case 0x21B6: return &s_mutshoot3;
	case 0x21C6: return &s_mutshoot4;
	case 0x21D6: return &s_mutchase1;
	case 0x21E6: return &s_mutchase1s;
	case 0x21F6: return &s_mutchase2;
	case 0x2206: return &s_mutchase3;
	case 0x2216: return &s_mutchase3s;
	case 0x2226: return &s_mutchase4;
	case 0x2236: return &s_mutdie1;
	case 0x2246: return &s_mutdie2;
	case 0x2256: return &s_mutdie3;
	case 0x2266: return &s_mutdie4;
	case 0x2276: return &s_mutdie5;
	case 0x2286: return &s_ssstand;
	case 0x2296: return &s_sspath1;
	case 0x22A6: return &s_sspath1s;
	case 0x22B6: return &s_sspath2;
	case 0x22C6: return &s_sspath3;
	case 0x22D6: return &s_sspath3s;
	case 0x22E6: return &s_sspath4;
	case 0x22F6: return &s_sspain;
	case 0x2306: return &s_sspain1;
	case 0x2316: return &s_ssshoot1;
	case 0x2326: return &s_ssshoot2;
	case 0x2336: return &s_ssshoot3;
	case 0x2346: return &s_ssshoot4;
	case 0x2356: return &s_ssshoot5;
	case 0x2366: return &s_ssshoot6;
	case 0x2376: return &s_ssshoot7;
	case 0x2386: return &s_ssshoot8;
	case 0x2396: return &s_ssshoot9;
	case 0x23A6: return &s_sschase1;
	case 0x23B6: return &s_sschase1s;
	case 0x23C6: return &s_sschase2;
	case 0x23D6: return &s_sschase3;
	case 0x23E6: return &s_sschase3s;
	case 0x23F6: return &s_sschase4;
	case 0x2406: return &s_ssdie1;
	case 0x2416: return &s_ssdie2;
	case 0x2426: return &s_ssdie3;
	case 0x2436: return &s_ssdie4;
	case 0x2446: return &s_bossstand;
	case 0x2456: return &s_bosschase1;
	case 0x2466: return &s_bosschase1s;
	case 0x2476: return &s_bosschase2;
	case 0x2486: return &s_bosschase3;
	case 0x2496: return &s_bosschase3s;
	case 0x24A6: return &s_bosschase4;
	case 0x24B6: return &s_bossdie1;
	case 0x24C6: return &s_bossdie2;
	case 0x24D6: return &s_bossdie3;
	case 0x24E6: return &s_bossdie4;
	case 0x24F6: return &s_bossshoot1;
	case 0x2506: return &s_bossshoot2;
	case 0x2516: return &s_bossshoot3;
	case 0x2526: return &s_bossshoot4;
	case 0x2536: return &s_bossshoot5;
	case 0x2546: return &s_bossshoot6;
	case 0x2556: return &s_bossshoot7;
	case 0x2566: return &s_bossshoot8;
	case 0x2576: return &s_gretelstand;
	case 0x2586: return &s_gretelchase1;
	case 0x2596: return &s_gretelchase1s;
	case 0x25A6: return &s_gretelchase2;
	case 0x25B6: return &s_gretelchase3;
	case 0x25C6: return &s_gretelchase3s;
	case 0x25D6: return &s_gretelchase4;
	case 0x25E6: return &s_greteldie1;
	case 0x25F6: return &s_greteldie2;
	case 0x2606: return &s_greteldie3;
	case 0x2616: return &s_greteldie4;
	case 0x2626: return &s_gretelshoot1;
	case 0x2636: return &s_gretelshoot2;
	case 0x2646: return &s_gretelshoot3;
	case 0x2656: return &s_gretelshoot4;
	case 0x2666: return &s_gretelshoot5;
	case 0x2676: return &s_gretelshoot6;
	case 0x2686: return &s_gretelshoot7;
	case 0x2696: return &s_gretelshoot8;
	case 0x26B8: return &s_schabbstand;
	case 0x26C8: return &s_schabbchase1;
	case 0x26D8: return &s_schabbchase1s;
	case 0x26E8: return &s_schabbchase2;
	case 0x26F8: return &s_schabbchase3;
	case 0x2708: return &s_schabbchase3s;
	case 0x2718: return &s_schabbchase4;
	case 0x2728: return &s_schabbdeathcam;
	case 0x2738: return &s_schabbdie1;
	case 0x2748: return &s_schabbdie2;
	case 0x2758: return &s_schabbdie3;
	case 0x2768: return &s_schabbdie4;
	case 0x2778: return &s_schabbdie5;
	case 0x2788: return &s_schabbdie6;
	case 0x2798: return &s_schabbshoot1;
	case 0x27A8: return &s_schabbshoot2;
	case 0x27B8: return &s_needle1;
	case 0x27C8: return &s_needle2;
	case 0x27D8: return &s_needle3;
	case 0x27E8: return &s_needle4;
	case 0x27F8: return &s_giftstand;
	case 0x2808: return &s_giftchase1;
	case 0x2818: return &s_giftchase1s;
	case 0x2828: return &s_giftchase2;
	case 0x2838: return &s_giftchase3;
	case 0x2848: return &s_giftchase3s;
	case 0x2858: return &s_giftchase4;
	case 0x2868: return &s_giftdeathcam;
	case 0x2878: return &s_giftdie1;
	case 0x2888: return &s_giftdie2;
	case 0x2898: return &s_giftdie3;
	case 0x28A8: return &s_giftdie4;
	case 0x28B8: return &s_giftdie5;
	case 0x28C8: return &s_giftdie6;
	case 0x28D8: return &s_giftshoot1;
	case 0x28E8: return &s_giftshoot2;
	case 0x28F8: return &s_fatstand;
	case 0x2908: return &s_fatchase1;
	case 0x2918: return &s_fatchase1s;
	case 0x2928: return &s_fatchase2;
	case 0x2938: return &s_fatchase3;
	case 0x2948: return &s_fatchase3s;
	case 0x2958: return &s_fatchase4;
	case 0x2968: return &s_fatdeathcam;
	case 0x2978: return &s_fatdie1;
	case 0x2988: return &s_fatdie2;
	case 0x2998: return &s_fatdie3;
	case 0x29A8: return &s_fatdie4;
	case 0x29B8: return &s_fatdie5;
	case 0x29C8: return &s_fatdie6;
	case 0x29D8: return &s_fatshoot1;
	case 0x29E8: return &s_fatshoot2;
	case 0x29F8: return &s_fatshoot3;
	case 0x2A08: return &s_fatshoot4;
	case 0x2A18: return &s_fatshoot5;
	case 0x2A28: return &s_fatshoot6;
	case 0x2A38: return &s_fakestand;
	case 0x2A48: return &s_fakechase1;
	case 0x2A58: return &s_fakechase1s;
	case 0x2A68: return &s_fakechase2;
	case 0x2A78: return &s_fakechase3;
	case 0x2A88: return &s_fakechase3s;
	case 0x2A98: return &s_fakechase4;
	case 0x2AA8: return &s_fakedie1;
	case 0x2AB8: return &s_fakedie2;
	case 0x2AC8: return &s_fakedie3;
	case 0x2AD8: return &s_fakedie4;
	case 0x2AE8: return &s_fakedie5;
	case 0x2AF8: return &s_fakedie6;
	case 0x2B08: return &s_fakeshoot1;
	case 0x2B18: return &s_fakeshoot2;
	case 0x2B28: return &s_fakeshoot3;
	case 0x2B38: return &s_fakeshoot4;
	case 0x2B48: return &s_fakeshoot5;
	case 0x2B58: return &s_fakeshoot6;
	case 0x2B68: return &s_fakeshoot7;
	case 0x2B78: return &s_fakeshoot8;
	case 0x2B88: return &s_fakeshoot9;
	case 0x2B98: return &s_fire1;
	case 0x2BA8: return &s_fire2;
	case 0x2BB8: return &s_mechastand;
	case 0x2BC8: return &s_mechachase1;
	case 0x2BD8: return &s_mechachase1s;
	case 0x2BE8: return &s_mechachase2;
	case 0x2BF8: return &s_mechachase3;
	case 0x2C08: return &s_mechachase3s;
	case 0x2C18: return &s_mechachase4;
	case 0x2C28: return &s_mechadie1;
	case 0x2C38: return &s_mechadie2;
	case 0x2C48: return &s_mechadie3;
	case 0x2C58: return &s_mechadie4;
	case 0x2C68: return &s_mechashoot1;
	case 0x2C78: return &s_mechashoot2;
	case 0x2C88: return &s_mechashoot3;
	case 0x2C98: return &s_mechashoot4;
	case 0x2CA8: return &s_mechashoot5;
	case 0x2CB8: return &s_mechashoot6;
	case 0x2CC8: return &s_hitlerchase1;
	case 0x2CD8: return &s_hitlerchase1s;
	case 0x2CE8: return &s_hitlerchase2;
	case 0x2CF8: return &s_hitlerchase3;
	case 0x2D08: return &s_hitlerchase3s;
	case 0x2D18: return &s_hitlerchase4;
	case 0x2D28: return &s_hitlerdeathcam;
	case 0x2D38: return &s_hitlerdie1;
	case 0x2D48: return &s_hitlerdie2;
	case 0x2D58: return &s_hitlerdie3;
	case 0x2D68: return &s_hitlerdie4;
	case 0x2D78: return &s_hitlerdie5;
	case 0x2D88: return &s_hitlerdie6;
	case 0x2D98: return &s_hitlerdie7;
	case 0x2DA8: return &s_hitlerdie8;
	case 0x2DB8: return &s_hitlerdie9;
	case 0x2DC8: return &s_hitlerdie10;
	case 0x2DD8: return &s_hitlershoot1;
	case 0x2DE8: return &s_hitlershoot2;
	case 0x2DF8: return &s_hitlershoot3;
	case 0x2E08: return &s_hitlershoot4;
	case 0x2E18: return &s_hitlershoot5;
	case 0x2E28: return &s_hitlershoot6;
	case 0x2E40: return &s_bjrun1;
	case 0x2E50: return &s_bjrun1s;
	case 0x2E60: return &s_bjrun2;
	case 0x2E70: return &s_bjrun3;
	case 0x2E80: return &s_bjrun3s;
	case 0x2E90: return &s_bjrun4;
	case 0x2EA0: return &s_bjjump1;
	case 0x2EB0: return &s_bjjump2;
	case 0x2EC0: return &s_bjjump3;
	case 0x2ED0: return &s_bjjump4;
	case 0x2EE0: return &s_deathcam;
	}
	return NULL;
}

REFKEEN_NS_E
#endif
