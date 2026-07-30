#!/bin/env bash

DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"
SOURCE_DIR="$ROOT/project.web/sources"
TEMP_DIR="$ROOT/package/"
PACKAGE_FILE="$ROOT/package.tar.xz"

missing=0

for cmd in readlink mkdir rm cp tar grep rg php node html-minifier-next google-closure-compiler xargs; do
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
rm -rf "${TEMP_DIR:?}"/*

DYN_HTML=(entry.html player.html sign-in.html)
STATIC_HTML=(list.html chat.html community.html documentation.html footer.html header.html help.html maintenance.html message.html privacy-policy.html setting.html share.html show.html terms-of-service.html about.html)
CSS_FILES=(chat.css community.css documentation.css entry.css footer.css header.css help.css list.css setting.css share.css show.css sign-in.css player.css about.css)
JS_FILES=(chat.js community.js entry.js help.js setting.js share.js show.js sign-in.js about.js)
PHP_FILES=(admin.php comment.php common.php download.php entry.php favicon.ico index.php list.php logo-white.png logo-colored.png pico.min.css player.php rank.php redis.php robots.txt score.php setting.php share.php sign.php token.php updrank.php upload.php markdown.php image.php notification.php reaction.php discord.php)
WASM_FILES=(player.wasm player.js)
COPY_FILES=(emoji-picker-element.js emoji-data.json 04.webp 05.webp 06.webp medium-zoom.min.js)

GENERATED=()
GENERATED+=("${DYN_HTML[@]}")
for file in "${STATIC_HTML[@]}"; do
	[[ "$file" == "message.html" ]] || GENERATED+=("$file")
done
GENERATED+=("${CSS_FILES[@]}")
GENERATED+=("${JS_FILES[@]}")

run_job() {
	local type="$1" file="$2"
	case "$type" in
	dyn)
		echo "minifying: $file"
		html-minifier-next \
		--collapse-whitespace \
		--remove-comments \
		--minify-js true \
		--minify-css true \
		--output="$TEMP_DIR/$file" \
		"$SOURCE_DIR/$file"
		;;
	html)
		echo "minifying: $file"
		php "$SOURCE_DIR/$file" \
		|& grep -vi "is already loaded in unknown on line 0" \
		| html-minifier-next \
		--collapse-whitespace \
		--remove-comments \
		--minify-js true \
		--minify-css true \
		> "$TEMP_DIR/$file"
		;;
	css)
		echo "minifying: $file"
		php "$SOURCE_DIR/$file" \
		|& grep -v "is already loaded in Unknown on line 0" \
		| NODE_PATH=$HOMEBREW_PREFIX/lib/node_modules \
		node $DIR/minify_css.js \
		> "$TEMP_DIR/$file"
		;;
	js)
		echo "compiling: $file"
		2>/dev/null \
		php "$SOURCE_DIR/$file" \
		|& grep -vi "is already loaded in unknown on line 0" \
		| google-closure-compiler \
		|& grep -vi "The compiler is waiting for input via stdin" \
		> "$TEMP_DIR/$file"
		;;
	copy)
		echo "copying: $file"
		cp "$SOURCE_DIR/$file" "$TEMP_DIR/$file"
		;;
	esac
}
export -f run_job
export SOURCE_DIR TEMP_DIR DIR HOMEBREW_PREFIX

# Detect core count portably (Linux + macOS).
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

{
	for file in "${DYN_HTML[@]}"; do echo "dyn $file"; done
	for file in "${STATIC_HTML[@]}"; do echo "html $file"; done
	for file in "${CSS_FILES[@]}"; do echo "css $file"; done
	for file in "${JS_FILES[@]}"; do echo "js $file"; done
	for file in "${PHP_FILES[@]}"; do echo "copy $file"; done
	for file in "${WASM_FILES[@]}"; do echo "copy $file"; done
	for file in "${COPY_FILES[@]}"; do echo "copy $file"; done
} | xargs -P "$JOBS" -L1 bash -c 'run_job "$@"' _
xargs_status=$?
if [[ "$xargs_status" -ne 0 ]]; then
	echo "warning: at least one parallel job exited non-zero (status $xargs_status); the verify step below will fail the build if output is bad" >&2
fi

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
# TODO: can I use the previous list
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
04.webp 05.webp 06.webp \
documentation.html documentation.css \
player.php player.css player.html player.js player.wasm \
image.php \
notification.php \
reaction.php emoji-picker-element.js emoji-data.json \
medium-zoom.min.js \
discord.php \
robots.txt

