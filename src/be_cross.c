/* Copyright (C) 2014-2017 NY00123
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

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "refkeen_config.h" // MUST precede other contents due to e.g., endianness-based ifdefs

#ifdef REFKEEN_PLATFORM_ANDROID
#include <android/log.h>
#endif

#include "be_cross.h"
#include "be_st.h" // For BE_ST_ExitWithErrorMsg

// C99
char *BE_Cross_ultoa_dec(uint32_t n, char *buffer);
char *BE_Cross_ltoa_dec(int32_t n, char *buffer);
char *BE_Cross_itoa_dec(int16_t n, char *buffer);

// Used for debugging
void BE_Cross_LogMessage(BE_Log_Message_Class_T msgClass, const char *format, ...)
{
	// TODO: For now we simply do this.
	va_list args;
	va_start(args, format);
	switch (msgClass)
	{
#ifdef REFKEEN_PLATFORM_ANDROID
	case BE_LOG_MSG_NORMAL:
		__android_log_vprint(ANDROID_LOG_INFO, "LOG_REFKEEN_INFO", format, args);
		break;
	case BE_LOG_MSG_WARNING:
		__android_log_vprint(ANDROID_LOG_WARN, "LOG_REFKEEN_WARN", format, args);
		break;
	case BE_LOG_MSG_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "LOG_REFKEEN_ERROR", format, args);
		break;
#else
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
#endif
	}
	va_end(args);
}

// C99
int BE_Cross_toupper(int c);
int BE_Cross_tolower(int c);
int BE_Cross_isupper(int c);
char *BE_Cross_safeandfastcstringcopy(char *dest, char *destEnd, const char *src);
char *BE_Cross_safeandfastcstringcopy_2strs(char *dest, char *destEnd, const char *src0, const char *src1);
char *BE_Cross_safeandfastcstringcopy_3strs(char *dest, char *destEnd, const char *src0, const char *src1, const char *src2);
char *BE_Cross_safeandfastcstringcopy_4strs(char *dest, char *destEnd, const char *src0, const char *src1, const char *src2, const char *src3);

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

// Similar to BE_Cross_strcasecmp, but compares up to count chars
int BE_Cross_strncasecmp(const char *s1, const char *s2, size_t count)
{
	unsigned char uc1, uc2;
	for (; count && (*s1) && (BE_Cross_toupper(*s1) == BE_Cross_toupper(*s2)); s1++, s2++, count--);
	// If done, return 0
	if (!count)
		return 0;
	// Otherwise behave as in BE_Cross_strcasecmp
	uc1 = (unsigned char)((char)(BE_Cross_toupper(*s1)));
	uc2 = (unsigned char)((char)(BE_Cross_toupper(*s2)));
	return ((int)uc1 - (int)uc2);
}

// Technically a little hack...
#if (defined REFKEEN_VER_CATARM) || (defined REFKEEN_VER_CATAPOC)
uint16_t BE_Cross_Compat_GetFarPtrRelocationSegOffset(void)
{
	uint16_t BE_ST_Compat_GetFarPtrRelocationSegOffset(void);
	return BE_ST_Compat_GetFarPtrRelocationSegOffset();
}
#endif


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

// Alternatives for Borland's randomize and random macros used in Catacomb Abyss
// A few pages about the algorithm:
// http://en.wikipedia.org/wiki/Linear_congruential_generator
// http://stackoverflow.com/questions/14672358/implemenation-for-borlandc-rand

static uint32_t g_crossRandomSeed = 0x015A4E36;

static int16_t BEL_Cross_rand(void)
{
	g_crossRandomSeed = 0x015A4E35*g_crossRandomSeed + 1;
	return ((int16_t)(g_crossRandomSeed >> 16) & 0x7FFF);
}

static void BEL_Cross_srand(uint16_t seed)
{
	g_crossRandomSeed = seed;
	BEL_Cross_rand();
}

int16_t BE_Cross_Brandom(int16_t num)
{
	// Cast to unsigned so integer overflow in multiplication is
	// well-defined, but division should still be signed
	return (((int32_t)((uint32_t)BEL_Cross_rand()*(uint32_t)num))/0x8000);
}

void BE_Cross_Brandomize(void)
{
	BEL_Cross_srand(time(NULL));
}

// UNSAFE alternative for Borland's getdate function used in Keen Dreams v1.00;
//
// Do NOT call the function from multiple threads!
// (Internally it uses localtime, which isn't thread-safe on Linux.)
void BE_Cross_GetLocalDate_UNSAFE(int *y, int *m, int *d)
{
	time_t t = time(NULL);
	struct tm *tmstruct = localtime(&t);
	*y = tmstruct->tm_year + 1900;
	*m = tmstruct->tm_mon + 1;
	*d = tmstruct->tm_mday;
}

static bool g_be_passArgsToMainFunc;
// Here the magic happens - used to clear a portion of the stack before
// changing to another "main" function (in case we get a loop).
// In a way, this should be similar to C++ exception handling,
// just C-compatible.
static jmp_buf g_be_mainFuncJumpBuffer;

// When a new main function is called in the middle (BE_Cross_Bexecv),
// by default the current (non-new) main function is stored as
// a "one time" function that shall *never* be called again.
//
// To prevent this, BE_Cross_Bexecv should get a finalizer function pointer,
// used to reset the sub-program as represented by current main function
// to its original state. (Emphasis on global and static variables.)
#define MAX_NUM_OF_ONE_TIME_MAIN_FUNCS 4

static int g_be_numOfOneTimeMainFuncs = 0;
static void (*g_be_oneTimeMainFuncs[MAX_NUM_OF_ONE_TIME_MAIN_FUNCS])(void);

/*static*/ void BEL_Cross_DoCallMainFunc(void)
{
	g_be_passArgsToMainFunc = false;
	setjmp(g_be_mainFuncJumpBuffer); // Ignore returned value, always doing the same thing
	// Do start!
	if (g_be_passArgsToMainFunc)
		((void (*)(int, const char **))be_lastSetMainFuncPtr)(id0_argc, id0_argv); // HACK
	else
		be_lastSetMainFuncPtr();
}

void BE_Cross_Bexecv(void (*mainFunc)(void), const char **argv, void (*finalizer)(void), bool passArgsToMainFunc)
{
	for (int i = 0; i < g_be_numOfOneTimeMainFuncs; ++i)
		if (g_be_oneTimeMainFuncs[i] == mainFunc)
			BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - One-time function unexpectedly called again!");

	if (finalizer)
		finalizer();
	else
	{
		if (g_be_numOfOneTimeMainFuncs == MAX_NUM_OF_ONE_TIME_MAIN_FUNCS)
			BE_ST_ExitWithErrorMsg("BE_Cross_Bexecv - Too many one-time functions called!");
		g_be_oneTimeMainFuncs[g_be_numOfOneTimeMainFuncs++] = be_lastSetMainFuncPtr;
	}

	// Note this does NOT work for memory not managed by us (e.g., simple calls to malloc)
	void BEL_Cross_ClearMemory(void);
	BEL_Cross_ClearMemory();

	id0_argv = argv;
	for (id0_argc = 0; *argv; ++id0_argc, ++argv)
		;

	be_lastSetMainFuncPtr = mainFunc;
	g_be_passArgsToMainFunc = passArgsToMainFunc;

	longjmp(g_be_mainFuncJumpBuffer, 0); // A little bit of magic
}
