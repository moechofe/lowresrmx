#!/bin/env bash

ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

cd "$ROOT/project.wasm"
source ~/Documents/emsdk/emsdk_env.sh
emmake make
cp player.js player.wasm "$ROOT/project.web/sources/"

cat >"$ROOT/project.web/sources/player.js" <<'EOF'
var Module = {};
Module.canvas = document.getElementById('canvas');
Module.arguments = [`./${document.querySelector("body").dataset["pid"]}.rmx`];
Module.print = console.log.bind(console);
Module.printErr = console.warn.bind(console);
Module.onAbort = function(what){
  const screen=document.querySelector('.screen');
  if(screen){
    screen.innerHTML =
      '<div class="unsupported">' +
      '<p class="unsupported-title">Can’t run this program</p>' +
      '<p>Your browser can’t run this program. Please update it to a newer version — on iPhone or iPad that means iOS 16.4 or later.</p>' +
      '</div>';
  }
};
EOF

awk >>"$ROOT/project.web/sources/player.js" '{print} $0 ~ /^\(function\(\) \{$/ {print "  Module.canvas.addEventListener(\"contextmenu\", e => e.preventDefault());\n  Module.canvas.addEventListener(\"mousedown\", e => window.focus());"}' player.js
