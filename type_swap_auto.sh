#!/bin/bash
# Calls type_swap.sh (exact path is in $1) with multiple inputs and outputs.
# Following arguments are the files to update.
# bash shell is required.
#
# WARNING: THIS SCRIPT MAY FAIL TO WORK AS EXPECTED AND REPLACE MORE OR LESS
# THAN INTENDED (Hint: It does replace more than intended, check comments).
#
# THIS SCRIPT COMES WITH NO WARRANTY, EXPRESSED OR IMPLIED.

if [ "$2" != "deD34a" ]; then
	echo "Wrong argument passed, please read script's notice before using."
else
	SCRIPT=$1
	shift
	shift
	"$SCRIPT" boolean id0_boolean_t $@
	"$SCRIPT" "unsigned char" id0_unsigned_char_t $@
	"$SCRIPT" "signed char" id0_signed_char_t $@
	"$SCRIPT" "unsigned"'$\t'"char" id0_unsigned_char_t $@
	"$SCRIPT" "signed"'$\t'"char" id0_signed_char_t $@
	"$SCRIPT" "char" id0_char_t $@
	"$SCRIPT" "byte" id0_byte_t $@
	# (unsigned/signed) long int -> Never mentioned in Catacomb 3-D,
	# otherwise convert the "natural" way
	"$SCRIPT" "unsigned long" id0_unsigned_long_t $@
	"$SCRIPT" "signed long" id0_signed_long_t $@
	"$SCRIPT" "unsigned"'$\t'"long" id0_unsigned_long_t $@
	"$SCRIPT" "signed"'$\t'"long" id0_signed_long_t $@
	"$SCRIPT" "long" id0_long_t $@
	"$SCRIPT" "longword" id0_longword_t $@
	"$SCRIPT" "short" id0_short_t $@
	"$SCRIPT" "unsigned int" id0_unsigned_int_t $@
	"$SCRIPT" "signed int" id0_signed_int_t $@
	"$SCRIPT" "unsigned"'$\t'"int" id0_unsigned_int_t $@
	"$SCRIPT" "signed"'$\t'"int" id0_signed_int_t $@
	"$SCRIPT" "word" id0_word_t $@
	"$SCRIPT" "int" id0_int_t $@
	"$SCRIPT" "unsigned" id0_unsigned_t $@
	# floats and doubles (Cat3D) - keep as-is for now
	#
	# BONUS (Manually swapped in the past due to problems with compiler?)
	"$SCRIPT" "_seg" id0_seg $@
	"$SCRIPT" "far" id0_far $@
	"$SCRIPT" "huge" id0_huge $@
fi
