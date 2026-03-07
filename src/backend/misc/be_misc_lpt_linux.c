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

#include "be_cross.h"
#include "be_misc_lpt.h"
#include "be_st_cfg.h"
#include <fcntl.h>
#include <linux/ppdev.h>
#include <sys/ioctl.h>
#include <unistd.h>

typedef struct
{
	int fd;
} BEL_ST_LPTHandle;

static BEL_ST_LPTHandle *BEL_ST_GetLPTHandle(BE_ST_LPTPort port)
{
	static BEL_ST_LPTHandle handles[BE_LPT_MAX];

	if (!g_refKeenCfg.lptPassthrough)
		return NULL;

	int lptNum = BE_ST_GetLPTFromPort(port);
	if (lptNum == 0 || lptNum > BE_LPT_MAX)
		return NULL;

	BEL_ST_LPTHandle *handle = &handles[lptNum-1];

	if(handle->fd != 0)
		return handle;

	char devName[] = "/dev/parport0";
	devName[sizeof(devName)-2] += lptNum-1;

	// Opening with read and/or write doesn't seem to matter for the ioctls
	int fd = open(devName, O_WRONLY);
	if (fd == -1)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "Failed to open parallel port %s\n", devName);
		return NULL;
	}

	if (ioctl(fd, PPCLAIM))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "Failed to claim parallel port %s\n", devName);
		close(fd);
		return NULL;
	}

	BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "Parallel port %s opened\n", devName);
	handle->fd = fd;
	return handle;
}

uint8_t BE_ST_LPTIn(BE_ST_LPTPort port)
{
	BE_ST_LPTRegister reg = BE_ST_GetLPTRegFromPort(port);
	uint8_t val = reg == BE_LPT_STATUS ? BE_LPT_STATUS_DISCONNECTED : 0;

	BEL_ST_LPTHandle *handle = BEL_ST_GetLPTHandle(port);
	if (handle == NULL)
		return val;

	switch (reg)
	{
		case BE_LPT_DATA:
			ioctl(handle->fd, PPRDATA, &val);
			break;
		case BE_LPT_STATUS:
			ioctl(handle->fd, PPRSTATUS, &val);
			break;
		case BE_LPT_CONTROL:
			ioctl(handle->fd, PPRCONTROL, &val);
			break;
		default:
			break;
	}
	return val;
}

void BE_ST_LPTOut(BE_ST_LPTPort port, uint8_t val)
{
	BEL_ST_LPTHandle *handle = BEL_ST_GetLPTHandle(port);
	if (handle == NULL)
		return;

	switch (BE_ST_GetLPTRegFromPort(port))
	{
		case BE_LPT_DATA:
			ioctl(handle->fd, PPWDATA, &val);
			break;
		case BE_LPT_CONTROL:
			ioctl(handle->fd, PPWCONTROL, &val);
			break;
		default:
			break;
	}
}

void BE_ST_LPTRelease(BE_ST_LPTPort port)
{
	BEL_ST_LPTHandle *handle = BEL_ST_GetLPTHandle(port);
	if (handle == NULL)
		return;

	ioctl(handle->fd, PPRELEASE);
	close(handle->fd);
	handle->fd = 0;
}
