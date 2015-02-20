#ifndef __BE_CROSS_H__
#define __BE_CROSS_H__

#include <stdbool.h>
#include <inttypes.h>

// WARNING: It's actually safer to include this earlier (e.g., for endianness
// related macros), but this header may be the one included first
#include "refkeen_config.h"

typedef enum BE_Log_Message_Class_T
{
	BE_LOG_MSG_NORMAL, BE_LOG_MSG_WARNING, BE_LOG_MSG_ERROR
} BE_Log_Message_Class_T;

#define BE_Cross_Swap16(x) ((uint16_t)(((uint16_t)(x)<<8)|((uint16_t)(x)>>8)))
#define BE_Cross_Swap32(x) ((uint32_t)(((uint32_t)(x)<<24)|(((uint32_t)(x)<<8)&0x00FF0000)|(((uint32_t)(x)>>8)&0x0000FF00)|((uint32_t)(x)>>24)))

#ifdef REFKEEN_ARCH_LITTLE_ENDIAN
#define BE_Cross_Swap16LE(x) (x)
#define BE_Cross_Swap16BE(x) BE_Cross_Swap16(x)
#define BE_Cross_Swap32LE(x) (x)
#define BE_Cross_Swap32BE(x) BE_Cross_Swap32(x)
#else
#define BE_Cross_Swap16LE(x) BE_Cross_Swap16(x)
#define BE_Cross_Swap16BE(x) (x)
#define BE_Cross_Swap32LE(x) BE_Cross_Swap32(x)
#define BE_Cross_Swap32BE(x) (x)
#endif

// Used for some resource definitions internally
#ifdef REFKEEN_ARCH_LITTLE_ENDIAN
#define BE_Cross_SwapGroup16LE(a, b)       a, b,
#define BE_Cross_SwapGroup24LE(a, b, c)    a, b, c,
#define BE_Cross_SwapGroup32LE(a, b, c, d) a, b, c, d,
#else
#define BE_Cross_SwapGroup16LE(a, b)       b, a,
#define BE_Cross_SwapGroup24LE(a, b, c)    c, b, a,
#define BE_Cross_SwapGroup32LE(a, b, c, d) d, c, b, a,
#endif

int32_t BE_Cross_FileLengthFromHandle(int handle);
char *BE_Cross_ultoa_dec(uint32_t n, char *buffer);
char *BE_Cross_ltoa_dec(int32_t n, char *buffer);
char *BE_Cross_itoa_dec(int16_t n, char *buffer);
// Used for debugging
void BE_Cross_LogMessage(BE_Log_Message_Class_T msgClass, const char *format, ...);
// More (possibly semi) standard C functions emulated,
// taking English locale into account (and more, but NOT all)
int BE_Cross_toupper(int c);
int BE_Cross_strcasecmp(const char *s1, const char *s2);

// puts replacement that writes to emulated text mode memory
inline void BE_Cross_puts(const char *str)
{
	void BE_SDL_puts(const char *str);
	BE_SDL_puts(str);
}
// Limited printf replacement (doesn't accept % format specifiers, but \n is OK)
inline void BE_Cross_Simplified_printf(const char *str)
{
	void BE_SDL_Simplified_printf(const char *str);
	BE_SDL_Simplified_printf(str);
}
// Similar limited cprintf replacement
inline void BE_Cross_Simplified_cprintf(const char *str)
{
	void BE_SDL_Simplified_cprintf(const char *str);
	BE_SDL_Simplified_cprintf(str);
}

// Semi cross-platform binary (non-textual) file I/O, where it can be used directly (config file)
size_t BE_Cross_readInt8LE(int handle, void *ptr);
size_t BE_Cross_readInt16LE(int handle, void *ptr);
size_t BE_Cross_readInt32LE(int handle, void *ptr);
size_t BE_Cross_readInt8LEBuffer(int handle, void *ptr, size_t nbyte);
size_t BE_Cross_readInt16LEBuffer(int handle, void *ptr, size_t nbyte);
size_t BE_Cross_readInt32LEBuffer(int handle, void *ptr, size_t nbyte);
// This exists for the EGAHEADs from the Catacombs
size_t BE_Cross_readInt24LEBuffer(int handle, void *ptr, size_t nbyte);
// A template for enum reading (from 16-bit little-endian int).
// A declaration and implementation must exist for each used type separately
// (implementation should be found in be_cross.c).
#if 0
size_t BE_Cross_read_EnumType_From16LE(int handle, EnumType *ptr);
#endif
// boolean implementation may be separated from enums, otherwise it's the same
size_t BE_Cross_read_boolean_From16LE(int handle, bool *ptr);
// booleans buffer
size_t BE_Cross_read_booleans_From16LEBuffer(int handle, bool *ptr, size_t nbyte);

