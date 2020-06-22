#ifndef BE_CROSS_STDINT_H
#define BE_CROSS_STDINT_H

#include <stdint.h>

// Drop-in replacements for integer types used in 16-bit real mode DOS programs
// written in the 90s. For example, be0_int_t can be used instead of int if the
// latter were originally translated to the equivalent of stdint.h's int16_t.

typedef char be0_char_t; // If be0_schar_t/be0_uchar_t doesn't make sense
typedef int8_t be0_schar_t; // If "char" was originally assumed to be signed
typedef uint8_t be0_uchar_t; // If "char" was originally assumed to be unsigned
typedef int8_t be0_signed_char_t;
typedef int8_t be0_char_signed_t;
typedef uint8_t be0_unsigned_char_t;
typedef int16_t be0_short_t;
typedef int16_t be0_int_t;
typedef uint16_t be0_unsigned_t;
typedef uint16_t be0_unsigned_int_t;
typedef int32_t be0_long_t;
typedef int32_t be0_signed_long_t;
typedef uint32_t be0_unsigned_long_t;

// These replacement macros exist for documentation for most

#define be0_seg
#define be0_far
#define be0_huge

#endif
