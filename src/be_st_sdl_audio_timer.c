#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#ifdef REFKEEN_RESAMPLER_NONE
// Nothing to add here
#elif (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
#include <libavutil/channel_layout.h>
//#include <libavutil/error.h> // av_err2str requires libavutil/libavutil-ffmpeg
#include <libswresample/swresample.h>
#elif (defined REFKEEN_RESAMPLER_LIBAVRESAMPLE)
//#include <libavutil/error.h> // av_err2str requires libavutil/libavutil-ffmpeg, which may actually be used, but let's not depend on this
#include <libavutil/opt.h>
#include <libavresample/avresample.h>
#elif (defined REFKEEN_RESAMPLER_LIBAVCODEC)
#include <libavcodec/avcodec.h>
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
#include <libresample.h>
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
#include <soxr.h>
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
#include <speex/speex_resampler.h>
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
#include <samplerate.h>
#else
#error "FATAL ERROR: No recognized Reflection Keen resampler macro is defined!"
#endif

#include "be_cross.h"
#include "be_st.h"
#include "be_st_sdl_private.h"
#include "opl/dbopl.h"

#define PC_PIT_RATE 1193182

#ifdef REFKEEN_CONFIG_THREADS
static SDL_mutex* g_sdlCallbackMutex = NULL;
#endif
static SDL_AudioSpec g_sdlAudioSpec;

bool g_sdlAudioSubsystemUp;

static bool g_sdlEmulatedOPLChipReady;
static uint32_t g_sdlSampleOffsetInSound, g_sdlSamplePerPart;
static uint64_t g_sdlScaledSampleOffsetInSound, g_sdlScaledSamplePerPart; // For resampling callback
static void (*g_sdlCallbackSDFuncPtr)(void) = 0;

#if (defined REFKEEN_RESAMPLER_LIBSAMPLERATE) || (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
#define MIXER_SAMPLE_FORMAT_FLOAT
#else
#define MIXER_SAMPLE_FORMAT_SINT16
#endif

// Used for filling with samples from alOut (alOut_low), in addition to the
// SDL CallBack itself (because waits between/after AdLib writes are expected)

#define OPL_NUM_OF_SAMPLES 2048 // About 40ms of OPL sound data
#define OPL_SAMPLE_RATE 49716
// Use this if the audio subsystem is disabled for most (we want a BYTES rate of 1000Hz, same units as used in values returned by SDL_GetTicks())
#define NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM 1000

#ifdef MIXER_SAMPLE_FORMAT_FLOAT
typedef float BE_ST_SndSample_T;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
typedef int16_t BE_ST_SndSample_T;
#endif

// Nearest-neighborhood sample rate conversion (from the OPL rate of ~49716Hz to mixing rate):
// For each AdLib samples out of 49716, tells how many times it should be repeated for the sake of conversion.
static int *g_sdlSampleRateConvTable;
// Current location in the conversion process;
static int g_sdlSampleRateConvCurrIndex; // Index to g_sdlSampleRateConvTable
static int g_sdlSampleRateConvCounter; // Counter for current cell of g_sdlALSampleRateConvTable
BE_ST_SndSample_T g_sdlSampleRateConvLastValue; // Last input sample
#ifdef REFKEEN_RESAMPLER_NONE
// Nothing to add here
#elif (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
static struct SwrContext *g_sdlSwrContext;
#elif (defined REFKEEN_RESAMPLER_LIBAVRESAMPLE)
static struct AVAudioResampleContext *g_sdlAvAudioResampleContext;
#elif (defined REFKEEN_RESAMPLER_LIBAVCODEC)
static struct AVResampleContext* g_sdlAvResampleContext = 0;
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
static void *g_sdlResampleHandle;
static double g_sdlResampleFactor;
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
static soxr_t g_sdlSoxr;
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
static SpeexResamplerState *g_sdlSpeexResamplerState = 0;
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
static SRC_STATE *g_sdlSrcResampler;
static SRC_DATA g_sdlSrcData;
#endif

static BE_ST_SndSample_T g_sdlALOutSamples[OPL_NUM_OF_SAMPLES];
static uint32_t g_sdlALOutSamplesEnd = 0;

// Used with resampling callback only
static BE_ST_SndSample_T *g_sdlPCOutSamples;
static uint32_t g_sdlPCOutNumOfSamples;
static uint32_t g_sdlPCOutSamplesEnd = 0;

// PC Speaker current status
static bool g_sdlPCSpeakerOn = false;
static BE_ST_SndSample_T g_sdlCurrentBeepSample;
static uint32_t g_sdlBeepHalfCycleCounter, g_sdlBeepHalfCycleCounterUpperBound;

// PIT timer divisor
static uint32_t g_sdlScaledTimerDivisor;

// A variable used for timing measurements
static uint32_t g_sdlLastTicks;


// A PRIVATE TimeCount variable we store
// (SD_GetTimeCount/SD_SetTimeCount should be called instead)
static uint32_t g_sdlTimeCount;

static void BEL_ST_Simple_CallBack(void *unused, Uint8 *stream, int len);
static void BEL_ST_Resampling_CallBack(void *unused, Uint8 *stream, int len);
static void YM3812Init(int numChips, int clock, int rate);

