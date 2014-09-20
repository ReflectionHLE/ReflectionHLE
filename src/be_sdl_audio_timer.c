#include "SDL.h"
#include "opl/dbopl.h"
#include "id_heads.h"
#include "be_cross.h"

#define PC_PIT_RATE 1193182

static SDL_mutex* g_sdlCallbackMutex = NULL;
static SDL_AudioSpec g_sdlAudioSpec;
static bool g_sdlAudioSubsystemUp = false;
static uint32_t g_sdlSampleOffsetInSound, g_sdlSamplePerPart;
static void (*g_sdlCallbackSDFuncPtr)(void) = 0;

// Used for filling with samples from alOut (alOut_lLw), in addition
// to SD_SDL_CallBack (because waits between/after AdLib writes are expected)
static int16_t g_sdlALOutSamples[512];
static uint32_t g_sdlALOutSamplesStart = 0, g_sdlALOutSamplesEnd = 0;

// PC Speaker current status
static bool g_sdlPCSpeakerOn = false;
static int16_t g_sdlCurrentBeepSample;
static uint32_t g_sdlBeepHalfCycleCounter, g_sdlBeepHalfCycleCounterUpperBound;

// PIT timer divisor
static id0_longword_t g_sdlScaledTimerDivisor;

// A few variables used for timing measurements (PC_PIT_RATE units per second)
static uint64_t g_sdlLastPITTickTime;


// A PRIVATE TimeCount variable we store
// (SD_GetTimeCount/SD_SetTimeCount should be called instead)
static id0_longword_t g_sdlTimeCount;

static void BEL_SDL_CallBack(void *unused, Uint8 *stream, int len);
static inline bool YM3812Init(int numChips, int clock, int rate);

void BE_SDL_InitAudio(void)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_WARNING, "SDL audio system initialization failed,\n%s\n", SDL_GetError());
	}
	else
	{
		g_sdlAudioSpec.freq = 49716; // OPL rate
		g_sdlAudioSpec.format = AUDIO_S16;
		g_sdlAudioSpec.channels = 1;
		// Under wine, small buffer sizes cause a lot of crackling, so
		// we doublt the buffer size. This will result in a tiny amount
		// (~10ms) of extra lag on windows, but it's a price I'm
		// prepared to pay to not have my ears explode.
#ifdef _WIN32
		g_sdlAudioSpec.samples = 1024;
#else
		g_sdlAudioSpec.samples = 512;
#endif
		g_sdlAudioSpec.callback = BEL_SDL_CallBack;
		g_sdlAudioSpec.userdata = NULL;
		if (SDL_OpenAudio(&g_sdlAudioSpec, NULL))
		{
			CK_Cross_LogMessage(CK_LOG_MSG_WARNING, "Cannot open SDL audio device,\n%s\n", SDL_GetError());
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
		}
		else
		{
			g_sdlCallbackMutex = SDL_CreateMutex();
			if (!g_sdlCallbackMutex)
			{
				CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "Cannot create recursive mutex for SDL audio callback,\n%s\nClosing SDL audio subsystem\n", SDL_GetError());
				SDL_CloseAudio();
				SDL_QuitSubSystem(SDL_INIT_AUDIO);
				g_sdlAudioSubsystemUp = false;
			}
			else
			{
				g_sdlAudioSubsystemUp = true;
			}
		}
		
		if (YM3812Init(1, 3579545, g_sdlAudioSpec.freq))
		{
			CK_Cross_LogMessage(CK_LOG_MSG_WARNING, "Preparation of emulated OPL chip has failed\n");
		}
	}
}

void BL_SDL_ShutdownAudio(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_DestroyMutex(g_sdlCallbackMutex);
		g_sdlCallbackMutex = NULL;
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		g_sdlAudioSubsystemUp = false;
	}
}

void BE_SDL_StartAudioSDService(void (*funcPtr)(void))
{
	if (g_sdlAudioSubsystemUp)
	{
		g_sdlCallbackSDFuncPtr = funcPtr;
		SDL_PauseAudio(0);
	}
}

void BE_SDL_StopAudioSDService(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_PauseAudio(1);
		g_sdlCallbackSDFuncPtr = 0;
	}
}

void BE_SDL_LockAudioRecursively(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_LockMutex(g_sdlCallbackMutex);
	}
}

void BE_SDL_UnlockAudioRecursively(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_UnlockMutex(g_sdlCallbackMutex);
	}
}

