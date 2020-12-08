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

#include "refkeen_config.h"

#include "SDL.h"

#include "be_cross.h"
#include "be_st.h"
#include "be_st_sdl_private.h"
#include "backend/audio/be_audio_mixer.h"
#include "backend/audio/be_audio_private.h"

#ifdef REFKEEN_CONFIG_THREADS
static SDL_mutex* g_sdlCallbackMutex = NULL;
#endif
static SDL_AudioSpec g_sdlAudioSpec;
/*static*/ SDL_AudioDeviceID g_sdlAudioDevice;

extern bool g_sdlAudioSubsystemUp;

#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
void BEL_ST_InterThread_CallBack(void *unused, Uint8 *stream, int len);
#endif

static void BEL_ST_MixerCallback(void *unused, Uint8 *stream, int len)
{
	BEL_ST_AudioMixerCallback((BE_ST_SndSample_T *)stream,
	                          len / (g_sdlAudioSpec.channels * sizeof(BE_ST_SndSample_T)));
}

bool BEL_ST_InitAudioSubsystem(int *freq, int *channels, int *bufferLen)
{
	SDL_AudioSpec desiredSpec;
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "SDL audio system initialization failed,\n%s\n", SDL_GetError());
		return false;
	}
	desiredSpec.freq = g_refKeenCfg.sndSampleRate;
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
	desiredSpec.format = AUDIO_F32SYS;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
	desiredSpec.format = AUDIO_S16SYS;
#endif
	desiredSpec.channels = MIXER_DEFAULT_CHANNELS_COUNT;
	// Should be some power-of-two roughly proportional to the sample rate; Using 1024 for 48000Hz.
	for (desiredSpec.samples = 1; desiredSpec.samples < g_refKeenCfg.sndSampleRate/64; desiredSpec.samples *= 2)
		;

#ifdef BE_ST_FILL_AUDIO_IN_MAIN_THREAD
	desiredSpec.callback = BEL_ST_InterThread_CallBack;
#else
	desiredSpec.callback = BEL_ST_MixerCallback;
#endif

	desiredSpec.userdata = NULL;
	BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Initializing audio subsystem, requested spec: freq %d, format %u, channels %d, samples %u\n", (int)desiredSpec.freq, (unsigned int)desiredSpec.format, (int)desiredSpec.channels, (unsigned int)desiredSpec.samples);
	g_sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &g_sdlAudioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (g_sdlAudioDevice <= 0)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "Cannot open SDL audio device,\n%s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}
#ifdef REFKEEN_CONFIG_THREADS
	g_sdlCallbackMutex = SDL_CreateMutex();
	if (!g_sdlCallbackMutex)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Cannot create recursive mutex for SDL audio callback,\n%s\nClosing SDL audio subsystem\n", SDL_GetError());
		SDL_CloseAudioDevice(g_sdlAudioDevice);
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}
#endif
	BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Audio subsystem initialized, received spec: freq %d, format %u, channels %d, samples %u, size %u\n", (int)g_sdlAudioSpec.freq, (unsigned int)g_sdlAudioSpec.format, (int)g_sdlAudioSpec.channels, (unsigned int)g_sdlAudioSpec.samples, (unsigned int)g_sdlAudioSpec.size);

	// Size may be reported as "0" on Android
	*freq = g_sdlAudioSpec.freq;
	*channels = g_sdlAudioSpec.channels;
	*bufferLen = g_sdlAudioSpec.size ?
	             (g_sdlAudioSpec.size / sizeof(BE_ST_SndSample_T)) : g_sdlAudioSpec.samples;
	return true;
}

void BEL_ST_ShutdownAudioSubsystem(void)
{
	SDL_PauseAudioDevice(g_sdlAudioDevice, 1);
	BEL_ST_AudioMixerShutdown();
#ifdef REFKEEN_CONFIG_THREADS
	SDL_DestroyMutex(g_sdlCallbackMutex);
	g_sdlCallbackMutex = NULL;
#endif
	SDL_CloseAudioDevice(g_sdlAudioDevice);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void BEL_ST_StartAudioSubsystem(void)
{
	SDL_PauseAudioDevice(g_sdlAudioDevice, 0);
}

void BE_ST_LockAudioRecursively(void)
{
#ifdef REFKEEN_CONFIG_THREADS
	if (g_sdlAudioSubsystemUp)
		SDL_LockMutex(g_sdlCallbackMutex);
#endif
}

void BE_ST_UnlockAudioRecursively(void)
{
#ifdef REFKEEN_CONFIG_THREADS
	if (g_sdlAudioSubsystemUp)
		SDL_UnlockMutex(g_sdlCallbackMutex);
#endif
}
