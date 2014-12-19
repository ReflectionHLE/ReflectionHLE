#!/bin/bash
# Calls type_swap.sh (exact path is in $1) with multiple inputs and outputs.
# bash shell is required.
#
# WARNING: THIS SCRIPT MAY FAIL TO WORK AS EXPECTED AND REPLACE MORE OR LESS
# THAN INTENDED (Hint: It does replace more than intended, check comments).
#
# THIS SCRIPT COMES WITH NO WARRANTY, EXPRESSED OR IMPLIED.

if [ "$2" != "deD34a" ]; then
	echo "Wrong argument passed, please read script's notice before using."
else
	"$1" boolean id0_boolean_t
	"$1" "unsigned char" id0_unsigned_char_t
	"$1" "signed char" id0_signed_char_t
	"$1" "unsigned"'$\t'"char" id0_unsigned_char_t
	"$1" "signed"'$\t'"char" id0_signed_char_t
	"$1" "char" id0_char_t
	"$1" "byte" id0_byte_t
	# (unsigned/signed) long int -> Never mentioned in Catacomb 3-D,
	# otherwise convert the "natural" way
	"$1" "unsigned long" id0_unsigned_long_t
	"$1" "signed long" id0_signed_long_t
	"$1" "unsigned"'$\t'"long" id0_unsigned_long_t
	"$1" "signed"'$\t'"long" id0_signed_long_t
	"$1" "long" id0_long_t
	"$1" "longword" id0_longword_t
	"$1" "short" id0_short_t
	"$1" "unsigned int" id0_unsigned_int_t
	"$1" "signed int" id0_signed_int_t
	"$1" "unsigned"'$\t'"int" id0_unsigned_int_t
	"$1" "signed"'$\t'"int" id0_signed_int_t
	"$1" "word" id0_word_t
	"$1" "int" id0_int_t
	"$1" "unsigned" id0_unsigned_t
	# floats and doubles (Cat3D) - keep as-is for now
	#
	# BONUS (Manually swapped as the compiler probably complains otherwise)
	#"$1" "_seg" id0_seg
	#"$1" "far" id0_far
	#"$1" "huge" id0_huge
fi
