#ifndef BE_CROSS_H
#define BE_CROSS_H

#include <inttypes.h>
#include "id_in.h" // ControlType
#include "id_sd.h" // SDMode, SMMode

typedef enum BE_Log_Message_Class_T
{
	BE_LOG_MSG_NORMAL, BE_LOG_MSG_WARNING, BE_LOG_MSG_ERROR
} BE_Log_Message_Class_T;

// TODO (CHOCO KEEN) BIG ENDIAN
#define BE_Cross_Swap16LE(x) (x)

int32_t BE_Cross_FileLengthFromHandle(int handle);
char *BE_Cross_ultoa_dec(uint32_t n, char *buffer);
char *BE_Cross_ltoa_dec(uint32_t n, char *buffer);
// Used for debugging
void BE_Cross_LogMessage(BE_Log_Message_Class_T msgClass, const char *format, ...);
// More (possibly semi) standard C functions emulated,
// taking English locale into account (and more, but NOT all)
int BE_Cross_toupper(int c);
int BE_Cross_strcasecmp(const char *s1, const char *s2);

void BE_Cross_textcolor(int color);
void BE_Cross_textbackground(int color);
void BE_Cross_clrscr(void);

// Semi cross-platform binary (non-textual) file I/O, where it can be used directly (config file)
size_t BE_Cross_readInt8LEBuffer(int handle, void *ptr, size_t count);
size_t BE_Cross_readInt16LE(int handle, void *ptr);
size_t BE_Cross_readInt32LE(int handle, void *ptr);
// Functions for read/writing an enum as a 16-bit value
size_t BE_Cross_read_SDMode_From16LE(int handle, SDMode *ptr);
size_t BE_Cross_read_SMMode_From16LE(int handle, SMMode *ptr);
size_t BE_Cross_read_ControlType_From16LE(int handle, ControlType *ptr);

// Same but for writing
size_t BE_Cross_writeInt8LEBuffer(int handle, const void *ptr, size_t count);
size_t BE_Cross_writeInt16LE(int handle, const void *ptr);
size_t BE_Cross_writeInt32LE(int handle, const void *ptr);

size_t BE_Cross_write_SDMode_To16LE(int handle, const SDMode *ptr);
size_t BE_Cross_write_SMMode_To16LE(int handle, const SMMode *ptr);
size_t BE_Cross_write_ControlType_TO16LE(int handle, const ControlType *ptr);


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

#endif // BE_CROSS_H