void BE_ST_InitAudio(void)
{
	g_sdlAudioSubsystemUp = false;
	g_sdlEmulatedOPLChipReady = false;
	if (g_refKeenCfg.sndSubSystem)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL audio system initialization failed,\n%s\n", SDL_GetError());
		}
		else
		{
			g_sdlAudioSpec.freq = g_refKeenCfg.sndSampleRate;
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
			g_sdlAudioSpec.format = AUDIO_F32SYS;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
			g_sdlAudioSpec.format = AUDIO_S16SYS;
#endif
			g_sdlAudioSpec.channels = 1;
			// Should be some power-of-two roughly proportional to the sample rate; Using 1024 for 48000Hz.
			for (g_sdlAudioSpec.samples = 1; g_sdlAudioSpec.samples < g_refKeenCfg.sndSampleRate/64; g_sdlAudioSpec.samples *= 2)
			{
			}
			g_sdlAudioSpec.callback = ((g_refKeenCfg.sndSampleRate == OPL_SAMPLE_RATE) || !g_refKeenCfg.oplEmulation) ? BEL_ST_Simple_CallBack : BEL_ST_Resampling_CallBack;
			g_sdlAudioSpec.userdata = NULL;
			if (SDL_OpenAudio(&g_sdlAudioSpec, NULL))
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "Cannot open SDL audio device,\n%s\n", SDL_GetError());
				SDL_QuitSubSystem(SDL_INIT_AUDIO);
			}
			else
			{
#ifdef REFKEEN_CONFIG_THREADS
				g_sdlCallbackMutex = SDL_CreateMutex();
				if (!g_sdlCallbackMutex)
				{
					BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Cannot create recursive mutex for SDL audio callback,\n%s\nClosing SDL audio subsystem\n", SDL_GetError());
					SDL_CloseAudio();
					SDL_QuitSubSystem(SDL_INIT_AUDIO);
				}
				else
#endif
				{
					BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Audio subsystem initialized, requested spec: freq %d, format %u, channels %d, samples %u\n", (int)g_sdlAudioSpec.freq, (unsigned int)g_sdlAudioSpec.format, (int)g_sdlAudioSpec.channels, (unsigned int)g_sdlAudioSpec.samples);
					g_sdlAudioSubsystemUp = true;
				}
			}
		}
	}
	// If the audio subsystem is off, let us simulate a byte rate
	// of 1000Hz (same as SDL_GetTicks() time units)
	if (!g_sdlAudioSubsystemUp)
	{
		g_sdlAudioSpec.freq = NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM / sizeof(BE_ST_SndSample_T);
		return;
	}

	if (g_refKeenCfg.oplEmulation)
	{
		YM3812Init(1, 3579545, OPL_SAMPLE_RATE);
		g_sdlEmulatedOPLChipReady = true;
		if (g_sdlAudioSpec.freq != OPL_SAMPLE_RATE)
		{
#ifndef REFKEEN_RESAMPLER_NONE
			if (g_refKeenCfg.useResampler)
			{
#if (!defined REFKEEN_RESAMPLER_LIBRESAMPLE) && (!defined REFKEEN_RESAMPLER_LIBAVCODEC)
				char errMsg[160];
#endif

#if (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
				g_sdlSwrContext = swr_alloc_set_opts(
					NULL,                // allocating a new context
					AV_CH_LAYOUT_MONO,   // out channels layout
					AV_SAMPLE_FMT_S16,   // out format
					g_sdlAudioSpec.freq, // out rate
					AV_CH_LAYOUT_MONO,   // in channels layout
					AV_SAMPLE_FMT_S16,   // in format
					OPL_SAMPLE_RATE,     // in rate
					0,
					NULL
				);
				if (g_sdlSwrContext == NULL)
					BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: swr_alloc_set_opts failed!");
				int error = swr_init(g_sdlSwrContext);
				if (error != 0)
				{
					// av_err2str requires libavutil/libavutil-ffmpeg, so don't convert code to string
					snprintf(errMsg, sizeof(errMsg), "BE_ST_InitAudio: swr_init failed! Error code: %d", error);
					BE_ST_ExitWithErrorMsg(errMsg);
				}
#elif (defined REFKEEN_RESAMPLER_LIBAVRESAMPLE)
				g_sdlAvAudioResampleContext = avresample_alloc_context();
				if (g_sdlAvAudioResampleContext == NULL)
					BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: avresample_alloc_context failed!");
				av_opt_set_int(g_sdlAvAudioResampleContext, "in_channel_layout",  AV_CH_LAYOUT_MONO,   0);
				av_opt_set_int(g_sdlAvAudioResampleContext, "out_channel_layout", AV_CH_LAYOUT_MONO,   0);
				av_opt_set_int(g_sdlAvAudioResampleContext, "in_sample_rate",     OPL_SAMPLE_RATE,     0);
				av_opt_set_int(g_sdlAvAudioResampleContext, "out_sample_rate",    g_sdlAudioSpec.freq, 0);
				av_opt_set_int(g_sdlAvAudioResampleContext, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0);
				av_opt_set_int(g_sdlAvAudioResampleContext, "out_sample_fmt",     AV_SAMPLE_FMT_S16,   0);
				int error = avresample_open(g_sdlAvAudioResampleContext);
				if (error != 0)
				{
					// av_err2str requires libavutil/libavutil-ffmpeg, so don't convert code to string
					snprintf(errMsg, sizeof(errMsg), "BE_ST_InitAudio: swr_init failed! Error code: %d", error);
					BE_ST_ExitWithErrorMsg(errMsg);
				}
#elif (defined REFKEEN_RESAMPLER_LIBAVCODEC)
				avcodec_register_all();
				g_sdlAvResampleContext = av_resample_init(
					g_sdlAudioSpec.freq,	// out rate
					OPL_SAMPLE_RATE,	// in rate
					16,	// filter length
					10,	// phase count
					0,	// linear FIR filter
					1.0	// cutoff frequency
				);
				if (g_sdlAvResampleContext == NULL)
					BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: av_resample_init failed!");
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
				g_sdlResampleFactor = (double)g_sdlAudioSpec.freq/OPL_SAMPLE_RATE;
				g_sdlResampleHandle = resample_open(0, g_sdlResampleFactor, g_sdlResampleFactor);
				if (g_sdlResampleHandle == NULL)
					BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: resample_open failed!");
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
				soxr_io_spec_t io_spec = soxr_io_spec(SOXR_INT16, SOXR_INT16);
				soxr_quality_spec_t q_spec = soxr_quality_spec(SOXR_LQ, 0); // Default quality spec adds an audible latency for resampling to 8000Hz
				soxr_error_t error;
				g_sdlSoxr = soxr_create(
					OPL_SAMPLE_RATE, // in rate
					g_sdlAudioSpec.freq, // out rate
					1, // channels
					&error,
					&io_spec,
					&q_spec,
					NULL // runtime spec
				);
				if (g_sdlSoxr == NULL)
				{
					snprintf(errMsg, sizeof(errMsg), "BE_ST_InitAudio: soxr_create failed!\nError: %s", soxr_strerror(error));
					BE_ST_ExitWithErrorMsg(errMsg);
				}
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
				int error;
				g_sdlSpeexResamplerState = speex_resampler_init(
					1, // channels
					OPL_SAMPLE_RATE, // in rate
					g_sdlAudioSpec.freq, // out rate
					0, // quality in the range 0-10 (10 is higher)
					&error
				);
				if (g_sdlSpeexResamplerState == NULL)
				{
					snprintf(errMsg, sizeof(errMsg), "BE_ST_InitAudio: speex_resampler_init failed! Error code: %d\nError: %s", error, speex_resampler_strerror(error));
					BE_ST_ExitWithErrorMsg(errMsg);
				}
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
				int error;
				g_sdlSrcResampler = src_new(SRC_SINC_FASTEST, 1, &error);
				if (g_sdlSrcResampler == NULL)
				{
					snprintf(errMsg, sizeof(errMsg), "BE_ST_InitAudio: src_new failed!\nError code: %d", error);
					BE_ST_ExitWithErrorMsg(errMsg);
				}
				g_sdlSrcData.data_in = g_sdlALOutSamples;
				g_sdlSrcData.src_ratio = (double)g_sdlAudioSpec.freq / OPL_SAMPLE_RATE;
#endif
			}
			else
#endif // REFKEEN_RESAMPLER_NONE
			{
				// The sum of all entries should be g_sdlAudioSpec.freq,
				// "uniformly" distributed over g_sdlALSampleRateConvTable
				g_sdlSampleRateConvTable = (int *)malloc(sizeof(int) * OPL_SAMPLE_RATE);
				if (g_sdlSampleRateConvTable == NULL)
					BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Failed to allocate memory for sample rate conversion!");
				for (int i = 0; i < OPL_SAMPLE_RATE; ++i)
				{
					// Using uint64_t cause an overflow is possible
					g_sdlSampleRateConvTable[i] = ((uint64_t)(i+1)*(uint64_t)g_sdlAudioSpec.freq/OPL_SAMPLE_RATE)-(uint64_t)i*(uint64_t)g_sdlAudioSpec.freq/OPL_SAMPLE_RATE;
				}
				g_sdlSampleRateConvCurrIndex = 0;
				g_sdlSampleRateConvCounter = 0;
			}
			g_sdlPCOutNumOfSamples = 2*g_sdlAudioSpec.samples;
			g_sdlPCOutSamples = (BE_ST_SndSample_T *)malloc(sizeof(BE_ST_SndSample_T) * g_sdlPCOutNumOfSamples); 
			if (g_sdlPCOutSamples == NULL)
				BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlPCOutSamples.)");
		}
	}
}

