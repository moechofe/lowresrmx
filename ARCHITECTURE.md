> Working with an AI agent? Start at 📝 `AGENTS.md`, which routes to the deep-dive docs in 📂 `doc.agent`.

- 📂 `asset.dev`

    Assets source.
    Contains the manual source 📝 `manual.md`.

- 📂 `asset.manual`

    Manual included in the iOS app.

- 📂 `asset.programs`

    Programs included in the iOS app.

- 📂 `backend.core`

    The backend engine containing the interpreter.

- 📂 `doc.agent`

    Deep-dive documentation for AI agents, indexed by 📝 `AGENTS.md`.

- 📂 `export.android/android-project`

    Android Studio template project for exported user's program.
    Contains a merged version of the backend and SDL frontend at 📝 `all.c` and 📝 `all.h` (with probably to much code in it).

- 📂 `frontend.sdl`

    SDL frontend for the desktop app, the WASM player and Android export player.

- 📂 `plugin.vscode`

    Syntax highlight extension for Visual Studio Code.

- 📂 `project.cmake`

    CMake project to build the desktop app.

- 📂 `project.flutter`

    Flutter project for the Android app.

- 📂 `project.flutter/client_app`

    Flutter client app.
    Contains a program list, a code editor, the player, the documentation.

- 📂 `project.flutter/core_plugin`

    Flutter FFI plugin to interop with the backend.
    Contains a merged version of the backend at 📝 `src/core.c` and 📝 `src/core.h`.

- 📂 `project.flutter/re_editor`

    A custom old version of [re_editor](https://pub.dev/packages/re_editor).

- 📂 `project.ios/LowResRMX`

    iOS client app.
    Contains a program list, a code editor, the player, the documentation, the official editor and some examples.
    Can share program to the website and run program from the website (and save them to the device).
    Program are stored through iCloud.

- 📂 `project.ios/LowResRMX.xcodeproj`

    Xcode project for the iOS app.

- 📂 `project.release`

    Release packaging and publishing for the desktop app (CPack: zip, NSIS, DMG) and the Android export template.

- 📂 `project.wasm`

    Emscripten project for the WASM player.

- 📂 `tool.dev`

    Developer scripts.
