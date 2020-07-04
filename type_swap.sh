#!/bin/sh
# $1 is input, $2 is output (e.g., $1 is boolean and $2 is id0_boolean_t).
# The following arguments are the files to update.
#
# Ensure all characters are PROPERLY passed in the arguments,
# including e.g., a tab (if any). Example: "unsigned\tlong"
#
# WARNING: THIS SCRIPT MAY FAIL TO WORK AS EXPECTED AND REPLACE MORE OR LESS
# THAN INTENDED (Hint: It does replace more than intended, check comments).
#
# THIS SCRIPT COMES WITH NO WARRANTY, EXPRESSED OR IMPLIED.

if [ "$#" -lt 3 ]; then
	echo "Wrong amount of arguments passed, please read script's notice before using."
else
	SRC=$1
	DEST=$2
	shift
	shift
	sed -i ':a;N;$!ba;s/\n'"\b$SRC"'\r/\n'"$DEST"'\r/g' $@
	sed -i ':a;N;$!ba;s/\n'"\b$SRC"'\n/\n'"$DEST"'\n/g' $@
	sed -i ':a;N;$!ba;s/\n'"\b$SRC"'\t/\n'"$DEST"'\t/g' $@
	sed -i ':a;N;$!ba;s/\n'"\b$SRC"')/\n'"$DEST"')/g' $@
	sed -i ':a;N;$!ba;s/\n'"\b$SRC"',/\n'"$DEST"',/g' $@
	sed -i ':a;N;$!ba;s/\n'"\b$SRC"' /\n'"$DEST"' /g' $@
	sed -i ':a;N;$!ba;s/\t'"\b$SRC"'\r/\t'"$DEST"'\r/g' $@
	sed -i ':a;N;$!ba;s/\t'"\b$SRC"'\n/\t'"$DEST"'\n/g' $@
	sed -i ':a;N;$!ba;s/\t'"\b$SRC"'\t/\t'"$DEST"'\t/g' $@
	sed -i ':a;N;$!ba;s/\t'"\b$SRC"')/\t'"$DEST"')/g' $@
	sed -i ':a;N;$!ba;s/\t'"\b$SRC"',/\t'"$DEST"',/g' $@
	sed -i ':a;N;$!ba;s/\t'"\b$SRC"' /\t'"$DEST"' /g' $@
	sed -i ':a;N;$!ba;s/('"\b$SRC"'\r/('"$DEST"'\r/g' $@
	sed -i ':a;N;$!ba;s/('"\b$SRC"'\n/('"$DEST"'\n/g' $@
	sed -i ':a;N;$!ba;s/('"\b$SRC"'\t/('"$DEST"'\t/g' $@
	sed -i ':a;N;$!ba;s/('"\b$SRC"')/('"$DEST"')/g' $@
	sed -i ':a;N;$!ba;s/('"\b$SRC"',/('"$DEST"',/g' $@
	sed -i ':a;N;$!ba;s/('"\b$SRC"' /('"$DEST"' /g' $@
	sed -i ':a;N;$!ba;s/,'"\b$SRC"'\r/,'"$DEST"'\r/g' $@
	sed -i ':a;N;$!ba;s/,'"\b$SRC"'\n/,'"$DEST"'\n/g' $@
	sed -i ':a;N;$!ba;s/,'"\b$SRC"'\t/,'"$DEST"'\t/g' $@
	sed -i ':a;N;$!ba;s/,'"\b$SRC"')/,'"$DEST"')/g' $@
	sed -i ':a;N;$!ba;s/,'"\b$SRC"',/,'"$DEST"',/g' $@
	sed -i ':a;N;$!ba;s/,'"\b$SRC"' /,'"$DEST"' /g' $@
	sed -i ':a;N;$!ba;s/ '"\b$SRC"'\r/ '"$DEST"'\r/g' $@
	sed -i ':a;N;$!ba;s/ '"\b$SRC"'\n/ '"$DEST"'\n/g' $@
	sed -i ':a;N;$!ba;s/ '"\b$SRC"'\t/ '"$DEST"'\t/g' $@
	sed -i ':a;N;$!ba;s/ '"\b$SRC"')/ '"$DEST"')/g' $@
	sed -i ':a;N;$!ba;s/ '"\b$SRC"',/ '"$DEST"',/g' $@
	sed -i ':a;N;$!ba;s/ '"\b$SRC"' / '"$DEST"' /g' $@
fi