static uint32_t g_sdlTicksOffset = 0;

void BE_ST_InitTiming(void)
{
	g_sdlTicksOffset = 0;
	g_sdlTimeCount = 0;
	g_sdlLastTicks = SDL_GetTicks();
}

void BE_ST_ShutdownAudio(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		if (g_sdlAudioSpec.callback == BEL_ST_Resampling_CallBack)
		{
#ifndef REFKEEN_RESAMPLER_NONE
			if (g_refKeenCfg.useResampler)
			{
#if (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
				swr_free(&g_sdlSwrContext);
#elif (defined REFKEEN_RESAMPLER_LIBAVRESAMPLE)
				avresample_free(&g_sdlAvAudioResampleContext);
#elif (defined REFKEEN_RESAMPLER_LIBAVCODEC)
				av_resample_close(g_sdlAvResampleContext);
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
				resample_close(g_sdlResampleHandle);
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
				soxr_delete(g_sdlSoxr);
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
				speex_resampler_destroy(g_sdlSpeexResamplerState);
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
				src_delete(g_sdlSrcResampler);
#endif
			}
			else
#endif // REFKEEN_RESAMPLER_NONE
			{
				free(g_sdlSampleRateConvTable);
			}
		}
#ifdef REFKEEN_CONFIG_THREADS
		SDL_DestroyMutex(g_sdlCallbackMutex);
		g_sdlCallbackMutex = NULL;
#endif
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		g_sdlAudioSubsystemUp = false;
	}
	g_sdlCallbackSDFuncPtr = 0; // Just in case this may be called after the audio subsystem was never really started (manual calls to callback)
}

void BE_ST_StartAudioSDService(void (*funcPtr)(void))
{
	g_sdlCallbackSDFuncPtr = funcPtr;
	if (g_sdlAudioSubsystemUp)
	{
		SDL_PauseAudio(0);
	}
}

