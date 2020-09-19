/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
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

#include <stdlib.h>
#include <string.h>

#include "refkeen_config.h"

#include "SDL.h"

#include "be_cross.h"
#include "be_st.h"
#include "be_st_sdl_private.h"
#include "backend/audio/be_audio_private.h"
#include "backend/audio/be_audio_resampling.h"
#include "backend/timing/be_timing.h"

extern bool g_sdlForceGfxControlUiRefresh;

#ifdef REFKEEN_CONFIG_THREADS
static SDL_mutex* g_sdlCallbackMutex = NULL;
#endif
static SDL_AudioSpec g_sdlAudioSpec;
SDL_AudioCallback g_sdlOurAudioCallback;
/*static*/ SDL_AudioDeviceID g_sdlAudioDevice;

int g_sdlOutputAudioFreq;

bool g_sdlAudioSubsystemUp;

static uint32_t g_sdlSamplesPartNum = 0;
// Simple callback: Scale is samples*PC_PIT_RATE
// Resampling callback (with OPL emulation): Scale is samples*PC_PIT_RATE*OPL_SAMPLE_RATE
uint64_t g_sdlScaledSamplesPerPartsTimesPITRate;
uint64_t g_sdlScaledSamplesInCurrentPart;
static uint64_t g_sdlScaledSampleOffsetInSound;

// Used for digitized sound playback
extern uint32_t g_sdlSoundEffectSamplesLeft;

// Use this if the audio subsystem is disabled for most (we want a BYTES rate of 1000Hz, same units as used in values returned by SDL_GetTicks())
#define NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM 1000

// This is used if the sound subsystem is disabled, *or* if it's enabled and BE_ST_FILL_AUDIO_IN_MAIN_THREAD is defined.
//
// If enabled, this buffer is actually split into two subbuffers:
// One for main thread use, the other being shared with the audio callback thread.
static BE_ST_SndSample_T *g_sdlCallbacksSamplesBuffer;
// If two sub-buffers are used, this is the size of a single one
static uint32_t g_sdlCallbacksSamplesBufferOnePartCount;
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
static uint32_t g_sdlSamplesRemainingForSDLAudioCallback;
#endif

/*static */uint32_t g_sdlManualAudioCallbackCallLastTicks;
static uint32_t g_sdlManualAudioCallbackCallDelayedSamples;

// Used with resampling callback only
static BESDLResamplingContext g_sdlMainResamplingContext;
static BE_ST_SndSample_T *g_sdlMiscOutSamples;
static uint32_t g_sdlMiscOutNumOfSamples;
static uint32_t g_sdlMiscOutSamplesEnd = 0;

#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
static void BEL_ST_InterThread_CallBack(void *unused, Uint8 *stream, int len);
#endif

static void BEL_ST_Simple_EmuCallBack(void *unused, Uint8 *stream, int len);
void BEL_ST_Resampling_EmuCallBack(void *unused, Uint8 *stream, int len);
static void BEL_ST_Simple_DigiCallBack(void *unused, Uint8 *stream, int len);
static void BEL_ST_Resampling_DigiCallBack(void *unused, Uint8 *stream, int len);

