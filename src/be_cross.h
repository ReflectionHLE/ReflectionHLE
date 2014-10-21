#ifndef __BE_CROSS_H__
#define __BE_CROSS_H__

#include <stdbool.h>
#include <inttypes.h>

typedef enum BE_Log_Message_Class_T
{
	BE_LOG_MSG_NORMAL, BE_LOG_MSG_WARNING, BE_LOG_MSG_ERROR
} BE_Log_Message_Class_T;

// TODO (CHOCO KEEN) BIG ENDIAN
#define BE_Cross_Swap16LE(x) (x)

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

// Semi cross-platform binary (non-textual) file I/O, where it can be used directly (config file)
size_t BE_Cross_readInt8LEBuffer(int handle, void *ptr, size_t nbyte);
size_t BE_Cross_readInt8LE(int handle, void *ptr);
size_t BE_Cross_readInt16LE(int handle, void *ptr);
size_t BE_Cross_readInt32LE(int handle, void *ptr);
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
size_t BE_Cross_writeInt8LEBuffer(int handle, const void *ptr, size_t nbyte);
size_t BE_Cross_writeInt8LE(int handle, const void *ptr);
size_t BE_Cross_writeInt16LE(int handle, const void *ptr);
size_t BE_Cross_writeInt32LE(int handle, const void *ptr);

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

// Used for saved game compatibility (temp2 field which may have a 16-bit offset pointer to an object state)
void BE_Cross_Compat_FillObjStatesWithDOSOffsets(void);
// Return a void* rather than statetype* only because of current header inclusion mess...
void* BE_Cross_Compat_GetObjStatePtrFromDOSOffset(uint16_t offset);

// Alternatives for Borland's randomize and random macros used in Catacomb Abyss
void BE_Cross_Brandomize(void);
int16_t BE_Cross_Brandom(int16_t num);


#endif // BE_CROSS_H