void BE_ST_StopAudioSDService(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_PauseAudio(1);
	}
	g_sdlCallbackSDFuncPtr = 0;
}

void BE_ST_LockAudioRecursively(void)
{
#ifdef REFKEEN_CONFIG_THREADS
	if (g_sdlAudioSubsystemUp)
	{
		SDL_LockMutex(g_sdlCallbackMutex);
	}
#endif
}

void BE_ST_UnlockAudioRecursively(void)
{
#ifdef REFKEEN_CONFIG_THREADS
	if (g_sdlAudioSubsystemUp)
	{
		SDL_UnlockMutex(g_sdlCallbackMutex);
	}
#endif
}

// Use this ONLY if audio subsystem isn't properly started up
void BE_ST_PrepareForManualAudioSDServiceCall(void)
{
	// HACK: Rather than using SDL_PauseAudio for deciding if
	// we call, just check if g_sdlCallbackSDFuncPtr is non-NULL
	// (not necessarily the same behaviors, but "good enough")
	if (g_sdlCallbackSDFuncPtr)
	{
		static uint32_t s_lastTicks;

		uint32_t currTicks = SDL_GetTicks();
		if (currTicks != s_lastTicks)
		{
			uint8_t buff[NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM];
			uint32_t bytesPassed = currTicks-s_lastTicks; // Using a sample rate of 500Hz here, or equivalenty the byte rate of 1000Hz
			bytesPassed &= ~(sizeof(BE_ST_SndSample_T) - 1); // Ensure value is divisible by sizeof(BE_ST_SndSample_T)
			BEL_ST_Simple_CallBack(NULL, buff, (bytesPassed <= sizeof(buff)) ? bytesPassed : sizeof(buff));
			s_lastTicks += bytesPassed;
		}
	}
}

bool BE_ST_IsEmulatedOPLChipReady(void)
{
	return g_sdlEmulatedOPLChipReady;
}

// Frequency is about 1193182Hz/spkVal
void BE_ST_PCSpeakerOn(uint16_t spkVal)
{
	g_sdlPCSpeakerOn = true;
	g_sdlCurrentBeepSample = 0;
	g_sdlBeepHalfCycleCounter = 0;
	g_sdlBeepHalfCycleCounterUpperBound = g_sdlAudioSpec.freq * spkVal;
}

void BE_ST_PCSpeakerOff(void)
{
	g_sdlPCSpeakerOn = false;
}

void BE_ST_BSound(uint16_t frequency)
{
	BE_ST_LockAudioRecursively();
	BE_ST_PCSpeakerOn(PC_PIT_RATE/(uint32_t)frequency);
	BE_ST_UnlockAudioRecursively();
}

void BE_ST_BNoSound(void)
{
	BE_ST_LockAudioRecursively();
	BE_ST_PCSpeakerOff();
	BE_ST_UnlockAudioRecursively();
}

/*******************************************************************************
OPL emulation, powered by dbopl from DOSBox and using bits of code from Wolf4SDL
*******************************************************************************/

Chip oplChip;

static inline void YM3812Init(int numChips, int clock, int rate)
{
	DBOPL_InitTables();
	Chip__Chip(&oplChip);
	Chip__Setup(&oplChip, rate);
}

static inline void YM3812Write(Chip *which, Bit32u reg, Bit8u val)
{
	Chip__WriteReg(which, reg, val);
}

static inline void YM3812UpdateOne(Chip *which, BE_ST_SndSample_T *stream, int length)
{
	Bit32s buffer[OPL_NUM_OF_SAMPLES * 2];
	int i;

	// length should be at least the max. samplesPerMusicTick
	// in Catacomb 3-D and Keen 4-6, which is param_samplerate / 700.
	// So 512 is sufficient for a sample rate of 358.4 kHz.
	if(length > OPL_NUM_OF_SAMPLES)
		length = OPL_NUM_OF_SAMPLES;

	Chip__GenerateBlock2(which, length, buffer);

	// GenerateBlock2 generates a number of "length" 32-bit mono samples
	// so we only need to convert them to 16-bit mono samples
	for(i = 0; i < length; i++)
	{
		// Scale volume
		Bit32s sample = 2*buffer[i];
		if(sample > 16383) sample = 16383;
		else if(sample < -16384) sample = -16384;
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
		stream[i] = (float)sample/32767.0f;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
		stream[i] = sample;
#endif
	}
}

// Drop-in replacement for id_sd.c:alOut
void BE_ST_ALOut(uint8_t reg,uint8_t val)
{
	BE_ST_LockAudioRecursively(); // RECURSIVE lock

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
	unsigned int length = OPL_SAMPLE_RATE / 10000;

	if (length > OPL_NUM_OF_SAMPLES - g_sdlALOutSamplesEnd)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_ST_ALOut overflow, want %u, have %u\n", length, OPL_NUM_OF_SAMPLES - g_sdlALOutSamplesEnd); // FIXME - Other thread
		length = OPL_NUM_OF_SAMPLES - g_sdlALOutSamplesEnd;
	}
	if (length)
	{
		YM3812UpdateOne(&oplChip, &g_sdlALOutSamples[g_sdlALOutSamplesEnd], length);
		g_sdlALOutSamplesEnd += length;
	}

	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
}

