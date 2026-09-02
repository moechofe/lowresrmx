#!/usr/bin/env bash
# Print the CHANGELOG.md section of a version, or a fallback line when it is missing.
# usage: notes_from_changelog.bash 1.0_68
set -euo pipefail
cd "$(dirname "$0")/.."
notes="$(awk -v tag="### $1" '$0==tag{f=1;next} f&&/^### /{exit} f' CHANGELOG.md)"
if [ -z "$(printf '%s' "$notes" | tr -d '[:space:]')" ]; then
	echo "See CHANGELOG.md for changes."
else
	printf '%s\n' "$notes"
fi
