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

// ID_CA.C

// this has been customized for WOLF

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "id_heads.h"
//#pragma hdrstop

//#pragma warn -pro
//#pragma warn -use

REFKEEN_NS_B

#define THREEBYTEGRSTARTS

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#pragma pack(push, 1)

typedef struct
{
  id0_unsigned_t bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
	id0_unsigned_t	RLEWtag;
	id0_long_t		headeroffsets[100];
	id0_byte_t		tileinfo[];
} mapfiletype;

#pragma pack(pop)


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

id0_byte_t 		id0_seg	*tinf;
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
id0_int_t			mapon;
#endif

id0_unsigned_t	id0_seg	*mapsegs[MAPPLANES];
maptype		id0_seg	*mapheaderseg[NUMMAPS];
id0_byte_t		id0_seg	*audiosegs[NUMSNDCHUNKS];
void		id0_seg	*grsegs[NUMCHUNKS];

id0_byte_t		id0_far	grneeded[NUMCHUNKS];
id0_byte_t		ca_levelbit,ca_levelnum;

BE_FILE_T			profilehandle,debughandle;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_boolean_t		CA_Started = false;
#endif

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_char_t		audioname[13]="AUDIO.";
#endif

// FIXME (REFKEEN): This should eventually be removed
extern BE_FILE_T (*ID_CA_File_Open_Handler)(const char *filename);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

// REFKEEN - Comment out what's unused
//extern	id0_long_t	*CGAhead;
extern	id0_long_t	*EGAhead;
//extern	id0_byte_t	*CGAdict;
extern	id0_byte_t	*EGAdict;
extern	id0_byte_t	*maphead;
//extern	id0_byte_t	*mapdict;
extern	id0_byte_t	*audiohead;
extern	id0_byte_t	*audiodict;


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_char_t extension[5],	// Need a string, not constant to change cache files
     gheadname[10]=GREXT"HEAD.",
     gfilename[10]=GREXT"GRAPH.",
     gdictname[10]=GREXT"DICT.",
     mheadname[10]="MAPHEAD.",
     mfilename[10]="MAPTEMP.",
     aheadname[10]="AUDIOHED.",
     afilename[10]="AUDIOT.";

void CA_CannotOpen(id0_char_t *string);
#endif

id0_long_t		id0_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
id0_long_t		id0_seg *audiostarts;	// array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode	*grhuffman;
#else
huffnode	grhuffman[255];
#endif

#ifdef AUDIOHEADERLINKED
huffnode	*audiohuffman;
#else
huffnode	audiohuffman[255];
#endif


BE_FILE_T			grhandle;		// handle to EGAGRAPH
BE_FILE_T			maphandle;		// handle to MAPTEMP / GAMEMAPS
BE_FILE_T			audiohandle;	// handle to AUDIOT / AUDIO

id0_long_t		chunkcomplen,chunkexplen;

SDMode		oldsoundmode;



/*** WARNING (REFKEEN): Input is assumed to Little-Endian, while the output is Native-Endian. Hence, function is renamed ***/
void	CAL_CarmackExpand_FromLE_ToNE (id0_unsigned_t id0_far *source, id0_unsigned_t id0_far *dest,
		id0_unsigned_t length);


#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE	3
//#define	GRFILEPOS(c) (*(id0_long_t id0_far *)(((id0_byte_t id0_far *)grstarts)+(c)*3)&0xffffff)
id0_long_t GRFILEPOS(id0_int_t c)
{
	id0_long_t value;
	id0_int_t	offset;

	offset = c*3;

	// REFKEEN - Just to make things safe, ensure no buffer overflow is
	// done here by accessing just 3 bytes (and be ready for Big Endian)
	value = 0;
#ifdef REFKEEN_ARCH_BIG_ENDIAN
	memcpy(((id0_byte_t *)&value) + 1, ((id0_byte_t id0_far *)grstarts)+offset, 3);
#else
	memcpy(&value, ((id0_byte_t id0_far *)grstarts)+offset, 3);
#endif
#if 0
	value = *(id0_long_t id0_far *)(((id0_byte_t id0_far *)grstarts)+offset);

	value &= 0x00ffffffl;
#endif

	if (value == 0xffffffl)
		value = -1;

	return value;
};
#else
#define FILEPOSSIZE	4
#define	GRFILEPOS(c) (grstarts[c])
#endif

/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/


// (REFKEEN) UNUSED FUNCTIONS
#if 0
/*
============================
=
= CA_OpenDebug / CA_CloseDebug
=
= Opens a binary file with the handle "debughandle"
=
============================
*/

void CA_OpenDebug (void)
{
	unlink ("DEBUG.TXT");
	debughandle = open("DEBUG.TXT", O_CREAT | O_WRONLY | O_TEXT);
}

void CA_CloseDebug (void)
{
	BE_Cross_close (debughandle);
}
#endif



/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

void CAL_GetGrChunkLength (id0_int_t chunk)
{
	BE_Cross_seek(grhandle,GRFILEPOS(chunk),SEEK_SET);
	BE_Cross_readInt32LE(grhandle, &chunkexplen);
	//read(grhandle,&chunkexplen,sizeof(chunkexplen));
	chunkcomplen = GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
}