void BE_ST_InitAudio(void)
{
	g_sdlAudioSubsystemUp = false;
	g_sdlEmulatedOPLChipReady = false;
	int inSampleRate = BE_Cross_GetSelectedGameVerSampleRate();
	bool doDigitized = (inSampleRate != 0);
	if (!doDigitized)
		inSampleRate = OPL_SAMPLE_RATE;

	if (g_refKeenCfg.sndSubSystem)
	{
#ifdef REFKEEN_PLATFORM_ANDROID
		setenv("SDL_AUDIODRIVER", "openslES", 0); // HACK intended to force this driver for (hopefully) significantly lower latency
#endif
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL audio system initialization failed,\n%s\n", SDL_GetError());
		}
		else
		{
			SDL_AudioSpec spec;
			spec.freq = g_refKeenCfg.sndSampleRate;
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
			spec.format = AUDIO_F32SYS;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
			spec.format = AUDIO_S16SYS;
#endif
			spec.channels = 1;
			// Should be some power-of-two roughly proportional to the sample rate; Using 1024 for 48000Hz.
			for (spec.samples = 1; spec.samples < g_refKeenCfg.sndSampleRate/64; spec.samples *= 2)
			{
			}

			if (doDigitized)
				g_sdlOurAudioCallback = (g_refKeenCfg.sndSampleRate == inSampleRate) ? BEL_ST_Simple_DigiCallBack : BEL_ST_Resampling_DigiCallBack;
			else
				g_sdlOurAudioCallback = ((g_refKeenCfg.sndSampleRate == inSampleRate) || !g_refKeenCfg.oplEmulation) ? BEL_ST_Simple_EmuCallBack : BEL_ST_Resampling_EmuCallBack;

#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
			spec.callback = BEL_ST_InterThread_CallBack;
#else
			spec.callback = g_sdlOurAudioCallback;
#endif

			spec.userdata = NULL;
			BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Initializing audio subsystem, requested spec: freq %d, format %u, channels %d, samples %u\n", (int)spec.freq, (unsigned int)spec.format, (int)spec.channels, (unsigned int)spec.samples);
			g_sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &spec, &g_sdlAudioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
			if (g_sdlAudioDevice <= 0)
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
					SDL_CloseAudioDevice(g_sdlAudioDevice);
					SDL_QuitSubSystem(SDL_INIT_AUDIO);
				}
				else
#endif
				{
					BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Audio subsystem initialized, received spec: freq %d, format %u, channels %d, samples %u, size %u\n", (int)g_sdlAudioSpec.freq, (unsigned int)g_sdlAudioSpec.format, (int)g_sdlAudioSpec.channels, (unsigned int)g_sdlAudioSpec.samples, (unsigned int)g_sdlAudioSpec.size);
					g_sdlOutputAudioFreq = g_sdlAudioSpec.freq;
					g_sdlAudioSubsystemUp = true;
				}
			}
		}
	}

	// If the audio subsystem is off, let us simulate a byte rate
	// of 1000Hz (same as SDL_GetTicks() time units)
	if (!g_sdlAudioSubsystemUp)
	{
		g_sdlOutputAudioFreq = doDigitized ? inSampleRate : (NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM / sizeof(BE_ST_SndSample_T));
		g_sdlOurAudioCallback = doDigitized ? BEL_ST_Simple_DigiCallBack : BEL_ST_Simple_EmuCallBack;

		g_sdlCallbacksSamplesBuffer = (BE_ST_SndSample_T *)malloc(NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM);
		if (!g_sdlCallbacksSamplesBuffer)
			BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlCallbacksSamplesBuffer.)");
		g_sdlCallbacksSamplesBufferOnePartCount = NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM / sizeof(BE_ST_SndSample_T);

		goto finish;
	}

#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
	// Size may be reported as "0" on Android, so use this just in case
	g_sdlCallbacksSamplesBufferOnePartCount = g_refKeenCfg.sndInterThreadBufferRatio * (g_sdlAudioSpec.size ? (g_sdlAudioSpec.size / sizeof(BE_ST_SndSample_T)) : g_sdlAudioSpec.samples);
	g_sdlCallbacksSamplesBuffer = (BE_ST_SndSample_T *)malloc(2*(g_sdlCallbacksSamplesBufferOnePartCount*sizeof(BE_ST_SndSample_T))); // Allocate TWO parts
	if (!g_sdlCallbacksSamplesBuffer)
		BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlCallbacksSamplesBuffer.)");
	g_sdlSamplesRemainingForSDLAudioCallback = 0;
