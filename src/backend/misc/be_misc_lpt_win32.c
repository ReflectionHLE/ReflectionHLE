/* Copyright (C) 2026 Braden "Blzut3" Obrzut
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "refkeen_config.h" // Unicode
#include "be_cross.h"
#include "be_misc_lpt.h"
#include "be_st_cfg.h"
#include <windows.h>

typedef void (_stdcall *BEL_ST_Out32Func)(short, short);
typedef short (_stdcall *BEL_ST_Inp32Func)(short);
typedef BOOL (_stdcall *BEL_ST_IsInpOutDriverOpenFunc)();

static BEL_ST_Out32Func g_Out32;
static BEL_ST_Inp32Func g_Inp32;
static BEL_ST_IsInpOutDriverOpenFunc g_IsInpOutDriverOpen;

static int BEL_ST_UseInpOut()
{
	static enum {
		BEL_INPOUT_UNLOADED,
		BEL_INPOUT_UNAVAILABLE,
		BEL_INPOUT_LOADED,
	} status;

	if (status != BEL_INPOUT_UNLOADED)
		return status == BEL_INPOUT_LOADED;

	if (g_refKeenCfg.lptPassthrough)
	{
#if (defined _M_IX86) || (defined __i386__)
		HINSTANCE library = LoadLibrary(TEXT("inpout32.dll"));
#else
		HINSTANCE library = LoadLibrary(TEXT("inpoutx64.dll"));
#endif

		if (library != NULL)
		{
			g_Out32 = (BEL_ST_Out32Func)GetProcAddress(library, "Out32");
			g_Inp32 = (BEL_ST_Inp32Func)GetProcAddress(library, "Inp32");
			g_IsInpOutDriverOpen = (BEL_ST_IsInpOutDriverOpenFunc)GetProcAddress(library, "IsInpOutDriverOpen");

			if (g_Out32 && g_Inp32 && g_IsInpOutDriverOpen)
			{
				if (g_IsInpOutDriverOpen())
				{
					BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Initialized InpOut32/InpOutx64 for LPT passthrough\n");
					status = BEL_INPOUT_LOADED;
					return 1;
				}

				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Could not load InpOut32/InpOutx64 driver for LPT passthrough\n");
			}
			else
			{
				BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Could not find required symbols in InpOut32/InpOutx64\n");
			}
			FreeLibrary(library);
		}
		else
		{
			BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "Could not load InpOut32/InpOutx64 for LPT passthrough\n");
		}
	}

	status = BEL_INPOUT_UNAVAILABLE;
	return 0;
}

uint8_t BE_ST_LPTIn(BE_ST_LPTPort port)
{
	if (BEL_ST_UseInpOut())
		return g_Inp32(port);

	return BE_ST_GetLPTRegFromPort(port) == BE_LPT_STATUS ? BE_LPT_STATUS_DISCONNECTED : 0;
}

void BE_ST_LPTOut(BE_ST_LPTPort port, uint8_t val)
{
	if (BEL_ST_UseInpOut())
		g_Out32(port, val);
}

void BE_ST_LPTRelease(BE_ST_LPTPort port)
{
}
