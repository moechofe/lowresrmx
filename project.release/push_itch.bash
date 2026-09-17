#!/usr/bin/env bash
# Push the packaged desktop zips to itch.io. usage: push_itch.bash <version> <dist-dir>
# Needs BUTLER_API_KEY in the environment, or a previous "butler login".
# butler is downloaded (linux-amd64) only when it is not already on PATH.
# The zips are unpacked before pushing so itch.io can diff/patch builds; the ZIP generator
# stores unix permissions, so the executable bit and the .app structure survive the round-trip.
set -euo pipefail
version="$1"
dist="$(cd "$2" && pwd)"
cd "$(dirname "$0")/.."
. project.release/release.conf

if [ -z "$ITCH_TARGET" ]; then
	echo "release.conf: ITCH_TARGET is empty, skipping itch.io push"
	exit 0
fi

butler="$(command -v butler || true)"
if [ -z "$butler" ]; then
	tmp="$(mktemp -d)"
	curl -sL -o "$tmp/butler.zip" https://broth.itch.zone/butler/linux-amd64/LATEST/archive/default
	unzip -q "$tmp/butler.zip" -d "$tmp"
	chmod +x "$tmp/butler"
	butler="$tmp/butler"
fi

for pair in $ITCH_CHANNELS; do
	artifact="${pair%%:*}"
	channel="${pair##*:}"
	work="$(mktemp -d)"
	unzip -q "$dist/LowResRMX-$version-$artifact.zip" -d "$work"
	"$butler" push "$work/LowResRMX-$version-$artifact" \
		"$ITCH_TARGET:$channel" --userversion "$version"
	rm -rf "$work"
done