// Same but for writing
size_t BE_Cross_writeInt8LE(int handle, const void *ptr);
size_t BE_Cross_writeInt16LE(int handle, const void *ptr);
size_t BE_Cross_writeInt32LE(int handle, const void *ptr);
size_t BE_Cross_writeInt8LEBuffer(int handle, const void *ptr, size_t nbyte);
size_t BE_Cross_writeInt16LEBuffer(int handle, const void *ptr, size_t nbyte);

#if 0
size_t BE_Cross_write_EnumType_To16LE(int handle, const EnumType *ptr);
#endif

size_t BE_Cross_write_boolean_To16LE(int handle, const bool *ptr);
size_t BE_Cross_write_booleans_To16LEBuffer(int handle, const bool *ptr, size_t nbyte);

// Assuming segPtr is replacement for a 16-bit segment pointer, and offInSegPtr
// is a replacement for an offset in this segment (pointing to a place in the
// emulated segment), increases offInSegPtr by count with wrapping
// (if offInSegPtr-segPtr >= 0x10000 after wrapping, offInSegPtr is reduced by 0x10000)
void BE_Cross_Wrapped_Add(uint8_t *segPtr, uint8_t **offInSegPtrPtr, uint16_t count);
// Same as above but with count == 1 forced
void BE_Cross_Wrapped_Inc(uint8_t *segPtr, uint8_t **offInSegPtrPtr);
// Similar; Used for copying from linear buffer to cyclic 10000 bytes long "segment"
void BE_Cross_LinearToWrapped_MemCopy(uint8_t *segDstPtr, uint8_t *offDstPtr, const uint8_t *linearSrc, uint16_t num);
// Vice-versa
void BE_Cross_WrappedToLinear_MemCopy(uint8_t *linearDst, const uint8_t *segSrcPtr, const uint8_t *offSrcPtr, uint16_t num);
// Similar, but under a common segment.
// ASSUMPTION: Buffers do not overlap!!
void BE_Cross_WrappedToWrapped_MemCopy(uint8_t *segCommonPtr, uint8_t *offDstPtr, const uint8_t *offSrcPtr, uint16_t num);
// Wrapped memset
void BE_Cross_Wrapped_MemSet(uint8_t *segPtr, uint8_t *offInSegPtr, int value, uint16_t num);

// Used for saved game compatibility where 16-bit offset/near pointers are involved
// (temp2 field which may have a 16-bit offset pointer to an object state)
void BE_Cross_Compat_FillObjStatesWithDOSPointers(void);
// Same as above, but in case 32-bit far pointers are saved/loaded instead
// (the exact segment depends on location of DOS EXE in memory so we allow a variable shift)
void BE_Cross_Compat_FillObjStatesWithRelocatedDOSPointers(uint_fast16_t segoffset);
// Return a void* rather than statetype* only because of current header inclusion mess...
void* BE_Cross_Compat_GetObjStatePtrFromDOSPointer(uint_fast32_t dosptr);
// Returns an offset that should be added to the 16-bit segments of 32-bit
// far pointers present in The Catacomb Armageddon/Apocalypse saved games
// (in the case of the original DOS exes, it depends on the locations of
// modified copies of them in memory)
inline uint16_t BE_Cross_Compat_GetFarPtrRelocationSegOffset(void)
{
	uint16_t BE_SDL_Compat_GetFarPtrRelocationSegOffset(void);
	return BE_SDL_Compat_GetFarPtrRelocationSegOffset();
}

// Alternatives for Borland's randomize and random macros used in Catacomb Abyss
void BE_Cross_Brandomize(void);
int16_t BE_Cross_Brandom(int16_t num);

// Hack for Catacomb Abyss' INTRO and LOADSCN
inline int32_t BE_Mem_FarCoreLeft(void)
{
	return 500000;
}

// Use this in cases an original DOS program attempts to access contents of
// segment no. 0 for some reason
extern uint8_t g_be_cross_dosZeroSeg[];

#endif // BE_CROSS_H
