//
//	ID Engine
//	ID_SD.c - Sound Manager for Wolfenstein 3D
//	v1.2
//	By Jason Blochowiak
//

//
//	This module handles dealing with generating sound on the appropriate
//		hardware
//
//	Depends on: User Mgr (for parm checking)
//
//	Globals:
//		For User Mgr:
//			SoundSourcePresent - Sound Source thingie present?
//			SoundBlasterPresent - SoundBlaster card present?
//			AdLibPresent - AdLib card present?
//			SoundMode - What device is used for sound effects
//				(Use SM_SetSoundMode() to set)
//			MusicMode - What device is used for music
//				(Use SM_SetMusicMode() to set)
//			DigiMode - What device is used for digitized sound effects
//				(Use SM_SetDigiDevice() to set)
//
//		For Cache Mgr:
//			NeedsDigitized - load digitized sounds?
//			NeedsMusic - load music?
//

#pragma hdrstop		// Wierdo thing with MUSE

//#include <dos.h>

#ifdef	_MUSE_      // Will be defined in ID_Types.h
#include "id_sd.h"
#else
#include "id_heads.h"
#endif
#pragma	hdrstop
#pragma	warn	-pia

#ifdef	nil
#undef	nil
#endif
#define	nil	0

// *** S3DNA RESTORATION ***
// Guessing some variables were redefined to be static
#ifdef GAMEVER_NOAH3D
#define GAMEVER_COND_STATIC static
#else
#define GAMEVER_COND_STATIC
#endif

// *** ALPHA RESTORATION ***
// A similar definition for SD_PlaySound
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define GAMEVER_COND_RET(x) return
#else
#define GAMEVER_COND_RET(x) return(x)
#endif



#define	SDL_SoundFinished()	{SoundNumber = SoundPriority = 0;}

// Macros for SoundBlaster stuff
#define	sbOut(n,b)	outportb((n) + sbLocation,b)
#define	sbIn(n)		inportb((n) + sbLocation)
#define	sbWriteDelay()	while (sbIn(sbWriteStat) & 0x80);
#define	sbReadDelay()	while (sbIn(sbDataAvail) & 0x80);

// Macros for AdLib stuff
#define	selreg(n)	outportb(alFMAddr,n)
#define	writereg(n)	outportb(alFMData,n)
#define	readstat()	inportb(alFMStatus)

//	Imports from ID_SD_A.ASM
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	void interrupt		SDL_t0FastAsmService(void),
						SDL_t0SlowAsmService(void);
#else
extern	void			SDL_SetDS(void),
						SDL_IndicatePC(id0_boolean_t on);
extern	void interrupt	SDL_t0ExtremeAsmService(void),
						SDL_t0FastAsmService(void),
						SDL_t0SlowAsmService(void);
#endif

//	Global variables
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_boolean_t		AdLibPresent,
#else
	id0_boolean_t		SoundSourcePresent,
				AdLibPresent,
#endif
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
				SoundBlasterPresent,
				NeedsDigitized,NeedsMusic;
#else
				SoundBlasterPresent,SBProPresent,
				NeedsDigitized,NeedsMusic,
				SoundPositioned;
#endif
	SDMode		SoundMode;
	SMMode		MusicMode;
	SDSMode		DigiMode;
	id0_longword_t	TimeCount;
	id0_word_t		HackCount;
	id0_word_t		*SoundTable;	// Really * seg *SoundTable, but that don't work
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	id0_boolean_t		ssIsTandy;
	id0_word_t		ssPort = 2;
#endif
#endif
	id0_int_t			DigiMap[LASTSOUND];

//	Internal variables
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
static	id0_boolean_t			DigiPlaying;
#endif
static	id0_boolean_t			SD_Started;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		id0_boolean_t			UseFastService;
#else
		GAMEVER_COND_STATIC id0_boolean_t			nextsoundpos;
#endif
		id0_longword_t		TimerDivisor,TimerCount;
static	id0_char_t			*ParmStrings[] =
						{
							"noal",
							"nosb",
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
							"nopro",
#endif
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
							"noss",
							"sst",
							"ss1",
							"ss2",
							"ss3",
#endif
							nil
						};
static	void			(*SoundUserHook)(void);
// *** S3DNA RESTORATION ***
// Apparently variables moved around here, while adding the MIDI handlers
#ifdef GAMEVER_NOAH3D
static	soundnames		DigiNumber;
static	id0_word_t			DigiPriority;
	soundnames		SoundNumber;
	id0_word_t			SoundPriority;
#else
		soundnames		SoundNumber,DigiNumber;
		id0_word_t			SoundPriority,DigiPriority;
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		GAMEVER_COND_STATIC id0_int_t				LeftPosition,RightPosition;
#endif
		void interrupt	(*t0OldService)(void);
		id0_long_t			LocalTime;
		GAMEVER_COND_STATIC id0_word_t			TimerRate;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
static	id0_word_t			DigiLeft,DigiPage;
	id0_word_t			NumDigi;
#else
		id0_word_t			NumDigi,DigiLeft,DigiPage;
#endif
		id0_word_t			id0_seg *DigiList;
		GAMEVER_COND_STATIC id0_word_t			DigiLastStart,DigiLastEnd;
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		id0_boolean_t			DigiPlaying;
#endif
static	id0_boolean_t			DigiMissed,DigiLastSegment;
static	memptr			DigiNextAddr;
static	id0_word_t			DigiNextLen;

//	SoundBlaster variables

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
static	id0_boolean_t					sbNoCheck;
#else
static	id0_boolean_t					sbNoCheck,sbNoProCheck;
#endif
static	volatile id0_boolean_t		sbSamplePlaying;
static	id0_byte_t					sbOldIntMask = -1;
static	volatile id0_byte_t			id0_huge *sbNextSegPtr;
// *** ALPHA RESTORATION ***
// Use hardcoded DMA channel, as in Keen Dreams
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
static	id0_byte_t					sbDMA = 1,
								sbDMAa1 = 0x83,sbDMAa2 = 2,sbDMAa3 = 3,
								sba1Vals[] = {0x87,0x83,0,0x82},
								sba2Vals[] = {0,2,0,6},
								sba3Vals[] = {1,3,0,7};
#endif
static	id0_int_t						sbLocation = -1,sbInterrupt = 7,sbIntVec = 0xf,
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
								sbIntVectors[] = {-1,-1,0xa,0xb,-1,0xd,-1,0xf};
#else
								sbIntVectors[] = {-1,-1,0xa,0xb,-1,0xd,-1,0xf,-1,-1,-1};
#endif
static	volatile id0_longword_t		sbNextSegLen;
static	volatile SampledSound	id0_huge *sbSamples;
static	void interrupt			(*sbOldIntHand)(void);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
static	id0_byte_t					sbpOldFMMix,sbpOldVOCMix;
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
//	SoundSource variables
		id0_boolean_t				ssNoCheck;
		id0_boolean_t				ssActive;
		id0_word_t				ssControl,ssStatus,ssData;
		id0_byte_t				ssOn,ssOff;
		volatile id0_byte_t		id0_far *ssSample;
		volatile id0_longword_t	ssLengthLeft;
#endif
#endif

//	PC Sound variables
		volatile id0_byte_t	pcLastSample,id0_far *pcSound;
		id0_longword_t		pcLengthLeft;
		id0_word_t			pcSoundLookup[255];

//	AdLib variables
		GAMEVER_COND_STATIC id0_boolean_t			alNoCheck;
		id0_byte_t			id0_far *alSound;
		id0_word_t			alBlock;
		id0_longword_t		alLengthLeft;
		id0_longword_t		alTimeCount;
		GAMEVER_COND_STATIC Instrument		alZeroInst;

// This table maps channel numbers to carrier and modulator op cells
static	id0_byte_t			carriers[9] =  { 3, 4, 5,11,12,13,19,20,21},
						modifiers[9] = { 0, 1, 2, 8, 9,10,16,17,18},
// This table maps percussive voice numbers to op cells
						pcarriers[5] = {19,0xff,0xff,0xff,0xff},
						pmodifiers[5] = {16,17,18,20,21};

//	Sequencer variables
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		id0_boolean_t			midiOn = false;
		id0_int_t			midiError = 0;
		float			midiTimeScale = 1.86;

		id0_byte_t	id0_far	*midiData, id0_far *midiDataStart;
		id0_byte_t	midiRunningStatus;
		id0_longword_t	midiLength, midiDeltaTime;

static	id0_word_t			alFXReg;

static	id0_byte_t	id0_far	*seqPtr;
static	id0_long_t	seqLength;
#else
		id0_boolean_t			sqActive;
static	id0_word_t			alFXReg;
static	ActiveTrack		*tracks[sqMaxTracks],
						mytracks[sqMaxTracks];
static	id0_word_t			sqMode,sqFadeStep;
		id0_word_t			id0_far *sqHack,id0_far *sqHackPtr,sqHackLen,sqHackSeqLen;
		id0_long_t			sqHackTime;
#endif

