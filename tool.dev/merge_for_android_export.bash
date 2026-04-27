#!/bin/bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"
INCLUDE="$ROOT/backend.core $ROOT/frontend.sdl"
DST="$ROOT/export.android/"

bash "$DIR/merge_c_code.bash" "$DIR/android.h" "$ROOT/backend.core $ROOT/frontend.sdl" "$ROOT/export.android/src/all"

