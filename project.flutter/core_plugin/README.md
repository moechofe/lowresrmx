### Setup

#### Ubuntu

    dart run ffigen --config ffigen.yaml --compiler-opts "-Isrc"

    # dart run ffigen --config ffigen.yaml --compiler-opts "-Isrc/core/machine -Isrc/core/overlay -Isrc/core/libraries -Isrc/core/interpreter -Isrc/core/datamanager -Isrc/core/ -Isrc/core/accessories"

To generate the `core.h`:

    npm install -g cpp-merge

    cd core
    cpp-merge -o ../core.h -i . merge.h
