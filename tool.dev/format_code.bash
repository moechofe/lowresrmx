#!/bin/bash

# apt install clang
# brew install clang-format
# brew install fd
# brew install swiftformat
# brew install uncrustify

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT"

# fd '\.[h|c]$' frontend.sdl backend.core | xargs clang-format --style="{\
# BasedOnStyle: Microsoft, \
# IndentWidth: 4, \
# TabWidth: 4, \
# UseTab: Always, \
# SpaceBeforeParens: Never, \
# BinPackArguments: false, \
# ContinuationIndentWidth: 0, \
# AlignAfterOpenBracket: DontAlign, \
# AlignOperands: DontAlign, \
# AlignEscapedNewlines: DontAlign, \
# AlignTrailingComments: {Kind: Never, OverEmptyLines: 0}, \
# AlignConsecutiveAssignments: None, \
# AlignConsecutiveDeclarations: None, \
# AlignConsecutiveMacros: None, \
# AlignConsecutiveBitFields: None, \
# AlignArrayOfStructures: None \
# }" -i

fd '\.swift$' project.ios/LowResRMX project.ios/Widgets | grep -v zstd | xargs -n1 swiftformat --quiet --swift-version 5.9 --allman true  --indent tab

fd '\.[h|m|c]$' project.ios/LowResRMX project.ios/Widgets frontend.sdl backend.core | xargs uncrustify -q -c tool.dev/uncrustify.cfg --no-backup --replace -l OC
