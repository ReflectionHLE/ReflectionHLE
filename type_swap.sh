#!/bin/sh
# $1 is input, $2 is output (e.g., $1 is boolean and $2 is id0_boolean_t)
#
# Ensure all characters are PROPERLY passed in the arguments,
# including e.g., a tab (if any). Example: "unsigned\tlong"
#
# WARNING: THIS SCRIPT MAY FAIL TO WORK AS EXPECTED AND REPLACE MORE OR LESS
# THAN INTENDED (Hint: It does replace more than intended, check comments).
#
# THIS SCRIPT COMES WITH NO WARRANTY, EXPRESSED OR IMPLIED.

if [ "$#" -ne 2 ]; then
	echo "Wrong amount of arguments passed, please read script's notice before using."
else
	sed -i ':a;N;$!ba;s/\n'"$1"'\r/\n'"$2"'\r/g' *
	sed -i ':a;N;$!ba;s/\n'"$1"'\n/\n'"$2"'\n/g' *
	sed -i ':a;N;$!ba;s/\n'"$1"'\t/\n'"$2"'\t/g' *
	sed -i ':a;N;$!ba;s/\n'"$1"')/\n'"$2"')/g' *
	sed -i ':a;N;$!ba;s/\n'"$1"',/\n'"$2"',/g' *
	sed -i ':a;N;$!ba;s/\n'"$1"' /\n'"$2"' /g' *
	sed -i ':a;N;$!ba;s/\t'"$1"'\r/\t'"$2"'\r/g' *
	sed -i ':a;N;$!ba;s/\t'"$1"'\n/\t'"$2"'\n/g' *
	sed -i ':a;N;$!ba;s/\t'"$1"'\t/\t'"$2"'\t/g' *
	sed -i ':a;N;$!ba;s/\t'"$1"')/\t'"$2"')/g' *
	sed -i ':a;N;$!ba;s/\t'"$1"',/\t'"$2"',/g' *
	sed -i ':a;N;$!ba;s/\t'"$1"' /\t'"$2"' /g' *
	sed -i ':a;N;$!ba;s/('"$1"'\r/('"$2"'\r/g' *
	sed -i ':a;N;$!ba;s/('"$1"'\n/('"$2"'\n/g' *
	sed -i ':a;N;$!ba;s/('"$1"'\t/('"$2"'\t/g' *
	sed -i ':a;N;$!ba;s/('"$1"')/('"$2"')/g' *
	sed -i ':a;N;$!ba;s/('"$1"',/('"$2"',/g' *
	sed -i ':a;N;$!ba;s/('"$1"' /('"$2"' /g' *
	sed -i ':a;N;$!ba;s/,'"$1"'\r/,'"$2"'\r/g' *
	sed -i ':a;N;$!ba;s/,'"$1"'\n/,'"$2"'\n/g' *
	sed -i ':a;N;$!ba;s/,'"$1"'\t/,'"$2"'\t/g' *
	sed -i ':a;N;$!ba;s/,'"$1"')/,'"$2"')/g' *
	sed -i ':a;N;$!ba;s/,'"$1"',/,'"$2"',/g' *
	sed -i ':a;N;$!ba;s/,'"$1"' /,'"$2"' /g' *
	sed -i ':a;N;$!ba;s/ '"$1"'\r/ '"$2"'\r/g' *
	sed -i ':a;N;$!ba;s/ '"$1"'\n/ '"$2"'\n/g' *
	sed -i ':a;N;$!ba;s/ '"$1"'\t/ '"$2"'\t/g' *
	sed -i ':a;N;$!ba;s/ '"$1"')/ '"$2"')/g' *
	sed -i ':a;N;$!ba;s/ '"$1"',/ '"$2"',/g' *
	sed -i ':a;N;$!ba;s/ '"$1"' / '"$2"' /g' *
fi
