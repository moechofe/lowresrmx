#!/bin/bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

[[ "$#" -ne 3 ]] && { echo "Usage: INPUT INCLUDES OUTPUT" ; exit 1; }
INPUT="$1"
INCLUDES="$2"
OUTPUT="$3"

# echo "INPUT: $INPUT"
# echo "INCLUDES: $INCLUDES"
# echo "OUTPUT: $OUTPUT"

H="$OUTPUT.h"
C="$OUTPUT.c"

included=()
list_include()
{
	local file="$1"
	local include
	local next
	# echo "---- $file"
	while read next; do
		# echo ".... $next"
		include=$(fd "\b$next$" $INCLUDES)
		if [[ " ${included[@]} " =~ " ${include} " ]]; then
			:
		else
			# echo "Include found: $include"
			list_include "$include"
			included+=("$include")
		fi
	done < <(rg '#include "(.*\.h)"' -or '$1' "$file")
}
list_include "$INPUT"
echo "${included[*]}" | xargs cat > "$H"
sed -i "/#include \"\(.*\)\.h\"/d" "$H"

fd \.c$ $INCLUDES | xargs cat > "$C"
sed -i "/#include \"\(.*\)\.h\"/d" "$C"
sed -i '1s/^/#include "'${H##*/}'"\n/' "$C"
