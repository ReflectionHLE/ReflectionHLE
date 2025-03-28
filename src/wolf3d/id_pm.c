/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2025 NY00123
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

//
//	ID_PM.C
//	Id Engine's Page Manager v1.0
//	Primary coder: Jason Blochowiak
//

#include "id_heads.h"
//#pragma hdrstop

REFKEEN_NS_B

//	Main Mem specific variables
	id0_boolean_t			MainPresent;
	memptr			MainMemPages[PMMaxMainMem];
	PMBlockAttr		MainMemUsed[PMMaxMainMem];
	id0_int_t				MainPagesAvail;

//	EMS specific variables
	id0_boolean_t			EMSPresent;
	id0_word_t			EMSAvail,EMSPagesAvail,EMSHandle,
					EMSPageFrame,EMSPhysicalPage;
	EMSListStruct	EMSList[EMSFrameCount];

//	XMS specific variables
	id0_boolean_t			XMSPresent;
	id0_word_t			XMSAvail,XMSPagesAvail,XMSHandle;
	BE_XMM_Control		*XMSDriver;
	id0_int_t				XMSProtectPage = -1;

//	File specific variables
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_char_t			PageFileName[] = {"VSWAP.WL1"};
#else
	id0_char_t			PageFileName[13] = {"VSWAP."};
#endif
	BE_FILE_T				PageFile = BE_CROSS_NIL_FILE;
	id0_word_t			ChunksInFile;
	id0_word_t			PMSpriteStart,PMSoundStart;

//	General usage variables
	id0_boolean_t			PMStarted;
#if 0 // REFKEEN: Unused boolean variables
					PMPanicMode,
					PMThrashing;
#endif
	id0_word_t			XMSPagesUsed,
					EMSPagesUsed,
					MainPagesUsed,
					PMNumBlocks;
	id0_long_t			PMFrameCount;
	PageListStruct	id0_far *PMPages,
					id0_seg *PMSegPages;

static	const id0_char_t		*ParmStrings[] = {"nomain","noems","noxms",id0_nil_t};

/////////////////////////////////////////////////////////////////////////////
//
//	EMS Management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_MapEMS() - Maps a logical page to a physical page
//
void
PML_MapEMS(id0_word_t logical,id0_word_t physical)
{
	if (BE_Cross_EMM_MapPage(EMSHandle, logical, physical) != BE_EMM_NO_ERROR)
#if 0
	_AL = physical;
	_BX = logical;
	_DX = EMSHandle;
	_AH = EMS_MAPPAGE;
asm	int	EMS_INT

	if (_AH)
#endif
		Quit("PML_MapEMS: Page mapping failed");
}

//
//	PML_StartupEMS() - Sets up EMS for Page Mgr's use
//		Checks to see if EMS driver is present
//      Verifies that EMS hardware is present
//		Make sure that EMS version is 3.2 or later
//		If there's more than our minimum (2 pages) available, allocate it (up
//			to the maximum we need)
//

	id0_char_t	EMMDriverName[9] = "EMMXXXX0";

id0_boolean_t
PML_StartupEMS(void)
{
	id0_int_t		i;
	id0_long_t	size;
	BE_EMM_AllocationInfo EMSPageCount;

	EMSPresent = false;			// Assume that we'll fail
	EMSAvail = 0;

	BE_EMM_File EMSDriver = BE_Cross_EMM_Open(EMMDriverName);	// try to open EMMXXXX0 device
	if (!EMSDriver)
		goto error;
#if 0
	_DX = (id0_word_t)EMMDriverName;
	_AX = 0x3d00;
	geninterrupt(0x21);			// try to open EMMXXXX0 device
asm	jnc	gothandle
	goto error;
#endif

	uint16_t EMSDriverInfo;
	if (!BE_Cross_EMM_Ioctl_GetDeviceData(EMSDriver, &EMSDriverInfo))	// get device info
		goto error;
#if 0
gothandle:
	_BX = _AX;
	_AX = 0x4400;
	geninterrupt(0x21);			// get device info
asm	jnc	gotinfo;
	goto error;
#endif

	if ((EMSDriverInfo & 0x80) != 0x80)
		goto error;
#if 0
gotinfo:
asm	and	dx,0x80
	if (!_DX)
		goto error;
#endif

	if (!BE_Cross_EMM_Ioctl_GetStatus(EMSDriver))	// get status
		goto error;
#if 0
	_AX = 0x4407;
	geninterrupt(0x21);			// get status
asm	jc	error
	if (!_AL)
		goto error;
#endif

	BE_Cross_EMM_Close(EMSDriver);	// close handle
#if 0
	_AH = 0x3e;
	geninterrupt(0x21);			// close handle
#endif

	if (BE_Cross_EMM_GetStatus())
		goto error;				// make sure EMS hardware is present
#if 0
	_AH = EMS_STATUS;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;				// make sure EMS hardware is present
#endif

	if (BE_Cross_EMM_GetVersion() < 0x32)	// only work on EMS 3.2 or greater (silly, but...)
		goto error;
#if 0
	_AH = EMS_VERSION;
	geninterrupt(EMS_INT);
	if (_AH || (_AL < 0x32))	// only work on EMS 3.2 or greater (silly, but...)
		goto error;
#endif

	EMSPageFrame = BE_Cross_EMM_GetPageFrame();
	if (!EMSPageFrame)
		goto error;				// find the page frame address
#if 0
	_AH = EMS_GETFRAME;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;				// find the page frame address
	EMSPageFrame = _BX;
#endif

	EMSPageCount = BE_Cross_EMM_GetUnallocedPageCount();
	if (EMSPageCount.pagesAvailable < 2)
		goto error;         	// Require at least 2 pages (32k)
	EMSAvail = EMSPageCount.pagesAvailable;
#if 0
	_AH = EMS_GETPAGES;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;
	if (_BX < 2)
		goto error;         	// Require at least 2 pages (32k)
	EMSAvail = _BX;
#endif

	// Don't hog all available EMS
	size = EMSAvail * (id0_long_t)EMSPageSize;
	if (size - (EMSPageSize * 2) > (ChunksInFile * (id0_long_t)PMPageSize))
	{
		size = (ChunksInFile * (id0_long_t)PMPageSize) + EMSPageSize;
		EMSAvail = size / EMSPageSize;
	}

	if (BE_Cross_EMM_AllocatePages(EMSAvail, &EMSHandle) != BE_EMM_NO_ERROR)
		goto error;
#if 0
	_AH = EMS_ALLOCPAGES;
	_BX = EMSAvail;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;
	EMSHandle = _DX;
#endif

	mminfo.EMSmem += EMSAvail * (id0_long_t)EMSPageSize;

	// Initialize EMS mapping cache
	for (i = 0;i < EMSFrameCount;i++)
		EMSList[i].baseEMSPage = -1;

	EMSPresent = true;			// We have EMS

error:
	return(EMSPresent);
}