// Frequency is about 1193182Hz/spkVal
void BE_SDL_PCSpeakerOn(id0_word_t spkVal)
{
	if (g_sdlAudioSubsystemUp)
	{
		g_sdlPCSpeakerOn = true;
		g_sdlCurrentBeepSample = 0;
		g_sdlBeepHalfCycleCounter = 0;
		g_sdlBeepHalfCycleCounterUpperBound = g_sdlAudioSpec.freq * spkVal;
	}
}

void BE_SDL_PCSpeakerOff(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		g_sdlPCSpeakerOn = false;
	}
}

/*******************************************************************************
OPL emulation, powered by dbopl from DOSBox and using bits of code from Wolf4SDL
*******************************************************************************/

Chip oplChip;

static inline bool YM3812Init(int numChips, int clock, int rate)
{
	DBOPL_InitTables();
	Chip__Chip(&oplChip);
	Chip__Setup(&oplChip, rate);
	return false;
}

static inline void YM3812Write(Chip *which, Bit32u reg, Bit8u val)
{
	Chip__WriteReg(which, reg, val);
}

static inline void YM3812UpdateOne(Chip *which, int16_t *stream, int length)
{
	Bit32s buffer[512 * 2];
	int i;

	// length is at maximum samplesPerMusicTick = param_samplerate / 700
	// so 512 is sufficient for a sample rate of 358.4 kHz (default 44.1 kHz)
	if(length > 512)
		length = 512;
#if 0
	if(which->opl3Active)
	{
		Chip__GenerateBlock3(which, length, buffer);

		// GenerateBlock3 generates a number of "length" 32-bit stereo samples
		// so we need to convert them to 16-bit mono samples
		for(i = 0; i < length; i++)
		{
			// Scale volume and pick one channel
			Bit32s sample = 2*buffer[2*i];
			if(sample > 16383) sample = 16383;
			else if(sample < -16384) sample = -16384;
			stream[i] = sample;
		}
	}
	else
#endif
	{
		Chip__GenerateBlock2(which, length, buffer);

		// GenerateBlock2 generates a number of "length" 32-bit mono samples
		// so we only need to convert them to 16-bit mono samples
		for(i = 0; i < length; i++)
		{
			// Scale volume
			Bit32s sample = 2*buffer[i];
			if(sample > 16383) sample = 16383;
			else if(sample < -16384) sample = -16384;
			stream[i] = (int16_t) sample;
		}
	}
}

// Drop-in replacement for id_sd.c:alOut
void BE_SDL_ALOut(id0_byte_t reg,id0_byte_t val)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_LockMutex(g_sdlCallbackMutex); // RECURSIVE lock

		// FIXME: The original code for alOut adds 6 reads of the
		// register port after writing to it (3.3 microseconds), and
		// then 35 more reads of register port after writing to the
		// data port (23 microseconds).
		//
		// It is apparently important for a portion of the fuse
		// breakage sound at the least. For now a hack is implied.
		YM3812Write(&oplChip, reg, val);
		// Hack comes with a "magic number"
		// that appears to make it work better
		int length = g_sdlAudioSpec.freq / 10000;
		if (length > sizeof(g_sdlALOutSamples)/sizeof(int16_t) - g_sdlALOutSamplesEnd)
			length = sizeof(g_sdlALOutSamples)/sizeof(int16_t) - g_sdlALOutSamplesEnd;
		if (length)
		{
			YM3812UpdateOne(&oplChip, &g_sdlALOutSamples[g_sdlALOutSamplesEnd], length);
			g_sdlALOutSamplesEnd += length;
		}

		SDL_UnlockMutex(g_sdlCallbackMutex); // RECURSIVE unlock
	}
}

/************************************************************************
PC Speaker emulation; The function mixes audio
into an EXISTING stream (of OPL sound data)
ASSUMPTION: The speaker is outputting sound (PCSpeakerUpdateOne == true).
************************************************************************/
static inline void PCSpeakerUpdateOne(int16_t *stream, int length)
{
	for (int loopVar = 0; loopVar < length; loopVar++, stream++)
	{
		*stream = (*stream + g_sdlCurrentBeepSample) / 2; // Mix
		g_sdlBeepHalfCycleCounter += 2 * PC_PIT_RATE;
		if (g_sdlBeepHalfCycleCounter >= g_sdlBeepHalfCycleCounterUpperBound)
		{
			g_sdlBeepHalfCycleCounter %= g_sdlBeepHalfCycleCounterUpperBound;
			// 32767 - too loud
			g_sdlCurrentBeepSample = 24575-g_sdlCurrentBeepSample;
		}
	}
}

