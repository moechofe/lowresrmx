#!/bin/bash

# apt install clang
# brew install fd
# brew install swiftformat

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT"

fd '\.[h|c]$' frontend.sdl backend.core | xargs clang-format --style="{\
BasedOnStyle: Microsoft, \
IndentWidth: 4, \
TabWidth: 4, \
UseTab: Always, \
SpaceBeforeParens: Never, \
BinPackArguments: false, \
AlignAfterOpenBracket: false, \
ContinuationIndentWidth: 0 \
}" -i

fd '\.swift$' project.ios | xargs -n1 swiftformat --quiet --swift-version 5.9 --allman true  --indent tab
