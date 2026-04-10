#!/bin/env bash

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT/project.wasm"
source ~/Documents/emsdk/emsdk_env.sh
emmake make
cp player.js player.wasm "$ROOT/project.web/sources/"

cat >"$ROOT/project.web/sources/player.js" <<'EOF'
var Module = {
	canvas: document.getElementById('canvas'),
	arguments: [`./${document.querySelector("body").dataset["pid"]}.rmx`],
	print: console.log.bind(console),
	printErr: console.warn.bind(console),
};
EOF

awk >>"$ROOT/project.web/sources/player.js" '{print} $0 ~ /^\(function\(\) \{$/ {print "  Module.canvas.addEventListener(\"contextmenu\", e => e.preventDefault());\n  Module.canvas.addEventListener(\"mousedown\", e => window.focus());"}' player.js
