#!/bin/env bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"
SOURCE_DIR="$ROOT/project.web/sources"
TEMP_DIR="$ROOT/package/"
PACKAGE_FILE="$ROOT/package.tar.xz"

missing=0

for cmd in readlink mkdir rm cp tar grep rg php node html-minifier-next google-closure-compiler; do
	if ! command -v "$cmd" >/dev/null 2>&1; then
		echo "error: required command not found: $cmd" >&2
		missing=1
	fi
done

# if [[ -z "${HOMEBREW_PREFIX:-}" ]]; then
# 	echo "error: HOMEBREW_PREFIX is not set (needed to locate node_modules for minify_css.js)" >&2
# 	missing=1
# elif [[ ! -d "$HOMEBREW_PREFIX/lib/node_modules" ]]; then
# 	echo "error: node modules directory not found: $HOMEBREW_PREFIX/lib/node_modules" >&2
# 	missing=1
# fi

if [[ ! -f "$DIR/minify_css.js" ]]; then
	echo "error: helper script not found: $DIR/minify_css.js" >&2
	missing=1
fi
if [[ ! -d "$SOURCE_DIR" ]]; then
	echo "error: source directory not found: $SOURCE_DIR" >&2
	missing=1
fi

if [[ "$missing" -ne 0 ]]; then
	echo "aborting: unmet requirements (see errors above)" >&2
	exit 1
fi

mkdir -p "$TEMP_DIR"
rm -rf "$TEMP_DIR/*"

# Files produced by php/minifier/compiler passes below. Only these are scanned
# for error markers at the end (copied assets like player.js legitimately
# contain words such as "warning" and "undefined").
GENERATED=()

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
	GENERATED+=("$file")
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
	[[ "$file" == "message.html" ]] || GENERATED+=("$file")
done \

# CSS files
for file in chat.css community.css documentation.css entry.css footer.css header.css help.css list.css setting.css share.css show.css sign-in.css player.css about.css; do
	echo "minifying: $file"
	php "$SOURCE_DIR/$file" \
	|& grep -v "is already loaded in Unknown on line 0" \
	| NODE_PATH=$HOMEBREW_PREFIX/lib/node_modules \
	node $DIR/minify_css.js \
	> "$TEMP_DIR/$file"
	GENERATED+=("$file")
done

# JS files
for file in chat.js community.js entry.js help.js setting.js share.js show.js sign-in.js about.js; do
	echo "compiling: $file"
	2>/dev/null \
	php "$SOURCE_DIR/$file" \
	|& grep -vi "is already loaded in unknown on line 0" \
	| google-closure-compiler \
	|& grep -vi "The compiler is waiting for input via stdin" \
	> "$TEMP_DIR/$file"
	GENERATED+=("$file")
done

# PHP files
for file in admin.php comment.php common.php download.php entry.php favicon.ico index.php list.php logo-white.png logo-colored.png pico.min.css player.php rank.php redis.php robots.txt score.php setting.php share.php sign.php token.php updrank.php upload.php markdown.php webhook.php image.php; do
	echo "copying: $file"
	cp "$SOURCE_DIR/$file" "$TEMP_DIR/$file"
done

# WASM/JS file
for file in player.wasm player.js; do
	echo "copying: $file"
	cp "$SOURCE_DIR/$file" "$TEMP_DIR/$file"
done

# Verify generated files: intercept error markers leaked by php / the
# minifier / the closure compiler into the output before packaging.
echo "verifying: ${#GENERATED[@]} generated files"
ERROR_MARKERS='PHP (Warning|Notice|Deprecated|Fatal error|Parse error)|(Fatal|Parse) error:|Uncaught |Stack trace|Xdebug|command not found|\[JSC_|[1-9][0-9]* error\(s\)| in .* on line [0-9]+'
found_errors=0
for file in "${GENERATED[@]}"; do
	if [[ ! -s "$TEMP_DIR/$file" ]]; then
		echo "error: generated file is empty (a pass likely failed): $file" >&2
		found_errors=1
	elif rg -n -i -e "$ERROR_MARKERS" "$TEMP_DIR/$file"; then
		echo "error: error marker found in generated file: $file" >&2
		found_errors=1
	fi
done

[[ "$found_errors" -ne 0 ]] && exit 1

# Create the package
echo "packaging: $PACKAGE_FILE"
[[ -f "$PACKAGE_FILE" ]] && rm "$PACKAGE_FILE"
tar cfJ "$PACKAGE_FILE" --owner=0 --group=0 --no-same-owner --no-same-permissions --mode=0644 -C "$TEMP_DIR" \
index.php \
redis.php admin.php \
common.php \
markdown.php webhook.php \
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
image.php \
robots.txt

# sitemap.xml