/**********************************************************************
PC Speaker emulation; The function adds audio data to the stream.
ASSUMPTION: The speaker is outputting sound (g_sdlPCSpeakerOn == true).
**********************************************************************/
static inline void PCSpeakerUpdateOne(BE_ST_SndSample_T *stream, int length)
{
	for (int loopVar = 0; loopVar < length; loopVar++, stream++)
	{
		*stream = g_sdlCurrentBeepSample;
		g_sdlBeepHalfCycleCounter += 2 * PC_PIT_RATE;
		if (g_sdlBeepHalfCycleCounter >= g_sdlBeepHalfCycleCounterUpperBound)
		{
			g_sdlBeepHalfCycleCounter %= g_sdlBeepHalfCycleCounterUpperBound;
			// Changing between 0 and max. possible value - too loud
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
			g_sdlCurrentBeepSample = (g_sdlCurrentBeepSample < 0.5f) ? 0.75f : 0.0f;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
			g_sdlCurrentBeepSample ^= 24575;
#endif
		}
	}
}

// WARNING: Possibly the wrong place to call the OPL emulator,
// but otherwise a separate dedicated thread may be required
static void BEL_ST_Simple_CallBack(void *unused, Uint8 *stream, int len)
{
	BE_ST_SndSample_T *currSamplePtr = (BE_ST_SndSample_T *)stream;
	uint32_t currNumOfSamples;
#if SDL_VERSION_ATLEAST(1,3,0)
	memset(stream, 0, len);
#endif
	/////////////////////////////
	BE_ST_LockAudioRecursively(); // RECURSIVE lock
	/////////////////////////////

	while (len)
	{
		if (!g_sdlSampleOffsetInSound)
		{
			// FUNCTION VARIABLE (We should use this and we want to kind-of separate what we have here from original code.)
			g_sdlCallbackSDFuncPtr();
		}
		// Now generate sound
		currNumOfSamples = BE_Cross_TypedMin(uint32_t, len/sizeof(BE_ST_SndSample_T), g_sdlSamplePerPart-g_sdlSampleOffsetInSound);
		// PC Speaker
		if (g_sdlPCSpeakerOn)
			PCSpeakerUpdateOne(currSamplePtr, currNumOfSamples);
		/*** AdLib (including hack for alOut delays) ***/
		if (g_sdlEmulatedOPLChipReady)
		{
			// We may have pending AL data ready, but probably less than required
			// for filling the stream buffer, so generate some silence.
			//
			// Make sure we don't overthrow the AL buffer, though.
			uint32_t targetALSamples = currNumOfSamples;
			if (targetALSamples > OPL_NUM_OF_SAMPLES)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Simple_CallBack AL overflow, want %u, have %u\n", targetALSamples, (unsigned int)OPL_NUM_OF_SAMPLES); // FIXME - Other thread
				targetALSamples = OPL_NUM_OF_SAMPLES;
			}
			// TODO Output overflow warning if there's any
			if (targetALSamples > g_sdlALOutSamplesEnd)
			{
				YM3812UpdateOne(&oplChip, &g_sdlALOutSamples[g_sdlALOutSamplesEnd], targetALSamples - g_sdlALOutSamplesEnd);
				g_sdlALOutSamplesEnd = targetALSamples;
			}
			// Mix with AL data
			for (uint32_t i = 0; i < targetALSamples; ++i)
				currSamplePtr[i] = (currSamplePtr[i] + g_sdlALOutSamples[i]) / 2;
			// Move pending AL data
			if (targetALSamples < g_sdlALOutSamplesEnd)
				memmove(g_sdlALOutSamples, &g_sdlALOutSamples[targetALSamples], sizeof(BE_ST_SndSample_T) * (g_sdlALOutSamplesEnd - targetALSamples));
			g_sdlALOutSamplesEnd -= targetALSamples;
		}
		// We're done for now
		currSamplePtr += currNumOfSamples;
		g_sdlSampleOffsetInSound += currNumOfSamples;
		len -= sizeof(BE_ST_SndSample_T)*currNumOfSamples;
		// End of part?
		if (g_sdlSampleOffsetInSound >= g_sdlSamplePerPart)
		{
			g_sdlSampleOffsetInSound = 0;
		}
	}

	///////////////////////////////
	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
	///////////////////////////////
}

