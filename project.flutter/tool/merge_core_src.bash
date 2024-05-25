#!/bin/bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/../..")"
SRC="$ROOT/backend.core"
DST="$ROOT/project.flutter/core_plugin/src"

cpp-merge -o "$DST/core.temp.h" -i "$SRC" "$DIR/merge_core.h"

cat "$DST/core.temp.h" \
| sed '/TargetConditionals.h/d' > "$DST/core.h"
rm "$DST/core.temp.h"

{
	echo '#include "core.h"' ;
	fd '\.c$' "$SRC" \
	| xargs cat \
	| sed '/^#include .*h"$/d';
} | >/dev/null tee "$DST/core.c"

