#!/usr/bin/env bash
# Build a macOS .icns from a square PNG. macOS only: needs sips and iconutil.
# usage: make_macos_icns.bash <source.png> <out.icns>
# The source is 512x512, so icon_512x512@2x (1024) is omitted rather than upscaled;
# macOS accepts an iconset without it.
set -euo pipefail
src="$1"
out="$2"
tmp="$(mktemp -d)"
iconset="$tmp/LowResRMX.iconset"
mkdir -p "$iconset"
for spec in \
	"16 icon_16x16" "32 icon_16x16@2x" \
	"32 icon_32x32" "64 icon_32x32@2x" \
	"128 icon_128x128" "256 icon_128x128@2x" \
	"256 icon_256x256" "512 icon_256x256@2x" \
	"512 icon_512x512"
do
	size="${spec%% *}"
	name="${spec##* }"
	sips -z "$size" "$size" "$src" --out "$iconset/$name.png" >/dev/null
done
iconutil -c icns "$iconset" -o "$out"
rm -rf "$tmp"
