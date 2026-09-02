#!/usr/bin/env bash
# Create the GitHub Release for a tag if it does not exist yet, then upload assets to it.
# Idempotent on purpose: a second per-platform workflow can add its own assets to the same tag.
# usage: gh_release.bash <version> <notes-file> <asset>...
# Needs GH_TOKEN in the environment, or a logged-in gh.
set -euo pipefail
version="$1"; shift
notes="$1"; shift
if ! gh release view "$version" >/dev/null 2>&1; then
	gh release create "$version" --title "LowResRMX $version" --notes-file "$notes"
fi
gh release upload "$version" --clobber "$@"