// BIG BIG FIXME: This is the VERY(?) wrong place to call the OPL emulator, etc!
static void BEL_SDL_CallBack(void *unused, Uint8 *stream, int len)
{
	int16_t *currSamplePtr = (int16_t *)stream;
	uint32_t currNumOfSamples;
	bool isPartCompleted;
#if SDL_VERSION_ATLEAST(1,3,0)
	memset(stream, 0, len);
#endif
	//////////////////////////////////
	SDL_LockMutex(g_sdlCallbackMutex); // RECURSIVE lock
	//////////////////////////////////
	
	while (len)
	{
		if (!g_sdlSampleOffsetInSound)
		{
			// FUNCTION VARIABLE (TODO any need to use a var?)
			g_sdlCallbackSDFuncPtr();
		}
		// Now generate sound
		isPartCompleted = (len >= 2*(g_sdlSamplePerPart-g_sdlSampleOffsetInSound));
		currNumOfSamples = isPartCompleted ? (g_sdlSamplePerPart-g_sdlSampleOffsetInSound) : (len/2);

		// AdLib (including hack for alOut delays)
		if (g_sdlALOutSamplesEnd-g_sdlALOutSamplesStart <= currNumOfSamples)
		{
			// Copy sound generated by alOut
			if (g_sdlALOutSamplesEnd-g_sdlALOutSamplesStart > 0)
				memcpy(currSamplePtr, &g_sdlALOutSamples[g_sdlALOutSamplesStart], 2*(g_sdlALOutSamplesEnd-g_sdlALOutSamplesStart));
			// Generate what's left
			if (currNumOfSamples-(g_sdlALOutSamplesEnd-g_sdlALOutSamplesStart) > 0)
				YM3812UpdateOne(&oplChip, currSamplePtr+(g_sdlALOutSamplesEnd-g_sdlALOutSamplesStart), currNumOfSamples-(g_sdlALOutSamplesEnd-g_sdlALOutSamplesStart));
			// Finally update these
			g_sdlALOutSamplesStart = g_sdlALOutSamplesEnd = 0;
		}
		else
		{
			// Already generated enough by alOut, to be copied
			memcpy(currSamplePtr, &g_sdlALOutSamples[g_sdlALOutSamplesStart], 2*currNumOfSamples);
			g_sdlALOutSamplesStart += currNumOfSamples;
		}
		// PC Speaker
		if (g_sdlPCSpeakerOn)
			PCSpeakerUpdateOne(currSamplePtr, currNumOfSamples);
		// We're done for now
		currSamplePtr += currNumOfSamples;
		g_sdlSampleOffsetInSound += currNumOfSamples;
		len -= 2*currNumOfSamples;
		// End of part?
		if (g_sdlSampleOffsetInSound >= g_sdlSamplePerPart)
		{
			g_sdlSampleOffsetInSound = 0;
		}
	}

	////////////////////////////////////
	SDL_UnlockMutex(g_sdlCallbackMutex); // RECURSIVE unlock
	////////////////////////////////////
}

// Here, the actual rate is about 1193182Hz/speed
void BE_SDL_SetTimer(id0_word_t speed, bool isALMusicOn)
{
	g_sdlSamplePerPart = (int32_t)speed * g_sdlAudioSpec.freq / PC_PIT_RATE;
	// In the original code, the id_sd.c:SDL_t0Service callback
	// is responsible for incrementing TimeCount at a given rate
	// (~70Hz), although the rate in which the service itself is
	// 560Hz with music on and 140Hz otherwise.
	g_sdlScaledTimerDivisor = isALMusicOn ? (speed*4) : speed;
}

id0_longword_t BE_SDL_GetTimeCount(void)
{
	// FIXME: What happens when SDL_GetTicks() reaches the upper bound?
	// May be challenging to fix... A proper solution should
	// only work with *differences between SDL_GetTicks values*.
	uint64_t currPitTicks = (uint64_t)(SDL_GetTicks()) * PC_PIT_RATE / 1000;
	uint32_t ticksToAdd = (currPitTicks - g_sdlLastPITTickTime) / g_sdlScaledTimerDivisor;
	g_sdlLastPITTickTime += ticksToAdd * g_sdlScaledTimerDivisor;
	g_sdlTimeCount += ticksToAdd;
	return g_sdlTimeCount;
}

void BE_SDL_SetTimeCount(id0_longword_t newcount)
{
	g_sdlTimeCount = newcount;
}
