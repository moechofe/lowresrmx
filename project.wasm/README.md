# Setup

Follow: https://emscripten.org/docs/getting_started/downloads.html#platform-notes-installation-instructions-sdk

    cd ~/Documents
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    git pull
    ./emsdk install latest
    ./emsdk activate latest

To compile:

    cd project.wasm
    source ~/Documents/emsdk/emsdk_env.sh
    emmake make
    cp player.js player.wasm ../project.web/sources/

Add this to `player.js` before the first `(function() {`

```js
var Module = {
  canvas: document.getElementById('canvas'),
  arguments: [window.location.href.replace(/\.(player)$/,'.rmx')],
  print: console.log.bind(console),
  printErr: console.warn.bind(console),
};
```

Add this to `player.js` after `(function() {`

```js
  Module.canvas.addEventListener('contextmenu', e => e.preventDefault());
  Module.canvas.addEventListener('mousedown', e => window.focus());
```