//	Internal routines
		void			SDL_DigitizedDone(void);

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetTimer0() - Sets system timer 0 to the specified speed
//
///////////////////////////////////////////////////////////////////////////
#pragma	argsused
static void
SDL_SetTimer0(id0_word_t speed)
{
#ifndef TPROF	// If using Borland's profiling, don't screw with the timer
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	pushf
asm	cli
#endif

	outportb(0x43,0x36);				// Change timer 0
	outportb(0x40,speed);
	outportb(0x40,speed >> 8);
	// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	// Kludge to handle special case for digitized PC sounds
	if (TimerDivisor == (1192030 / (TickBase * 100)))
		TimerDivisor = (1192030 / (TickBase * 10));
	else
#endif
		TimerDivisor = speed;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	popf
#endif
#else
	TimerDivisor = 0x10000;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetIntsPerSec() - Uses SDL_SetTimer0() to set the number of
//		interrupts generated by system timer 0 per second
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SetIntsPerSec(id0_word_t ints)
{
	TimerRate = ints;
	SDL_SetTimer0(1192030 / ints);
}

static void
SDL_SetTimerSpeed(void)
{
	id0_word_t	rate;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)

	if
	(
		(MusicMode == smm_AdLib)
	||	((DigiMode == sds_SoundSource) && DigiPlaying)
	)
	{
		rate = TickBase * 10;
		UseFastService = true;
	}
	else
	{
		rate = TickBase * 2;
		UseFastService = false;
	}

	if (rate != TimerRate)
	{
		setvect(8,UseFastService ? SDL_t0FastAsmService : SDL_t0SlowAsmService);
		SDL_SetIntsPerSec(rate);
		TimerRate = rate;
	}
#else
	void interrupt	(*isr)(void);

	// *** S3DNA RSETORATION ***
#ifndef GAMEVER_NOAH3D
	if ((DigiMode == sds_PC) && DigiPlaying)
	{
		rate = TickBase * 100;
		isr = SDL_t0ExtremeAsmService;
	}
	else if
	(
		(MusicMode == smm_AdLib)
	||	((DigiMode == sds_SoundSource) && DigiPlaying)
	)
#else
	if (MusicMode == smm_AdLib)
#endif
	{
		rate = TickBase * 10;
		isr = SDL_t0FastAsmService;
	}
	else
	{
		rate = TickBase * 2;
		isr = SDL_t0SlowAsmService;
	}

	if (rate != TimerRate)
	{
		setvect(8,isr);
		SDL_SetIntsPerSec(rate);
		TimerRate = rate;
	}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
}