/*
==========================
=
= CA_FarRead
=
= Read from a file to a far pointer
=
==========================
*/

id0_boolean_t CA_FarRead (BE_FILE_T handle, id0_byte_t id0_far *dest, id0_long_t length)
{
	if (length>0xffffl)
		Quit ("CA_FarRead doesn't support 64K reads yet!");
	// Ported from ASM
	int bytesread = BE_Cross_readInt8LEBuffer(handle, dest, length);
	if (bytesread == 0)
	{
		// Keep errno as set by read
		return false;
	}

	if (bytesread != length)
	{
		/* FIXME (REFKEEN) - Is that the right way? */
		errno = 11;
		//errno = EINVFMT; // user manager knows this is bad read
		return false;
	}
	return true;
}


/*
==========================
=
= CA_SegWrite
=
= Write from a file to a far pointer
=
==========================
*/

id0_boolean_t CA_FarWrite (BE_FILE_T handle, id0_byte_t id0_far *source, id0_long_t length)
{
	if (length>0xffffl)
		Quit ("CA_FarWrite doesn't support 64K reads yet!");
	// Ported from ASM
	int byteswritten = BE_Cross_writeInt8LEBuffer(handle, source, length);
	if (byteswritten == 0)
	{
		// Keep errno as set by write
		return false;
	}

	if (byteswritten != length)
	{
		errno = ENOMEM; // user manager knows this is bad write
		return false;
	}
	return true;
}


// (REFKEEN) UNUSED FUNCTION
#if 0
/*
==========================
=
= CA_ReadFile
=
= Reads a file into an allready allocated buffer
=
==========================
*/

id0_boolean_t CA_ReadFile (const id0_char_t *filename, memptr *ptr)
{
	id0_int_t handle;
	id0_long_t size;

	if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return false;

	size = filelength (handle);
	if (!CA_FarRead (handle,*ptr,size))
	{
		close (handle);
		return false;
	}
	close (handle);
	return true;
}
#endif


/*
==========================
=
= CA_WriteFile
=
= Writes a file from a memory buffer
=
==========================
*/

id0_boolean_t CA_WriteFile (const id0_char_t *filename, void id0_far *ptr, id0_long_t length)
{
	BE_FILE_T handle;
	id0_long_t size;

	handle = BE_Cross_open_rewritable_for_overwriting(filename);
	//handle = open(filename,O_CREAT | O_BINARY | O_WRONLY,
	//			S_IREAD | S_IWRITE | S_IFREG);

	if (!BE_Cross_IsFileValid(handle))
		return false;

	if (!CA_FarWrite (handle,(id0_byte_t *)ptr,length))
	{
		BE_Cross_close (handle);
		return false;
	}
	BE_Cross_close (handle);
	return true;
}



/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

id0_boolean_t CA_LoadFile (const id0_char_t *filename, memptr *ptr)
{
	BE_FILE_T handle;
	id0_long_t size;

	// TODO (REFKEEN) - Use just BE_Cross_open_readonly_for_reading
	// for now, even for the March 1992 prototype, in which this
	// function may theoretically be called for demo files.
	// However, it's apparently impossible to record demos from the proto.
	if (!BE_Cross_IsFileValid(handle = BE_Cross_open_readonly_for_reading(filename)))
	//if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return false;

	size = BE_Cross_FileLengthFromHandle (handle);
	MM_GetPtr (ptr,size);
	if (!CA_FarRead (handle,(id0_byte_t *)(*ptr),size))
	{
		BE_Cross_close (handle);
		return false;
	}
	BE_Cross_close (handle);
	return true;
}

/*
============================================================================

		COMPRESSION routines, see JHUFF.C for more

============================================================================
*/



/*
===============
=
= CAL_OptimizeNodes
=
= Goes through a huffman table and changes the 256-511 node numbers to the
= actular address of the node.  Must be called before CAL_HuffExpand
=
===============
*/

void CAL_OptimizeNodes (huffnode *table)
{
  /*** COMMENTED OUT - Not really useful in modern platforms as-is ***/
#if 0
  huffnode *node;
  id0_int_t i;

  node = table;

  for (i=0;i<255;i++)
  {
	if (node->bit0 >= 256)
	  node->bit0 = (id0_unsigned_t)(table+(node->bit0-256));
	if (node->bit1 >= 256)
	  node->bit1 = (id0_unsigned_t)(table+(node->bit1-256));
	node++;
  }
#endif
}



/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
= If screenhack, the data is decompressed in four planes directly
= to the screen
=
======================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void CAL_HuffExpand (id0_byte_t id0_huge *source, id0_byte_t id0_huge *dest,
  id0_long_t length,huffnode *hufftable)
#else
void CAL_HuffExpand (id0_byte_t id0_huge *source, id0_byte_t id0_huge *dest,
  id0_long_t length,huffnode *hufftable, id0_boolean_t screenhack)
