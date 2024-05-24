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
