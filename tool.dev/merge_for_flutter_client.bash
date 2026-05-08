#!/bin/bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

bash "$DIR/merge_c_code.bash" "$DIR/flutter.h" "$ROOT/backend.core" "$ROOT/project.flutter/core_plugin/src/core"

