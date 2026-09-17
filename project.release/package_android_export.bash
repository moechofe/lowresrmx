#!/usr/bin/env bash
# Zip the Android export template so a user can turn it into their own Google Play app.
# usage: package_android_export.bash <version> <out-dir>
# No Android SDK, Gradle or Java needed: the template is committed, prebuilt SDL3 .aar included.
set -euo pipefail
version="$1"
out="$(cd "$2" && pwd)"
cd "$(dirname "$0")/.."
name="LowResRMX-$version-android-export"
work="$(mktemp -d)"
mkdir -p "$work/$name"
# Tracked files at HEAD only: export.android also holds ~140 MB of Gradle caches and build
# output that must never ship, and packaging HEAD keeps the artifact reproducible from the tag.
git archive HEAD export.android | tar -x -C "$work/$name" --strip-components=1
# The wrapper must be runnable whatever mode git recorded for it.
chmod +x "$work/$name/android-project/gradlew"
printf '# LowResRMX %s - Android export template\n' "$version" \
	| cat - "$work/$name/README.md" > "$work/README.md"
mv "$work/README.md" "$work/$name/README.md"
cp LICENSE "$work/$name/LICENSE.txt"
rm -f "$out/$name.zip"
(cd "$work" && zip -qr "$out/$name.zip" "$name")
rm -rf "$work"
echo "wrote $out/$name.zip"
