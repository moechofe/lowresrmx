#!/bin/env bash

# Builds the headless harness runner for node. Unlike compile_wasm_player.bash this artifact is
# never copied into project.web/sources and never gets a Module preamble: it is not served to a
# browser, it is run as `node project.wasm/build/test_player.js test.suite/`.

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT/project.wasm"
source ~/Documents/emsdk/emsdk_env.sh
emmake make build/test_player.js