#endif

	if (g_refKeenCfg.oplEmulation)
	{
		BEL_ST_ResetOPLChip();
		g_sdlEmulatedOPLChipReady = true;
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
		g_sdlALOutNumOfSamples = g_sdlCallbacksSamplesBufferOnePartCount*OPL_SAMPLE_RATE/g_sdlOutputAudioFreq;
#else
		g_sdlALOutNumOfSamples = 2*(2*g_sdlAudioSpec.samples*OPL_SAMPLE_RATE/g_sdlOutputAudioFreq); // Leave some room for calls to BE_ST_OPL2Write
#endif
		g_sdlALOutSamples = (BE_ST_SndSample_T *)malloc(sizeof(BE_ST_SndSample_T) * g_sdlALOutNumOfSamples);
		if (g_sdlALOutSamples == NULL)
			BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlALOutSamples.)");
	}

	if ((doDigitized || g_sdlEmulatedOPLChipReady) && (g_sdlOutputAudioFreq != inSampleRate))
	{
		// Should allocate this first, for srcData.data_in (libsamplerate)
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
		g_sdlMiscOutNumOfSamples = g_sdlCallbacksSamplesBufferOnePartCount;
#else
		g_sdlMiscOutNumOfSamples = 2*g_sdlAudioSpec.samples;
#endif
		g_sdlMiscOutSamples = (BE_ST_SndSample_T *)malloc(sizeof(BE_ST_SndSample_T) * g_sdlMiscOutNumOfSamples); 
		if (g_sdlMiscOutSamples == NULL)
			BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlMiscOutSamples.)");

		BEL_ST_InitResampling(&g_sdlMainResamplingContext,
				      g_sdlOutputAudioFreq, inSampleRate,
				      doDigitized ? g_sdlMiscOutSamples : g_sdlALOutSamples);
	}

	// As stated above, BE_ST_BSound may be called,
	// so better start generation of samples
	SDL_PauseAudioDevice(g_sdlAudioDevice, 0);

finish:
	// Regardless of the audio subsystem being off or on, have *some*
	// rate set (being ~18.2Hz). In DEMOCAT from The Catacomb Abyss v1.13,
	// BE_ST_BSound may be called, so be ready to generate samples.
	BE_ST_SetTimer(0);

	g_sdlManualAudioCallbackCallLastTicks = SDL_GetTicks();
	g_sdlManualAudioCallbackCallDelayedSamples = 0;
}

void BE_ST_ShutdownAudio(void)
{
	if (g_sdlAudioSubsystemUp)
	{
		SDL_PauseAudioDevice(g_sdlAudioDevice, 1);
		if ((g_sdlOurAudioCallback == BEL_ST_Resampling_EmuCallBack) || (g_sdlOurAudioCallback == BEL_ST_Resampling_DigiCallBack))
			BEL_ST_ShutdownResampling(&g_sdlMainResamplingContext);
#ifdef REFKEEN_CONFIG_THREADS
		SDL_DestroyMutex(g_sdlCallbackMutex);
		g_sdlCallbackMutex = NULL;
#endif
		SDL_CloseAudioDevice(g_sdlAudioDevice);
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		g_sdlAudioSubsystemUp = false;
	}

	free(g_sdlCallbacksSamplesBuffer);
	g_sdlCallbacksSamplesBuffer = NULL;
	free(g_sdlALOutSamples);
	g_sdlALOutSamples = NULL;
	free(g_sdlMiscOutSamples);
	g_sdlMiscOutSamples = NULL;

	g_sdlTimerIntFuncPtr = 0; // Just in case this may be called after the audio subsystem was never really started (manual calls to callback)
}

void BE_ST_StartAudioAndTimerInt(void (*funcPtr)(void))
{
	BE_ST_LockAudioRecursively();

	g_sdlTimerIntFuncPtr = funcPtr;
	BE_ST_SET_TIMER_INT_COUNTER_SET(0);

	BE_ST_UnlockAudioRecursively();
}

