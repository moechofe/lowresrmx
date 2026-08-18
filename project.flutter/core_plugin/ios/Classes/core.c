// Relative import to be able to reuse the C sources.
// See the comment in ../core_plugin.podspec for more information.
// Mirrors the source list in ../../src/CMakeLists.txt, which compiles both
// core_plugin.c and core.c — without this forwarder the whole engine is
// missing at link time on iOS.
#include "../../src/core.c"
