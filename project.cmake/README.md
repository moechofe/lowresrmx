## Setup

Tested with [`SDL2-2.30.0`](https://github.com/libsdl-org/SDL/tree/release-2.30.0).

Checkout the SDL github repository into a subdirectory📁 in this folder:

    `project.cmake/SDL2-2.30.0`

The `CMakeLists.txt` file📄 will look inside this folder using the variable `${SDL}`, so you'll have to pass it to the cmake configure step: `-DSDL=./SDL2-2.30.0`.

On VSCode, this settings appear in `.vscode/settings.json` for the [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools):

```json
"cmake.configureArgs": [
    "-DSDL=./SDL2-2.30.0"
],
```


