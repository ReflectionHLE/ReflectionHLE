#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// For template implementations of reads/writes of enums from/to 16 little-endian integers...
#ifdef REFKEEN_VER_KDREAMS
#include "kd_def.h"
#elif defined REFKEEN_VER_CAT3D
#include "c3_def.h"
#elif defined REFKEEN_VER_CATADVENTURES
#include "def.h"
#else
#error "FATAL ERROR: No Reflection port game macro is defined!"
#endif

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

char *BE_Cross_ltoa_dec(int32_t n, char *buffer)
{
	sprintf(buffer, "%"PRId32, n);
	return buffer;
}

char *BE_Cross_itoa_dec(int16_t n, char *buffer)
{
	sprintf(buffer, "%"PRId16, n);
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

// C99
void BE_Cross_puts(const char *str);
void BE_Cross_Simplified_printf(const char *str);
void BE_Cross_Simplified_cprintf(const char *str);
uint16_t BE_Cross_Compat_GetFarPtrRelocationSegOffset(void);
inline int32_t BE_Mem_FarCoreLeft(void);

size_t BE_Cross_readInt8LE(int handle, void *ptr)
{
	return read(handle, ptr, 1);
}

size_t BE_Cross_readInt16LE(int handle, void *ptr)
{
	size_t bytesread = read(handle, ptr, 2);
#ifdef BE_CROSS_IS_BIGENDIAN
	if (bytesread == 2)
	{
		*(uint16_t *)ptr = BE_Cross_Swap16(*(uint16_t *) ptr);
	}
#endif
	return bytesread;
}

size_t BE_Cross_readInt32LE(int handle, void *ptr)
{
	size_t bytesread = read(handle, ptr, 4);
#ifdef BE_CROSS_IS_BIGENDIAN
	if (bytesread == 4)
	{
		*(uint32_t *)ptr = BE_Cross_Swap16(*(uint32_t *) ptr);
	}
#endif
	return bytesread;
}

size_t BE_Cross_readInt8LEBuffer(int handle, void *ptr, size_t nbyte)
{
	return read(handle, ptr, nbyte);
}

size_t BE_Cross_readInt16LEBuffer(int handle, void *ptr, size_t nbyte)
{
#ifndef CK_CROSS_IS_BIGENDIAN
	return read(handle, ptr, nbyte);
#else
	size_t result = read(handle, ptr, nbytes);
	for (uint16_t *currptr = (uint16_t *)ptr, *endptr = currptr + result/2; currptr < endptr; ++currptr)
	{
		*currptr = BE_Cross_Swap16LE(*currptr);
	}
	return result;
#endif
}

size_t BE_Cross_writeInt8LE(int handle, const void *ptr)
{
	return write(handle, ptr, 1);
}

size_t BE_Cross_writeInt16LE(int handle, const void *ptr)
{
#ifndef CK_CROSS_IS_BIGENDIAN
	return write(handle, ptr, 2);
#else
	uint16_t val = BE_Cross_Swap16(*(uint16_t *) ptr);
	return write(handle, &val, 2);
#endif
}

size_t BE_Cross_writeInt32LE(int handle, const void *ptr)
{
#ifndef CK_CROSS_IS_BIGENDIAN
	return write(handle, ptr, 4);
#else
	uint32_t val = BE_Cross_Swap32(*(uint32_t *) ptr);
	return write(handle, &val, 4);
#endif
}

size_t BE_Cross_writeInt8LEBuffer(int handle, const void *ptr, size_t nbyte)
{
	return write(handle, ptr, nbyte);
}

size_t BE_Cross_writeInt16LEBuffer(int handle, const void *ptr, size_t nbyte)
{
#ifndef CK_CROSS_IS_BIGENDIAN
	return write(handle, ptr, nbyte);
#else
	size_t result = 0;
	for (uint16_t *currptr = (uint16_t *)ptr, *endptr = currptr + nbyte/2; currptr < endptr; ++currptr)
	{
		val = SDL_Swap16(*currptr);
		size_t bytesread = write(handle, currptr, 2);
		result += bytesread;
		if (bytesread < 2)
		{
			break;
		}
	}
	return result;
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
BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(classtype)
#if (defined REFKEEN_VER_CAT3D) || (defined REFKEEN_VER_CATADVENTURES)
BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(dirtype)
#endif

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

size_t BE_Cross_read_booleans_From16LEBuffer(int handle, bool *ptr, size_t nbyte)
{
	uint16_t val;
	size_t totalbytesread = 0, currbytesread;
	for (size_t curbyte = 0; curbyte < nbyte; curbyte += 2, ++ptr)
	{
		currbytesread = read(handle, &val, 2);
		totalbytesread += currbytesread;
		if (currbytesread < 2)
		{
			return totalbytesread;
		}
		*ptr = val; // No need to swap byte-order here
	}
	return totalbytesread;
}


size_t BE_Cross_write_boolean_To16LE(int handle, const bool *ptr)
{
	uint16_t val = BE_Cross_Swap16LE((uint16_t)(*ptr)); // Better to swap just in case...
	return write(handle, &val, 2);
}

size_t BE_Cross_write_booleans_To16LEBuffer(int handle, const bool *ptr, size_t nbyte)
{
	uint16_t val;
	size_t totalbyteswritten = 0, currbyteswritten;
	for (size_t curbyte = 0; curbyte < nbyte; curbyte += 2, ++ptr)
	{
		val = BE_Cross_Swap16LE((uint16_t)(*ptr)); // Better to swap just in case...
		currbyteswritten = write(handle, &val, 2);
		totalbyteswritten += currbyteswritten;
		if (currbyteswritten < 2)
		{
			return totalbyteswritten;
		}
	}
	return totalbyteswritten;
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

// C99
int32_t BE_Mem_FarCoreLeft(void);