#endif
{
	id0_unsigned_t code;
	huffnode *headptr = hufftable+254; // head node is always node 254
	huffnode *nodeon = headptr;
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	// REFKEEN: The screenhack == true case is somewhat modified. Instead
	// of using dest, just write from offset bufferofs in screen memory.
	id0_unsigned_t screendstptr, screendstendptr;
	int plane;
	if (screenhack)
	{
		plane = 0;
		screendstptr = bufferofs;
		screendstendptr = bufferofs + (length >> 2);
	}
#endif

	//------------
	// expand data
	//------------

	// Ported from ASM
	id0_byte_t id0_huge *srcptr = source, *dstptr = dest, *dstendptr = dest+length;
	id0_byte_t byteval = *(srcptr++); // load first byte
	id0_byte_t bitmask = 1;
	do
	{
		// take bit0 or bit1 path from node
		code = (byteval & bitmask) ? nodeon->bit1 : nodeon->bit0;
		if (bitmask & 0x80)
		{
			byteval = *(srcptr++); // load next byte
			bitmask = 1; // back to first bit
		}
		else
		{
			bitmask <<= 1; // advance to next bit position
		}
		// if < 256 it's a byte, else move node
		if (code >= 256)
		{
			// NOTE: CAL_OptimizeNodes does nothing now
			// and we do refer to hufftable itself here
			nodeon = hufftable + (code-256);
			continue;
		}
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (screenhack)
		{
			// write a decompressed byte out
			BE_ST_VGAUpdateGFXByteInPlane(screendstptr++, code, plane);
			nodeon = headptr; // back to the head node for next bit

			if (screendstptr == screendstendptr) // done plane?
			{
				if (++plane == 4)
					break;
				screendstptr = bufferofs;
			}
			continue;
		}
#endif
		*(dstptr++) = code; // write a decompressed byte out
		nodeon = headptr; // back to the head node for next bit

		if (dstptr == dstendptr) // done?
		{
			break;
		}
	} while (true);
}


/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG	0xa7
#define FARTAG	0xa8

