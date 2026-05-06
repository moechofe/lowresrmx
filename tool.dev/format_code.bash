#!/bin/bash

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT"

fd '\.[h|c]$' frontend.sdl | xargs clang-format --style="{BasedOnStyle: Microsoft, IndentWidth: 4, TabWidth: 4, UseTab: Always, LineEnding: LF}" -i
