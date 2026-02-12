#!/bin/env bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"
SOURCE_DIR="$ROOT/project.web/sources"
TEMP_DIR="$ROOT/package/"
PACKAGE_FILE="$ROOT/package.tar.xz"

mkdir -p "$TEMP_DIR"
rm -rf "$TEMP_DIR/*"

# # sitemap
# echo "generating: sitemap.xml"
# php "$ROOT/tool.dev/generate_sitemap_xml.php" \
# |& grep -vi "is already loaded in unknown on line 0" \
# | html-minifier-next \
# --collapse-whitespace \
# > "$TEMP_DIR/sitemap.xml"

# dynamic HTML files
for file in entry.html player.html sign-in.html; do
	echo "minifying: $file"
	html-minifier-next \
	--collapse-whitespace \
	--remove-comments \
	--minify-js true \
	--minify-css true \
	--output="$TEMP_DIR/$file" \
	"$SOURCE_DIR/$file"
done

# static HTML files
for file in list.html chat.html community.html documentation.html footer.html header.html help.html maintenance.html message.html privacy-policy.html setting.html share.html show.html terms-of-service.html about.html; do
	echo "minifying: $file"
	php "$SOURCE_DIR/$file" \
	|& grep -vi "is already loaded in unknown on line 0" \
	| html-minifier-next \
	--collapse-whitespace \
	--remove-comments \
	--minify-js true \
	--minify-css true \
	> "$TEMP_DIR/$file"
done \

# CSS files
for file in chat.css community.css documentation.css entry.css footer.css header.css help.css list.css setting.css share.css show.css sign-in.css player.css about.css; do
	echo "minifying: $file"
	php "$SOURCE_DIR/$file" \
	|& grep -v "is already loaded in Unknown on line 0" \
	| NODE_PATH=$HOMEBREW_PREFIX/lib/node_modules \
	node $DIR/minify_css.js \
	> "$TEMP_DIR/$file"
done

# JS files
for file in chat.js community.js entry.js help.js setting.js share.js show.js sign-in.js player.js about.js; do
	echo "compiling: $file"
	2>/dev/null \
	php "$SOURCE_DIR/$file" \
	|& grep -vi "is already loaded in unknown on line 0" \
	| google-closure-compiler \
	|& grep -vi "The compiler is waiting for input via stdin" \
	> "$TEMP_DIR/$file"
done

# PHP files
for file in admin.php comment.php common.php download.php entry.php favicon.ico index.php list.php logo-white.png logo-colored.png pico.min.css player.php private.php rank.php redis.php robots.txt score.php setting.php share.php sign.php token.php updrank.php upload.php markdown.php; do
	echo "copying: $file"
	cp "$SOURCE_DIR/$file" "$TEMP_DIR/$file"
done

# WASM file
for file in player.wasm; do
	echo "copying: $file"
	cp "$SOURCE_DIR/$file" "$TEMP_DIR/$file"
done

# Create the package
echo "packaging: $PACKAGE_FILE"
[[ -f "$PACKAGE_FILE" ]] && rm "$PACKAGE_FILE"
tar cfJ "$PACKAGE_FILE" --owner=0 --group=0 --no-same-owner --no-same-permissions --mode=0644 -C "$TEMP_DIR" \
index.php \
redis.php admin.php \
common.php \
markdown.php \
token.php sign.php sign-in.html \
upload.php download.php \
sign-in.css sign-in.js \
share.php list.php \
comment.php \
score.php rank.php updrank.php \
community.html community.js community.css \
show.html show.js show.css \
chat.html chat.js chat.css \
help.html help.js help.css \
entry.php entry.html entry.css entry.js \
setting.php setting.html setting.css setting.js \
share.html share.js share.css \
header.html header.css \
footer.html footer.css \
list.php list.html list.css \
favicon.ico logo-white.png logo-colored.png \
message.html privacy-policy.html terms-of-service.html \
pico.min.css \
about.html about.css about.js \
documentation.html documentation.css \
player.php player.css player.html player.js player.wasm \
robots.txt

# sitemap.xml
