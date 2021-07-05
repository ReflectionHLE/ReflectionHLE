/* Copyright (C) 2014-2021 NY00123
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

#include "refkeen.h"
#include "be_audio_mixer.h"
#include "nukedopl/opl3.h"

bool g_sdlEmulatedOPLChipReady;

// Used for filling with samples from BE_ST_OPL2Write,
// in addition to the SDL audio CallBack itself
// (because waits between/after OPL writes are expected)
static BE_ST_AudioMixerSource *g_oplMixerSource;

static opl3_chip g_oplChip;

bool BE_ST_IsEmulatedOPLChipReady(void)
{
	return g_sdlEmulatedOPLChipReady;
}

void BEL_ST_ResetOPLChip(void)
{
	OPL3_Reset(&g_oplChip, OPL_SAMPLE_RATE);
}

void BEL_ST_SetOPLMixerSource(BE_ST_AudioMixerSource *src)
{
	g_oplMixerSource = src;
}

#define OPL_NUM_OF_SAMPLES 2048 // About 40ms of OPL sound data

void BEL_ST_GenOPLSamples(BE_ST_SndSample_T *stream, int length)
{
	Bit16s buffer[OPL_NUM_OF_SAMPLES * 2];
	int i;

	// length should be at least the max. samplesPerMusicTick
	// in Catacomb 3-D and Keen 4-6, which is param_samplerate / 700.
	// So 512 is sufficient for a sample rate of 358.4 kHz, which is
	// significantly higher than the OPL rate anyway.
	if(length > OPL_NUM_OF_SAMPLES)
		length = OPL_NUM_OF_SAMPLES;

	// Output is 16-bit stereo sound
	OPL3_GenerateStream(&g_oplChip, buffer, length);
	for(i = 0; i < length; i++)
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
		stream[i] = (float)buffer[2*i]/32767.0f;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
		stream[i] = buffer[2*i];
#endif
}

void BE_ST_OPL2Write(uint8_t reg, uint8_t val)
{
	if (!g_sdlEmulatedOPLChipReady)
		return; // Wolf3D may call this function from wl_menu.c on quit

	BE_ST_LockAudioRecursively(); // RECURSIVE lock

	// Per the AdLib manual, this function should simulate 6 reads
	// of the register port after writing to it (3.3 microseconds),
	// and then 35 more reads of the register port after
	// writing to the data port (23 microseconds).
	//
	// The above appears to be important for reproduction
	// of the fuse breakage sound in Keen 5 at the least,
	// as well as a few sound effects in The Catacomb Abyss
	// (hitting a locked gate, teleportation sound effect).
	OPL3_WriteReg(&g_oplChip, reg, val);
	// FIXME: For now we roughly simulate the above delays with a
	// hack, using a "magic number" that appears to make this work.
	unsigned int length = OPL_SAMPLE_RATE / 10000;

	if (length > g_oplMixerSource->in.size - g_oplMixerSource->in.num)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_ST_OPL2Write overflow, want %u, have %u\n", length, g_oplMixerSource->in.size - g_oplMixerSource->in.num); // FIXME - other thread
		length = g_oplMixerSource->in.size - g_oplMixerSource->in.num;
	}
	if (length)
	{
		BEL_ST_GenOPLSamples(&g_oplMixerSource->in.buffer[g_oplMixerSource->in.num], length);
		g_oplMixerSource->in.num += length;
	}

	BE_ST_UnlockAudioRecursively(); // RECURSIVE unlock
}
