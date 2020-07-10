//
//	ID_PM.H
//	Header file for Id Engine's Page Manager
//

//	NOTE! PMPageSize must be an even divisor of EMSPageSize, and >= 1024
#define	EMSPageSize		16384
#define	EMSPageSizeSeg	(EMSPageSize >> 4)
#define	EMSPageSizeKB	(EMSPageSize >> 10)
#define	EMSFrameCount	4
#define	PMPageSize		4096
#define	PMPageSizeSeg	(PMPageSize >> 4)
#define	PMPageSizeKB	(PMPageSize >> 10)
#define	PMEMSSubPage	(EMSPageSize / PMPageSize)

#define	PMMinMainMem	10			// Min acceptable # of pages from main
#define	PMMaxMainMem	100			// Max number of pages in main memory

#define	PMThrashThreshold	1	// Number of page thrashes before panic mode
#define	PMUnThrashThreshold	5	// Number of non-thrashing frames before leaving panic mode

typedef	enum
		{
			pml_Unlocked,
			pml_Locked
		} PMLockType;

typedef	enum
		{
			pmba_Unused = 0,
			pmba_Used = 1,
			pmba_Allocated = 2
		} PMBlockAttr;

typedef	struct
		{
			id0_longword_t	offset;		// Offset of chunk into file
			id0_word_t		length;		// Length of the chunk

			id0_int_t			xmsPage;	// If in XMS, (xmsPage * PMPageSize) gives offset into XMS handle

			PMLockType	locked;		// If set, this page can't be purged
			id0_int_t			emsPage;	// If in EMS, logical page/offset into page
			id0_int_t			mainPage;	// If in Main, index into handle array

			id0_longword_t	lastHit;	// Last frame number of hit
		} PageListStruct;

typedef	struct
		{
			id0_int_t			baseEMSPage;	// Base EMS page for this phys frame
			id0_longword_t	lastHit;		// Last frame number of hit
		} EMSListStruct;

extern	id0_boolean_t			XMSPresent,EMSPresent;
extern	id0_word_t			XMSPagesAvail,EMSPagesAvail;

extern	id0_word_t			ChunksInFile,
						PMSpriteStart,PMSoundStart;
extern	PageListStruct	id0_far *PMPages;

#define	PM_GetSoundPage(v)	PM_GetPage(PMSoundStart + (v))
#define	PM_GetSpritePage(v)	PM_GetPage(PMSpriteStart + (v))

#define	PM_LockMainMem()	PM_SetMainMemPurge(0)
#define	PM_UnlockMainMem()	PM_SetMainMemPurge(3)


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
extern	id0_char_t	PageFileName[13];
#endif


extern	void	PM_Startup(void),
				PM_Shutdown(void),
				PM_Reset(void),
				PM_Preload(id0_boolean_t (*update)(id0_word_t current,id0_word_t total)),
				PM_NextFrame(void),
				PM_SetPageLock(id0_int_t pagenum,PMLockType lock),
				PM_SetMainPurge(id0_int_t level),
				PM_CheckMainMem(void);
extern	memptr	PM_GetPageAddress(id0_int_t pagenum),
				PM_GetPage(id0_int_t pagenum);		// Use this one to cache page

void PM_SetMainMemPurge(id0_int_t level);