void BE_ST_StopAudioAndTimerInt(void)
{
	BE_ST_LockAudioRecursively();

	g_sdlTimerIntFuncPtr = 0;

	BE_ST_UnlockAudioRecursively();
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

// Use this ONLY if audio subsystem isn't properly
// started up if BE_ST_FILL_AUDIO_IN_MAIN_THREAD is not defined
void BE_ST_PrepareForManualAudioCallbackCall(void)
{
	uint32_t currTicks = SDL_GetTicks();

	// If e.g., we call this function from BE_ST_PrepareForGameStartupWithoutAudio
	if (!g_sdlOurAudioCallback)
		return;

	if (currTicks == g_sdlManualAudioCallbackCallLastTicks)
		return;

	// Using g_sdlOutputAudioFreq as the rate, we (generally) lose precision in the following division,
	// so we use g_sdlManualAudioCallbackCallDelayedSamples to accumulate lost samples.
	uint64_t dividend = ((uint64_t)g_sdlOutputAudioFreq)*(currTicks-g_sdlManualAudioCallbackCallLastTicks) + g_sdlManualAudioCallbackCallDelayedSamples;
	uint32_t samplesPassed = dividend/1000;
	g_sdlManualAudioCallbackCallDelayedSamples = dividend%1000;

	uint32_t samplesToProcess = samplesPassed;
	// Buffer has some constant size, so loop if required (which may hint at an overflow)
	for (; samplesToProcess >= g_sdlCallbacksSamplesBufferOnePartCount; samplesToProcess -= g_sdlCallbacksSamplesBufferOnePartCount)
		g_sdlOurAudioCallback(NULL, (Uint8 *)g_sdlCallbacksSamplesBuffer, g_sdlCallbacksSamplesBufferOnePartCount * sizeof(BE_ST_SndSample_T));
	if (samplesToProcess > 0)
		g_sdlOurAudioCallback(NULL, (Uint8 *)g_sdlCallbacksSamplesBuffer, samplesToProcess * sizeof(BE_ST_SndSample_T));
	g_sdlManualAudioCallbackCallLastTicks = currTicks;
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
	if (g_sdlAudioSubsystemUp)
	{
		// Pass samples to audio callback thread (as much as we can)
		BE_ST_LockAudioRecursively();
		// Note that if we filled more than g_sdlCallbacksSamplesBufferOnePartCount, and thus discarded some samples, they won't be covered here.
		int samplesToCopy = BE_Cross_TypedMin(int, samplesPassed, g_sdlCallbacksSamplesBufferOnePartCount - g_sdlSamplesRemainingForSDLAudioCallback);
		// NOTE: We copy to the SECOND HALF of the buffer!
		memcpy(g_sdlCallbacksSamplesBuffer + g_sdlCallbacksSamplesBufferOnePartCount + g_sdlSamplesRemainingForSDLAudioCallback, g_sdlCallbacksSamplesBuffer, samplesToCopy * sizeof(BE_ST_SndSample_T));
		g_sdlSamplesRemainingForSDLAudioCallback += samplesToCopy;
		BE_ST_UnlockAudioRecursively();
	}
#endif
}


// A (relatively) simple callback, used for copying samples from main thread to SDL audio callback thread
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
static void BEL_ST_InterThread_CallBack(void *unused, Uint8 *stream, int len)
{
	BE_ST_LockAudioRecursively();

	int samplesToCopy = BE_Cross_TypedMin(int, g_sdlSamplesRemainingForSDLAudioCallback, len / sizeof(BE_ST_SndSample_T));
	memcpy(stream, g_sdlCallbacksSamplesBuffer + g_sdlCallbacksSamplesBufferOnePartCount, samplesToCopy*sizeof(BE_ST_SndSample_T));
	// Shift remaining samples
	memmove(g_sdlCallbacksSamplesBuffer + g_sdlCallbacksSamplesBufferOnePartCount, g_sdlCallbacksSamplesBuffer + g_sdlCallbacksSamplesBufferOnePartCount + samplesToCopy, (g_sdlSamplesRemainingForSDLAudioCallback - samplesToCopy) * sizeof(BE_ST_SndSample_T));
	g_sdlSamplesRemainingForSDLAudioCallback -= samplesToCopy;

	BE_ST_UnlockAudioRecursively();
	// No need to have lock here
	if (samplesToCopy < len / (int)sizeof(BE_ST_SndSample_T))
		memset(stream + samplesToCopy * sizeof(BE_ST_SndSample_T), 0, len - samplesToCopy * sizeof(BE_ST_SndSample_T));
}
#endif


// WARNING: Possibly the wrong place to call the OPL emulator,
// but otherwise a separate dedicated thread may be required
static void BEL_ST_Simple_EmuCallBack(void *unused, Uint8 *stream, int len)
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
		if (!g_sdlScaledSampleOffsetInSound)
		{
			// FUNCTION VARIABLE (We should use this and we want to kind-of separate what we have here from original code.)
			if (g_sdlTimerIntFuncPtr)
			{
				g_sdlTimerIntFuncPtr();
#ifndef BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO
				BE_ST_SET_TIMER_INT_COUNTER_INC();
#endif
			}
		}
		// Now generate sound
		currNumOfSamples = BE_Cross_TypedMin(uint32_t, len/sizeof(BE_ST_SndSample_T), g_sdlScaledSamplesInCurrentPart-g_sdlScaledSampleOffsetInSound);
		// PC Speaker
		BEL_ST_GenPCSpeakerSamples(currSamplePtr, currNumOfSamples);
		/*** AdLib (including hack for BE_ST_OPL2Write delays) ***/
		if (g_sdlEmulatedOPLChipReady)
		{
			// We may have pending AL data ready, but probably less than required
			// for filling the stream buffer, so generate some silence.
			//
			// Make sure we don't overthrow the AL buffer, though.
			uint32_t targetALSamples = currNumOfSamples;
			if (targetALSamples > g_sdlALOutNumOfSamples)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Simple_EmuCallBack AL overflow, want %u, have %u\n", targetALSamples, g_sdlALOutNumOfSamples); // FIXME - Other thread
				targetALSamples = g_sdlALOutNumOfSamples;
			}
			// TODO Output overflow warning if there's any
			if (targetALSamples > g_sdlALOutSamplesEnd)
			{
				BEL_ST_GenOPLSamples(&g_sdlALOutSamples[g_sdlALOutSamplesEnd], targetALSamples - g_sdlALOutSamplesEnd);
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
		g_sdlScaledSampleOffsetInSound += currNumOfSamples;
		len -= sizeof(BE_ST_SndSample_T)*currNumOfSamples;
		// End of part?
		if (g_sdlScaledSampleOffsetInSound >= g_sdlScaledSamplesInCurrentPart)
		{
			g_sdlScaledSampleOffsetInSound = 0;
			if (++g_sdlSamplesPartNum == PC_PIT_RATE)
				g_sdlSamplesPartNum = 0;
			g_sdlScaledSamplesInCurrentPart = (g_sdlSamplesPartNum + 1) * g_sdlScaledSamplesPerPartsTimesPITRate / PC_PIT_RATE - g_sdlSamplesPartNum * g_sdlScaledSamplesPerPartsTimesPITRate / PC_PIT_RATE;
		}
	}

	///////////////////////////////
	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
	///////////////////////////////
}

