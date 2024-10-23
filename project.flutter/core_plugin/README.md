To generate `core_plugin_bindings_generated.dart`.

> Every times changes is made on `core_plugin.h` or `core_plugin.c`

    dart run ffigen --config ffigen.yaml --compiler-opts "-Isrc"

To generate `core.h` and `core.c`:

    cd lowresrmx/tool.dev
    bash merge.bash

To debug the `core_plugin`, I run the `client_app` on linux and launch the `"Attach core_plugin linux"` configuration from `./.vscode/launch.json` and select `lowresrmx`, type the root password and place a break point on the `src/...` code.
