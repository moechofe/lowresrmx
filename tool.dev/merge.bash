#!/bin/bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"
INCLUDE="$ROOT/backend.core"
DST="$ROOT/project.flutter/core_plugin/src"

H="$DST/core.h"
C="$DST/core.c"

included=()
list_include()
{
	local file="$1"
	local include
	local next
	while read next; do
		include=$(fd "\b$next$" "$INCLUDE")
		if [[ " ${included[@]} " =~ " ${include} " ]]; then
			:
		else
			# echo "Include found: $include"
			list_include "$include"
			included+=("$include")
		fi
	done < <(rg '#include "(.*\.h)"' -or '$1' "$file")
}
list_include "$DIR/merge.h"
echo "${included[*]}" | xargs cat > "$H"
sed -i "/#include \"\(.*\)\.h\"/d" "$H"

fd \.c$ "$INCLUDE" | xargs cat > "$C"
sed -i "s/#include \"\(.*\)\.h\"/#include \"core.h\"/g" "$C"

