/* Copyright (C) 2014 NY00123
 *
 * This file is part of Chocolate Keen Dreams.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "id_heads.h"

int32_t BE_Cross_FileLengthFromHandle(int handle)
{
    off_t origOffset = lseek(handle, 0, SEEK_CUR);
    off_t len = lseek(handle, 0, SEEK_END);
    lseek(handle, origOffset, SEEK_SET);
    return len;
}

char *BE_Cross_ultoa_dec(uint32_t n, char *buffer)
{
	sprintf(buffer, "%"PRIu32, n);
	return buffer;
}

char *BE_Cross_ltoa_dec(uint32_t n, char *buffer)
{
	sprintf(buffer, "%"PRId32, n);
	return buffer;
}

// Used for debugging
void BE_Cross_LogMessage(BE_Log_Message_Class_T msgClass, const char *format, ...)
{
	// TODO: For now we simply do this.
	va_list args;
	va_start(args, format);
	switch (msgClass)
	{
	case BE_LOG_MSG_NORMAL:
		vprintf(format, args);
		break;
	case BE_LOG_MSG_WARNING:
		fprintf(stderr, "Warning: ");
		vfprintf(stderr, format, args);
		break;
	case BE_LOG_MSG_ERROR:
		fprintf(stderr, "Error: ");
		vfprintf(stderr, format, args);
		break;
	}
	va_end(args);
}

// More (possibly semi) standard C functions emulated,
// taking English locale into account (and more, but NOT all)
int BE_Cross_toupper(int c)
{
	return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : c;
}

int BE_Cross_strcasecmp(const char *s1, const char *s2)
{
	unsigned char uc1, uc2;
	/* This one is easy. We don't care if a value is signed or not. */
	/* All that matters here is consistency (everything is signed). */
	for (; (*s1) && (BE_Cross_toupper(*s1) == BE_Cross_toupper(*s2)); s1++, s2++);
	/* But now, first we cast from int to char, and only *then* to */
	/* unsigned char, so the correct difference can be calculated. */
	uc1 = (unsigned char)((char)(BE_Cross_toupper(*s1)));
	uc2 = (unsigned char)((char)(BE_Cross_toupper(*s2)));
	/* We should still cast back to int, for a signed difference. */
	/* Assumption: An int can store any unsigned char value.      */
	return ((int)uc1 - (int)uc2);
}

void BE_Cross_textcolor(int color)
{
	BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_textcolor not implemented\n");
}

void BE_Cross_textbackground(int color)
{
	BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_textbackground not implemented\n");
}

void BE_Cross_clrscr(void)
{
	BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_clrscr not implemented\n");
}

size_t BE_Cross_readInt8LEBuffer(int handle, void *ptr, size_t count)
{
	return read(handle, ptr, count);
}

size_t BE_Cross_readInt16LE(int handle, void *ptr)
{
	size_t bytesread = read(handle, ptr, 2);
#ifdef BE_CROSS_IS_BIGENDIAN
	for (size_t loopVar = 0; loopVar < count/2; loopVar++, ((uint16_t *) ptr)++)
		*(uint16_t *) ptr = BE_Cross_Swap16(*(uint16_t *) ptr);
#endif
	return bytesread;
}

size_t BE_Cross_readInt32LE(int handle, void *ptr)
{
	size_t bytesread = read(handle, ptr, 4);
#ifdef BE_CROSS_IS_BIGENDIAN
	for (size_t loopVar = 0; loopVar < count/4; loopVar++, ((uint32_t *) ptr)++)
		*(uint32_t *) ptr = BE_Cross_Swap16(*(uint32_t *) ptr);
#endif
	return bytesread;
}

size_t BE_Cross_writeInt8LEBuffer(int handle, const void *ptr, size_t count)
{
	return write(handle, ptr, count);
}

size_t BE_Cross_writeInt16LE(int handle, const void *ptr)
{
#ifndef CK_CROSS_IS_BIGENDIAN
	return write(handle, ptr, 2);
#else
	uint16_t val = BE_Cross_Swap16(*(uint16_t *) ptr);;
	return write(handle, &val, 2);
#endif
}

size_t BE_Cross_writeInt32LE(int handle, const void *ptr)
{
#ifndef CK_CROSS_IS_BIGENDIAN
	return write(handle, ptr, 4);
#else
	uint32_t val = BE_Cross_Swap32(*(uint32_t *) ptr);;
	return write(handle, &val, 4);
#endif
}

// Template implementation of enum reads/writes
#define BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(ourSampleEnum) \
size_t BE_Cross_read_ ## ourSampleEnum ## _From16LE (int handle, ourSampleEnum *ptr) \
{ \
	uint16_t val; \
	size_t bytesread = read(handle, &val, 2); \
	if (bytesread == 2) \
	{ \
		*ptr = (ourSampleEnum)BE_Cross_Swap16LE(val); \
	} \
	return bytesread; \
} \
\
size_t BE_Cross_write_ ## ourSampleEnum ## _To16LE (int handle, const ourSampleEnum *ptr) \
{ \
	uint16_t val = BE_Cross_Swap16LE((uint16_t)(*ptr)); \
	return write(handle, &val, 2); \
}

BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(SDMode)
BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(SMMode)
BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(ControlType)

size_t BE_Cross_read_boolean_From16LE(int handle, bool *ptr)
{
	uint16_t val;
	size_t bytesread = read(handle, &val, 2);
	if (bytesread == 2)
	{
		*ptr = val; // No need to swap byte-order here
	}
	return bytesread;
}

size_t BE_Cross_write_boolean_To16LE(int handle, const bool *ptr)
{
	uint16_t val = BE_Cross_Swap16LE((uint16_t)(*ptr));
	return write(handle, &val, 2);
}

void BE_Cross_Wrapped_Add(uint8_t *segPtr, uint8_t **offInSegPtrPtr, uint16_t count)
{
	*offInSegPtrPtr += count;
	if (*offInSegPtrPtr - segPtr >= 0x10000)
	{
		*offInSegPtrPtr -= 0x10000;
	}
}

void BE_Cross_Wrapped_Inc(uint8_t *segPtr, uint8_t **offInSegPtrPtr)
{
	++(*offInSegPtrPtr);
	if (*offInSegPtrPtr - segPtr >= 0x10000)
	{
		*offInSegPtrPtr -= 0x10000;
	}
}

void BE_Cross_LinearToWrapped_MemCopy(uint8_t *segDstPtr, uint8_t *offDstPtr, const uint8_t *linearSrc, uint16_t num)
{
	uint16_t bytesToEnd = (segDstPtr+0x10000)-offDstPtr;
	if (num <= bytesToEnd)
	{
		memcpy(offDstPtr, linearSrc, num);
	}
	else
	{
		memcpy(offDstPtr, linearSrc, bytesToEnd);
		memcpy(segDstPtr, linearSrc+bytesToEnd, num-bytesToEnd);
	}
}

void BE_Cross_WrappedToLinear_MemCopy(uint8_t *linearDst, const uint8_t *segSrcPtr, const uint8_t *offSrcPtr, uint16_t num)
{
	uint16_t bytesToEnd = (segSrcPtr+0x10000)-offSrcPtr;
	if (num <= bytesToEnd)
	{
		memcpy(linearDst, offSrcPtr, num);
	}
	else
	{
		memcpy(linearDst, offSrcPtr, bytesToEnd);
		memcpy(linearDst+bytesToEnd, segSrcPtr, num-bytesToEnd);
	}
}

void BE_Cross_WrappedToWrapped_MemCopy(uint8_t *segCommonPtr, uint8_t *offDstPtr, const uint8_t *offSrcPtr, uint16_t num)
{
	uint16_t srcBytesToEnd = (segCommonPtr+0x10000)-offSrcPtr;
	uint16_t dstBytesToEnd = (segCommonPtr+0x10000)-offDstPtr;
	if (num <= srcBytesToEnd)
	{
		// Source is linear: Same as BE_Cross_LinearToWrapped_MemCopy here
		if (num <= dstBytesToEnd)
		{
			memcpy(offDstPtr, offSrcPtr, num);
		}
		else
		{
			memcpy(offDstPtr, offSrcPtr, dstBytesToEnd);
			memcpy(segCommonPtr, offSrcPtr+dstBytesToEnd, num-dstBytesToEnd);
		}
		return;
	}
	// Otherwise, check if at least the destination is linear
	if (num <= dstBytesToEnd)
	{
		// Destination is linear: Same as BE_Cross_WrappedToLinear_MemCopy, non-linear source
		memcpy(offDstPtr, offSrcPtr, srcBytesToEnd);
		memcpy(offDstPtr+srcBytesToEnd, segCommonPtr, num-srcBytesToEnd);

		return;
	}
	// BOTH buffers have wrapping. We don't check separately if
	// srcBytesToEnd==dstBytesToEnd (in such a case offDstPtr==offSrcPtr...)
	if (srcBytesToEnd <= dstBytesToEnd)
	{
		memcpy(offDstPtr, offSrcPtr, srcBytesToEnd);
		memcpy(offDstPtr+srcBytesToEnd, segCommonPtr, dstBytesToEnd-srcBytesToEnd);
		memcpy(segCommonPtr, segCommonPtr+(dstBytesToEnd-srcBytesToEnd), num-dstBytesToEnd);
	}
	else // srcBytesToEnd > dstBytesToEnd
	{
		memcpy(offDstPtr, offSrcPtr, dstBytesToEnd);
		memcpy(segCommonPtr, offSrcPtr+dstBytesToEnd, srcBytesToEnd-dstBytesToEnd);
		memcpy(segCommonPtr+(srcBytesToEnd-dstBytesToEnd), segCommonPtr, num-srcBytesToEnd);
	}
}

void BE_Cross_Wrapped_MemSet(uint8_t *segPtr, uint8_t *offInSegPtr, int value, uint16_t num)
{
	uint16_t bytesToEnd = (segPtr+0x10000)-offInSegPtr;
	if (num <= bytesToEnd)
	{
		memset(offInSegPtr, value, num);
	}
	else
	{
		memset(offInSegPtr, value, bytesToEnd);
		memset(segPtr, value, num-bytesToEnd);
	}
}
