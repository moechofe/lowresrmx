## Setup for Ubuntu 24.04.3 LTS

1. Checkout the [SDL2-2.30.12](https://github.com/libsdl-org/SDL/tree/release-2.30.12) github repository somewhere:

    ```bash
    cd $HOME/Documents
    git clone --branch release-2.30.12 https://github.com/libsdl-org/SDL.git SDL2-2.30.12
    ```

2. Create a [Symbolic link](https://en.wikipedia.org/wiki/Symbolic_link) into the project:

    ```bash
    cd LowResRMX/project.cmake
    ln -s $HOME/Documents/SDL2-2.30.12 SDL2
    ```

### Using [Visual Studio Code](https://code.visualstudio.com/) and [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools).

The `CMakeLists.txt` file will look inside this folder using the variable `${SDL}`.

1. Edit `.vscode/settings.json` for the :

    ```json
    "cmake.configureArgs": [
        "-DSDL=./SDL2"
    ],
    ```

2. Run `CMake: Configure` from the Command Palette.

3. Run `CMake: Debug`.

### Using a terminal

```bash
cd LowResRMX/project.cmake
mkdir build && cd build
cmake -DSDL="./SDL2" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
./LowResRMX
```

## Note:

I added a patch to `SDL_pipewire.c` but I forgot which one. It was a simple cast.
