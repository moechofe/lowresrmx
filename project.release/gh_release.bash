#!/usr/bin/env bash
# Create the GitHub Release for a tag with its notes and all of its assets in ONE call.
# The repo has release immutability enabled: assets are locked the moment a release is published,
# so they can never be added afterwards. `gh release create <tag> <files...>` is the only working
# shape - gh creates a draft, uploads the assets, then publishes. Never split this into
# `gh release create` + `gh release upload`, that is what immutability rejects.
# Notes and title are still mutable, and are set here at creation time anyway.
# usage: gh_release.bash <version> <notes-file> <asset>...
# Needs GH_TOKEN in the environment, or a logged-in gh.
set -euo pipefail
version="$1"; shift
notes="$1"; shift
if gh release view "$version" >/dev/null 2>&1; then
	echo "::error::release $version already exists; an immutable release cannot receive assets - delete the release (and re-tag) before re-running" >&2
	exit 1
fi
gh release create "$version" --title "LowResRMX $version" --notes-file "$notes" "$@"
