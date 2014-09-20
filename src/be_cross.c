#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "be_cross.h"

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
