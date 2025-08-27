
- 📂 `asset.dev`

    Contains some original assets from LowResNX, the bitmap fonts included in the core, the source for the documentation.

- 📂 `asset.manual`

    The exported manual included in the mobile apps.

- 📂 `asset.programs`

    Programs to be included in the mobile apps.

- 📂 `backend.core`

    The backend engine containing the intepreter.

- 📂 `frontend.sdl`

    A frontend using SDL used to create a desktop app.

- 📂 `plugin.vscode`

    A syntax highlight  for Visual Studio Code.

- 📂 `project.cmake`

    A CMake project to build the desktop app.

    #### Tested environment

    1. Ubuntu 💚

        - Ubuntu 22.04.2 LTS
        - Clang 14.0
        - Cmake 3.22

    2. WSL on Windows 10 🤦‍♂️

        - Windows 10.0.19045
        - WSL 1.2.5

    3. Windows 10 💚

        - Windows 10 Home 22H2 19045
        - Visual Studio Community 2019 x86
        - Cmake 3.27

    4. EndeavourOS

        - EndeavourOS 2025.03.19
        - Gcc 14.2.1
        - Cmake 4.0.1

        ```
        sudo pacman -S dbus
        ```

        I added a patch to `SDL_pipewire.c` but I forgot which one. It was a simple cast.

- 📂 `project.flutter` 💗

    The new mobile app in developement.

    To build it, you'll need a Android device with [on-device developer options enabled](https://developer.android.com/studio/debug/dev-options), and [Flutter](https://docs.flutter.dev/get-started/install).

    Navigate to the 📂 `project.flutter/client_app` folder with a terminal and type:

    ```
    flutter devices
    ```

    And copy the id of your connected (mobile) device.

    ```
    flutter run -d <device-id>
    ```

    #### Tested environment

    1. Ubuntu 💚

        - Flutter 3.22.1
        - Visual Studio Code 1.89.1
            - Flutter extension v3.88.0

    2. Windows 10 💚

        - Flutter 3.22.1
        - Visual Studio Code 1.89.1
            - Flutter extension v3.88.0

- 📂 `project.ios` 💗

    A mobile app modified from the original LowResNX app by "Timo".

    It's actually usable.

- 📂 `project.web`

    A makefile project to build a web player.

- 📂 `test.programs`

    A collection of programs to test the features.

- 📂 `tool.dev`

    Original script to export image as characters binary format.

- 📂 `web.community`

    A project for a community website
