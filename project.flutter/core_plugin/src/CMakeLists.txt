# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(core_plugin_library VERSION 0.0.1 LANGUAGES C)

add_library(core_plugin SHARED
  "core_plugin.c"
  "core.c"
)

set_target_properties(core_plugin PROPERTIES
  PUBLIC_HEADER core_plugin.h
  OUTPUT_NAME "core_plugin"
)

target_compile_definitions(core_plugin PUBLIC DART_SHARED_LIB)

# Pixel byte order the engine must emit. core_plugin.c forces the ANativeWindow to
# WINDOW_FORMAT_RGBA_8888, and the Dart fallback decodes as rgba8888, so every target built
# through CMake wants R,G,B,A. iOS is built from the podspec instead and sets ABGR=0 there,
# because its CVPixelBuffer is kCVPixelFormatType_32BGRA.
target_compile_definitions(core_plugin PUBLIC ABGR=1)

if (ANDROID)
  target_link_libraries(core_plugin android log)
endif()
