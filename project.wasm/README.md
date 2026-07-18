# Setup

Follow: https://emscripten.org/docs/getting_started/downloads.html#platform-notes-installation-instructions-sdk

    cd ~/Documents
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    git pull
    ./emsdk install latest
    ./emsdk activate latest

# Compile

    bash tool.dev/compile_wasm_player.bash
