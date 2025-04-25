/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
 *
 * This file is primarily based on:
 * Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// ID_GLOB.H


#include <ALLOC.H>
#include <CTYPE.H>
#include <DOS.H>
#include <ERRNO.H>
#include <FCNTL.H>
#include <IO.H>
#include <MEM.H>
#include <PROCESS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <SYS\STAT.H>

#define __ID_GLOB__

//--------------------------------------------------------------------------

//EXTENSION is defined in VERSION.H, which is included by GFXE_BM.H and AUDIOBM.H
//#define	EXTENSION	"C3D"

extern	char far introscn;

#include "GFXE_BM.H"
#include "AUDIOBM.H"

//--------------------------------------------------------------------------

#define BIOMENACE	//this used to be CAT3D

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

#define GRMODE	EGAGR

#if GRMODE == EGAGR
#define GREXT	"EGA"
#endif
#if GRMODE == CGAGR
#define GREXT	"CGA"
#endif

//#define PROFILE

//
//	ID Engine
//	Types.h - Generic types, #defines, etc.
//	v1.0d1
//

#ifndef	__TYPES__
#define	__TYPES__

typedef	enum	{false,true}	boolean;
typedef	unsigned	char		byte;
typedef	unsigned	int			word;
typedef	unsigned	long		longword;
typedef	byte *					Ptr;

typedef	struct
		{
			int	x,y;
		} Point;
typedef	struct
		{
			Point	ul,lr;
		} Rect;

#define	nil	((void *)0)

#endif

#include "ID_MM.H"
#include "ID_CA.H"
#include "ID_VW.H"
#include "ID_RF.H"	//added refresh manager back in for BioMenace
#include "ID_IN.H"
#include "ID_SD.H"
#include "ID_US.H"


void	Quit (char *error);		// defined in user program