// WARNING: Possibly the wrong place to call the OPL emulator *and* the
// resampler, but otherwise a separate dedicated thread (or two) may be required
static void BEL_ST_Resampling_CallBack(void *unused, Uint8 *stream, int len)
{
	BE_ST_SndSample_T *currSamplePtr = (BE_ST_SndSample_T *)stream;
	uint64_t currNumOfScaledSamples;
	uint64_t scaledSamplesToGenerate = (uint64_t)(len/sizeof(BE_ST_SndSample_T)) * OPL_SAMPLE_RATE; // TODO consider lcm?
	uint64_t scaledSamplesToGenerateNextTime = (uint64_t)g_sdlAudioSpec.freq * OPL_SAMPLE_RATE / 100; // ~10ms
#if SDL_VERSION_ATLEAST(1,3,0)
	memset(stream, 0, len);
#endif
	/////////////////////////////
	BE_ST_LockAudioRecursively(); // RECURSIVE lock
	/////////////////////////////
	
	while (len)
	{
		uint64_t processedScaledInputSamples = 0;
		if (g_sdlPCOutSamplesEnd > 0)
		{
			processedScaledInputSamples = BE_Cross_TypedMin(uint64_t, scaledSamplesToGenerate, (uint64_t)g_sdlPCOutSamplesEnd * OPL_SAMPLE_RATE);
			scaledSamplesToGenerate -= processedScaledInputSamples;
		}
		while (scaledSamplesToGenerate)
		{
			if (!g_sdlScaledSampleOffsetInSound)
			{
				// FUNCTION VARIABLE (We should use this and we want to kind-of separate what we have here from original code.)
				g_sdlCallbackSDFuncPtr();
			}
			// Now generate sound
			currNumOfScaledSamples = BE_Cross_TypedMin(uint64_t, scaledSamplesToGenerate, g_sdlScaledSamplePerPart-g_sdlScaledSampleOffsetInSound);
			processedScaledInputSamples += currNumOfScaledSamples;
			/*** FIXME - Scaling back to the original rates may be a bit inaccurate (due to divisions) ***/

			// PC Speaker
			uint32_t targetPCSamples = processedScaledInputSamples / OPL_SAMPLE_RATE;
			if (targetPCSamples > g_sdlPCOutNumOfSamples)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Resampling_CallBack PC overflow, want %u, have %u\n", targetPCSamples, g_sdlPCOutNumOfSamples);
				targetPCSamples = g_sdlPCOutNumOfSamples;
			}
			if (targetPCSamples > g_sdlPCOutSamplesEnd)
			{
				if (g_sdlPCSpeakerOn)
					PCSpeakerUpdateOne(&g_sdlPCOutSamples[g_sdlPCOutSamplesEnd], targetPCSamples - g_sdlPCOutSamplesEnd);
				else
					memset(&g_sdlPCOutSamples[g_sdlPCOutSamplesEnd], 0, sizeof(BE_ST_SndSample_T)*(targetPCSamples - g_sdlPCOutSamplesEnd));

				g_sdlPCOutSamplesEnd = targetPCSamples;
			}
			// AdLib:
			//
			// We may have pending AL data ready, but probably less than required
			// for filling the stream buffer, so generate some silence.
			//
			// Make sure we don't overthrow the AL buffer, though.
			uint32_t targetALSamples = processedScaledInputSamples / g_sdlAudioSpec.freq;
			if (targetALSamples > OPL_NUM_OF_SAMPLES)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Resampling_CallBack AL overflow, want %u, have %u\n", targetALSamples, (unsigned int)OPL_NUM_OF_SAMPLES);
				targetALSamples = OPL_NUM_OF_SAMPLES;
			}
			if (targetALSamples > g_sdlALOutSamplesEnd)
			{
				YM3812UpdateOne(&oplChip, &g_sdlALOutSamples[g_sdlALOutSamplesEnd], targetALSamples - g_sdlALOutSamplesEnd);
				g_sdlALOutSamplesEnd = targetALSamples;
			}
			// We're done with current part for now
			g_sdlScaledSampleOffsetInSound += currNumOfScaledSamples;
			scaledSamplesToGenerate -= currNumOfScaledSamples;
			// End of part?
			if (g_sdlScaledSampleOffsetInSound >= g_sdlScaledSamplePerPart)
			{
				g_sdlScaledSampleOffsetInSound = 0;
			}
		}

		// Try to resample the AL data we have (as much as possible)
		uint32_t maxSamplesToOutput = BE_Cross_TypedMin(uint32_t, len / sizeof(BE_ST_SndSample_T), g_sdlPCOutSamplesEnd); // Not taking plain (len / sizeof(BE_ST_SndSample_T)), just to make it safer

		// Just some type differences
#ifdef REFKEEN_RESAMPLER_LIBSOXR
		size_t samples_consumed, samples_produced;
#elif (defined REFKEEN_RESAMPLER_NONE) || (defined REFKEEN_RESAMPLER_LIBSPEEXDSP) || (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
		uint32_t samples_consumed, samples_produced;
#elif (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE) || (defined REFKEEN_RESAMPLER_LIBAVRESAMPLE) || (defined REFKEEN_RESAMPLER_LIBAVCODEC) || (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
		int samples_consumed, samples_produced;
#endif

#ifndef REFKEEN_RESAMPLER_NONE
		if (g_refKeenCfg.useResampler)
		{
#if (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
			samples_consumed = g_sdlALOutSamplesEnd;
			const uint8_t * inPtrs[] = { (uint8_t *)g_sdlALOutSamples, NULL };
			uint8_t *outPtrs[] =  { (uint8_t *)currSamplePtr, NULL };
			samples_produced = swr_convert(g_sdlSwrContext, outPtrs, maxSamplesToOutput, inPtrs, g_sdlALOutSamplesEnd);
#elif (defined REFKEEN_RESAMPLER_LIBAVRESAMPLE)
			samples_consumed = g_sdlALOutSamplesEnd;
			uint8_t *inPtr = (uint8_t *)g_sdlALOutSamples;
			uint8_t *outPtr = (uint8_t *)currSamplePtr;
			// Previously generated output samples may be queued, and we should read them separately
			int pending_produced_samples = avresample_available(g_sdlAvAudioResampleContext);
			if (pending_produced_samples > 0)
			{
				samples_produced = BE_Cross_TypedMin(int, maxSamplesToOutput, pending_produced_samples);
				avresample_read(g_sdlAvAudioResampleContext, &outPtr, samples_produced);
				outPtr += sizeof(BE_ST_SndSample_T)*samples_produced;
			}
			else
				samples_produced = 0;

			if ((uint32_t)samples_produced < maxSamplesToOutput)
				samples_produced += avresample_convert(g_sdlAvAudioResampleContext, &outPtr, 0, maxSamplesToOutput - samples_produced, &inPtr, 0, g_sdlALOutSamplesEnd);
#elif (defined REFKEEN_RESAMPLER_LIBAVCODEC)
			samples_consumed = 0;
			samples_produced = av_resample(g_sdlAvResampleContext, currSamplePtr, g_sdlALOutSamples, &samples_consumed, g_sdlALOutSamplesEnd, maxSamplesToOutput, 1);
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
			samples_consumed = 0;
			samples_produced = resample_process(g_sdlResampleHandle, g_sdlResampleFactor, g_sdlALOutSamples, g_sdlALOutSamplesEnd, 0, &samples_consumed, currSamplePtr, maxSamplesToOutput);
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
			samples_consumed = samples_produced = 0;
			soxr_process(g_sdlSoxr, g_sdlALOutSamples, g_sdlALOutSamplesEnd, &samples_consumed, currSamplePtr, maxSamplesToOutput, &samples_produced);
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
			samples_consumed = g_sdlALOutSamplesEnd;
			samples_produced = maxSamplesToOutput;
			speex_resampler_process_int(g_sdlSpeexResamplerState, 0, g_sdlALOutSamples, &samples_consumed, currSamplePtr, &samples_produced);
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
			g_sdlSrcData.data_out = currSamplePtr;
			g_sdlSrcData.input_frames = g_sdlALOutSamplesEnd;
			g_sdlSrcData.output_frames = maxSamplesToOutput;
			src_process(g_sdlSrcResampler, &g_sdlSrcData);
			samples_consumed = g_sdlSrcData.input_frames_used;
			samples_produced = g_sdlSrcData.output_frames_gen;
#endif
		}
		else
