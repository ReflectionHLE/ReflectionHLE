// ID_CA.H
//===========================================================================

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define NUMMAPS		30
#define MAPPLANES	3
#else
#define NUMMAPS		60
#define MAPPLANES	2
#endif

#define UNCACHEGRCHUNK(chunk)	{MM_FreePtr(&grsegs[chunk]);grneeded[chunk]&=~ca_levelbit;}

//===========================================================================

typedef	struct
{
	id0_long_t		planestart[3];
	id0_unsigned_t	planelength[3];
	id0_unsigned_t	width,height;
	id0_char_t		name[16];
} maptype;

//===========================================================================

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	id0_boolean_t		CA_Started;
#endif

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
extern	id0_char_t		audioname[13];
#endif

extern	id0_byte_t 		id0_seg	*tinf;
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	id0_int_t			mapon;
#endif

extern	id0_unsigned_t	id0_seg	*mapsegs[MAPPLANES];
extern	maptype		id0_seg	*mapheaderseg[NUMMAPS];
extern	id0_byte_t		id0_seg	*audiosegs[NUMSNDCHUNKS];
extern	void		id0_seg	*grsegs[NUMCHUNKS];

extern	id0_byte_t		id0_far	grneeded[NUMCHUNKS];
extern	id0_byte_t		ca_levelbit,ca_levelnum;

extern	id0_char_t		*titleptr[8];

extern	BE_FILE_T			profilehandle,debughandle;

extern	id0_char_t		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			mfilename[10],
			aheadname[10],
			afilename[10];

extern id0_long_t		id0_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
extern id0_long_t		id0_seg *audiostarts;	// array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern	void	(*drawcachebox)		(id0_char_t *title, id0_unsigned_t numcache);
extern	void	(*updatecachebox)	(void);
extern	void	(*finishcachebox)	(void);

//===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite (id0_unsigned_t segment,id0_unsigned_t source,id0_unsigned_t dest,
	id0_unsigned_t width, id0_unsigned_t height, id0_unsigned_t pixshift);

//===========================================================================

void CA_OpenDebug (void);
void CA_CloseDebug (void);
id0_boolean_t CA_FarRead (BE_FILE_T handle, id0_byte_t id0_far *dest, id0_long_t length);
id0_boolean_t CA_FarWrite (BE_FILE_T handle, id0_byte_t id0_far *source, id0_long_t length);
id0_boolean_t CA_ReadFile (id0_char_t *filename, memptr *ptr);
id0_boolean_t CA_LoadFile (id0_char_t *filename, memptr *ptr);
id0_boolean_t CA_WriteFile (id0_char_t *filename, void id0_far *ptr, id0_long_t length);

id0_long_t CA_RLEWCompress (id0_unsigned_t id0_huge *source, id0_long_t length, id0_unsigned_t id0_huge *dest,
  id0_unsigned_t rlewtag);

void CA_RLEWexpand (id0_unsigned_t id0_huge *source, id0_unsigned_t id0_huge *dest,id0_long_t length,
  id0_unsigned_t rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk (id0_int_t chunk);
void CA_LoadAllSounds (void);

void CA_UpLevel (void);
void CA_DownLevel (void);

void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheGrChunk (id0_int_t chunk);
void CA_CacheMap (id0_int_t mapnum);

void CA_CacheMarks (void);

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void CA_CacheScreen (id0_int_t chunk);
#endif
