#!/bin/bash

# apt install clang
# brew install fd
# brew install swiftformat
# brew install uncrustify
# dart ships with the Flutter SDK

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT"

fd '\.swift$' project.ios/LowResRMX project.ios/Widgets project.ios/LowResRMXTests | grep -v zstd | xargs -n1 swiftformat --quiet --swift-version 5.9 --allman true  --indent tab

fd '\.[h|m|c]$' project.ios/LowResRMX project.ios/Widgets frontend.sdl backend.core test.harness | xargs uncrustify -q -c tool.dev/uncrustify.cfg --no-backup --replace -l OC

dart format project.flutter/client_app/lib project.flutter/client_app/test project.flutter/client_app/test_ffi project.flutter/client_app/integration_test project.flutter/core_plugin/lib project.flutter/core_plugin/android