/*** WARNING (REFKEEN): Input is assumed to Little-Endian, while the output is Native-Endian. Hence, function is renamed. ***/
void CAL_CarmackExpand_FromLE_ToNE (id0_unsigned_t id0_far *source, id0_unsigned_t id0_far *dest, id0_unsigned_t length)
{
	id0_unsigned_t	ch,chhigh,count,offset;
	id0_unsigned_t	id0_far *copyptr, /*id0_far *inptr, */id0_far *outptr;
	id0_unsigned_char_t *inptr;

	length/=2;

	inptr = (id0_unsigned_char_t *)source;
	//inptr = source;
	outptr = dest;

	while (length)
	{
		// REFKEEN: Little-Endian input and safe unaligned accesses
		memcpy(&ch, inptr, 2);
		ch = BE_Cross_Swap16LE(ch);
		inptr += 2;
		//ch = *inptr++;
		chhigh = ch>>8;
		if (chhigh == NEARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
				ch |= *inptr++;
				//ch |= *((id0_unsigned_char_t id0_far *)inptr)++;
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *inptr++;
				//offset = *((id0_unsigned_char_t id0_far *)inptr)++;
				copyptr = outptr - offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else if (chhigh == FARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
				ch |= *inptr++;
				//ch |= *((id0_unsigned_char_t id0_far *)inptr)++;
				*outptr++ = ch;
				length --;
			}
			else
			{
				// REFKEEN: Little-Endian input and safe unaligned accesses
				memcpy(&offset, inptr, 2);
				offset = BE_Cross_Swap16LE(offset);
				inptr += 2;
				//offset = *inptr++;
				copyptr = dest + offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else
		{
			*outptr++ = ch;
			length --;
		}
	}
}



/*
======================
=
= CA_RLEWcompress
=
======================
*/

id0_long_t CA_RLEWCompress (id0_unsigned_t id0_huge *source, id0_long_t length, id0_unsigned_t id0_huge *dest,
  id0_unsigned_t rlewtag)
{
  id0_long_t complength;
  id0_unsigned_t value,count,i;
  id0_unsigned_t id0_huge *start,id0_huge *end;

  start = dest;

  end = source + (length+1)/2;

//
// compress it
//
  do
  {
	count = 1;
	value = *source++;
	while (*source == value && source<end)
	{
	  count++;
	  source++;
	}
	if (count>3 || value == rlewtag)
	{
    //
    // send a tag / count / value string
    //
      *dest++ = rlewtag;
      *dest++ = count;
      *dest++ = value;
    }
    else
    {
    //
    // send word without compressing
    //
      for (i=1;i<=count;i++)
	*dest++ = value;
	}

  } while (source<end);

  complength = 2*(dest-start);
  return complength;
}


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (id0_unsigned_t id0_huge *source, id0_unsigned_t id0_huge *dest,id0_long_t length,
  id0_unsigned_t rlewtag)
{
  id0_unsigned_t value, count, i;
  id0_unsigned_t id0_huge *end = dest + (length)/2;


//
// expand it
//
  do
  {
	value = *source++;
	if (value != rlewtag)
	//
	// uncompressed
	//
	  *dest++=value;
	else
	{
	//
	// compressed string
	//
	  count = *source++;
	  value = *source++;
	  for (i=1;i<=count;i++)
	    *dest++ = value;
	}
  } while (dest<end);
}



/*
=============================================================================

					 CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	id0_char_t fname[13];
#endif
	BE_FILE_T handle;
	memptr compseg;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("Loading graphics headers...\n");
#endif

#ifdef GRHEADERLINKED

	grhuffman = (huffnode *)EGAdict;
	grstarts = EGAhead;

	CAL_OptimizeNodes (grhuffman);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler(GREXT"DICT." EXTENSION)))
	//if ((handle = open(GREXT"DICT."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open " GREXT"DICT." EXTENSION"!");
#else
	strcpy(fname,gdictname);
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler(fname)))
	//if ((handle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif

	BE_Cross_readInt16LEBuffer(handle, &grhuffman, sizeof(grhuffman));
	BE_Cross_close(handle);
	CAL_OptimizeNodes (grhuffman);
//
// load the data offsets from ???head.ext
//
	MM_GetPtr ((memptr *)&grstarts,(NUMCHUNKS+1)*FILEPOSSIZE);

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler(GREXT"HEAD." EXTENSION)))
	//if ((handle = open(GREXT"HEAD."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open " GREXT"HEAD." EXTENSION"!");
#else
	strcpy(fname,gheadname);
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler(fname)))
	//if ((handle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif

	// REFKEEN - Hack for Big Endian (even though it may be useless)
#ifdef THREEBYTEGRSTARTS
	BE_Cross_readInt24LEBuffer(handle, grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);
#else
	BE_Cross_readInt32LEBuffer(handle, grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);
#endif
	//CA_FarRead(handle, (memptr)grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);

	BE_Cross_close(handle);


#endif

//
// Open the graphics file, leaving it open until the game is finished
//
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	grhandle = ID_CA_File_Open_Handler(GREXT"GRAPH." EXTENSION);
	//grhandle = open(GREXT"GRAPH."EXTENSION, O_RDONLY | O_BINARY);
	if (!BE_Cross_IsFileValid(grhandle))
		Quit ("Cannot open " GREXT"GRAPH." EXTENSION"!");
#else
	strcpy(fname,gfilename);
	strcat(fname,extension);

	grhandle = ID_CA_File_Open_Handler(fname);
	//grhandle = open(fname, O_RDONLY | O_BINARY);
	if (!BE_Cross_IsFileValid(grhandle))
		CA_CannotOpen(fname);
#endif


//
// load the pic and sprite headers into the arrays in the data segment
//
	MM_GetPtr((memptr *)&pictable,NUMPICS*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,(id0_byte_t *)compseg,chunkcomplen);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	CAL_HuffExpand ((id0_byte_t *)compseg, (id0_byte_t id0_huge *)pictable,NUMPICS*sizeof(pictabletype),grhuffman);
#else
	CAL_HuffExpand ((id0_byte_t *)compseg, (id0_byte_t id0_huge *)pictable,NUMPICS*sizeof(pictabletype),grhuffman,false);
#endif
	MM_FreePtr(&compseg);
	// REFKEEN - Big Endian support
#ifdef REFKEEN_ARCH_BIG_ENDIAN
	for (int i = 0; i < NUMPICS; ++i)
	{
		pictable[i].width = BE_Cross_Swap16LE(pictable[i].width);
		pictable[i].height = BE_Cross_Swap16LE(pictable[i].height);
	}
#endif
}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
	id0_int_t	i;
	BE_FILE_T handle;
	id0_long_t length,pos;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	id0_char_t fname[13];
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("Loading maps...\n");
#endif

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef MAPHEADERLINKED
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler("MAPHEAD." EXTENSION)))
	//if ((handle = open("MAPHEAD."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open MAPHEAD." EXTENSION"!");
#else
	strcpy(fname,mheadname);
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler(fname)))
	//if ((handle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif

	length = BE_Cross_FileLengthFromHandle(handle);
	MM_GetPtr ((memptr *)&tinf,length);
	CA_FarRead(handle, tinf, length);
	BE_Cross_close(handle);
	// REFKEEN - Big Endian support
#ifdef REFKEEN_ARCH_BIG_ENDIAN
	mapfiletype id0_seg *tinfasmapfile = (mapfiletype id0_seg *)tinf;
	tinfasmapfile->RLEWtag = BE_Cross_Swap16LE(tinfasmapfile->RLEWtag);
	for (int i = 0; i < BE_Cross_ArrayLen(tinfasmapfile->headeroffsets); ++i)
	{
		tinfasmapfile->headeroffsets[i] = BE_Cross_Swap32LE(tinfasmapfile->headeroffsets[i]);
	}
#endif

#else

	tinf = maphead;

#endif

//
// open the data file
//
#ifdef CARMACIZED
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(maphandle = ID_CA_File_Open_Handler("GAMEMAPS."EXTENSION)))
	//if ((maphandle = open("GAMEMAPS."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open GAMEMAPS."EXTENSION"!");
#else
	strcpy(fname,"GAMEMAPS.");
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(maphandle = ID_CA_File_Open_Handler(fname)))
	//if ((maphandle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif
#else
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(maphandle = ID_CA_File_Open_Handler("MAPTEMP." EXTENSION)))
	//if ((maphandle = open("MAPTEMP."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open MAPTEMP." EXTENSION"!");
#else
	strcpy(fname,mfilename);
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(maphandle = ID_CA_File_Open_Handler(fname)))
	//if ((maphandle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif
#endif

//
// load all map header
//
	for (i=0;i<NUMMAPS;i++)
	{
		pos = ((mapfiletype	id0_seg *)tinf)->headeroffsets[i];
		if (pos<0)						// $FFFFFFFF start is a sparse map
			continue;

		MM_GetPtr((memptr *)&mapheaderseg[i],sizeof(maptype));
		MM_SetLock((memptr *)&mapheaderseg[i],true);
		BE_Cross_seek(maphandle,pos,SEEK_SET);
		CA_FarRead (maphandle,(id0_byte_t *)mapheaderseg[i],sizeof(maptype));
	}

//
// allocate space for 3 64*64 planes
//
	for (i=0;i<MAPPLANES;i++)
	{
		MM_GetPtr ((memptr *)&mapsegs[i],64*64*2);
		MM_SetLock ((memptr *)&mapsegs[i],true);
	}
}


//==========================================================================


/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
	BE_FILE_T handle;
	id0_long_t length;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	id0_char_t fname[13];
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("Loading sound headers...\n");
#endif

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler("AUDIOHED." EXTENSION)))
	//if ((handle = open("AUDIOHED."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open AUDIOHED." EXTENSION"!");
#else
	strcpy(fname,aheadname);
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(handle = ID_CA_File_Open_Handler(fname)))
	//if ((handle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif

	length = BE_Cross_FileLengthFromHandle(handle);
	MM_GetPtr ((memptr *)&audiostarts,length);
	BE_Cross_readInt32LEBuffer(handle, audiostarts, length);
	//CA_FarRead(handle, (id0_byte_t id0_far *)audiostarts, length);
	BE_Cross_close(handle);
#else
	audiohuffman = (huffnode *)audiodict;
	CAL_OptimizeNodes (audiohuffman);
	audiostarts = (id0_long_t *)audiohead;
#endif

//
// open the data file
//
#ifndef AUDIOHEADERLINKED
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (!BE_Cross_IsFileValid(audiohandle = ID_CA_File_Open_Handler("AUDIOT." EXTENSION)))
	//if ((audiohandle = open("AUDIOT."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open AUDIOT." EXTENSION"!");
#else
	strcpy(fname,afilename);
	strcat(fname,extension);

	if (!BE_Cross_IsFileValid(audiohandle = ID_CA_File_Open_Handler(fname)))
	//if ((audiohandle = open(fname,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif
#else
	if (!BE_Cross_IsFileValid(audiohandle = ID_CA_File_Open_Handler("AUDIO."EXTENSION)))
	//if ((audiohandle = open("AUDIO."EXTENSION,
	//	 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open AUDIO."EXTENSION"!");
#endif
}

//==========================================================================


/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
#ifdef PROFILE
	unlink ("PROFILE.TXT");
	profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("CA_Startup: ");
#endif

	CAL_SetupMapFile ();
	CAL_SetupGrFile ();
	CAL_SetupAudioFile ();

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	mapon = -1;
#endif
	ca_levelbit = 1;
	ca_levelnum = 0;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	CA_Started = true;
#endif

}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!CA_Started)
		return;
#endif
#ifdef PROFILE
	BE_Cross_close (profilehandle);
#endif

	BE_Cross_close (maphandle);
	BE_Cross_close (grhandle);
	BE_Cross_close (audiohandle);
}

//===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

void CA_CacheAudioChunk (id0_int_t chunk)
{
	id0_long_t	pos,compressed;
#ifdef AUDIOHEADERLINKED
	id0_long_t	expanded;
	memptr	bigbufferseg;
	id0_byte_t	id0_far *source;
#endif

	if (audiosegs[chunk])
	{
		MM_SetPurge ((memptr *)&audiosegs[chunk],0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = audiostarts[chunk];
	compressed = audiostarts[chunk+1]-pos;

	BE_Cross_seek(audiohandle,pos,SEEK_SET);

#ifndef AUDIOHEADERLINKED

	MM_GetPtr ((memptr *)&audiosegs[chunk],compressed);
	if (mmerror)
		return;

	CA_FarRead(audiohandle,audiosegs[chunk],compressed);

#else

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(audiohandle,(id0_byte_t *)bufferseg,compressed);
		source = (id0_byte_t *)bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		if (mmerror)
			return;
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(audiohandle,(id0_byte_t *)bigbufferseg,compressed);
		source = (id0_byte_t *)bigbufferseg;
	}

	// REFKEEN - Big Endian support
	expanded = BE_Cross_Swap32LE(*(id0_long_t id0_far *)source);
	//expanded = *(id0_long_t id0_far *)source;
	source += 4;			// skip over length
	MM_GetPtr ((memptr *)&audiosegs[chunk],expanded);
	if (mmerror)
		goto done;
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	CAL_HuffExpand (source,audiosegs[chunk],expanded,audiohuffman);
#else
	CAL_HuffExpand (source,audiosegs[chunk],expanded,audiohuffman,false);
#endif

done:
	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
#endif
	// REFKEEN - Big Endian support, possibly the most complicated case,
	// since we need to know the exact type of each chunk.
	//
	// FIXME (consider): With matching proper data and definitions, this
	// should be OK, but otherwise one could consider delaying the swaps
	// to runtime (although it's a stretch).

	// Sanity check before doing the swaps on BE
	// (ignoring STARTDIGISOUNDS as ID_SD doesn't even have code for that)
#if !((STARTPCSOUNDS <= STARTADLIBSOUNDS) && (STARTADLIBSOUNDS <= STARTMUSIC))
#error "ID_CA.C sanity check for Big-Endian byte swaps (audio chunks) has failed!"
#endif

#ifdef REFKEEN_ARCH_BIG_ENDIAN
	if (chunk < STARTMUSIC) // Sound effects
	{
		SoundCommon *sndCommonPtr = (SoundCommon *)audiosegs[chunk];
		sndCommonPtr->length = BE_Cross_Swap32LE(sndCommonPtr->length);
		sndCommonPtr->priority = BE_Cross_Swap16LE(sndCommonPtr->priority);
		if ((chunk >= STARTDIGISOUNDS) /*&& (chunk < STARTMUSIC)*/) // Digitized sounds
		{
			SampledSound *sampledSndPtr = (SampledSound *)audiosegs[chunk];
			sampledSndPtr->hertz = BE_Cross_Swap16LE(sampledSndPtr->hertz);
		}
	}
	else // Music chunk
	{
		MusicGroup *musicPtr = (MusicGroup *)audiosegs[chunk];
		musicPtr->length = BE_Cross_Swap16LE(musicPtr->length);
		// Swap the delays only
		for (id0_word_t bytesLeft = musicPtr->length, *musicData = 1 + musicPtr->values; bytesLeft >= 4; bytesLeft -= 4, musicData += 2)
			*musicData = BE_Cross_Swap16LE(*musicData);
	}
