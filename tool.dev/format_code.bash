#!/bin/bash

# apt install clang
# brew install fd
# brew install swiftformat
# brew install uncrustify

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT"

fd '\.swift$' project.ios/LowResRMX project.ios/Widgets | grep -v zstd | xargs -n1 swiftformat --quiet --swift-version 5.9 --allman true  --indent tab

fd '\.[h|m|c]$' project.ios/LowResRMX project.ios/Widgets frontend.sdl backend.core | xargs uncrustify -q -c tool.dev/uncrustify.cfg --no-backup --replace -l OC
