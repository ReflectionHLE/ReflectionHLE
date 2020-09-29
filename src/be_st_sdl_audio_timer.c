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
#include "backend/audio/be_audio_main_thread.h"
#include "backend/audio/be_audio_mixer.h"
#include "backend/audio/be_audio_private.h"
#include "backend/audio/be_audio_resampling.h"
#include "backend/timing/be_timing.h"

int g_sdlOutputAudioFreq;

bool g_sdlAudioSubsystemUp;
bool g_sdlAudioInitDone; // Even if audio subsystem isn't brought up

void BE_ST_InitAudio(void)
{
	g_sdlAudioSubsystemUp = false;
	g_sdlEmulatedOPLChipReady = false;
	int samplesForSourceBuffer;
	int audioDeviceFlags = BE_Cross_GetSelectedGameVerAudioDeviceFlags();
	int expectedCallbackBufferLen = 0;

	if (g_refKeenCfg.sndSubSystem)
		expectedCallbackBufferLen = BEL_ST_InitAudioSubsystem();

	int freq = g_sdlOutputAudioFreq;

	if (g_sdlAudioSubsystemUp)
	{
#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
		samplesForSourceBuffer =
			BEL_ST_PrepareMainThreadForAudio(&freq, expectedCallbackBufferLen);
#else
		samplesForSourceBuffer = 2*expectedCallbackBufferLen;
#endif
	}
	else
	{
		samplesForSourceBuffer = BEL_ST_PrepareMainThreadForAudio(&freq, 0);
		g_sdlOutputAudioFreq = freq;
	}

	BEL_ST_AudioMixerInit(freq);

	if (((audioDeviceFlags & BE_AUDIO_DEVICE_PCSPKR_REQUIRED) == BE_AUDIO_DEVICE_PCSPKR_REQUIRED) ||
	    (g_sdlAudioSubsystemUp && ((audioDeviceFlags & BE_AUDIO_DEVICE_PCSPKR) == BE_AUDIO_DEVICE_PCSPKR)))
	{
		BEL_ST_SetPCSpeakerSampleRate(freq);
		BEL_ST_AudioMixerAddSource(
			freq,
			samplesForSourceBuffer,
			BEL_ST_GenPCSpeakerSamples);
	}

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

	BEL_ST_ClearMainThreadAudioResources();

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
