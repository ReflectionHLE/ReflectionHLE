//
//	ID Engine
//	ID_SD.h - Sound Manager Header
//	Version for Wolfenstein
//	By Jason Blochowiak
//

#ifndef	__ID_SD__
#define	__ID_SD__

// *** PRE-V1.4 APOGEE RESTORATION ***
// With no declaration, so the compiler assumes args are passed as 16-bit ints
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
void	alOut(id0_byte_t n,id0_byte_t b);
#endif

#ifdef	__DEBUG__
#define	__DEBUG_SoundMgr__
#endif

// REFKEEN - Macro used for disabling unused code, may be re-enabled if required
#define REFKEEN_SD_ENABLE_SOUNDSOURCE 0

#define	TickBase	70		// 70Hz per tick - used as a base for timer 0

typedef	enum	{
					sdm_Off,
					sdm_PC,sdm_AdLib,
				}	SDMode;
typedef	enum	{
					smm_Off,smm_AdLib
				}	SMMode;
typedef	enum	{
					// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
					sds_Off,sds_SoundBlaster
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
					sds_Off,sds_SoundSource,sds_SoundBlaster
#else
					sds_Off,sds_PC,sds_SoundSource,sds_SoundBlaster
#endif
				}	SDSMode;
typedef	struct
		{
			id0_longword_t	length;
			id0_word_t		priority;
		} SoundCommon;

//	PC Sound stuff
#define	pcTimer		0x42
#define	pcTAccess	0x43
#define	pcSpeaker	0x61

#define	pcSpkBits	3

typedef	struct
		{
			SoundCommon	common;
			id0_byte_t		data[1];
		} PCSound;

// 	Registers for the Sound Blaster card - needs to be offset by n0 (0x10,0x20,0x30,0x40,0x50,0x60)
#define	sbReset		0x206	// W
#define	sbFMStatus	0x208	// R
#define	sbFMAddr	0x208	// W
#define	sbFMData	0x209	// W
#define	sbReadData	0x20a	// R
#define	sbWriteCmd	0x20c	// W
#define	sbWriteData	0x20c	// W
#define	sbWriteStat	0x20c	// R
#define	sbDataAvail	0x20e	// R

//	Registers for the Sound Blaster Pro card - needs to be offset by n0 (0x20 or 0x40)
#define	sbpLFMStatus	0x200	// R
#define	sbpLFMAddr		0x200	// W
#define	sbpLFMData		0x201	// W
#define	sbpRFMStatus	0x202	// R
#define	sbpRFMAddr		0x202	// W
#define	sbpRFMData		0x203	// W
#define	sbpMixerAddr	0x204	// W
#define	sbpMixerData	0x205	// RW
#define	sbpCDData		0x210	// R
#define	sbpCDCommand	0x210	// W
#define	sbpCDStatus		0x211	// R
#define	sbpCDReset		0x212	// W

//	SBPro Mixer addresses
#define	sbpmReset		0x00
#define	sbpmVoiceVol	0x04
#define	sbpmMicMix		0x0a
#define	sbpmFilterADC	0x0c
#define	sbpmControl		0x0e
#define	sbpmMasterVol	0x22
#define	sbpmFMVol		0x26
#define	sbpmCDVol		0x28
#define	sbpmLineVol		0x2e

typedef	struct
		{
			SoundCommon	common;
			id0_word_t		hertz;
			id0_byte_t		bits,
						reference,
						data[1];
		} SampledSound;

// 	Registers for the AdLib card
#define	alFMStatus	0x388	// R
#define	alFMAddr	0x388	// W
#define	alFMData	0x389	// W

//	Register addresses
// Operator stuff
#define	alChar		0x20
#define	alScale		0x40
#define	alAttack	0x60
#define	alSus		0x80
#define	alWave		0xe0
// Channel stuff
#define	alFreqL		0xa0
#define	alFreqH		0xb0
#define	alFeedCon	0xc0
// Global stuff
#define	alEffects	0xbd

typedef	struct
		{
			id0_byte_t	mChar,cChar,
					mScale,cScale,
					mAttack,cAttack,
					mSus,cSus,
					mWave,cWave,
					nConn,

					// These are only for Muse - these bytes are really unused
					voice,
					mode,
					unused[3];
		} Instrument;