// WARNING: Possibly the wrong place to call the OPL emulator *and* the
// resampler, but otherwise a separate dedicated thread (or two) may be required
void BEL_ST_Resampling_EmuCallBack(void *unused, Uint8 *stream, int len)
{
	BE_ST_SndSample_T *currSamplePtr = (BE_ST_SndSample_T *)stream;
	uint64_t currNumOfScaledSamples;
	uint64_t scaledSamplesToGenerate = (uint64_t)(len/sizeof(BE_ST_SndSample_T)) * OPL_SAMPLE_RATE; // TODO consider lcm?
	uint64_t scaledSamplesToGenerateNextTime = (uint64_t)g_sdlOutputAudioFreq * OPL_SAMPLE_RATE / 100; // ~10ms
#if SDL_VERSION_ATLEAST(1,3,0)
	memset(stream, 0, len);
#endif
	/////////////////////////////
	BE_ST_LockAudioRecursively(); // RECURSIVE lock
	/////////////////////////////
	
	while (len)
	{
		uint64_t processedScaledInputSamples = 0;
		// g_sdlMiscOutSamples is used as PC Speaker samples buffer here
		if (g_sdlMiscOutSamplesEnd > 0)
		{
			processedScaledInputSamples = BE_Cross_TypedMin(uint64_t, scaledSamplesToGenerate, (uint64_t)g_sdlMiscOutSamplesEnd * OPL_SAMPLE_RATE);
			scaledSamplesToGenerate -= processedScaledInputSamples;
		}
		while (scaledSamplesToGenerate)
		{
			if (!g_sdlScaledSampleOffsetInSound)
			{
				// FUNCTION VARIABLE (We should use this and we want to kind-of separate what we have here from original code.)
				if (g_sdlTimerIntFuncPtr)
				{
					g_sdlTimerIntFuncPtr();
#ifndef BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO
					BE_ST_SET_TIMER_INT_COUNTER_INC();
#endif
				}
			}
			// Now generate sound
			currNumOfScaledSamples = BE_Cross_TypedMin(uint64_t, scaledSamplesToGenerate, g_sdlScaledSamplesInCurrentPart-g_sdlScaledSampleOffsetInSound);
			processedScaledInputSamples += currNumOfScaledSamples;
			/*** FIXME - Scaling back to the original rates may be a bit inaccurate (due to divisions) ***/

			// PC Speaker
			uint32_t targetPCSamples = processedScaledInputSamples / OPL_SAMPLE_RATE;
			if (targetPCSamples > g_sdlMiscOutNumOfSamples)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Resampling_EmuCallBack PC overflow, want %u, have %u\n", targetPCSamples, g_sdlMiscOutNumOfSamples);
				targetPCSamples = g_sdlMiscOutNumOfSamples;
			}
			if (targetPCSamples > g_sdlMiscOutSamplesEnd)
			{
				BEL_ST_GenPCSpeakerSamples(&g_sdlMiscOutSamples[g_sdlMiscOutSamplesEnd], targetPCSamples - g_sdlMiscOutSamplesEnd);
				g_sdlMiscOutSamplesEnd = targetPCSamples;
			}
			// AdLib:
			//
			// We may have pending AL data ready, but probably less than required
			// for filling the stream buffer, so generate some silence.
			//
			// Make sure we don't overthrow the AL buffer, though.
			uint32_t targetALSamples = processedScaledInputSamples / g_sdlOutputAudioFreq;
			if (targetALSamples > g_sdlALOutNumOfSamples)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Resampling_EmuCallBack AL overflow, want %u, have %u\n", targetALSamples, g_sdlALOutNumOfSamples);
				targetALSamples = g_sdlALOutNumOfSamples;
			}
			if (targetALSamples > g_sdlALOutSamplesEnd)
			{
				BEL_ST_GenOPLSamples(&g_sdlALOutSamples[g_sdlALOutSamplesEnd], targetALSamples - g_sdlALOutSamplesEnd);
				g_sdlALOutSamplesEnd = targetALSamples;
			}
			// We're done with current part for now
			g_sdlScaledSampleOffsetInSound += currNumOfScaledSamples;
			scaledSamplesToGenerate -= currNumOfScaledSamples;
			// End of part?
			if (g_sdlScaledSampleOffsetInSound >= g_sdlScaledSamplesInCurrentPart)
			{
				g_sdlScaledSampleOffsetInSound = 0;
				if (++g_sdlSamplesPartNum == PC_PIT_RATE)
					g_sdlSamplesPartNum = 0;
				g_sdlScaledSamplesInCurrentPart = (g_sdlSamplesPartNum + 1) * g_sdlScaledSamplesPerPartsTimesPITRate / PC_PIT_RATE - g_sdlSamplesPartNum * g_sdlScaledSamplesPerPartsTimesPITRate / PC_PIT_RATE;
			}
		}

		// Try to resample the AL data we have (as much as possible)
		uint32_t maxSamplesToOutput = BE_Cross_TypedMin(uint32_t, len / sizeof(BE_ST_SndSample_T), g_sdlMiscOutSamplesEnd); // Not taking plain (len / sizeof(BE_ST_SndSample_T)), just to make it safer

		uint32_t samples_consumed, samples_produced;
		BEL_ST_DoResample(&g_sdlMainResamplingContext, &samples_consumed, &samples_produced, g_sdlALOutSamples, currSamplePtr, g_sdlALOutSamplesEnd, maxSamplesToOutput);

		len -= sizeof(BE_ST_SndSample_T)*samples_produced;
		// Mix PC Speaker output
		for (uint32_t i = 0; i < BE_Cross_TypedMin(uint32_t, samples_produced, g_sdlMiscOutSamplesEnd); ++i)
			currSamplePtr[i] = (currSamplePtr[i] + g_sdlMiscOutSamples[i]) / 2;

		currSamplePtr += samples_produced;

		if ((samples_consumed > 0) && (samples_consumed < g_sdlALOutSamplesEnd))
			memmove(g_sdlALOutSamples, &g_sdlALOutSamples[samples_consumed], sizeof(BE_ST_SndSample_T)*(g_sdlALOutSamplesEnd - samples_consumed));
		g_sdlALOutSamplesEnd -= samples_consumed;

		/*** Note: Casting to unsigned for suppression of warnings again ***/
		if ((unsigned)samples_produced >= g_sdlMiscOutSamplesEnd)
			g_sdlMiscOutSamplesEnd = 0;
		else if ((samples_produced > 0) && (samples_produced < g_sdlMiscOutSamplesEnd))
		{
			memmove(g_sdlMiscOutSamples, &g_sdlMiscOutSamples[samples_produced], sizeof(BE_ST_SndSample_T)*(g_sdlMiscOutSamplesEnd - samples_produced));
			g_sdlMiscOutSamplesEnd -= samples_produced;
		}

		// Resampling may add some latency (audible or not), so generate a few more input samples if required.
		// In case we're stuck with no change, again we should try to generate more samples.
		if (!samples_consumed && !samples_produced)
			scaledSamplesToGenerateNextTime += (uint64_t)g_sdlOutputAudioFreq * OPL_SAMPLE_RATE / 1000; // ~1ms
		scaledSamplesToGenerate = scaledSamplesToGenerateNextTime;
	}
	///////////////////////////////
	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
	///////////////////////////////
}