#endif
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds (void)
{
	id0_unsigned_t	start,i;

	switch (oldsoundmode)
	{
	case sdm_Off:
		goto cachein;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		if (audiosegs[start])
			MM_SetPurge ((memptr *)&audiosegs[start],3);		// make purgable

cachein:

	switch (SoundMode)
	{
	case sdm_Off:
		return;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		CA_CacheAudioChunk (start);

	oldsoundmode = SoundMode;
}

//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (id0_int_t chunk, id0_byte_t id0_far *source)
{
	id0_long_t	expanded;


	if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
	{
	//
	// expanded sizes of tile8/16/32 are implicit
	//

#define BLOCK		64
#define MASKBLOCK	128

		if (chunk<STARTTILE8M)			// tile 8s are all in one chunk!
			expanded = BLOCK*NUMTILE8;
		else if (chunk<STARTTILE16)
			expanded = MASKBLOCK*NUMTILE8M;
		else if (chunk<STARTTILE16M)	// all other tiles are one/chunk
			expanded = BLOCK*4;
		else if (chunk<STARTTILE32)
			expanded = MASKBLOCK*4;
		else if (chunk<STARTTILE32M)
			expanded = BLOCK*16;
		else
			expanded = MASKBLOCK*16;
	}
	else
	{
	//
	// everything else has an explicit size longword
	//
		// REFKEEN - Safe unaligned accesses (e.g., from CA_CacheMarks)
		// and Big Endian support
		memcpy(&expanded, source, 4);
		expanded = BE_Cross_Swap32LE(expanded);
		//expanded = *(id0_long_t id0_far *)source;
		source += 4;			// skip over length
	}

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	MM_GetPtr (&grsegs[chunk],expanded);
	if (mmerror)
		return;
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	CAL_HuffExpand (source,(id0_byte_t *)(grsegs[chunk]),expanded,grhuffman);
#else
	CAL_HuffExpand (source,(id0_byte_t *)(grsegs[chunk]),expanded,grhuffman,false);
#endif
	// REFKEEN - Big Endian support
#ifdef REFKEEN_ARCH_BIG_ENDIAN
	if (((chunk >= STARTFONT) && (chunk < STARTFONT+NUMFONT)) || ((chunk >= STARTFONTM) && (chunk < STARTFONTM+NUMFONTM)))
	{
		fontstruct *font = (fontstruct *)(grsegs[chunk]);
		font->height = BE_Cross_Swap16LE(font->height);
		for (int i = 0; i < (int)BE_Cross_ArrayLen(font->location); ++i)
			font->location[i] = BE_Cross_Swap16LE(font->location[i]);
	}
#endif
}


// *** ALPHA RESTORATION ***
// A seemingly unused function very similar to CA_CacheGrChunk,
// right from Catacomb 3-D, and also virtually identical
// to the earlier revision from Keen Dreams
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
/*
======================
=
= CAL_ReadGrChunk
=
= Gets a chunk off disk, optimizing reads to general buffer
=
======================
*/

void CAL_ReadGrChunk (id0_int_t chunk)
{
	id0_long_t	pos,compressed;
	memptr	bigbufferseg;
	id0_byte_t	id0_far *source;
	id0_int_t		next;

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = GRFILEPOS(chunk);
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;

	compressed = GRFILEPOS(next)-pos;

	BE_Cross_seek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,(id0_byte_t *)bufferseg,compressed);
		source = (id0_byte_t *)bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		if (mmerror)
			return;
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(grhandle,(id0_byte_t *)bigbufferseg,compressed);
		source = (id0_byte_t *)bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk (id0_int_t chunk)
{
	id0_long_t	pos,compressed;
	memptr	bigbufferseg;
	id0_byte_t	id0_far *source;
	id0_int_t		next;

	grneeded[chunk] |= ca_levelbit;		// make sure it doesn't get removed
	if (grsegs[chunk])
	{
		MM_SetPurge (&grsegs[chunk],0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = GRFILEPOS(chunk);
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;

	compressed = GRFILEPOS(next)-pos;

	BE_Cross_seek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,(id0_byte_t *)bufferseg,compressed);
		source = (id0_byte_t *)bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(grhandle,(id0_byte_t *)bigbufferseg,compressed);
		source = (id0_byte_t *)bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
}



// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
//==========================================================================

/*
======================
=
= CA_CacheScreen
=
= Decompresses a chunk from disk straight onto the screen
=
======================
*/

void CA_CacheScreen (id0_int_t chunk)
{
	id0_long_t	pos,compressed,expanded;
	memptr	bigbufferseg;
	id0_byte_t	id0_far *source;
	id0_int_t		next;

//
// load the chunk into a buffer
//
	pos = GRFILEPOS(chunk);
	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;
	compressed = GRFILEPOS(next)-pos;

	BE_Cross_seek(grhandle,pos,SEEK_SET);

	MM_GetPtr(&bigbufferseg,compressed);
	MM_SetLock (&bigbufferseg,true);
	CA_FarRead(grhandle,(id0_byte_t *)bigbufferseg,compressed);
	source = (id0_byte_t *)bigbufferseg;

	expanded = *(id0_long_t id0_far *)source;
	source += 4;			// skip over length

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	// REFKEEN: We just ignore the dest param and internally use bufferofs instead
	CAL_HuffExpand (source,0,expanded,grhuffman,true);
//	CAL_HuffExpand (source,MK_FP(SCREENSEG,bufferofs),expanded,grhuffman,true);
	VW_MarkUpdateBlock (0,0,319,199);
	MM_FreePtr(&bigbufferseg);
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

//==========================================================================

/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/

void CA_CacheMap (id0_int_t mapnum)
{
	id0_long_t	pos,compressed;
	id0_int_t		plane;
	memptr	*dest,bigbufferseg;
	id0_unsigned_t	size;
	id0_unsigned_t	id0_far	*source;
#ifdef CARMACIZED
	memptr	buffer2seg;
	id0_long_t	expanded;
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	mapon = mapnum;
#endif

//
// load the planes into the allready allocated buffers
//
	size = 64*64*2;

	for (plane = 0; plane<MAPPLANES; plane++)
	{
		pos = mapheaderseg[mapnum]->planestart[plane];
		compressed = mapheaderseg[mapnum]->planelength[plane];
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		if (!compressed)
			continue;
#endif

		dest = (memptr *)&mapsegs[plane];

		BE_Cross_seek(maphandle,pos,SEEK_SET);
		if (compressed<=BUFFERSIZE)
			source = (id0_unsigned_t *)bufferseg;
		else
		{
			MM_GetPtr(&bigbufferseg,compressed);
			MM_SetLock (&bigbufferseg,true);
			source = (id0_unsigned_t *)bigbufferseg;
		}

		CA_FarRead(maphandle,(id0_byte_t id0_far *)source,compressed);
#ifdef CARMACIZED
		//
		// unhuffman, then unRLEW
		// The huffman'd chunk has a two byte expanded length first
		// The resulting RLEW chunk also does, even though it's not really
		// needed
		//
		expanded = BE_Cross_Swap16LE(*source); // REFKEEN - Big Endian support
		//expanded = *source;
		source++;
		MM_GetPtr (&buffer2seg,expanded);
		CAL_CarmackExpand_FromLE_ToNE (source, (id0_unsigned_t id0_far *)buffer2seg,expanded);
		CA_RLEWexpand (((id0_unsigned_t id0_far *)buffer2seg)+1,(id0_unsigned_t *)(*dest),size,
		((mapfiletype id0_seg *)tinf)->RLEWtag);
		MM_FreePtr (&buffer2seg);

#else
		//
		// unRLEW, skipping expanded length
		//
		CA_RLEWexpand (source+1, (id0_unsigned_t *)(*dest), size,
		((mapfiletype id0_seg *)tinf)->RLEWtag);
#endif

		if (compressed>BUFFERSIZE)
			MM_FreePtr(&bigbufferseg);
	}
}

//===========================================================================

/*
======================
=
= CA_UpLevel
=
= Goes up a bit level in the needed lists and clears it out.
= Everything is made purgable
=
======================
*/

void CA_UpLevel (void)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	id0_int_t	i;
#endif

	if (ca_levelnum==7)
		Quit ("CA_UpLevel: Up past level 7!");

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge ((memptr *)&grsegs[i],3);
#endif
	ca_levelbit<<=1;
	ca_levelnum++;
}

//===========================================================================

/*
======================
=
= CA_DownLevel
=
= Goes down a bit level in the needed lists and recaches
= everything from the lower level
=
======================
*/

void CA_DownLevel (void)
{
	if (!ca_levelnum)
		Quit ("CA_DownLevel: Down past level 0!");
	ca_levelbit>>=1;
	ca_levelnum--;
	CA_CacheMarks();
}

//===========================================================================

/*
======================
=
= CA_ClearMarks
=
= Clears out all the marks at the current level
=
======================
*/

void CA_ClearMarks (void)
{
	id0_int_t i;

	for (i=0;i<NUMCHUNKS;i++)
		grneeded[i]&=~ca_levelbit;
}


//===========================================================================

/*
======================
=
= CA_ClearAllMarks
=
= Clears out all the marks on all the levels
=
======================
*/

void CA_ClearAllMarks (void)
{
	memset (grneeded,0,sizeof(grneeded));
	//_fmemset (grneeded,0,sizeof(grneeded));
	ca_levelbit = 1;
	ca_levelnum = 0;
}


//===========================================================================


/*
======================
=
= CA_FreeGraphics
=
======================
*/


void CA_SetGrPurge (void)
{
	id0_int_t i;

//
// free graphics
//
	CA_ClearMarks ();

	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge ((memptr *)&grsegs[i],3);
}



/*
======================
=
= CA_SetAllPurge
=
= Make everything possible purgable
=
======================
*/

void CA_SetAllPurge (void)
{
	id0_int_t i;


//
// free sounds
//
	for (i=0;i<NUMSNDCHUNKS;i++)
		if (audiosegs[i])
			MM_SetPurge ((memptr *)&audiosegs[i],3);

//
// free graphics
//
	CA_SetGrPurge ();
}


//===========================================================================

/*
======================
=
= CA_CacheMarks
=
======================
*/
#define MAXEMPTYREAD	1024

void CA_CacheMarks (void)
{
	id0_int_t 	i,next,numcache;
	id0_long_t	pos,endpos,nextpos,nextendpos,compressed;
	id0_long_t	bufferstart,bufferend;	// file position of general buffer
	id0_byte_t	id0_far *source;
	memptr	bigbufferseg;

	numcache = 0;
//
// go through and make everything not needed purgable
//
	for (i=0;i<NUMCHUNKS;i++)
		if (grneeded[i]&ca_levelbit)
		{
			if (grsegs[i])					// its allready in memory, make
				MM_SetPurge(&grsegs[i],0);	// sure it stays there!
			else
				numcache++;
		}
		else
		{
			if (grsegs[i])					// not needed, so make it purgeable
				MM_SetPurge(&grsegs[i],3);
		}

	if (!numcache)			// nothing to cache!
		return;


//
// go through and load in anything still needed
//
	bufferstart = bufferend = 0;		// nothing good in buffer now

	for (i=0;i<NUMCHUNKS;i++)
		if ( (grneeded[i]&ca_levelbit) && !grsegs[i])
		{
			pos = GRFILEPOS(i);
			if (pos<0)
				continue;

			next = i +1;
			while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
				next++;

			compressed = GRFILEPOS(next)-pos;
			endpos = pos+compressed;

			if (compressed<=BUFFERSIZE)
			{
				if (bufferstart<=pos
				&& bufferend>= endpos)
				{
				// data is allready in buffer
					source = (id0_byte_t id0_seg *)bufferseg+(pos-bufferstart);
				}
				else
				{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
					while ( next < NUMCHUNKS )
					{
						while (next < NUMCHUNKS &&
						!(grneeded[next]&ca_levelbit && !grsegs[next]))
							next++;
						if (next == NUMCHUNKS)
							continue;

						nextpos = GRFILEPOS(next);
						while (GRFILEPOS(++next) == -1)	// skip past any sparse tiles
							;
						nextendpos = GRFILEPOS(next);
						if (nextpos - endpos <= MAXEMPTYREAD
						&& nextendpos-pos <= BUFFERSIZE)
							endpos = nextendpos;
						else
							next = NUMCHUNKS;			// read pos to posend
					}

					BE_Cross_seek(grhandle,pos,SEEK_SET);
					CA_FarRead(grhandle,(id0_byte_t *)bufferseg,endpos-pos);
					bufferstart = pos;
					bufferend = endpos;
					source = (id0_byte_t *)bufferseg;
				}
			}
			else
			{
			// big chunk, allocate temporary buffer
				MM_GetPtr(&bigbufferseg,compressed);
				if (mmerror)
					return;
				MM_SetLock (&bigbufferseg,true);
				BE_Cross_seek(grhandle,pos,SEEK_SET);
				CA_FarRead(grhandle,(id0_byte_t *)bigbufferseg,compressed);
				source = (id0_byte_t *)bigbufferseg;
			}

			CAL_ExpandGrChunk (i,source);
			if (mmerror)
				return;

			if (compressed>BUFFERSIZE)
				MM_FreePtr(&bigbufferseg);

		}
}

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void CA_CannotOpen(id0_char_t *string)
{
 id0_char_t str[30];

 strcpy(str,"Can't open ");
 strcat(str,string);
 strcat(str,"!\n");
 Quit (str);
}
#endif

REFKEEN_NS_E
