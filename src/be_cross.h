#ifndef BE_CROSS_H
#define BE_CROSS_H

#include <inttypes.h>

typedef enum BE_Log_Message_Class_T
{
	BE_LOG_MSG_NORMAL, BE_LOG_MSG_WARNING, BE_LOG_MSG_ERROR
} BE_Log_Message_Class_T;

int32_t BE_Cross_FileLengthFromHandle(int handle);
char *BE_Cross_ultoa_dec(uint32_t n, char *buffer);
char *BE_Cross_ltoa_dec(uint32_t n, char *buffer);
// Used for debugging
void BE_Cross_LogMessage(BE_Log_Message_Class_T msgClass, const char *format, ...);
// More (possibly semi) standard C functions emulated,
// taking English locale into account (and more, but NOT all)
int BE_Cross_toupper(int c);
int BE_Cross_strcasecmp(const char *s1, const char *s2);

inline void BE_Cross_textcolor(int color)
{
	BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_textcolor not implemented\n");
}

inline void BE_Cross_textbackground(int color)
{
	BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_textbackground not implemented\n");
}

inline void BE_Cross_clrscr(void)
{
	BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_Cross_clrscr not implemented\n");
}

#endif // BE_CROSS_H
