/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "refkeen_config.h"

#include "be_cross.h"
#include "be_sound_device_flags.h"
#include "be_st.h"
#include "be_st_sdl_private.h"
#include "backend/audio/be_audio_mixer.h"
#include "backend/audio/be_audio_private.h"
#include "backend/audio/be_audio_resampling.h"
#include "backend/timing/be_timing.h"

int g_sdlOutputAudioFreq;

bool g_sdlAudioSubsystemUp;
static bool g_sdlAudioInitDone; // Even if audio subsystem isn't brought up

// Use this if the audio subsystem is disabled for most (we want a BYTES rate of 1000Hz, same units as used in values returned by BEL_ST_GetTicksMS())
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

void BE_ST_InitAudio(void)
{
	g_sdlAudioSubsystemUp = false;
	g_sdlEmulatedOPLChipReady = false;
	int samplesForSourceBuffer;
	int audioDeviceFlags = BE_Cross_GetSelectedGameVerAudioDeviceFlags();
	int expectedCallbackBufferLen = 0;

	if (g_refKeenCfg.sndSubSystem)
		expectedCallbackBufferLen = BEL_ST_InitAudioSubsystem();

	if (g_sdlAudioSubsystemUp)
	{
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
		// Size may be reported as "0" on Android, so use this just in case
		g_sdlCallbacksSamplesBufferOnePartCount = g_refKeenCfg.sndInterThreadBufferRatio * expectedCallbackBufferLen;
		g_sdlCallbacksSamplesBuffer = (BE_ST_SndSample_T *)malloc(2*(g_sdlCallbacksSamplesBufferOnePartCount*sizeof(BE_ST_SndSample_T))); // Allocate TWO parts
		if (!g_sdlCallbacksSamplesBuffer)
			BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlCallbacksSamplesBuffer.)");
		g_sdlSamplesRemainingForSDLAudioCallback = 0;
		samplesForSourceBuffer = g_sdlCallbacksSamplesBufferOnePartCount;
#else
		samplesForSourceBuffer = 2*expectedCallbackBufferLen;
#endif
	}
	else
	{
		// If the audio subsystem is off, let us simulate a byte rate
		// of 1000Hz (same as BEL_ST_GetTicksMS() time units)
		g_sdlOutputAudioFreq = NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM / sizeof(BE_ST_SndSample_T);
		g_sdlCallbacksSamplesBuffer = (BE_ST_SndSample_T *)malloc(NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM);
		if (!g_sdlCallbacksSamplesBuffer)
			BE_ST_ExitWithErrorMsg("BE_ST_InitAudio: Out of memory! (Failed to allocate g_sdlCallbacksSamplesBuffer.)");
		g_sdlCallbacksSamplesBufferOnePartCount = NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM / sizeof(BE_ST_SndSample_T);

		samplesForSourceBuffer = NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM;
	}

	BEL_ST_AudioMixerInit(g_sdlOutputAudioFreq);

	if (((audioDeviceFlags & BE_AUDIO_DEVICE_PCSPKR_REQUIRED) == BE_AUDIO_DEVICE_PCSPKR_REQUIRED) ||
	    (g_sdlAudioSubsystemUp && ((audioDeviceFlags & BE_AUDIO_DEVICE_PCSPKR) == BE_AUDIO_DEVICE_PCSPKR)))
		BEL_ST_AudioMixerAddSource(
			g_sdlOutputAudioFreq,
			samplesForSourceBuffer,
			BEL_ST_GenPCSpeakerSamples);

	if (((audioDeviceFlags & BE_AUDIO_DEVICE_DIGI_REQUIRED) == BE_AUDIO_DEVICE_DIGI_REQUIRED) ||
	    (g_sdlAudioSubsystemUp && ((audioDeviceFlags & BE_AUDIO_DEVICE_DIGI) == BE_AUDIO_DEVICE_DIGI)))
		BEL_ST_SetDigiMixerSource(
			BEL_ST_AudioMixerAddSource(
				8000,
				samplesForSourceBuffer,
				BEL_ST_GenDigiSamples));

	if (g_sdlAudioSubsystemUp && g_refKeenCfg.oplEmulation &&
	    ((audioDeviceFlags & BE_AUDIO_DEVICE_OPL) == BE_AUDIO_DEVICE_OPL))
	{
		BEL_ST_ResetOPLChip();
		BEL_ST_SetOPLMixerSource(
			BEL_ST_AudioMixerAddSource(
				OPL_SAMPLE_RATE,
				// Leave some room for calls to BE_ST_OPL2Write
				2*samplesForSourceBuffer,
				BEL_ST_GenOPLSamples));
		g_sdlEmulatedOPLChipReady = true;
	}

	// Regardless of the audio subsystem being off or on, have *some*
	// rate set (being ~18.2Hz). In DEMOCAT from The Catacomb Abyss v1.13,
	// BE_ST_BSound may be called, so be ready to generate samples.
	BE_ST_SetTimer(0);

	g_sdlManualAudioCallbackCallLastTicks = BEL_ST_GetTicksMS();
	g_sdlManualAudioCallbackCallDelayedSamples = 0;

	g_sdlAudioInitDone = true;

	// As stated above, BE_ST_BSound may be called,
	// so better start generation of samples
	if (g_sdlAudioSubsystemUp)
		BEL_ST_StartAudioSubsystem();
}

void BE_ST_ShutdownAudio(void)
{
	g_sdlAudioInitDone = false;

	if (g_sdlAudioSubsystemUp)
		BEL_ST_ShutdownAudioSubsystem();

	free(g_sdlCallbacksSamplesBuffer);
	g_sdlCallbacksSamplesBuffer = NULL;

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

// Use this ONLY if audio subsystem isn't properly
// started up if BE_ST_FILL_AUDIO_IN_MAIN_THREAD is not defined
void BE_ST_PrepareForManualAudioCallbackCall(void)
{
	uint32_t currTicks = BEL_ST_GetTicksMS();

	// If e.g., we call this function from BE_ST_PrepareForGameStartupWithoutAudio
	if (!g_sdlAudioInitDone)
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
		BEL_ST_AudioMixerCallback(g_sdlCallbacksSamplesBuffer, g_sdlCallbacksSamplesBufferOnePartCount);
	if (samplesToProcess > 0)
		BEL_ST_AudioMixerCallback(g_sdlCallbacksSamplesBuffer, samplesToProcess);
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
void BEL_ST_InterThread_CallBack(void *unused, Uint8 *stream, int len)
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
