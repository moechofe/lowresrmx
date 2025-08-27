# Setup

Follow: https://emscripten.org/docs/getting_started/downloads.html#platform-notes-installation-instructions-sdk

    cd ~/Documents
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    git pull
    ./emsdk install 3.1.9
    ./emsdk activate 3.1.9

To compile:

    cd project.web
    source ~/Documents/emsdk/emsdk_env.sh
    emmake make

    php -S localhost:8088