//
//	PML_ShutdownEMS() - If EMS was used, deallocate it
//
void
PML_ShutdownEMS(void)
{
	if (EMSPresent)
	{
		if (BE_Cross_EMM_DeallocatePages(EMSHandle) != BE_EMM_NO_ERROR)
#if 0
	asm	mov	ah,EMS_FREEPAGES
	asm	mov	dx,[EMSHandle]
	asm	int	EMS_INT
		if (_AH)
#endif
			Quit ("PML_ShutdownEMS: Error freeing EMS");
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	XMS Management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_StartupXMS() - Starts up XMS for the Page Mgr's use
//		Checks for presence of an XMS driver
//		Makes sure that there's at least a page of XMS available
//		Allocates any remaining XMS (rounded down to the nearest page size)
//
id0_boolean_t
PML_StartupXMS(void)
{
	BE_XMM_Error XMSError;

	XMSPresent = false;					// Assume failure
	XMSAvail = 0;

	if (!BE_Cross_GetXmsDriverInstalled())
		goto error;
#if 0
asm	mov	ax,0x4300
asm	int	XMS_INT         				// Check for presence of XMS driver
	if (_AL != 0x80)
		goto error;
#endif


	XMSDriver = BE_Cross_GetXmsControl();
#if 0
asm	mov	ax,0x4310
asm	int	XMS_INT							// Get address of XMS driver
asm	mov	[WORD PTR XMSDriver],bx
asm	mov	[WORD PTR XMSDriver+2],es		// function pointer to XMS driver
#endif

	// Find out how much XMS is available
	XMSError = XMSDriver->queryFreeExtendedMemory(&XMSAvail, NULL);
	// *** PRE-V1.4 APOGEE + SOD RESTORATION ***
#if (GAMEVER_WOLFREV > 19921007L)
	if (BE_Cross_XMM_IsError(XMSError))
#else
	if (!XMSAvail)				// AJR: bugfix 10/8/92
#endif
		goto error;

#if 0
	XMS_CALL(XMS_QUERYFREE);			// Find out how much XMS is available
	XMSAvail = _AX;
	// *** PRE-V1.4 APOGEE + SOD RESTORATION ***
#if (GAMEVER_WOLFREV <= 19921007L)
	if (_BL)
#else
	if (!_AX)				// AJR: bugfix 10/8/92
#endif
		goto error;
#endif

	XMSAvail &= ~(PMPageSizeKB - 1);	// Round off to nearest page size
	if (XMSAvail < (PMPageSizeKB * 2))	// Need at least 2 pages
		goto error;

	XMSError = XMSDriver->allocateExtendedMemoryBlock(XMSAvail, &XMSHandle);
#if 0
	_DX = XMSAvail;
	XMS_CALL(XMS_ALLOC);				// And do the allocation
	XMSHandle = _DX;
#endif

	// *** PRE-V1.4 APOGEE + SOD RESTORATION ***
#if (GAMEVER_WOLFREV <= 19921007L)
	if (BE_Cross_XMM_IsError(XMSError))
#else
	if (XMSError != BE_XMM_SUCCESS)				// AJR: bugfix 10/8/92
#endif
#if 0
#if (GAMEVER_WOLFREV <= 19921007L)
	if (_BL)
#else
	if (!_AX)				// AJR: bugfix 10/8/92
#endif
#endif
	{
		XMSAvail = 0;
		goto error;
	}

	mminfo.XMSmem += XMSAvail * 1024;

	XMSPresent = true;
error:
	return(XMSPresent);
}

//
//	PML_XMSCopy() - Copies a main/EMS page to or from XMS
//		Will round an odd-length request up to the next even value
//
void
PML_XMSCopy(id0_boolean_t toxms,id0_byte_t id0_far *addr,id0_word_t xmspage,id0_word_t length)
{
	id0_longword_t	xoffset;
	struct
	{
		id0_longword_t	length;
		id0_word_t		source_handle;
		id0_longword_t	source_offset;
		id0_word_t		target_handle;
		id0_longword_t	target_offset;
	} copy;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (!addr)
		Quit("PML_XMSCopy: zero address");
#endif

	xoffset = (id0_longword_t)xmspage * PMPageSize;

	copy.length = (length + 1) & ~1;
	copy.source_handle = toxms? 0 : XMSHandle;
	copy.source_offset = toxms? BE_Cross_GetPtrXmsOff(addr) : xoffset;
	copy.target_handle = toxms? XMSHandle : 0;
	copy.target_offset = toxms? xoffset : BE_Cross_GetPtrXmsOff(addr);

	if (XMSDriver->moveExtendedMemoryBlock(*((BE_XMM_ExtendedMemoryMove*)&copy)) != BE_XMM_SUCCESS)
#if 0
asm	push si
	_SI = (id0_word_t)&copy;
	XMS_CALL(XMS_MOVE);
asm	pop	si
	if (!_AX)
#endif
		Quit("PML_XMSCopy: Error on copy");
}

#if 1
#define	PML_CopyToXMS(s,t,l)	PML_XMSCopy(true,(id0_byte_t*)(s),(t),(l))
#define	PML_CopyFromXMS(t,s,l)	PML_XMSCopy(false,(id0_byte_t*)(t),(s),(l))
#else
//
//	PML_CopyToXMS() - Copies the specified number of bytes from the real mode
//		segment address to the specified XMS page
//
void
PML_CopyToXMS(id0_byte_t id0_far *source,id0_int_t targetpage,id0_word_t length)
{
	PML_XMSCopy(true,source,targetpage,length);
}

//
//	PML_CopyFromXMS() - Copies the specified number of bytes from an XMS
//		page to the specified real mode address
//
void
PML_CopyFromXMS(id0_byte_t id0_far *target,id0_int_t sourcepage,id0_word_t length)
{
	PML_XMSCopy(false,target,sourcepage,length);
}
#endif

//
//	PML_ShutdownXMS()
//
void
PML_ShutdownXMS(void)
{
	if (XMSPresent)
	{
		if (BE_Cross_XMM_IsError(XMSDriver->freeExtendedMemoryBlock(XMSHandle)))
#if 0
		_DX = XMSHandle;
		XMS_CALL(XMS_FREE);
		if (_BL)
#endif
			Quit("PML_ShutdownXMS: Error freeing XMS");
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	Main memory code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PM_SetMainMemPurge() - Sets the purge level for all allocated main memory
//		blocks. This shouldn't be called directly - the PM_LockMainMem() and
//		PM_UnlockMainMem() macros should be used instead.
//
void
PM_SetMainMemPurge(id0_int_t level)
{
	id0_int_t	i;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!PMStarted)
		return;
#endif
	for (i = 0;i < PMMaxMainMem;i++)
		if (MainMemPages[i])
			MM_SetPurge(&MainMemPages[i],level);
}

//
//	PM_CheckMainMem() - If something besides the Page Mgr makes requests of
//		the Memory Mgr, some of the Page Mgr's blocks may have been purged,
//		so this function runs through the block list and checks to see if
//		any of the blocks have been purged. If so, it marks the corresponding
//		page as purged & unlocked, then goes through the block list and
//		tries to reallocate any blocks that have been purged.
//	This routine now calls PM_LockMainMem() to make sure that any allocation
//		attempts made during the block reallocation sweep don't purge any
//		of the other blocks. Because PM_LockMainMem() is called,
//		PM_UnlockMainMem() needs to be called before any other part of the
//		program makes allocation requests of the Memory Mgr.
//
void
PM_CheckMainMem(void)
{
	id0_boolean_t			allocfailed;
	id0_int_t				i,n;
	memptr			*p;
	PMBlockAttr		*used;
	PageListStruct	id0_far *page;

	if (!MainPresent)
		return;

	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
	{
		n = page->mainPage;
		if (n != -1)						// Is the page using main memory?
		{
			if (!MainMemPages[n])			// Yep, was the block purged?
			{
				page->mainPage = -1;		// Yes, mark page as purged & unlocked
				page->locked = pml_Unlocked;
			}
		}
	}

	// Prevent allocation attempts from purging any of our other blocks
	PM_LockMainMem();
	allocfailed = false;
	for (i = 0,p = MainMemPages,used = MainMemUsed;i < PMMaxMainMem;i++,p++,used++)
	{
		if (!*p)							// If the page got purged
		{
			if (*used & pmba_Allocated)		// If it was allocated
			{
				*used = (PMBlockAttr)(*used & ~pmba_Allocated);	// Mark as unallocated
				MainPagesAvail--;			// and decrease available count
			}

			if (*used & pmba_Used)			// If it was used
			{
				*used = (PMBlockAttr)(*used & ~pmba_Used);		// Mark as unused
				MainPagesUsed--;			// and decrease used count
			}

			if (!allocfailed)
			{
				MM_BombOnError(false);
				MM_GetPtr(p,PMPageSize);		// Try to reallocate
				if (mmerror)					// If it failed,
					allocfailed = true;			//  don't try any more allocations
				else							// If it worked,
				{
					*used = (PMBlockAttr)(*used | pmba_Allocated);	// Mark as allocated
					MainPagesAvail++;			// and increase available count
				}
				MM_BombOnError(true);
			}
		}
	}
	if (mmerror)
		mmerror = false;
}

//
//	PML_StartupMainMem() - Allocates as much main memory as is possible for
//		the Page Mgr. The memory is allocated as non-purgeable, so if it's
//		necessary to make requests of the Memory Mgr, PM_UnlockMainMem()
//		needs to be called.
//
void
PML_StartupMainMem(void)
{
	id0_int_t		i,n;
	memptr	*p;

	MainPagesAvail = 0;
	MM_BombOnError(false);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	for (i = 0,p = MainMemPages;(i < PMMaxMainMem) && !mmerror;i++,p++)
#else
	for (i = 0,p = MainMemPages;i < PMMaxMainMem;i++,p++)
#endif
	{
		MM_GetPtr(p,PMPageSize);
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		if (!mmerror)
			MainPagesAvail++;
#else
		if (mmerror)
			break;

		MainPagesAvail++;
#endif
		MainMemUsed[i] = pmba_Allocated;
	}
	MM_BombOnError(true);
	if (mmerror)
		mmerror = false;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (MainPagesAvail < PMMinMainMem)
	{
		// *** S3DNA RESTORATION ***
		// A little hack for error message (from WL_DEF.H)
		extern	id0_char_t str[80];

		sprintf(str, "PM_StartupMainMem(): Not enough main memory (need %d more pages)!", PMMinMainMem-MainPagesAvail);
		Quit(str);
	}
	else
		BE_ST_printf("Main memory (%" ID0_PRIlu "kb)\n", (id0_long_t)MainPagesAvail<<2);
#else
	if (MainPagesAvail < PMMinMainMem)
		Quit("PM_SetupMainMem: Not enough main memory");
#endif
	MainPresent = true;
}

//
//	PML_ShutdownMainMem() - Frees all of the main memory blocks used by the
//		Page Mgr.
//
void
PML_ShutdownMainMem(void)
{
	id0_int_t		i;
	memptr	*p;

	// DEBUG - mark pages as unallocated & decrease page count as appropriate
	for (i = 0,p = MainMemPages;i < PMMaxMainMem;i++,p++)
		if (*p)
			MM_FreePtr(p);
}

/////////////////////////////////////////////////////////////////////////////
//
//	File management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_ReadFromFile() - Reads some data in from the page file
//
void
PML_ReadFromFile(id0_byte_t id0_far *buf,id0_long_t offset,id0_word_t length)
{
	if (!buf)
		Quit("PML_ReadFromFile: Null pointer");
	if (!offset)
		Quit("PML_ReadFromFile: Zero offset");
	if (BE_Cross_seek(PageFile,offset,SEEK_SET) != 0)
		Quit("PML_ReadFromFile: Seek failed");
	if (!CA_FarRead(PageFile,buf,length))
		Quit("PML_ReadFromFile: Read failed");
}

//
//	PML_OpenPageFile() - Opens the page file and sets up the page info
//
void
PML_OpenPageFile(void)
{
	id0_int_t				i;
	id0_long_t			size;
	void			id0_seg *buf;
	id0_longword_t		id0_far *offsetptr;
	id0_word_t			id0_far *lengthptr;
	PageListStruct	id0_far *page;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("Opening %s\n", PageFileName);
#endif

	PageFile = BE_Cross_open_readonly_for_reading(PageFileName);
//	PageFile = open(PageFileName,O_RDONLY + O_BINARY);
	if (!BE_Cross_IsFileValid(PageFile))
//	if (PageFile == -1)
		Quit("PML_OpenPageFile: Unable to open page file");

	// Read in header variables
	BE_Cross_readInt16LE(PageFile,&ChunksInFile);
	BE_Cross_readInt16LE(PageFile,&PMSpriteStart);
	BE_Cross_readInt16LE(PageFile,&PMSoundStart);

	// Allocate and clear the page list
	PMNumBlocks = ChunksInFile;
	MM_GetPtr((memptr *)&PMSegPages,sizeof(PageListStruct) * PMNumBlocks);
	MM_SetLock((memptr *)&PMSegPages,true);
	PMPages = (PageListStruct id0_far *)PMSegPages;
	memset(PMPages,0,sizeof(PageListStruct) * PMNumBlocks);
//	_fmemset(PMPages,0,sizeof(PageListStruct) * PMNumBlocks);

	// Read in the chunk offsets
	size = sizeof(id0_longword_t) * ChunksInFile;
	MM_GetPtr(&buf,size);
	if (!CA_FarRead(PageFile,(id0_byte_t id0_far *)buf,size))
		Quit("PML_OpenPageFile: Offset read failed");
	offsetptr = (id0_longword_t id0_far *)buf;
	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
		page->offset = *offsetptr++;
	MM_FreePtr(&buf);

	// Read in the chunk lengths
	size = sizeof(id0_word_t) * ChunksInFile;
	MM_GetPtr(&buf,size);
	if (!CA_FarRead(PageFile,(id0_byte_t id0_far *)buf,size))
		Quit("PML_OpenPageFile: Length read failed");
	lengthptr = (id0_word_t id0_far *)buf;
	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
		page->length = *lengthptr++;
	MM_FreePtr(&buf);
}

//
//  PML_ClosePageFile() - Closes the page file
//
void
PML_ClosePageFile(void)
{
	if (BE_Cross_IsFileValid(PageFile))
		BE_Cross_close(PageFile);
	if (PMSegPages)
	{
		MM_SetLock((memptr *)&PMSegPages,false);
		MM_FreePtr((void id0_seg **)&PMSegPages);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	Allocation, etc., code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_GetEMSAddress()
//
// 		Page is in EMS, so figure out which EMS physical page should be used
//  		to map our page in. If normal page, use EMS physical page 3, else
//  		use the physical page specified by the lock type
//
#if 1
//#pragma argsused	// DEBUG - remove lock parameter
memptr
PML_GetEMSAddress(id0_int_t page,PMLockType lock)
{
	id0_int_t		i,emspage;
	id0_word_t	emsoff,emsbase,offset;

	emsoff = page & (PMEMSSubPage - 1);
	emsbase = page - emsoff;

	emspage = -1;
	// See if this page is already mapped in
	for (i = 0;i < EMSFrameCount;i++)
	{
		if (EMSList[i].baseEMSPage == emsbase)
		{
			emspage = i;	// Yep - don't do a redundant remapping
			break;
		}
	}

	// If page isn't already mapped in, find LRU EMS frame, and use it
	if (emspage == -1)
	{
		id0_longword_t last = ID0_MAXLONG;
		for (i = 0;i < EMSFrameCount;i++)
		{
			if (EMSList[i].lastHit < last)
			{
				emspage = i;
				last = EMSList[i].lastHit;
			}
		}

		EMSList[emspage].baseEMSPage = emsbase;
		PML_MapEMS(page / PMEMSSubPage,emspage);
	}

	if (emspage == -1)
		Quit("PML_GetEMSAddress: EMS find failed");

	EMSList[emspage].lastHit = PMFrameCount;
	offset = emspage * EMSPageSizeSeg;
	offset += emsoff * PMPageSizeSeg;
	return((memptr)BE_Cross_BGetPtrFromSeg(EMSPageFrame + offset));
}
#else
memptr
PML_GetEMSAddress(id0_int_t page,PMLockType lock)
{
	id0_word_t	emspage;

	emspage = (lock < pml_EMSLock)? 3 : (lock - pml_EMSLock);

	PML_MapEMS(page / PMEMSSubPage,emspage);

	return((memptr)(EMSPageFrame + (emspage * EMSPageSizeSeg)
			+ ((page & (PMEMSSubPage - 1)) * PMPageSizeSeg)));
}
#endif

//
//	PM_GetPageAddress() - Returns the address of a given page
//		Maps in EMS if necessary
//		Returns nil if block isn't cached into Main Memory or EMS
//
//
memptr
PM_GetPageAddress(id0_int_t pagenum)
{
	PageListStruct	id0_far *page;

	page = &PMPages[pagenum];
	if (page->mainPage != -1)
		return(MainMemPages[page->mainPage]);
	else if (page->emsPage != -1)
		return(PML_GetEMSAddress(page->emsPage,page->locked));
	else
		return(id0_nil_t);
}

//
//	PML_GiveLRUPage() - Returns the page # of the least recently used
//		present & unlocked main/EMS page (or main page if mainonly is true)
//
id0_int_t
PML_GiveLRUPage(id0_boolean_t mainonly)
{
	id0_int_t				i,lru;
	id0_long_t			last;
	PageListStruct	id0_far *page;

	for (i = 0,page = PMPages,lru = -1,last = ID0_MAXLONG;i < ChunksInFile;i++,page++)
	{
		if
		(
			(page->lastHit < last)
		&&	((page->emsPage != -1) || (page->mainPage != -1))
		&& 	(page->locked == pml_Unlocked)
		&&	(!(mainonly && (page->mainPage == -1)))
		)
		{
			last = page->lastHit;
			lru = i;
		}
	}

	if (lru == -1)
		Quit("PML_GiveLRUPage: LRU Search failed");
	return(lru);
}

//
//	PML_GiveLRUXMSPage() - Returns the page # of the least recently used
//		(and present) XMS page.
//	This routine won't return the XMS page protected (by XMSProtectPage)
//
id0_int_t
PML_GiveLRUXMSPage(void)
{
	id0_int_t				i,lru;
	id0_long_t			last;
	PageListStruct	id0_far *page;

	for (i = 0,page = PMPages,lru = -1,last = ID0_MAXLONG;i < ChunksInFile;i++,page++)
	{
		if
		(
			(page->xmsPage != -1)
		&&	(page->lastHit < last)
		&&	(i != XMSProtectPage)
		)
		{
			last = page->lastHit;
			lru = i;
		}
	}
	return(lru);
}

//
//	PML_PutPageInXMS() - If page isn't in XMS, find LRU XMS page and replace
//		it with the main/EMS page
//
void
PML_PutPageInXMS(id0_int_t pagenum)
{
	id0_int_t				usexms;
	PageListStruct	id0_far *page;

	if (!XMSPresent)
		return;

	page = &PMPages[pagenum];
	if (page->xmsPage != -1)
		return;					// Already in XMS

	if (XMSPagesUsed < XMSPagesAvail)
		page->xmsPage = XMSPagesUsed++;
	else
	{
		usexms = PML_GiveLRUXMSPage();
		if (usexms == -1)
			Quit("PML_PutPageInXMS: No XMS LRU");
		page->xmsPage = PMPages[usexms].xmsPage;
		PMPages[usexms].xmsPage = -1;
	}
	PML_CopyToXMS(PM_GetPageAddress(pagenum),page->xmsPage,page->length);
}

//
//	PML_TransferPageSpace() - A page is being replaced, so give the new page
//		the old one's address space. Returns the address of the new page.
//
memptr
PML_TransferPageSpace(id0_int_t orignum,id0_int_t newnum)
{
	memptr			addr;
	PageListStruct	id0_far *origpage,id0_far *newpage;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (orignum == newnum)
		Quit("PML_TransferPageSpace: Identity replacement");
#endif

	origpage = &PMPages[orignum];
	newpage = &PMPages[newnum];

	if (origpage->locked != pml_Unlocked)
		Quit("PML_TransferPageSpace: Killing locked page");

	if ((origpage->emsPage == -1) && (origpage->mainPage == -1))
		Quit("PML_TransferPageSpace: Reusing non-existent page");

	// Copy page that's about to be purged into XMS
	PML_PutPageInXMS(orignum);

	// Get the address, and force EMS into a physical page if necessary
	addr = PM_GetPageAddress(orignum);

	// Steal the address
	newpage->emsPage = origpage->emsPage;
	newpage->mainPage = origpage->mainPage;

	// Mark replaced page as purged
	origpage->mainPage = origpage->emsPage = -1;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (!addr)
		Quit("PML_TransferPageSpace: Zero replacement");
#endif

	return(addr);
}

//
//	PML_GetAPageBuffer() - A page buffer is needed. Either get it from the
//		main/EMS free pool, or use PML_GiveLRUPage() to find which page to
//		steal the buffer from. Returns a far pointer to the page buffer, and
//		sets the fields inside the given page structure appropriately.
//		If mainonly is true, free EMS will be ignored, and only main pages
//		will be looked at by PML_GiveLRUPage().
//
id0_byte_t id0_far *
PML_GetAPageBuffer(id0_int_t pagenum,id0_boolean_t mainonly)
{
	id0_byte_t			id0_far *addr = id0_nil_t;
	id0_int_t				i,n;
	PMBlockAttr		*used;
	PageListStruct	id0_far *page;

	page = &PMPages[pagenum];
	if ((EMSPagesUsed < EMSPagesAvail) && !mainonly)
	{
		// There's remaining EMS - use it
		page->emsPage = EMSPagesUsed++;
		addr = (id0_byte_t *)PML_GetEMSAddress(page->emsPage,page->locked);
	}
	else if (MainPagesUsed < MainPagesAvail)
	{
		// There's remaining main memory - use it
		for (i = 0,n = -1,used = MainMemUsed;i < PMMaxMainMem;i++,used++)
		{
			if ((*used & pmba_Allocated) && !(*used & pmba_Used))
			{
				n = i;
				*used = (PMBlockAttr)(*used | pmba_Used);
				break;
			}
		}
		if (n == -1)
			Quit("PML_GetPageBuffer: MainPagesAvail lied");
		addr = (id0_byte_t *)MainMemPages[n];
		if (!addr)
			Quit("PML_GetPageBuffer: Purged main block");
		page->mainPage = n;
		MainPagesUsed++;
	}
	else
		addr = (id0_byte_t *)PML_TransferPageSpace(PML_GiveLRUPage(mainonly),pagenum);

	if (!addr)
		Quit("PML_GetPageBuffer: Search failed");
	return(addr);
}

//
//	PML_GetPageFromXMS() - If page is in XMS, find LRU main/EMS page and
//		replace it with the page from XMS. If mainonly is true, will only
//		search for LRU main page.
//	XMSProtectPage is set to the page to be retrieved from XMS, so that if
//		the page from which we're stealing the main/EMS from isn't in XMS,
//		it won't copy over the page that we're trying to get from XMS.
//		(pages that are being purged are copied into XMS, if possible)
//
memptr
PML_GetPageFromXMS(id0_int_t pagenum,id0_boolean_t mainonly)
{
	id0_byte_t			id0_far *checkaddr;
	memptr			addr = id0_nil_t;
	PageListStruct	id0_far *page;

	page = &PMPages[pagenum];
	if (XMSPresent && (page->xmsPage != -1))
	{
		XMSProtectPage = pagenum;
		checkaddr = PML_GetAPageBuffer(pagenum,mainonly);
		if (BE_Cross_GetPtrNormalizedOff(checkaddr))
			Quit("PML_GetPageFromXMS: Non segment pointer");
		addr = (memptr)checkaddr;
		PML_CopyFromXMS(addr,page->xmsPage,page->length);
		XMSProtectPage = -1;
	}

	return(addr);
}

//
//	PML_LoadPage() - A page is not in main/EMS memory, and it's not in XMS.
//		Load it into either main or EMS. If mainonly is true, the page will
//		only be loaded into main.
//
void
PML_LoadPage(id0_int_t pagenum,id0_boolean_t mainonly)
{
	id0_byte_t			id0_far *addr;
	PageListStruct	id0_far *page;

	addr = PML_GetAPageBuffer(pagenum,mainonly);
	page = &PMPages[pagenum];
	PML_ReadFromFile(addr,page->offset,page->length);
}

//
//	PM_GetPage() - Returns the address of the page, loading it if necessary
//		First, check if in Main Memory or EMS
//		Then, check XMS
//		If not in XMS, load into Main Memory or EMS
//
//#pragma warn -pia
memptr
PM_GetPage(id0_int_t pagenum)
{
	memptr	result;

	if (pagenum >= ChunksInFile)
		Quit("PM_GetPage: Invalid page request");

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,10	// bright green
asm	out	dx,al
#endif

	if (!(result = PM_GetPageAddress(pagenum)))
	{
		id0_boolean_t mainonly = (pagenum >= PMSoundStart);
	// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
if (!PMPages[pagenum].offset)	// JDC: sparse page
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	return 0;
#else
	Quit ("Tried to load a sparse page!");
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
		if (!(result = PML_GetPageFromXMS(pagenum,mainonly)))
		{
#if 0 // REFKEEN: Unused variable
			if (PMPages[pagenum].lastHit == PMFrameCount)
				PMThrashing++;
#endif

			PML_LoadPage(pagenum,mainonly);
			result = PM_GetPageAddress(pagenum);
		}
	}
	PMPages[pagenum].lastHit = PMFrameCount;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif

	return(result);
}
//#pragma warn +pia

//
//	PM_SetPageLock() - Sets the lock type on a given page
//		pml_Unlocked: Normal, page can be purged
//		pml_Locked: Cannot be purged
//		pml_EMS?: Same as pml_Locked, but if in EMS, use the physical page
//					specified when returning the address. For sound stuff.
//
void
PM_SetPageLock(id0_int_t pagenum,PMLockType lock)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (pagenum < PMSoundStart)
		Quit("PM_SetPageLock: Locking/unlocking non-sound page");
#endif

	PMPages[pagenum].locked = lock;
}

//
//	PM_Preload() - Loads as many pages as possible into all types of memory.
//		Calls the update function after each load, indicating the current
//		page, and the total pages that need to be loaded (for thermometer).
//
void
PM_Preload(id0_boolean_t (*update)(id0_word_t current,id0_word_t total))
{
	id0_int_t				i,j,
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
					page,totalxms,oogypage;
#else
					page,oogypage;
#endif
	id0_word_t			current,total,
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
					totalnonxms,
#else
					totalnonxms,totalxms,
#endif
					mainfree,maintotal,
					emsfree,emstotal,
					xmsfree,xmstotal;
	memptr			addr;
	PageListStruct	id0_far *p;

	// *** ALPHA RESTORATION ***
	// Similar to the later code revision, but with some differences
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	j = 0;
	for (i = totalnonxms = 0;i < ChunksInFile;i++)
	{
		if ( PMPages[i].emsPage != -1 || PMPages[i].mainPage != -1 )
			continue;			// already in main mem
		totalnonxms++;
	}

	emsfree = EMSPagesAvail - EMSPagesUsed;
	if (EMSPresent && emsfree)
	{
		emstotal = (totalnonxms > emsfree) ? emsfree : totalnonxms;
		totalnonxms -= emstotal;
	}
	else
		emstotal = 0;

	mainfree = MainPagesAvail - MainPagesUsed;
	if (MainPresent && mainfree)
		maintotal = (totalnonxms > mainfree) ? mainfree : totalnonxms;
	else
		maintotal = 0;

	xmsfree = XMSPagesAvail - XMSPagesUsed;
	if (XMSPresent && (xmsfree > 1))
	{
		xmsfree--;
		for (i = totalxms = 0;i < ChunksInFile;i++)
			if (PMPages[i].xmsPage == -1)
				totalxms++;

		xmstotal = (totalxms > xmsfree) ? xmsfree : totalxms;
	}
	else
		xmstotal = 0;

	if (xmstotal)
	{
		if (maintotal)
			maintotal--;
		else if (emstotal)
			emstotal--;
	}


	total = maintotal + emstotal + xmstotal;

	if (!total)
		return;

	current = page = 0;

//
// cache main/ems blocks
//
	while (maintotal + emstotal > current)
	{
		while ( ( PMPages[page].mainPage != -1 || PMPages[page].emsPage != -1 )
			&&	page < ChunksInFile )
			page++;

		if (page >= ChunksInFile)
			break;

		PM_GetPage(page);

		if (update(current,total))
		{
			j = 1;
			break;
		}
		current++;
		page++;
	}

//
// load stuff to XMS
//
	if (xmstotal && !j)
	{
		if (page >= ChunksInFile)
			oogypage = 0;
		else
			oogypage = page;
		addr = PM_GetPage(oogypage);
		if (!addr)
			Quit("PM_Preload: XMS buffer failed");

		if (page + xmstotal >= ChunksInFile)
			page = ChunksInFile - xmstotal;

		p = &PMPages[page];

		while (current < total)
		{
			while ( PMPages[page].xmsPage != -1 && page < ChunksInFile)
			{
				page++;
				p++;
			}

			if (page >= ChunksInFile)
				break;

			p->xmsPage = XMSPagesUsed++;
			if (XMSPagesUsed > XMSPagesAvail)
				Quit("PM_Preload: Exceeded XMS pages");
			if (p->length > PMPageSize)
				Quit("PM_Preload: Page too long");

			PML_ReadFromFile((id0_byte_t id0_far *)addr,p->offset,p->length);
			PML_CopyToXMS((id0_byte_t id0_far *)addr,p->xmsPage,p->length);

			if (update(current,total))
			{
				j = 1;
				break;
			}
			current++;
			page++;
			p++;
		}

		p = &PMPages[oogypage];
		PML_ReadFromFile((id0_byte_t id0_far *)addr,p->offset,p->length);
	}

	update(total,total);

#else // GAMEVER_WOLFREV > GV_WR_WL920312
	mainfree = (MainPagesAvail - MainPagesUsed) + (EMSPagesAvail - EMSPagesUsed);
	xmsfree = (XMSPagesAvail - XMSPagesUsed);

	xmstotal = maintotal = 0;

	for (i = 0;i < ChunksInFile;i++)
	{
		if (!PMPages[i].offset)
			continue;			// sparse

		if ( PMPages[i].emsPage != -1 || PMPages[i].mainPage != -1 )
			continue;			// already in main mem

		if ( mainfree )
		{
			maintotal++;
			mainfree--;
		}
		else if ( xmsfree && (PMPages[i].xmsPage == -1) )
		{
			xmstotal++;
			xmsfree--;
		}
	}


	total = maintotal + xmstotal;

	if (!total)
		return;

	page = 0;
	current = 0;

//
// cache main/ems blocks
//
	while (maintotal)
	{
		while ( !PMPages[page].offset || PMPages[page].mainPage != -1
			||	PMPages[page].emsPage != -1 )
			page++;

		if (page >= ChunksInFile)
			Quit ("PM_Preload: Pages>=ChunksInFile");

		PM_GetPage(page);

		page++;
		current++;
		maintotal--;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (update)
#endif
		update(current,total);
	}

//
// load stuff to XMS
//
	if (xmstotal)
	{
		for (oogypage = 0 ; PMPages[oogypage].mainPage == -1 ; oogypage++)
		;
		addr = PM_GetPage(oogypage);
		if (!addr)
			Quit("PM_Preload: XMS buffer failed");

		while (xmstotal)
		{
			while ( !PMPages[page].offset || PMPages[page].xmsPage != -1 )
				page++;

			if (page >= ChunksInFile)
				Quit ("PM_Preload: Pages>=ChunksInFile");

			p = &PMPages[page];

			p->xmsPage = XMSPagesUsed++;
			if (XMSPagesUsed > XMSPagesAvail)
				Quit("PM_Preload: Exceeded XMS pages");
			if (p->length > PMPageSize)
				Quit("PM_Preload: Page too long");

			PML_ReadFromFile((id0_byte_t id0_far *)addr,p->offset,p->length);
			PML_CopyToXMS((id0_byte_t id0_far *)addr,p->xmsPage,p->length);

			page++;
			current++;
			xmstotal--;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (update)
#endif
			update(current,total);
		}

		p = &PMPages[oogypage];
		PML_ReadFromFile((id0_byte_t id0_far *)addr,p->offset,p->length);
	}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
if (update)
#endif
	update(total,total);
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
}

/////////////////////////////////////////////////////////////////////////////
//
//	General code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PM_NextFrame() - Increments the frame counter and adjusts the thrash
//		avoidence variables
//
//		If currently in panic mode (to avoid thrashing), check to see if the
//			appropriate number of frames have passed since the last time that
//			we would have thrashed. If so, take us out of panic mode.
//
//
void
PM_NextFrame(void)
{
	id0_int_t	i;

	// Frame count overrun - kill the LRU hit entries & reset frame count
	if (++PMFrameCount >= ID0_MAXLONG - 4)
	{
		for (i = 0;i < PMNumBlocks;i++)
			PMPages[i].lastHit = 0;
		PMFrameCount = 0;
	}

#if 0
	for (i = 0;i < PMSoundStart;i++)
	{
		if (PMPages[i].locked)
		{
			id0_char_t buf[40];
			sprintf(buf,"PM_NextFrame: Page %d is locked",i);
			Quit(buf);
		}
	}
#endif

#if 0 // REFKEEN: Unused variables
	if (PMPanicMode)
	{
		// DEBUG - set border color
		if ((!PMThrashing) && (!--PMPanicMode))
		{
			// DEBUG - reset border color
		}
	}
	if (PMThrashing >= PMThrashThreshold)
		PMPanicMode = PMUnThrashThreshold;
	PMThrashing = false;
#endif
}

//
//	PM_Reset() - Sets up caching structures
//
void
PM_Reset(void)
{
	id0_int_t				i;
	PageListStruct	id0_far *page;

	XMSPagesAvail = XMSAvail / PMPageSizeKB;

	EMSPagesAvail = EMSAvail * (EMSPageSizeKB / PMPageSizeKB);
	EMSPhysicalPage = 0;

	MainPagesUsed = EMSPagesUsed = XMSPagesUsed = 0;

//	PMPanicMode = 0;	// REFKEEN: Unused variable

	// Initialize page list
	for (i = 0,page = PMPages;i < PMNumBlocks;i++,page++)
	{
		page->mainPage = -1;
		page->emsPage = -1;
		page->xmsPage = -1;
		page->locked = pml_Unlocked;
	}
}

//
//	PM_Startup() - Start up the Page Mgr
//
void
PM_Startup(void)
{
	id0_boolean_t	nomain,noems,noxms;
	id0_int_t		i;

	if (PMStarted)
		return;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("PM_Startup: ");
#endif

	nomain = noems = noxms = false;
	for (i = 1;i < id0_argc;i++)
	{
		switch (US_CheckParm(id0_argv[i],ParmStrings))
		{
		case 0:
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			BE_ST_printf("Main memory disabled\n");
#endif
			nomain = true;
			break;
		case 1:
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			BE_ST_printf("EMS disabled\n");
#endif
			noems = true;
			break;
		case 2:
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			BE_ST_printf("XMS disabled\n");
#endif
			noxms = true;
			break;
		}
	}

	PML_OpenPageFile();

	if (!noems)
		PML_StartupEMS();
	if (!noxms)
		PML_StartupXMS();

	if (nomain && !EMSPresent)
		Quit("PM_Startup: No main or EMS");
	else
		PML_StartupMainMem();

	PM_Reset();

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (EMSPresent)
		BE_ST_printf("EMS detected (%" ID0_PRIlu "kb)\n", (id0_long_t)EMSPagesAvail<<2);
	else
		BE_ST_printf("EMS not detected\n");

	if (XMSPresent)
		BE_ST_printf("XMS detected (%" ID0_PRIlu "kb)\n", (id0_long_t)XMSPagesAvail<<2);
	else
		BE_ST_printf("XMS not detected\n");

#endif

	PMStarted = true;
}

//
//	PM_Shutdown() - Shut down the Page Mgr
//
void
PM_Shutdown(void)
{
	PML_ShutdownXMS();
	PML_ShutdownEMS();

	if (!PMStarted)
		return;

	PML_ClosePageFile();

	PML_ShutdownMainMem();
}

REFKEEN_NS_E
