#!/bin/env bash

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
SOURCE_DIR="$SCRIPT_DIR/../sources"
PACKAGE_FILE="$SCRIPT_DIR/package.tar.xz"

rm "$PACKAGE_FILE"
tar cfJ "$PACKAGE_FILE" --owner=0 --group=0 --no-same-owner --no-same-permissions -C "$SOURCE_DIR" \
index.php \
redis.php \
common.php common.js \
token.php sign.php sign-in.html \
upload.php download.php \
player.php player.js player.wasm \
share.php list.php delete.php \
publish.php post.php comment.php \
score.php rank.php updrank.php \
community.html community.js community.css \
show.html show.js show.css \
chat.html chat.js chat.css \
help.html help.js help.css \
entry.php entry.html entry.css entry.js \
share.html share.js share.css \
header.html header.js header.css \
list.php list.html list.js list.css \
favicon.ico logo.png \
message.html privacy-policy.html terms-of-service.html \
pico.min.css
