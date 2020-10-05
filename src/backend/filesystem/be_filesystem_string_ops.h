#ifndef BE_FILESYSTEM_STRING_OPS_H
#define BE_FILESYSTEM_STRING_OPS_H

#include "be_cross.h"
#include "be_filesystem_char.h"

// FIXME: These shouldn't be file-system related, but for now they're here

#ifdef REFKEEN_PLATFORM_WINDOWS

static inline TCHAR *BEL_Cross_safeandfastctstringcopy(TCHAR *dest, TCHAR *destEnd, const TCHAR *src)
{
	TCHAR ch;
	// We assume that initially, destEnd - dest > 0.
	do
	{
		ch = *src++;
		*dest++ = ch; // This includes the null terminator if there's the room
	} while ((dest < destEnd) && ch);
	// These work in case dest == destEnd, and also if not
	--dest;
	*dest = _T('\0');
	return dest; // WARNING: This differs from strcpy!!
}

static inline TCHAR *BEL_Cross_safeandfastctstringcopy_2strs(TCHAR *dest, TCHAR *destEnd, const TCHAR *src0, const TCHAR *src1)
{
	return BEL_Cross_safeandfastctstringcopy(BEL_Cross_safeandfastctstringcopy(dest, destEnd, src0), destEnd, src1);
}

static inline TCHAR *BEL_Cross_safeandfastctstringcopy_3strs(TCHAR *dest, TCHAR *destEnd, const TCHAR *src0, const TCHAR *src1, const TCHAR *src2)
{
	return BEL_Cross_safeandfastctstringcopy(BEL_Cross_safeandfastctstringcopy(BEL_Cross_safeandfastctstringcopy(dest, destEnd, src0), destEnd, src1), destEnd, src2);
}

static inline TCHAR *BEL_Cross_safeandfastctstringcopy_4strs(TCHAR *dest, TCHAR *destEnd, const TCHAR *src0, const TCHAR *src1, const TCHAR *src2, const TCHAR *src3)
{
	return BEL_Cross_safeandfastctstringcopy(BEL_Cross_safeandfastctstringcopy(BEL_Cross_safeandfastctstringcopy(BEL_Cross_safeandfastctstringcopy(dest, destEnd, src0), destEnd, src1), destEnd, src2), destEnd, src3);
}

// Appends a narrow string to a wide string on Windows
static inline TCHAR *BEL_Cross_safeandfastcstringcopytoctstring(TCHAR *dest, TCHAR *destEnd, const char *src)
{
	TCHAR ch;
	// We assume that initially, destEnd - dest > 0.
	do
	{
		ch = *src++; // Casting to TCHAR
		*dest++ = ch; // This includes the null terminator if there's the room
	} while ((dest < destEnd) && ch);
	// These work in case dest == destEnd, and also if not
	--dest;
	*dest = _T('\0');
	return dest; // WARNING: This differs from strcpy!!
}

// Another function, similar to BE_Cross_strcasecmp, but:
// - The first argument is a TCHAR string.
// - It is assumed BOTH strings are ASCII strings.
static inline int BEL_Cross_tstr_to_cstr_ascii_casecmp(const TCHAR *s1, const char *s2)
{
	unsigned char uc1;
	unsigned char uc2;
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

#else

#define BEL_Cross_safeandfastctstringcopy BE_Cross_safeandfastcstringcopy
#define BEL_Cross_safeandfastctstringcopy_2strs BE_Cross_safeandfastcstringcopy_2strs
#define BEL_Cross_safeandfastctstringcopy_3strs BE_Cross_safeandfastcstringcopy_3strs
#define BEL_Cross_safeandfastctstringcopy_4strs BE_Cross_safeandfastcstringcopy_4strs
#define BEL_Cross_safeandfastcstringcopytoctstring BE_Cross_safeandfastcstringcopy
#define BEL_Cross_tstr_to_cstr_ascii_casecmp BE_Cross_strcasecmp

#endif

// Returns pointer to first occurrence of a non-ASCII char,
// or end-of-string NULL terminator in case it's an ASCII string.
static inline const TCHAR *BEL_Cross_tstr_find_nonascii_ptr(const TCHAR *s)
{
	for (; ((unsigned int)(*s) >= 32) && ((unsigned int)(*s) < 127); ++s)
		;
	return s;
}

#endif // BE_FILESYSTEM_STRING_OPS_H