//
//	SoundBlaster code
//

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBStopSample() - Stops any active sampled sound and causes DMA
//		requests from the SoundBlaster to cease
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SBStopSample(void)
{
	id0_byte_t	is;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	pushf
asm	cli
#endif

	if (sbSamplePlaying)
	{
		sbSamplePlaying = false;
		// *** PRE-V1.4 APOGEE RESTORATION (EXCLUDING V1.0) ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		asm	pushf
		asm	cli
#endif

		sbWriteDelay();
		sbOut(sbWriteCmd,0xd0);	// Turn off DSP DMA

		is = inportb(0x21);	// Restore interrupt mask bit
		if (sbOldIntMask & (1 << sbInterrupt))
			is |= (1 << sbInterrupt);
		else
			is &= ~(1 << sbInterrupt);
		outportb(0x21,is);
		// *** PRE-V1.4 APOGEE RESTORATION (EXCLUDING V1.0) ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		asm	popf
#endif
	}

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBPlaySeg() - Plays a chunk of sampled sound on the SoundBlaster
//	Insures that the chunk doesn't cross a bank boundary, programs the DMA
//	 controller, and tells the SB to start doing DMA requests for DAC
//
///////////////////////////////////////////////////////////////////////////
static id0_longword_t
SDL_SBPlaySeg(volatile id0_byte_t id0_huge *data,id0_longword_t length)
{
	id0_unsigned_t		datapage;
	id0_longword_t		dataofs,uselen;

	uselen = length;
	datapage = FP_SEG(data) >> 12;
	dataofs = ((FP_SEG(data) & 0xfff) << 4) + FP_OFF(data);
	if (dataofs >= 0x10000)
	{
		datapage++;
		dataofs -= 0x10000;
	}

	if (dataofs + uselen > 0x10000)
		uselen = 0x10000 - dataofs;

	uselen--;

	// Program the DMA controller

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
asm	pushf
asm	cli
#endif
	// *** ALPHA RESTORATION ***
	// Use hardcoded DMA channel, as in Keen Dreams
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	outportb(0x0a,5);							// Mask off channel 1 DMA
#else
	outportb(0x0a,sbDMA | 4);					// Mask off DMA on channel sbDMA
#endif
	outportb(0x0c,0);							// Clear byte ptr flip-flop to lower byte
	outportb(0x0b,0x49);						// Set transfer mode for D/A conv
	// *** ALPHA RESTORATION ***
	// Use hardcoded DMA channel, as in Keen Dreams
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	outportb(0x02,(id0_byte_t)dataofs);				// Give LSB of address
	outportb(0x02,(id0_byte_t)(dataofs >> 8));		// Give MSB of address
	outportb(0x83,(id0_byte_t)datapage);				// Give page of address
	outportb(0x03,(id0_byte_t)uselen);				// Give LSB of length
	outportb(0x03,(id0_byte_t)(uselen >> 8));			// Give MSB of length
	outportb(0x0a,1);							// Turn on channel 1 DMA
#else
	outportb(sbDMAa2,(id0_byte_t)dataofs);			// Give LSB of address
	outportb(sbDMAa2,(id0_byte_t)(dataofs >> 8));		// Give MSB of address
	outportb(sbDMAa1,(id0_byte_t)datapage);			// Give page of address
	outportb(sbDMAa3,(id0_byte_t)uselen);				// Give LSB of length
	outportb(sbDMAa3,(id0_byte_t)(uselen >> 8));		// Give MSB of length
	outportb(0x0a,sbDMA);						// Re-enable DMA on channel sbDMA
#endif

	// Start playing the thing

	// *** PRE-V1.4 APOGEE RESTORATION (EXCLUDING V1.0) ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
asm	popf
asm	pushf
asm	cli
#endif
	sbWriteDelay();
	sbOut(sbWriteCmd,0x14);
	sbWriteDelay();
	sbOut(sbWriteData,(id0_byte_t)uselen);
	sbWriteDelay();
	sbOut(sbWriteData,(id0_byte_t)(uselen >> 8));
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
asm	popf
#endif

	return(uselen + 1);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBService() - Services the SoundBlaster DMA interrupt
//
///////////////////////////////////////////////////////////////////////////
static void interrupt
SDL_SBService(void)
{
	id0_longword_t	used;

	sbIn(sbDataAvail);	// Ack interrupt to SB

	if (sbNextSegPtr)
	{
		used = SDL_SBPlaySeg(sbNextSegPtr,sbNextSegLen);
		if (sbNextSegLen <= used)
			sbNextSegPtr = nil;
		else
		{
			sbNextSegPtr += used;
			sbNextSegLen -= used;
		}
	}
	else
	{
		SDL_SBStopSample();
		SDL_DigitizedDone();
	}

	outportb(0x20,0x20);	// Ack interrupt
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBPlaySample() - Plays a sampled sound on the SoundBlaster. Sets up
//		DMA to play the sound
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SBPlaySample(id0_byte_t id0_huge *data,id0_longword_t len)
{
	id0_longword_t	used;

	SDL_SBStopSample();

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	pushf
asm	cli
#endif

	used = SDL_SBPlaySeg(data,len);
	if (len <= used)
		sbNextSegPtr = nil;
	else
	{
		sbNextSegPtr = data + used;
		sbNextSegLen = len - used;
	}

	// Save old interrupt status and unmask ours

	// *** PRE-V1.4 APOGEE RESTORATION (EXCLUDING V1.0) ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
asm	pushf
asm	cli
#endif
	sbOldIntMask = inportb(0x21);
	outportb(0x21,sbOldIntMask & ~(1 << sbInterrupt));

	sbWriteDelay();
	sbOut(sbWriteCmd,0xd4);						// Make sure DSP DMA is enabled

	// *** PRE-V1.4 APOGEE RESTORATION (EXCLUDING V1.0) ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
asm	popf
#endif

	sbSamplePlaying = true;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	popf
#endif
}

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
///////////////////////////////////////////////////////////////////////////
//
//	SDL_PositionSBP() - Sets the attenuation levels for the left and right
//		channels by using the mixer chip on the SB Pro. This hits a hole in
//		the address map for normal SBs.
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PositionSBP(id0_int_t leftpos,id0_int_t rightpos)
{
	id0_byte_t	v;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (!SBProPresent)
		return;
#endif

	leftpos = 15 - leftpos;
	rightpos = 15 - rightpos;
	v = ((leftpos & 0x0f) << 4) | (rightpos & 0x0f);

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	pushf
asm	cli
#endif

	sbOut(sbpMixerAddr,sbpmVoiceVol);
	sbOut(sbpMixerData,v);

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
asm	popf
#endif
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CheckSB() - Checks to see if a SoundBlaster resides at a
//		particular I/O location
//
///////////////////////////////////////////////////////////////////////////
static id0_boolean_t
SDL_CheckSB(id0_int_t port)
{
	id0_int_t	i;

	sbLocation = port << 4;		// Initialize stuff for later use

	sbOut(sbReset,true);		// Reset the SoundBlaster DSP
asm	mov	dx,0x388				// Wait >4usec
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx

	sbOut(sbReset,false);		// Turn off sb DSP reset
asm	mov	dx,0x388				// Wait >100usec
asm	mov	cx,100
usecloop:
asm	in	al,dx
asm	loop usecloop

	for (i = 0;i < 100;i++)
	{
		if (sbIn(sbDataAvail) & 0x80)		// If data is available...
		{
			if (sbIn(sbReadData) == 0xaa)	// If it matches correct value
				return(true);
			else
			{
				sbLocation = -1;			// Otherwise not a SoundBlaster
				return(false);
			}
		}
	}
	sbLocation = -1;						// Retry count exceeded - fail
	return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	Checks to see if a SoundBlaster is in the system. If the port passed is
//		-1, then it scans through all possible I/O locations. If the port
//		passed is 0, then it uses the default (2). If the port is >0, then
//		it just passes it directly to SDL_CheckSB()
//
///////////////////////////////////////////////////////////////////////////
static id0_boolean_t
SDL_DetectSoundBlaster(id0_int_t port)
{
	id0_int_t	i;

	if (port == 0)					// If user specifies default, use 2
		port = 2;
	if (port == -1)
	{
		if (SDL_CheckSB(2))			// Check default before scanning
			return(true);

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (SDL_CheckSB(4))			// Check other SB Pro location before scan
			return(true);
#endif

		for (i = 1;i <= 6;i++)		// Scan through possible SB locations
		{
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			if (i == 2)
#else
			if ((i == 2) || (i == 4))
#endif
				continue;

			if (SDL_CheckSB(i))		// If found at this address,
				return(true);		//	return success
		}
		return(false);				// All addresses failed, return failure
	}
	else
		return(SDL_CheckSB(port));	// User specified address or default
}

// *** ALPHA RESTORATION ***
// Use hardcoded DMA channel, as in Keen Dreams
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBSetDMA() - Sets the DMA channel to be used by the SoundBlaster
//		code. Sets up sbDMA, and sbDMAa1-sbDMAa3 (used by SDL_SBPlaySeg()).
//
///////////////////////////////////////////////////////////////////////////
void
SDL_SBSetDMA(id0_byte_t channel)
{
	if (channel > 3)
		Quit("SDL_SBSetDMA() - invalid SoundBlaster DMA channel");

	sbDMA = channel;
	sbDMAa1 = sba1Vals[channel];
	sbDMAa2 = sba2Vals[channel];
	sbDMAa3 = sba3Vals[channel];
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartSB() - Turns on the SoundBlaster
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartSB(void)
{
	id0_byte_t	timevalue,test;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	sbIntVec = sbIntVectors[sbInterrupt];
	if (sbIntVec < 0)
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		Quit("SDL_StartSB: Illegal interrupt number for SoundBlaster");
#else
		Quit("SDL_StartSB: Illegal or unsupported interrupt number for SoundBlaster");
#endif
#endif

	sbOldIntHand = getvect(sbIntVec);	// Get old interrupt handler
	setvect(sbIntVec,SDL_SBService);	// Set mine

	sbWriteDelay();
	sbOut(sbWriteCmd,0xd1);				// Turn on DSP speaker

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	// Set the SoundBlaster DAC time constant for 11KHz
	timevalue = 256 - (1000000 / 11000);
#else
	// Set the SoundBlaster DAC time constant for 7KHz
	timevalue = 256 - (1000000 / 7000);
#endif
	sbWriteDelay();
	sbOut(sbWriteCmd,0x40);
	sbWriteDelay();
	sbOut(sbWriteData,timevalue);

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	SBProPresent = false;
	if (sbNoProCheck)
		return;
#endif

	// Check to see if this is a SB Pro
	sbOut(sbpMixerAddr,sbpmFMVol);
	sbpOldFMMix = sbIn(sbpMixerData);
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	sbOut(sbpMixerAddr,0x2e);
	sbOut(sbpMixerData,0xa5);
	test = sbIn(sbpMixerData);
	if (test == 0xa5)
#else
	sbOut(sbpMixerData,0xbb);
	test = sbIn(sbpMixerData);
	if (test == 0xbb)
#endif
	{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		sbOut(sbpMixerAddr,0x26);
#endif
		// Boost FM output levels to be equivilent with digitized output
		sbOut(sbpMixerData,0xff);
		test = sbIn(sbpMixerData);
		if (test == 0xff)
		{
			SBProPresent = true;

			// Save old Voice output levels (SB Pro)
			sbOut(sbpMixerAddr,sbpmVoiceVol);
			sbpOldVOCMix = sbIn(sbpMixerData);

			// Turn SB Pro stereo DAC off
			sbOut(sbpMixerAddr,sbpmControl);
			sbOut(sbpMixerData,0);				// 0=off,2=on
		}
	}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutSB() - Turns off the SoundBlaster
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutSB(void)
{
	SDL_SBStopSample();

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (SBProPresent)
	{
		// Restore FM output levels (SB Pro)
		sbOut(sbpMixerAddr,sbpmFMVol);
		sbOut(sbpMixerData,sbpOldFMMix);

		// Restore Voice output levels (SB Pro)
		sbOut(sbpMixerAddr,sbpmVoiceVol);
		sbOut(sbpMixerData,sbpOldVOCMix);
	}
#endif

	setvect(sbIntVec,sbOldIntHand);		// Set vector back
}

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
//	Sound Source Code

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSStopSample() - Stops a sample playing on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SSStopSample(void)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
asm	pushf
asm	cli
#endif

	(id0_long_t)ssSample = 0;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSService() - Handles playing the next sample on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SSService(void)
{
	id0_boolean_t	gotit;
	id0_byte_t	v;

	while (ssSample)
	{
	asm	mov		dx,[ssStatus]	// Check to see if FIFO is currently empty
	asm	in		al,dx
	asm	test	al,0x40
	asm	jnz		done			// Nope - don't push any more data out

		v = *ssSample++;
		if (!(--ssLengthLeft))
		{
			(id0_long_t)ssSample = 0;
			SDL_DigitizedDone();
		}

	asm	mov		dx,[ssData]		// Pump the value out
	asm	mov		al,[v]
	asm	out		dx,al

	asm	mov		dx,[ssControl]	// Pulse printer select
	asm	mov		al,[ssOff]
	asm	out		dx,al
	asm	push	ax
	asm	pop		ax
	asm	mov		al,[ssOn]
	asm	out		dx,al

	asm	push	ax				// Delay a short while
	asm	pop		ax
	asm	push	ax
	asm	pop		ax
	}
done:;
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSPlaySample() - Plays the specified sample on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SSPlaySample(id0_byte_t id0_huge *data,id0_longword_t len)
{
asm	pushf
asm	cli

	ssLengthLeft = len;
	ssSample = (volatile id0_byte_t id0_far *)data;

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartSS() - Sets up for and turns on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartSS(void)
{
	if (ssPort == 3)
		ssControl = 0x27a;	// If using LPT3
	else if (ssPort == 2)
		ssControl = 0x37a;	// If using LPT2
	else
		ssControl = 0x3be;	// If using LPT1
	ssStatus = ssControl - 1;
	ssData = ssStatus - 1;

	ssOn = 0x04;
	if (ssIsTandy)
		ssOff = 0x0e;				// Tandy wierdness
	else
		ssOff = 0x0c;				// For normal machines

	outportb(ssControl,ssOn);		// Enable SS
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutSS() - Turns off the Sound Source
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutSS(void)
{
	outportb(ssControl,ssOff);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CheckSS() - Checks to see if a Sound Source is present at the
//		location specified by the sound source variables
//
///////////////////////////////////////////////////////////////////////////
static id0_boolean_t
SDL_CheckSS(void)
{
	id0_boolean_t		present = false;
	id0_longword_t	lasttime;

	// Turn the Sound Source on and wait awhile (4 ticks)
	SDL_StartSS();

	lasttime = TimeCount;
	while (TimeCount < lasttime + 4)
		;

asm	mov		dx,[ssStatus]	// Check to see if FIFO is currently empty
asm	in		al,dx
asm	test	al,0x40
asm	jnz		checkdone		// Nope - Sound Source not here

asm	mov		cx,32			// Force FIFO overflow (FIFO is 16 bytes)
outloop:
asm	mov		dx,[ssData]		// Pump a neutral value out
asm	mov		al,0x80
asm	out		dx,al

asm	mov		dx,[ssControl]	// Pulse printer select
asm	mov		al,[ssOff]
asm	out		dx,al
asm	push	ax
asm	pop		ax
asm	mov		al,[ssOn]
asm	out		dx,al

asm	push	ax				// Delay a short while before we do this again
asm	pop		ax
asm	push	ax
asm	pop		ax

asm	loop	outloop

asm	mov		dx,[ssStatus]	// Is FIFO overflowed now?
asm	in		al,dx
asm	test	al,0x40
asm	jz		checkdone		// Nope, still not - Sound Source not here

	present = true;			// Yes - it's here!

checkdone:
	SDL_ShutSS();
	return(present);
}

static id0_boolean_t
SDL_DetectSoundSource(void)
{
	for (ssPort = 1;ssPort <= 3;ssPort++)
		if (SDL_CheckSS())
			return(true);
	return(false);
}
#endif // REFKEEN_SD_ENABLE_SOUNDSOURCE
#endif // GAMEVER_NOAH3D

//
//	PC Sound code
//

// *** S3DNA + ALPHA RESTORATION ***
// PC sampled sound support was not present in the alpha, and was
// removed in S3DNA. Furthermore, the usual PC sound playback routines
// were originally located *after* the various digitized sound
// related functions in the alpha.
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef GAMEVER_NOAH3D
///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCPlaySample() - Plays the specified sample on the PC speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCPlaySample(id0_byte_t id0_huge *data,id0_longword_t len)
{
asm	pushf
asm	cli

	SDL_IndicatePC(true);

	pcLengthLeft = len;
	pcSound = (volatile id0_byte_t id0_far *)data;

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCStopSample() - Stops a sample playing on the PC speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCStopSample(void)
{
asm	pushf
asm	cli

	(id0_long_t)pcSound = 0;

	SDL_IndicatePC(false);

asm	in	al,0x61		  	// Turn the speaker off
asm	and	al,0xfd			// ~2
asm	out	0x61,al

asm	popf
}
#endif // !defined GAMEVER_NOAH3D

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCPlaySound() - Plays the specified sound on the PC speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCPlaySound(PCSound id0_far *sound)
{
asm	pushf
asm	cli

	pcLastSample = -1;
	pcLengthLeft = sound->common.length;
	pcSound = sound->data;

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCStopSound() - Stops the current sound playing on the PC Speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCStopSound(void)
{
asm	pushf
asm	cli

	(id0_long_t)pcSound = 0;

asm	in	al,0x61		  	// Turn the speaker off
asm	and	al,0xfd			// ~2
asm	out	0x61,al

asm	popf
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCService() - Handles playing the next sample in a PC sound
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PCService(void)
{
	id0_byte_t	s;
	id0_word_t	t;

	if (pcSound)
	{
		s = *pcSound++;
		if (s != pcLastSample)
		{
		asm	pushf
		asm	cli

			pcLastSample = s;
			if (s)					// We have a frequency!
			{
				t = pcSoundLookup[s];
			asm	mov	bx,[t]

			asm	mov	al,0xb6			// Write to channel 2 (speaker) timer
			asm	out	43h,al
			asm	mov	al,bl
			asm	out	42h,al			// Low byte
			asm	mov	al,bh
			asm	out	42h,al			// High byte

			asm	in	al,0x61			// Turn the speaker & gate on
			asm	or	al,3
			asm	out	0x61,al
			}
			else					// Time for some silence
			{
			asm	in	al,0x61		  	// Turn the speaker & gate off
			asm	and	al,0xfc			// ~3
			asm	out	0x61,al
			}

		asm	popf
		}

		if (!(--pcLengthLeft))
		{
			SDL_PCStopSound();
			SDL_SoundFinished();
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutPC() - Turns off the pc speaker
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutPC(void)
{
asm	pushf
asm	cli

	pcSound = 0;

asm	in	al,0x61		  	// Turn the speaker & gate off
asm	and	al,0xfc			// ~3
asm	out	0x61,al

asm	popf
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

//
//	Stuff for digitized sounds
//
memptr
SDL_LoadDigiSegment(id0_word_t page)
{
	memptr	addr;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,10	// bright green
asm	out	dx,al
#endif

	addr = PM_GetSoundPage(page);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_SetPageLock(page,pml_Locked);
#else
	PM_SetPageLock(PMSoundStart + page,pml_Locked);
#endif

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

	return(addr);
}

void
SDL_PlayDigiSegment(memptr addr,id0_word_t len)
{
	switch (DigiMode)
	{
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case sds_PC:
    	SDL_PCPlaySample(addr,len);
		break;
#endif
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	case sds_SoundSource:
		SDL_SSPlaySample(addr,len);
		break;
#endif
#endif
	case sds_SoundBlaster:
		SDL_SBPlaySample(addr,len);
		break;
	}
}

void
SD_StopDigitized(void)
{
	id0_int_t	i;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!SD_Started)
		return;
#endif
asm	pushf
asm	cli

	DigiLeft = 0;
	DigiNextAddr = nil;
	DigiNextLen = 0;
	DigiMissed = false;
	DigiPlaying = false;
	DigiNumber = DigiPriority = 0;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	SoundPositioned = false;
#endif
	// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
		SDL_SoundFinished();
#endif

	switch (DigiMode)
	{
	// *** ALPHA RESTORATION ***
	// A few typos
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	case sds_SoundSource:
		SDL_SBStopSample();
		break;
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	case sds_SoundBlaster:
		SDL_SSStopSample();
		break;
#endif
#else
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	case sds_PC:
		SDL_PCStopSample();
		break;
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	case sds_SoundSource:
		SDL_SSStopSample();
		break;
#endif
#endif
	case sds_SoundBlaster:
		SDL_SBStopSample();
		break;
#endif
	}

asm	popf

	for (i = DigiLastStart;i < DigiLastEnd;i++)
		PM_SetPageLock(i + PMSoundStart,pml_Unlocked);
	DigiLastStart = 1;
	DigiLastEnd = 0;
}

void
SD_Poll(void)
{
	if (DigiLeft && !DigiNextAddr)
	{
		DigiNextLen = (DigiLeft >= PMPageSize)? PMPageSize : (DigiLeft % PMPageSize);
		DigiLeft -= DigiNextLen;
		if (!DigiLeft)
			DigiLastSegment = true;
		DigiNextAddr = SDL_LoadDigiSegment(DigiPage++);
	}
	if (DigiMissed && DigiNextAddr)
	{
		SDL_PlayDigiSegment(DigiNextAddr,DigiNextLen);
		DigiNextAddr = nil;
		DigiMissed = false;
		if (DigiLastSegment)
		{
			DigiPlaying = false;
			DigiLastSegment = false;
		}
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (midiError)
	{
		id0_char_t str[80];
		sprintf(str,"SD_Poll: midiError = %d\n", midiError);
		Quit(str);
	}
#else
	SDL_SetTimerSpeed();
#endif
}

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void
SD_SetPosition(id0_int_t leftpos,id0_int_t rightpos)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if ((leftpos < 0) || (leftpos > 15) || (rightpos < 0) || (rightpos > 15))
#else
	if
	(
		(leftpos < 0)
	||	(leftpos > 15)
	||	(rightpos < 0)
	||	(rightpos > 15)
	||	((leftpos == 15) && (rightpos == 15))
	)
#endif
		Quit("SD_SetPosition: Illegal position");
#endif

	switch (DigiMode)
	{
	case sds_SoundBlaster:
		SDL_PositionSBP(leftpos,rightpos);
		break;
	}
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

void
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
SD_PlayDigitized(id0_word_t which)
#else
SD_PlayDigitized(id0_word_t which,id0_int_t leftpos,id0_int_t rightpos)
#endif
{
	id0_word_t	len;
	memptr	addr;

	if (!DigiMode)
		return;

	SD_StopDigitized();
	if (which >= NumDigi)
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	{
		id0_char_t str[80];
		sprintf(str,"SD_PlayDigitized: Bad sound number (%d)\n",which);
		Quit(str);
	}
#else
		Quit("SD_PlayDigitized: bad sound number");
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	SD_SetPosition(leftpos,rightpos);
#endif

	DigiPage = DigiList[(which * 2) + 0];
	DigiLeft = DigiList[(which * 2) + 1];

	DigiLastStart = DigiPage;
	DigiLastEnd = DigiPage + ((DigiLeft + (PMPageSize - 1)) / PMPageSize);

	len = (DigiLeft >= PMPageSize)? PMPageSize : (DigiLeft % PMPageSize);
	addr = SDL_LoadDigiSegment(DigiPage++);

	DigiPlaying = true;
	DigiLastSegment = false;

	SDL_PlayDigiSegment(addr,len);
	DigiLeft -= len;
	if (!DigiLeft)
		DigiLastSegment = true;

	SD_Poll();
}

void
SDL_DigitizedDone(void)
{
	if (DigiNextAddr)
	{
		SDL_PlayDigiSegment(DigiNextAddr,DigiNextLen);
		DigiNextAddr = nil;
		DigiMissed = false;
	}
	else
	{
		if (DigiLastSegment)
		{
			DigiPlaying = false;
			DigiLastSegment = false;
			// *** S3DNA + SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			DigiPriority = 0;
#endif
#ifndef GAMEVER_NOAH3D
			if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
			{
				SDL_SoundFinished();
			}
#endif
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#ifndef GAMEVER_NOAH3D
			else
#endif
				DigiNumber = DigiPriority = 0;
#endif
			SoundPositioned = false;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			SDL_SBStopSample();
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
		}
		else
			DigiMissed = true;
	}
}

void
SD_SetDigiDevice(SDSMode mode)
{
	id0_boolean_t	devicenotpresent;

	if (mode == DigiMode)
		return;

	SD_StopDigitized();

	devicenotpresent = false;
	switch (mode)
	{
	// *** ALPHA RESTORATION ***
	// Apparently, this case was originally located here
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	case sds_SoundSource:
		if (!SoundSourcePresent)
			devicenotpresent = true;
		break;
#endif
	case sds_SoundBlaster:
		if (!SoundBlasterPresent)
		{
			// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
			if (SoundSourcePresent)
				mode = sds_SoundSource;
			else
#endif
				devicenotpresent = true;
		}
		break;
	// *** S3DNA + ALPHA RESTORATION ***
	// Not present in S3DNA and located elsewhere in the alpha
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	case sds_SoundSource:
		if (!SoundSourcePresent)
			devicenotpresent = true;
		break;
#endif
	}

	if (!devicenotpresent)
	{
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
		if (DigiMode == sds_SoundSource)
			SDL_ShutSS();
#endif
#endif

		DigiMode = mode;

		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
		if (mode == sds_SoundSource)
			SDL_StartSS();
#endif
#endif

		SDL_SetTimerSpeed();
	}
}

void
SDL_SetupDigi(void)
{
	memptr	list;
	id0_word_t	id0_far *p,
			pg;
	id0_int_t		i;

	PM_UnlockMainMem();
	MM_GetPtr(&list,PMPageSize);
	PM_CheckMainMem();
	p = (id0_word_t id0_far *)MK_FP(PM_GetPage(ChunksInFile - 1),0);
	_fmemcpy((void id0_far *)list,(void id0_far *)p,PMPageSize);
	pg = PMSoundStart;
	for (i = 0;i < PMPageSize / (sizeof(id0_word_t) * 2);i++,p += 2)
	{
		if (pg >= ChunksInFile - 1)
			break;
		pg += (p[1] + (PMPageSize - 1)) / PMPageSize;
	}
	PM_UnlockMainMem();
	MM_GetPtr((memptr *)&DigiList,i * sizeof(id0_word_t) * 2);
	_fmemcpy((void id0_far *)DigiList,(void id0_far *)list,i * sizeof(id0_word_t) * 2);
	MM_FreePtr(&list);
	NumDigi = i;

	for (i = 0;i < LASTSOUND;i++)
		DigiMap[i] = -1;
}

// *** ALPHA RESTORATION ***
// These PC sound code routines were originally located here. Note
// that there's no copy of the commented out SDL_PCService routine.
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
//
//	PC Sound code
//

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCPlaySound() - Plays the specified sound on the PC speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCPlaySound(PCSound id0_far *sound)
{
asm	pushf
asm	cli

	pcLastSample = -1;
	pcLengthLeft = sound->common.length;
	pcSound = sound->data;

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCStopSound() - Stops the current sound playing on the PC Speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCStopSound(void)
{
asm	pushf
asm	cli

	(id0_long_t)pcSound = 0;

asm	in	al,0x61		  	// Turn the speaker off
asm	and	al,0xfd			// ~2
asm	out	0x61,al

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutPC() - Turns off the pc speaker
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutPC(void)
{
asm	pushf
asm	cli

	pcSound = 0;

asm	in	al,0x61		  	// Turn the speaker & gate off
asm	and	al,0xfc			// ~3
asm	out	0x61,al

asm	popf
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

// 	AdLib Code

///////////////////////////////////////////////////////////////////////////
//
//	alOut(n,b) - Puts b in AdLib card register n
//
///////////////////////////////////////////////////////////////////////////
void
alOut(id0_byte_t n,id0_byte_t b)
{
asm	pushf
asm	cli

asm	mov	dx,0x388
asm	mov	al,[n]
asm	out	dx,al
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	mov	dx,0x389
#else
asm	inc	dx
#endif
asm	mov	al,[b]
asm	out	dx,al

asm	popf

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	mov	dx,0x388
#else
asm	dec	dx
#endif
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx

asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx

asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx

asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetInstrument() - Puts an instrument into a generator
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SetInstrument(id0_int_t track,id0_int_t which,Instrument id0_far *inst,id0_boolean_t percussive)
{
	id0_byte_t		c,m;

	if (percussive)
	{
		c = pcarriers[which];
		m = pmodifiers[which];
	}
	else
	{
		c = carriers[which];
		m = modifiers[which];
	}

	tracks[track - 1]->inst = *inst;
	tracks[track - 1]->percussive = percussive;

	alOut(m + alChar,inst->mChar);
	alOut(m + alScale,inst->mScale);
	alOut(m + alAttack,inst->mAttack);
	alOut(m + alSus,inst->mSus);
	alOut(m + alWave,inst->mWave);

	// Most percussive instruments only use one cell
	if (c != 0xff)
	{
		alOut(c + alChar,inst->cChar);
		alOut(c + alScale,inst->cScale);
		alOut(c + alAttack,inst->cAttack);
		alOut(c + alSus,inst->cSus);
		alOut(c + alWave,inst->cWave);
	}

	alOut(which + alFeedCon,inst->nConn);	// DEBUG - I think this is right
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALStopSound() - Turns off any sound effects playing through the
//		AdLib card
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_ALStopSound(void)
{
asm	pushf
asm	cli

	(id0_long_t)alSound = 0;
	alOut(alFreqH + 0,0);

asm	popf
}

static void
SDL_AlSetFXInst(Instrument id0_far *inst)
{
	id0_byte_t		c,m;

	m = modifiers[0];
	c = carriers[0];
	alOut(m + alChar,inst->mChar);
	alOut(m + alScale,inst->mScale);
	alOut(m + alAttack,inst->mAttack);
	alOut(m + alSus,inst->mSus);
	alOut(m + alWave,inst->mWave);
	alOut(c + alChar,inst->cChar);
	alOut(c + alScale,inst->cScale);
	alOut(c + alAttack,inst->cAttack);
	alOut(c + alSus,inst->cSus);
	alOut(c + alWave,inst->cWave);

	// Note: Switch commenting on these lines for old MUSE compatibility
//	alOut(alFeedCon,inst->nConn);
	alOut(alFeedCon,0);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALPlaySound() - Plays the specified sound on the AdLib card
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_ALPlaySound(AdLibSound id0_far *sound)
{
	Instrument	id0_far *inst;
	id0_byte_t		id0_huge *data;

	SDL_ALStopSound();

asm	pushf
asm	cli

	alLengthLeft = sound->common.length;
	data = sound->data;
	data++;
	data--;
	alSound = (id0_byte_t id0_far *)data;
	alBlock = ((sound->block & 7) << 2) | 0x20;
	inst = &sound->inst;

	if (!(inst->mSus | inst->cSus))
	{
	asm	popf
		Quit("SDL_ALPlaySound() - Bad instrument");
	}

	SDL_AlSetFXInst(&alZeroInst);	// DEBUG
	SDL_AlSetFXInst(inst);

asm	popf
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
// 	SDL_ALSoundService() - Plays the next sample out through the AdLib card
//
///////////////////////////////////////////////////////////////////////////
//static void
void
SDL_ALSoundService(void)
{
	id0_byte_t	s;

	if (alSound)
	{
		s = *alSound++;
		if (!s)
			alOut(alFreqH + 0,0);
		else
		{
			alOut(alFreqL + 0,s);
			alOut(alFreqH + 0,alBlock);
		}

		if (!(--alLengthLeft))
		{
			(id0_long_t)alSound = 0;
			alOut(alFreqH + 0,0);
			SDL_SoundFinished();
		}
	}
}
#endif

#if 0
void
SDL_ALService(void)
{
	id0_byte_t	a,v;
	id0_word_t	w;

	if (!sqActive)
		return;

	while (sqHackLen && (sqHackTime <= alTimeCount))
	{
		w = *sqHackPtr++;
		sqHackTime = alTimeCount + *sqHackPtr++;
	asm	mov	dx,[w]
	asm	mov	[a],dl
	asm	mov	[v],dh
		alOut(a,v);
		sqHackLen -= 4;
	}
	alTimeCount++;
	if (!sqHackLen)
	{
		sqHackPtr = (id0_word_t id0_far *)sqHack;
		sqHackLen = sqHackSeqLen;
		alTimeCount = sqHackTime = 0;
	}
}
#endif

// *** S3DNA RESTORATION ***
// Recreated MIDI to AL translation code
#ifdef GAMEVER_NOAH3D
static id0_word_t
fixword(id0_word_t w)
{
	return ((w&0xFF00)>>8)+((w&0xFF)<<8);
}

static id0_longword_t
fixlongword(id0_longword_t d)
{
	return ((d&0xFF000000)>>24)+((d&0x00FF0000)>>8)
		+((d&0xFF00)<<8)+((d&0xFF)<<24);
}

static id0_longword_t
MIDI_VarLength(void)
{
	id0_longword_t value = 0;
	while (*midiData & 0x80)
		value = (value << 7) + (*midiData++ & 0x7F);
	value = (value << 7) + *midiData++;
	return value;
}



static id0_word_t	NoteTable[12] = {0x157,0x16b,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x220,0x241,0x263,0x287};

static id0_byte_t	drums = 0;

static inst_t	instrument[14] = {
	{0x21, 0x31, 0x4f, 0x00, 0xf2, 0xd2, 0x52, 0x73, 0x00, 0x00, 0x06},
	{0x01, 0x31, 0x4f, 0x04, 0xf0, 0x90, 0xff, 0x0f, 0x00, 0x00, 0x06},
	{0x31, 0x22, 0x10, 0x04, 0x83, 0xf4, 0x9f, 0x78, 0x00, 0x00, 0x0a},
	{0x11, 0x31, 0x05, 0x00, 0xf9, 0xf1, 0x25, 0x34, 0x00, 0x00, 0x0a},
	{0x31, 0x61, 0x1c, 0x80, 0x41, 0x92, 0x0b, 0x3b, 0x00, 0x00, 0x0e},
	{0x21, 0x21, 0x19, 0x80, 0x43, 0x85, 0x8c, 0x2f, 0x00, 0x00, 0x0c},
	{0x21, 0x24, 0x94, 0x05, 0xf0, 0x90, 0x09, 0x0a, 0x00, 0x00, 0x0a},
	{0x21, 0xa2, 0x83, 0x8d, 0x74, 0x65, 0x17, 0x17, 0x00, 0x00, 0x07},
	{0x01, 0x01, 0x00, 0x00, 0xff, 0xff, 0x07, 0x07, 0x00, 0x00, 0x07},
	{0x10, 0x00, 0x00, 0x00, 0xd8, 0x87, 0x4a, 0x3c, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x11, 0x11, 0xfa, 0xfa, 0xb5, 0xb5, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x88, 0xb5, 0x00, 0x00, 0x00},
	{0x15, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
	{0x21, 0x11, 0x4c, 0x00, 0xf1, 0xf2, 0x63, 0x72, 0x00, 0x00, 0xc0}
};



void
MIDI_SkipMetaEvent(void)
{
	id0_longword_t length = MIDI_VarLength();
	midiData += length;
}

void
MIDI_NoteOff(id0_int_t channel, id0_int_t note, id0_int_t velocity)
{
	id0_unsigned_t	fnumber;
	id0_byte_t	octave;
	if (channel == 9)
	{
		switch (note)
		{
		case 0x23:
		case 0x24:
			drums &= 0xef;
			break;
		case 0x26:
		case 0x28:
			drums &= 0xf7;
			break;
		case 0x2a:
			drums &= 0xfe;
			break;
		default:
			midiError = -11;
		}
		alOut(alEffects,alChar|drums);
		return;
	}

	fnumber = NoteTable[note%12];
	octave = ((note/12)&7)<<2;
	alOut(alFreqL + 1 + channel, fnumber&0xFF);
	alOut(alFreqH + 1 + channel, octave+((fnumber>>8)&3));
}

void
MIDI_NoteOn(id0_int_t channel, id0_byte_t note, id0_byte_t velocity)
{
	id0_unsigned_t	fnumber;
	id0_int_t	octave;
	if (velocity)
	{
		if (channel == 9)
		{
			switch (note)
			{
			case 0x23:
			case 0x24:
				drums |= 0x10;
				break;
			case 0x26:
				drums |= 0x8;
				break;
			case 0x28:
				drums |= 0x4;
				break;
			case 0x2a:
				drums |= 1;
				break;
			default:
				midiError = -11;
			}
			alOut(alEffects,alChar|drums);
		}
		else
		{
			fnumber = NoteTable[note%12];
			octave = note/12;
			alOut(alFreqL + 1 + channel, fnumber&0xFF);
			alOut(alFreqH + 1 + channel, alChar|(octave<<2)|((fnumber>>8)&3));
		}
	}
	else
		MIDI_NoteOff (channel,note,velocity);
}

void
MIDI_ControllerChange(id0_int_t channel, id0_int_t id, id0_int_t value)
{
}

void
MIDI_ProgramChange(id0_int_t channel, id0_int_t id)
{
	// S3DNA RESTORATION - While an inst_t pointer can be used with direct
	// access to all fields, based on generated machine code it looks like
	// this wasn't the way the code was written
	id0_byte_t	*inst;
	if (channel == 9)
	{
		id0_int_t	note;
		id0_unsigned_t	fnumber;
		id0_int_t	octave;

		inst = &instrument[9];
		alOut(modifiers[6]+alChar, *inst++);
		alOut(carriers[6]+alChar, *inst++);
		alOut(modifiers[6]+alScale, *inst++);
		alOut(carriers[6]+alScale, *inst++);
		alOut(modifiers[6]+alAttack, *inst++);
		alOut(carriers[6]+alAttack, *inst++);
		alOut(modifiers[6]+alSus, *inst++);
		alOut(carriers[6]+alSus, *inst++);
		alOut(modifiers[6]+alWave, *inst++);
		alOut(carriers[6]+alWave, *inst++);

		alOut(alFeedCon+6, *inst);

		note = 24;
		fnumber = NoteTable[note%12];
		octave = ((note/12)&7)<<2;
		alOut(alFreqL+6,fnumber&0xFF);
		alOut(alFreqH+6,octave+((fnumber>>8)&3));
		note = 24;
		fnumber = NoteTable[note%12];
		octave = ((note/12)&7)<<2;
		alOut(alFreqL+7,fnumber&0xFF);
		alOut(alFreqH+7,octave+((fnumber>>8)&3));
		note = 24;
		fnumber = NoteTable[note%12];
		octave = ((note/12)&7)<<2;
		alOut(alFreqL+8,fnumber&0xFF);
		alOut(alFreqH+8,octave+((fnumber>>8)&3));

		inst = &instrument[10];
		alOut(0x31,*inst); inst += 2;
		alOut(0x51,*inst); inst += 2;
		alOut(0x71,*inst); inst += 2;
		alOut(0x91,*inst); inst += 2;

		alOut(0xF1,*inst);
		alOut(0xC7,0);

		inst = &instrument[12];
		alOut(0x32,*inst); inst += 2;
		alOut(0x52,*inst); inst += 2;
		alOut(0x72,*inst); inst += 2;
		alOut(0x92,*inst); inst += 2;

		alOut(0xF2,*inst);

		inst = &instrument[11];
		alOut(0x34,*inst); inst += 2;
		alOut(0x54,*inst); inst += 2;
		alOut(0x74,*inst); inst += 2;
		alOut(0x94,*inst); inst += 2;

		alOut(0xF4,*inst);
		alOut(0xC8,0);

		inst = &instrument[10];
		alOut(0x35,*inst); inst += 2;
		alOut(0x55,*inst); inst += 2;
		alOut(0x75,*inst); inst += 2;
		alOut(0x95,*inst); inst += 2;

		alOut(0xF5,*inst);

		return;
	}

	if (channel < 5)
	{
		switch (id & 0xF8)
		{
		case 0:
			inst = &instrument[0];
			break;
		case 8:
			inst = &instrument[8];
			break;
		case 16:
			inst = &instrument[1];
			break;
		case 24:
			inst = &instrument[0];
			break;
		case 32:
			inst = &instrument[2];
			break;
		case 40:
		case 48:
			inst = &instrument[0];
			break;
		case 56:
		case 64:
			inst = &instrument[6];
			break;
		case 72:
			inst = &instrument[7];
			break;
		case 80:
		case 88:
		case 96:
			inst = &instrument[0];
			break;
		case 104:
		case 112:
		case 120:
			inst = &instrument[8];
			break;
		default:
			midiError = -8;
			return;
		}
		
		alOut(modifiers[channel+1]+alChar, *inst++);
		alOut(carriers[channel+1]+alChar, *inst++);
		alOut(modifiers[channel+1]+alScale, *inst++);
		alOut(carriers[channel+1]+alScale, *inst++);
		alOut(modifiers[channel+1]+alAttack, *inst++);
		alOut(carriers[channel+1]+alAttack, *inst++);
		alOut(modifiers[channel+1]+alSus, *inst++);
		alOut(carriers[channel+1]+alSus, *inst++);
		alOut(modifiers[channel+1]+alWave, *inst++);
		alOut(carriers[channel+1]+alWave, *inst++);

		alOut(alFeedCon+channel, *inst);
	}
}

void
MIDI_ChannelPressure(id0_int_t channel, id0_int_t id)
{
}

void
MIDI_ProcessEvent(id0_byte_t event)
{
	id0_byte_t	note,velocity,id,value;
	switch (event&0xF0)
	{
	case 0x80:
		note = *midiData++;
		velocity = *midiData++;
		MIDI_NoteOff(event&0xF,note,velocity);
		break;
	case 0x90:
		note = *midiData++;
		velocity = *midiData++;
		MIDI_NoteOn(event&0xF,note,velocity);
		break;
	case 0xB0:
		id = *midiData++;
		value = *midiData++;
		MIDI_ControllerChange(event&0xF,id,value);
		break;
	case 0xC0:
		value = *midiData++;
		MIDI_ProgramChange(event&0xF,value);
		break;
	case 0xD0:
		value = *midiData++;
		MIDI_ChannelPressure(event&0xF,value);
		break;
	default:
		midiError = -7;
		break;
	}
}

static void
MIDI_DoEvent(void)
{
	id0_byte_t	event;
	id0_longword_t	length;
	id0_longword_t	tempo;

	event = *midiData++;
	if (!(event & 0x80))
	{
		if (!(midiRunningStatus | 0x00))
			return;

		midiData--;
		MIDI_ProcessEvent(midiRunningStatus);
	}
	else if (event < 0xF0)
	{
		midiRunningStatus = event;
		MIDI_ProcessEvent(midiRunningStatus);
	}
	else if (event == 0xF0)
	{
		midiRunningStatus = 0;
		midiError = -4;
	}
	else if (event == 0xF7)
	{
		midiRunningStatus = 0;
		midiError = -5;
	}
	else if (event == 0xFF)
	{
		midiRunningStatus = 0;
		event = *midiData++;
		switch (event)
		{
		case 0x51:
			length = MIDI_VarLength();
			tempo = ((id0_long_t)(*midiData)<<16) + (id0_long_t)((*(midiData+1))<<8) + (*(midiData+2));
			midiTimeScale = (double)tempo/2.74176e5;
			midiTimeScale *= 1.1;
			midiData += length;
			break;
		case 0x2F:
			midiData = midiDataStart;
			midiDeltaTime = 0;
			break;
		default:
			MIDI_SkipMetaEvent();
			break;
		}
	}
	else
		midiError = -6;
}

void
MIDI_IRQService(void)
{
	id0_int_t	maxevent = 0;

	if (!midiOn)
		return;

	if (midiDeltaTime)
	{
		midiDeltaTime--;
		return;
	}

	while (!midiDeltaTime && (maxevent++ < 32))
	{
		MIDI_DoEvent();
		midiDeltaTime = MIDI_VarLength();
	}

	if (maxevent >= 32)
		midiError = -1;
	else if (midiDeltaTime & 0xFFFF0000)
	{
		midiError = -2;
		return;
	}

	midiDeltaTime = midiDeltaTime * midiTimeScale;
}

#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutAL() - Shuts down the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutAL(void)
{
asm	pushf
asm	cli

	alOut(alEffects,0);
	alOut(alFreqH + 0,0);
	SDL_AlSetFXInst(&alZeroInst);
	alSound = 0;

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanAL() - Totally shuts down the AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_CleanAL(void)
{
	id0_int_t	i;

asm	pushf
asm	cli

	alOut(alEffects,0);
	for (i = 1;i < 0xf5;i++)
		alOut(i,0);

asm	popf
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartAL() - Starts up the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartAL(void)
{
	alFXReg = 0;
	alOut(alEffects,alFXReg);
	SDL_AlSetFXInst(&alZeroInst);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_DetectAdLib() - Determines if there's an AdLib (or SoundBlaster
//		emulating an AdLib) present
//
///////////////////////////////////////////////////////////////////////////
static id0_boolean_t
SDL_DetectAdLib(void)
{
	id0_byte_t	status1,status2;
	id0_int_t		i;

	alOut(4,0x60);	// Reset T1 & T2
	alOut(4,0x80);	// Reset IRQ
	status1 = readstat();
	alOut(2,0xff);	// Set timer 1
	alOut(4,0x21);	// Start timer 1
#if 0
	SDL_Delay(TimerDelay100);
#else
asm	mov	dx,0x388
asm	mov	cx,100
usecloop:
asm	in	al,dx
asm	loop usecloop
#endif

	status2 = readstat();
	alOut(4,0x60);
	alOut(4,0x80);

	if (((status1 & 0xe0) == 0x00) && ((status2 & 0xe0) == 0xc0))
	{
		for (i = 1;i <= 0xf5;i++)	// Zero all the registers
			alOut(i,0);

		alOut(1,0x20);	// Set WSE=1
		alOut(8,0);		// Set CSM=0 & SEL=0

		return(true);
	}
	else
		return(false);
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_t0Service() - My timer 0 ISR which handles the different timings and
//		dispatches to whatever other routines are appropriate
//
///////////////////////////////////////////////////////////////////////////
static void interrupt
SDL_t0Service(void)
{
static	id0_word_t	count = 1;

#if 1	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,4	// red
asm	out	dx,al
#endif

	HackCount++;

	if ((MusicMode == smm_AdLib) || (DigiMode == sds_SoundSource))
	{
		SDL_ALService();
		SDL_SSService();
//		if (!(++count & 7))
		if (!(++count % 10))
		{
			LocalTime++;
			TimeCount++;
			if (SoundUserHook)
				SoundUserHook();
		}
//		if (!(count & 3))
		if (!(count % 5))
		{
			switch (SoundMode)
			{
			case sdm_PC:
				SDL_PCService();
				break;
			case sdm_AdLib:
				SDL_ALSoundService();
				break;
			}
		}
	}
	else
	{
		if (!(++count & 1))
		{
			LocalTime++;
			TimeCount++;
			if (SoundUserHook)
				SoundUserHook();
		}
		switch (SoundMode)
		{
		case sdm_PC:
			SDL_PCService();
			break;
		case sdm_AdLib:
			SDL_ALSoundService();
			break;
		}
	}

asm	mov	ax,[WORD PTR TimerCount]
asm	add	ax,[WORD PTR TimerDivisor]
asm	mov	[WORD PTR TimerCount],ax
asm	jnc	myack
	t0OldService();			// If we overflow a word, time to call old int handler
asm	jmp	olddone
myack:;
	outportb(0x20,0x20);	// Ack the interrupt
olddone:;

#if 1	// for debugging
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
}
#endif

////////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////
static void
SDL_ShutDevice(void)
{
	switch (SoundMode)
	{
	case sdm_PC:
		SDL_ShutPC();
		break;
	case sdm_AdLib:
		SDL_ShutAL();
		break;
	}
	SoundMode = sdm_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanDevice() - totally shuts down all sound devices
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_CleanDevice(void)
{
	if ((SoundMode == sdm_AdLib) || (MusicMode == smm_AdLib))
		SDL_CleanAL();
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartDevice(void)
{
	switch (SoundMode)
	{
	case sdm_AdLib:
		SDL_StartAL();
		break;
	}
	SoundNumber = SoundPriority = 0;
}

//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
id0_boolean_t
SD_SetSoundMode(SDMode mode)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_boolean_t	result;
#else
	id0_boolean_t	result = false;
#endif
	id0_word_t	tableoffset;

	SD_StopSound();

#ifndef	_MUSE_
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if ((mode == sdm_AdLib) && !AdLibPresent)
		mode = sdm_PC;
#endif

	switch (mode)
	{
	case sdm_Off:
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_PC:
		tableoffset = STARTPCSOUNDS;
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_AdLib:
		if (AdLibPresent)
		{
			tableoffset = STARTADLIBSOUNDS;
			NeedsDigitized = false;
			result = true;
		}
		break;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	default:
		result = false;
#endif
	}
#else
	result = true;
#endif

	if (result && (mode != SoundMode))
	{
		SDL_ShutDevice();
		SoundMode = mode;
#ifndef	_MUSE_
		SoundTable = (id0_word_t *)(&audiosegs[tableoffset]);
#endif
		SDL_StartDevice();
	}

	SDL_SetTimerSpeed();

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
id0_boolean_t
SD_SetMusicMode(SMMode mode)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_boolean_t	result;
#else
	id0_boolean_t	result = false;
#endif

	SD_FadeOutMusic();
	while (SD_MusicPlaying())
		;

	switch (mode)
	{
	case smm_Off:
		NeedsMusic = false;
		result = true;
		break;
	case smm_AdLib:
		if (AdLibPresent)
		{
			NeedsMusic = true;
			result = true;
		}
		break;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	default:
		result = false;
#endif
	}

	if (result)
		MusicMode = mode;

	SDL_SetTimerSpeed();

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Startup() - starts up the Sound Mgr
//		Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////
void
SD_Startup(void)
{
	id0_int_t	i;

	if (SD_Started)
		return;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	printf("SD_Startup: ");
#else
	SDL_SetDS();
#endif

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	ssIsTandy = false;
	ssNoCheck = false;
#endif
#endif
	alNoCheck = false;
	sbNoCheck = false;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	sbNoProCheck = false;
#endif
#ifndef	_MUSE_
	for (i = 1;i < _argc;i++)
	{
		switch (US_CheckParm(_argv[i],ParmStrings))
		{
		case 0:						// No AdLib detection
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			printf("Skipping AdLib detection\n");
#endif
			alNoCheck = true;
			break;
		case 1:						// No SoundBlaster detection
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			printf("Skipping SoundBlaster detection\n");
#endif
			sbNoCheck = true;
			break;
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
		// Disable following check and define offset under a macro
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define GAMEVER_SD_OFFSET -1
#else
		case 2:						// No SoundBlaster Pro detection
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			printf("Skipping SoundBlaster Pro detection\n");
#endif
			sbNoProCheck = true;
			break;
#define GAMEVER_SD_OFFSET 0
#endif
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
		case 3+GAMEVER_SD_OFFSET:
			ssNoCheck = true;		// No Sound Source detection
			break;
		case 4+GAMEVER_SD_OFFSET:						// Tandy Sound Source handling
			ssIsTandy = true;
			break;
		case 5+GAMEVER_SD_OFFSET:						// Sound Source present at LPT1
			ssPort = 1;
			ssNoCheck = SoundSourcePresent = true;
			break;
		case 6+GAMEVER_SD_OFFSET:                     // Sound Source present at LPT2
			ssPort = 2;
			ssNoCheck = SoundSourcePresent = true;
			break;
		case 7+GAMEVER_SD_OFFSET:                     // Sound Source present at LPT3
			ssPort = 3;
			ssNoCheck = SoundSourcePresent = true;
			break;
#endif // REFKEEN_SD_ENABLE_SOUNDSOURCE
#endif
		}
	}
#endif

	SoundUserHook = 0;

	t0OldService = getvect(8);	// Get old timer 0 ISR

	LocalTime = TimeCount = alTimeCount = 0;

	SD_SetSoundMode(sdm_Off);
	SD_SetMusicMode(smm_Off);

		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	if (!ssNoCheck)
		SoundSourcePresent = SDL_DetectSoundSource();
#endif
#endif

	if (!alNoCheck)
	{
		AdLibPresent = SDL_DetectAdLib();
		if (AdLibPresent && !sbNoCheck)
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			SoundBlasterPresent = SDL_DetectSoundBlaster(-1);
#else
		{
			id0_int_t port = -1;
			id0_char_t *env = getenv("BLASTER");
			if (env)
			{
				id0_long_t temp;
				while (*env)
				{
					while (isspace(*env))
						env++;

					switch (toupper(*env))
					{
					case 'A':
						temp = strtol(env + 1,&env,16);
						if
						(
							(temp >= 0x210)
						&&	(temp <= 0x260)
						&&	(!(temp & 0x00f))
						)
							port = (temp - 0x200) >> 4;
						else
							Quit("SD_Startup: Unsupported address value in BLASTER");
						break;
					case 'I':
						temp = strtol(env + 1,&env,10);
						if
						(
							(temp >= 0)
						&&	(temp <= 10)
						&&	(sbIntVectors[temp] != -1)
						)
						{
							sbInterrupt = temp;
							sbIntVec = sbIntVectors[sbInterrupt];
						}
						else
							Quit("SD_Startup: Unsupported interrupt value in BLASTER");
						break;
					case 'D':
						temp = strtol(env + 1,&env,10);
						if ((temp == 0) || (temp == 1) || (temp == 3))
							SDL_SBSetDMA(temp);
						else
							Quit("SD_Startup: Unsupported DMA value in BLASTER");
						break;
					default:
						while (isspace(*env))
							env++;
						while (*env && !isspace(*env))
							env++;
						break;
					}
				}
			}
			SoundBlasterPresent = SDL_DetectSoundBlaster(port);
		}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (AdLibPresent)
		printf("AdLib detected\n");
	if (SoundBlasterPresent)
		printf("SoundBlaster detected (A%03X, I%d, D%d)\n", sbLocation+0x200, sbInterrupt, sbDMA);
	if (!AdLibPresent && !SoundBlasterPresent)
		printf("No sound devices detected\n");
#endif

	for (i = 0;i < 255;i++)
		pcSoundLookup[i] = i * 60;

	if (SoundBlasterPresent)
		SDL_StartSB();

	SDL_SetupDigi();

	SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Default() - Sets up the default behaviour for the Sound Mgr whether
//		the config file was present or not.
//
///////////////////////////////////////////////////////////////////////////
void
SD_Default(id0_boolean_t gotit,SDMode sd,SMMode sm)
{
	id0_boolean_t	gotsd,gotsm;

	gotsd = gotsm = gotit;

	if (gotsd)	// Make sure requested sound hardware is available
	{
		switch (sd)
		{
		case sdm_AdLib:
			gotsd = AdLibPresent;
			break;
		}
	}
	if (!gotsd)
	{
		if (AdLibPresent)
			sd = sdm_AdLib;
		else
			sd = sdm_PC;
	}
	if (sd != SoundMode)
		SD_SetSoundMode(sd);


	if (gotsm)	// Make sure requested music hardware is available
	{
		switch (sm)
		{
		case sdm_AdLib:
			gotsm = AdLibPresent;
			break;
		}
	}
	if (!gotsm)
	{
		if (AdLibPresent)
			sm = smm_AdLib;
	}
	if (sm != MusicMode)
		SD_SetMusicMode(sm);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Shutdown() - shuts down the Sound Mgr
//		Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////
void
SD_Shutdown(void)
{
	if (!SD_Started)
		return;

	SD_MusicOff();
	SD_StopSound();
	SDL_ShutDevice();
	SDL_CleanDevice();

	if (SoundBlasterPresent)
		SDL_ShutSB();

		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	if (SoundSourcePresent)
		SDL_ShutSS();
#endif
#endif

	asm	pushf
	asm	cli

	SDL_SetTimer0(0);

	setvect(8,t0OldService);

	asm	popf

	SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetUserHook() - sets the routine that the Sound Mgr calls every 1/70th
//		of a second from its timer 0 ISR
//
///////////////////////////////////////////////////////////////////////////
void
SD_SetUserHook(void (* hook)(void))
{
	SoundUserHook = hook;
}

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
///////////////////////////////////////////////////////////////////////////
//
//	SD_PositionSound() - Sets up a stereo imaging location for the next
//		sound to be played. Each channel ranges from 0 to 15.
//
///////////////////////////////////////////////////////////////////////////
void
SD_PositionSound(id0_int_t leftvol,id0_int_t rightvol)
{
	LeftPosition = leftvol;
	RightPosition = rightvol;
	nextsoundpos = true;
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void
#else
id0_boolean_t
#endif
SD_PlaySound(soundnames sound)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	id0_boolean_t		ispos;
#endif
	SoundCommon	id0_far *s;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	id0_int_t	lp,rp;

	lp = LeftPosition;
	rp = RightPosition;
	LeftPosition = 0;
	RightPosition = 0;

	ispos = nextsoundpos;
	nextsoundpos = false;
#endif

	if (sound == -1)
		GAMEVER_COND_RET(false);

	s = MK_FP(SoundTable[sound],0);
	if ((SoundMode != sdm_Off) && !s)
		Quit("SD_PlaySound() - Uncached sound");

	if ((DigiMode != sds_Off) && (DigiMap[sound] != -1))
	{
		// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
		if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
		{
			if (s->priority < SoundPriority)
				GAMEVER_COND_RET(false);

			SDL_PCStopSound();

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			SoundPositioned = ispos;
#endif
			SD_PlayDigitized(DigiMap[sound],lp,rp);
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			SoundPositioned = ispos;
#endif
			SoundNumber = sound;
			SoundPriority = s->priority;
		}
		else
#endif
		{
			// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (!defined GAMEVER_NOAH3D)
		asm	pushf
		asm	cli
			if (DigiPriority && !DigiNumber)
			{
			asm	popf
				Quit("SD_PlaySound: Priority without a sound");
			}
		asm	popf
#endif

			if (s->priority < DigiPriority)
				GAMEVER_COND_RET(false);

			// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			SoundPositioned = ispos;
#endif
			SD_PlayDigitized(DigiMap[sound],lp,rp);
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			SoundPositioned = ispos;
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
			DigiNumber = sound;
			DigiPriority = s->priority;
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			SD_PlayDigitized(DigiMap[sound]);
#endif
		}

		GAMEVER_COND_RET(true);
	}

	if (SoundMode == sdm_Off)
		GAMEVER_COND_RET(false);
	if (!s->length)
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		Quit("SD_PlaySound: Zero length sound");
#else
		Quit("SD_PlaySound() - Zero length sound");
#endif
	if (s->priority < SoundPriority)
		GAMEVER_COND_RET(false);

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (SD_SoundPlaying() == sound)
		switch (sound)
		{
		case NOWAYSND:
		case ESCPRESSEDSND:
		case MOVEGUN1SND:
		case MOVEGUN2SND:
		case HITWALLSND:
		case SHOOTSND:
		case SHOOTDOORSND:
			GAMEVER_COND_RET(false);
		}
#endif

	switch (SoundMode)
	{
	case sdm_PC:
		SDL_PCPlaySound((void id0_far *)s);
		break;
	case sdm_AdLib:
		SDL_ALPlaySound((void id0_far *)s);
		break;
	}

	SoundNumber = sound;
	SoundPriority = s->priority;

	GAMEVER_COND_RET(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//		no sound is playing
//
///////////////////////////////////////////////////////////////////////////
id0_word_t
SD_SoundPlaying(void)
{
	id0_boolean_t	result = false;

	switch (SoundMode)
	{
	case sdm_PC:
		result = pcSound? true : false;
		break;
	case sdm_AdLib:
		result = alSound? true : false;
		break;
	}

	if (result)
		return(SoundNumber);
	else
		return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void
SD_StopSound(void)
{
	if (DigiPlaying)
		SD_StopDigitized();

	switch (SoundMode)
	{
	case sdm_PC:
		SDL_PCStopSound();
		break;
	case sdm_AdLib:
		SDL_ALStopSound();
		break;
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	SoundPositioned = false;
#endif

	SDL_SoundFinished();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void
SD_WaitSoundDone(void)
{
	while (SD_SoundPlaying())
		;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOn(void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	midiOn = true;
#else
	sqActive = true;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOff(void)
{
	id0_word_t	i;


	switch (MusicMode)
	{
	case smm_AdLib:
		alFXReg = 0;
		alOut(alEffects,0);
		for (i = 0;i < sqMaxTracks;i++)
			alOut(alFreqH + i + 1,0);
		break;
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	midiOn = false;
#else
	sqActive = false;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void
SD_StartMusic(MusicGroup id0_far *music)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_char_t str[80];
	id0_unsigned_int_t i;

	SD_MusicOff();
	if (MusicMode != smm_AdLib)
		return;

asm	pushf
asm	cli

	seqPtr = music->values;
	seqLength = music->length;
	if (_fstrncmp(seqPtr,"MThd",4))
		Quit("SD_StartMusic: MIDI header expected!\n");

	i = fixword(((id0_word_t id0_far *)seqPtr)[4]);
	if (i)
	{
		sprintf(str,"SD_StartMusic: Invalid or unsupported MIDI file format (%04X)\n", i);
		Quit(str);
	}
	i = fixword(((id0_word_t id0_far *)seqPtr)[5]);
	if (i != 1)
	{
		sprintf(str,"SD_StartMusic: MIDI file type 0 with %d tracks?\n",i);
		Quit(str);
	}

	seqPtr += fixlongword(((id0_longword_t id0_far *)seqPtr)[1]) + 8;
	if (_fstrncmp(seqPtr,"MTrk",4))
		Quit("SD_StartMusic: MIDI track header expected!\n");

	seqLength = fixlongword(((id0_longword_t id0_far *)seqPtr)[1]);
	if (!seqLength)
		Quit("SD_StartMusic: MIDI track is 0 length!\n");

	seqPtr += 8;
	midiData = seqPtr;
	midiDataStart = seqPtr;
	midiLength = seqLength;
	midiDeltaTime = 0;
	midiDeltaTime = MIDI_VarLength();
	if (midiDeltaTime & 0xFFFF0000)
	{
		midiError = -2;
		return;
	}

	midiRunningStatus = 0;
	MIDI_ProgramChange(9,0);
	alOut(alEffects,alChar);
	drums = 0;
	SD_MusicOn();

asm	popf
#else
	SD_MusicOff();
asm	pushf
asm	cli

	if (MusicMode == smm_AdLib)
	{
		sqHackPtr = sqHack = music->values;
		sqHackSeqLen = sqHackLen = music->length;
		sqHackTime = 0;
		alTimeCount = 0;
		SD_MusicOn();
	}

asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//		to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////
void
SD_FadeOutMusic(void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (MusicMode == smm_AdLib)
		// DEBUG - quick hack to turn the music off
		SD_MusicOff();
#else
	switch (MusicMode)
	{
	case smm_AdLib:
		// DEBUG - quick hack to turn the music off
		SD_MusicOff();
		break;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicPlaying() - returns true if music is currently playing, false if
//		not
//
///////////////////////////////////////////////////////////////////////////
id0_boolean_t
SD_MusicPlaying(void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (MusicMode == smm_AdLib)
		// DEBUG - not written
		return false;

	return false;
#else
	id0_boolean_t	result;

	switch (MusicMode)
	{
	case smm_AdLib:
		result = false;
		// DEBUG - not written
		break;
	default:
		result = false;
	}

	return(result);
#endif
}