#endif // REFKEEN_RESAMPLER_NONE
		{
			samples_consumed = samples_produced = 0;
			/*** Note: Casting to unsigned in order to suppress comparison-related warnings ***/
			while (((unsigned)samples_consumed < g_sdlALOutSamplesEnd) && ((unsigned)samples_produced < maxSamplesToOutput))
			{
				if (g_sdlSampleRateConvCounter)
				{
					currSamplePtr[samples_produced++] = g_sdlSampleRateConvLastValue;
					--g_sdlSampleRateConvCounter;
				}
				else
				{
					g_sdlSampleRateConvLastValue = g_sdlALOutSamples[samples_consumed++];
					g_sdlSampleRateConvCounter = g_sdlSampleRateConvTable[g_sdlSampleRateConvCurrIndex];
					g_sdlSampleRateConvCurrIndex = (g_sdlSampleRateConvCurrIndex+1) % OPL_SAMPLE_RATE;
				}
			}
		}

		len -= sizeof(BE_ST_SndSample_T)*samples_produced;
		// Mix PC Speaker output
		for (uint32_t i = 0; i < BE_Cross_TypedMin(uint32_t, samples_produced, g_sdlPCOutSamplesEnd); ++i)
			currSamplePtr[i] = (currSamplePtr[i] + g_sdlPCOutSamples[i]) / 2;

		currSamplePtr += samples_produced;

		/*** Note: Casting to unsigned for suppression of warnings again ***/
		if (((unsigned)samples_consumed > 0) && ((unsigned)samples_consumed < g_sdlALOutSamplesEnd))
			memmove(g_sdlALOutSamples, &g_sdlALOutSamples[samples_consumed], sizeof(BE_ST_SndSample_T)*(g_sdlALOutSamplesEnd - samples_consumed));
		g_sdlALOutSamplesEnd -= samples_consumed;

		if ((unsigned)samples_produced >= g_sdlPCOutSamplesEnd)
			g_sdlPCOutSamplesEnd = 0;
		else if (((unsigned)samples_produced > 0) && ((unsigned)samples_produced < g_sdlPCOutSamplesEnd))
		{
			memmove(g_sdlPCOutSamples, &g_sdlPCOutSamples[samples_produced], sizeof(BE_ST_SndSample_T)*(g_sdlPCOutSamplesEnd - samples_produced));
			g_sdlPCOutSamplesEnd -= samples_produced;
		}

		// Resampling may add some latency (audible or not), so generate a few more input samples if required.
		// In case we're stuck with no change, again we should try to generate more samples.
		if (!samples_consumed && !samples_produced)
			scaledSamplesToGenerateNextTime += (uint64_t)g_sdlAudioSpec.freq * OPL_SAMPLE_RATE / 1000; // ~1ms
		scaledSamplesToGenerate = scaledSamplesToGenerateNextTime;
	}
	///////////////////////////////
	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
	///////////////////////////////
}

// Here, the actual rate is about 1193182Hz/speed
// NOTE: isALMusicOn is irrelevant for Keen Dreams (even with its music code)
void BE_ST_SetTimer(uint16_t speed, bool isALMusicOn)
{
	BE_ST_LockAudioRecursively(); // RECURSIVE lock

	g_sdlSamplePerPart = (int32_t)speed * g_sdlAudioSpec.freq / PC_PIT_RATE;
	g_sdlScaledSamplePerPart = g_sdlSamplePerPart * OPL_SAMPLE_RATE;
	// In the original code, the id_sd.c:SDL_t0Service callback
	// is responsible for incrementing TimeCount at a given rate
	// (~70Hz), although the rate in which the service itself is
	// 560Hz with music on and 140Hz otherwise.
	g_sdlScaledTimerDivisor = isALMusicOn ? (speed*8) : (speed*2);

	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
}

void BEL_ST_TicksDelayWithOffset(int sdltickstowait);
void BEL_ST_TimeCountWaitByPeriod(int16_t timetowait);

