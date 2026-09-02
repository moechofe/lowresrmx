#!/usr/bin/env bash
# Print shell assignments for a 1.0_NN release tag. Frontend-agnostic: the desktop jobs need
# only VERSION, an iOS job needs MARKETING_VERSION and BUILD_NUMBER.
# usage: version_from_tag.bash 1.0_68
#        eval "$(bash project.release/version_from_tag.bash 1.0_68)"
set -euo pipefail
v="$1"
echo "VERSION=$v"
echo "MARKETING_VERSION=${v%%_*}"
echo "BUILD_NUMBER=${v##*_}"