typedef	struct
		{
			SoundCommon	common;
			Instrument	inst;
			id0_byte_t		block,
						data[1];
		} AdLibSound;

//
//	Sequencing stuff
//
#define	sqMaxTracks	10
#define	sqMaxMoods	1	// DEBUG

#define	sev_Null		0	// Does nothing
#define	sev_NoteOff		1	// Turns a note off
#define	sev_NoteOn		2	// Turns a note on
#define	sev_NotePitch	3	// Sets the pitch of a currently playing note
#define	sev_NewInst		4	// Installs a new instrument
#define	sev_NewPerc		5	// Installs a new percussive instrument
#define	sev_PercOn		6	// Turns a percussive note on
#define	sev_PercOff		7	// Turns a percussive note off
#define	sev_SeqEnd		-1	// Terminates a sequence

// 	Flags for MusicGroup.flags
#define	sf_Melodic		0
#define	sf_Percussive	1

#if 1
typedef	struct
		{
			id0_word_t	length,
					values[1];
		} MusicGroup;
#else
typedef	struct
		{
			id0_word_t	flags,
					count,
					offsets[1];
		} MusicGroup;
#endif

// *** S3DNA RESTORATION ***
// inst_t is mentioned in debugging symbols, while ActiveTrack is not (and is also unused)
#ifdef GAMEVER_NOAH3D
typedef struct	{
			id0_byte_t	mChar,cChar,
					mScale,cScale,
					mAttack,cAttack,
					mSustain,cSustain,
					mWave,cWave,
					mFeedConn;
		} inst_t;
#else
typedef	struct
		{
			/* This part needs to be set up by the user */
			id0_word_t        mood,id0_far *moods[sqMaxMoods];

			/* The rest is set up by the code */
			Instrument	inst;
			id0_boolean_t		percussive;
			id0_word_t		id0_far *seq;
			id0_longword_t	nextevent;
		} ActiveTrack;
#endif

#define	sqmode_Normal		0
#define	sqmode_FadeIn		1
#define	sqmode_FadeOut		2

#define	sqMaxFade		64	// DEBUG


// Global variables
extern	id0_boolean_t		AdLibPresent,
					// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
					SoundSourcePresent,
#endif
					SoundBlasterPresent,
					NeedsMusic,					// For Caching Mgr
					SoundPositioned;
extern	SDMode		SoundMode;
extern	SDSMode		DigiMode;
extern	SMMode		MusicMode;
extern	id0_boolean_t		DigiPlaying;
extern	id0_int_t			DigiMap[];
extern	id0_longword_t	TimeCount;					// Global time in ticks

// Function prototypes
extern	void	SD_Startup(void),
				SD_Shutdown(void),
				SD_Default(id0_boolean_t gotit,SDMode sd,SMMode sm),

// *** ALPHA RESTORATION ***
// Not exactly pretty, but works
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		SD_PlaySound(soundnames sound),
#else
				SD_PositionSound(id0_int_t leftvol,id0_int_t rightvol);
extern	id0_boolean_t	SD_PlaySound(soundnames sound);
extern	void	SD_SetPosition(id0_int_t leftvol,id0_int_t rightvol),
#endif
				SD_StopSound(void),
				SD_WaitSoundDone(void),

				SD_StartMusic(MusicGroup id0_far *music),
				SD_MusicOn(void),
				SD_MusicOff(void),
				SD_FadeOutMusic(void),

				SD_SetUserHook(void (*hook)(void));
extern	id0_boolean_t	SD_MusicPlaying(void),
				SD_SetSoundMode(SDMode mode),
				SD_SetMusicMode(SMMode mode);
extern	id0_word_t	SD_SoundPlaying(void);

extern	void	SD_SetDigiDevice(SDSMode),
				// *** ALPHA RESTORATION ***/
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
				SD_PlayDigitized(id0_word_t which),
#else
				SD_PlayDigitized(id0_word_t which,id0_int_t leftpos,id0_int_t rightpos),
#endif
				SD_StopDigitized(void),
				SD_Poll(void);

#ifdef	_MUSE_	// MUSE Goes directly to the lower level routines
extern	void	SDL_PCPlaySound(PCSound id0_far *sound),
				SDL_PCStopSound(void),
				SDL_ALPlaySound(AdLibSound id0_far *sound),
				SDL_ALStopSound(void);
#endif

#endif