/*** Digitized sounds callbacks ***/


static void BEL_ST_Simple_DigiCallBack(void *unused, Uint8 *stream, int len)
{
	if (len <= 0)
		return; // Just in case

	/////////////////////////////
	BE_ST_LockAudioRecursively(); // RECURSIVE lock
	/////////////////////////////

	len /= sizeof(BE_ST_SndSample_T); // Convert to samples

	// A little bit of cheating since we don't actually call any timer handler here
	g_sdlScaledSampleOffsetInSound += (uint64_t)len * PC_PIT_RATE;
#ifndef BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO
	BE_ST_SET_TIMER_INT_COUNTER_ADD(g_sdlScaledSampleOffsetInSound / g_sdlScaledSamplesPerPartsTimesPITRate);
#endif
	g_sdlScaledSampleOffsetInSound %= g_sdlScaledSamplesPerPartsTimesPITRate;

	BEL_ST_GenDigiSamples((BE_ST_SndSample_T *)stream, len);

	///////////////////////////////
	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
	///////////////////////////////
}

static void BEL_ST_Resampling_DigiCallBack(void *unused, Uint8 *stream, int len)
{
	/////////////////////////////
	BE_ST_LockAudioRecursively(); // RECURSIVE lock
	/////////////////////////////

	// A little bit of cheating since we don't actually call any timer handler here
	g_sdlScaledSampleOffsetInSound += (len / sizeof(BE_ST_SndSample_T)) * PC_PIT_RATE;
#ifndef BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO
	BE_ST_SET_TIMER_INT_COUNTER_ADD(g_sdlScaledSampleOffsetInSound / g_sdlScaledSamplesPerPartsTimesPITRate);
#endif
	g_sdlScaledSampleOffsetInSound %= g_sdlScaledSamplesPerPartsTimesPITRate;

	while (len > 0)
	{
		if (g_sdlSoundEffectSamplesLeft > 0) // Input is always SINT16, output may differ
		{
			int samplesToCopy = BE_Cross_TypedMin(uint32_t, g_sdlMiscOutNumOfSamples - g_sdlMiscOutSamplesEnd, g_sdlSoundEffectSamplesLeft);
			BEL_ST_GenDigiSamples(&g_sdlMiscOutSamples[g_sdlMiscOutSamplesEnd], samplesToCopy);
			g_sdlMiscOutSamplesEnd += samplesToCopy;
		}
		if (g_sdlMiscOutSamplesEnd < g_sdlMiscOutNumOfSamples)
			memset(&g_sdlMiscOutSamples[g_sdlMiscOutSamplesEnd], 0, sizeof(BE_ST_SndSample_T)*(g_sdlMiscOutNumOfSamples - g_sdlMiscOutSamplesEnd));

		// Resample
		uint32_t samples_consumed, samples_produced;
		BEL_ST_DoResample(&g_sdlMainResamplingContext, &samples_consumed, &samples_produced, g_sdlMiscOutSamples, (BE_ST_SndSample_T *)stream, g_sdlMiscOutNumOfSamples, len/sizeof(BE_ST_SndSample_T));
		stream += sizeof(BE_ST_SndSample_T) * samples_produced;
		len -= sizeof(BE_ST_SndSample_T) * samples_produced;
		// Move pending sound data
		g_sdlMiscOutSamplesEnd = g_sdlMiscOutNumOfSamples - samples_consumed;
		if (g_sdlMiscOutSamplesEnd > 0)
			memmove(g_sdlMiscOutSamples, &g_sdlMiscOutSamples[samples_consumed], sizeof(BE_ST_SndSample_T) * g_sdlMiscOutSamplesEnd);
	}

	/////////////////////////////
	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
	/////////////////////////////
}
