#!/bin/env bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"


cd "$DIR"
2>/dev/null node convert_manual.js
