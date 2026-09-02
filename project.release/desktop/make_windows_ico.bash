#!/usr/bin/env bash
# Regenerate the committed LowResRMX.ico from the 512x512 app icon. Needs ImageMagick.
# The .ico is committed because CI has no ImageMagick and MSVC needs it at compile time.
set -euo pipefail
cd "$(dirname "$0")/../.."
convert asset.dev/icon-512.png \
	-define icon:auto-resize=256,128,64,48,32,16 \
	project.release/desktop/LowResRMX.ico
echo "wrote project.release/desktop/LowResRMX.ico"
