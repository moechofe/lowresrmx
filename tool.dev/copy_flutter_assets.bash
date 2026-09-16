#!/bin/env bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

SOURCE_DIR="$ROOT/asset.programs"
DEST_DIR="$ROOT/project.flutter/client_app/asset/programs"

mkdir -p "$DEST_DIR"

for file in "$SOURCE_DIR"/*; do
	if [ -f "$file" ]; then
		echo "copying: $(basename "$file")"
		cp "$file" "$DEST_DIR/"
	fi
done
