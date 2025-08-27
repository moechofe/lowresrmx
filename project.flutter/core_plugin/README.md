To generate `core_plugin_bindings_generated.dart`.

> Every times changes are made on `core_plugin.h` or `core_plugin.c`.

    cd project.flutter/core_plugin
    dart run ffigen --config ffigen.yaml --compiler-opts "-Isrc"

I didn't manage to use the source from `backend.core` and make the debugger working, so I decide to merge the whole code into two files. To generate `core.h` and `core.c` from `backend.core`:

    cd tool.dev
    bash merge_backcore_for_flutter.bash

This will generate the files: `core.c` and `core.h` in `project.cmake/core_plugin/src`

> Every times changes are made on `backend.core/*`.

To debug the `core_plugin`, I run the `client_app` on linux and launch the `"Attach core_plugin linux"` configuration from `./.vscode/launch.json` and select `lowresrmx`, type the root password and place a break point on the `src/...` code.