uint32_t BE_ST_GetTimeCount(void)
{
	// FIXME: What happens when SDL_GetTicks() reaches the upper bound?
	// May be challenging to fix... A proper solution should
	// only work with *differences between SDL_GetTicks values*.

	// WARNING: This must have offset subtracted! (So the game "thinks" it gets the correct (but actually delayed) TimeCount value)
	uint32_t currOffsettedSdlTicks = SDL_GetTicks() - g_sdlTicksOffset;
	uint32_t ticksToAdd = (uint64_t)currOffsettedSdlTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor) - (uint64_t)g_sdlLastTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor);
	g_sdlTimeCount += ticksToAdd;
	g_sdlLastTicks = currOffsettedSdlTicks;
	return g_sdlTimeCount;
}

void BE_ST_SetTimeCount(uint32_t newcount)
{
	g_sdlTimeCount = newcount;
}

void BE_ST_TimeCountWaitForDest(uint32_t dsttimecount)
{
	BEL_ST_TimeCountWaitByPeriod((int32_t)dsttimecount-(int32_t)g_sdlTimeCount);
}

void BE_ST_TimeCountWaitFromSrc(uint32_t srctimecount, int16_t timetowait)
{
	BEL_ST_TimeCountWaitByPeriod((srctimecount+timetowait)-g_sdlTimeCount);
}

void BEL_ST_TimeCountWaitByPeriod(int16_t timetowait)
{
	if (timetowait <= 0)
	{
		return;
	}
	// COMMENTED OUT - Do NOT refresh TimeCount and g_sdlLastTicks
	//BE_ST_GetTimeCount();

	// We want to find the minimal currSdlTicks value (mod 2^32) such that...
	// ticksToWait <= (uint64_t)currSdlTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor) - (uint64_t)g_sdlLastTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor)
	// (WARNING: The divisions here are INTEGER divisions!)
	// i.e.,
	// ticksToWait + (uint64_t)g_sdlLastTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor) <= (uint64_t)currSdlTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor)
	// That is, ignoring the types and up to the division by PC_PIT_RATE being an integer:
	// currSdlTicks >= [ticksToWait + (uint64_t)g_sdlLastTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor)]*1000*g_sdlScaledTimerDivisor / PC_PIT_RATE
	// So it may seem like we can replace ">=" with "=" and give it a go.
	//
	// PROBLEM: The last division IS (or at leat should be) integer. Suppose e.g., the condition is
	// currSdlTicks >= (PC_PIT_RATE-100) / PC_PIT_RATE
	// Then we get currSdlTicks = 0, which is not what we want (the minimal integer which is at at least this ratio as a noninteger is 1.)
	//
	// SOLUTION: Add PC_PIC_RATE-1 to the numerator.

	uint32_t nextSdlTicks = ((timetowait + (uint64_t)g_sdlLastTicks * (uint64_t)PC_PIT_RATE / (1000*g_sdlScaledTimerDivisor))*1000*g_sdlScaledTimerDivisor + (PC_PIT_RATE-1)) / PC_PIT_RATE;
	// NOTE: nextSdlTicks is already adjusted in terms of offset, so we can simply reset it here
	g_sdlTicksOffset = 0;
	BEL_ST_TicksDelayWithOffset(nextSdlTicks-SDL_GetTicks());
}


void BE_ST_WaitVBL(int16_t number)
{
	// (REFKEEN) Difference from vanilla: If the input value is not
	// positive then the game practically hangs for quite long
	// (ok, more than 7 minutes). It basically feels like a true hang,
	// which isn't desired anyway. So we assume that number > 0.

	// TODO (REFKEEN) Make a difference based on HW?

	// Simulate waiting while in vertical retrace first, and then
	// waiting while NOT in vertical retrace. In practice, we jump
	// to the very beginning of the next "refresh cycle".
	// This is repeated for a total of 'length' times.

	uint32_t currSdlTicks = SDL_GetTicks();
	uint32_t nextSdlTicks = (int)number*1000000/70086 + currSdlTicks - g_sdlTicksOffset;
	// First iteration takes a bit less time again, so we want
	// the value "mod" about 1000/70.086 (VGA adapter);
	// For the sake of a bit better precision we further multiply by 1000
	nextSdlTicks -= (((uint64_t)1000*(uint64_t)nextSdlTicks) % ((uint64_t)1000000000/70086))/1000;
	g_sdlTicksOffset = 0; // Can reset this, taking g_sdlTicksOffset into account above
	BEL_ST_TicksDelayWithOffset(nextSdlTicks-currSdlTicks);
}

// Call during a busy loop of some unknown duration (e.g., waiting for key press/release)
void BE_ST_ShortSleep(void)
{
	BEL_ST_SleepMS(1);
	// TODO: Make this more efficient
	BEL_ST_UpdateHostDisplay();
	BE_ST_PollEvents();
}


void BE_ST_Delay(uint16_t msec) // Replacement for delay from dos.h
{
	BEL_ST_TicksDelayWithOffset(msec);
}

void BEL_ST_TicksDelayWithOffset(int sdltickstowait)
{
	if (sdltickstowait <= (int32_t)g_sdlTicksOffset)
	{
		// Already waited for this time earlier, no need to do so now
		if (sdltickstowait > 0)
		{
			g_sdlTicksOffset -= sdltickstowait;
		}
		BE_ST_PollEvents(); // Still safer to do this
		return;
	}
	uint32_t nextSdlTicks = SDL_GetTicks() + sdltickstowait - g_sdlTicksOffset;
	BEL_ST_UpdateHostDisplay();
	BE_ST_PollEvents();
	uint32_t currSdlTicks = SDL_GetTicks();
	while ((int32_t)(currSdlTicks - nextSdlTicks) < 0)
	{
		BEL_ST_SleepMS(1);
		BE_ST_PollEvents();
		currSdlTicks = SDL_GetTicks();
	} 
	g_sdlTicksOffset = (currSdlTicks - nextSdlTicks);
}
