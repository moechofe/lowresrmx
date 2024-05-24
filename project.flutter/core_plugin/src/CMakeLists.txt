cmake_minimum_required(VERSION 3.10)
project(core_plugin_library VERSION 0.0.1 LANGUAGES C)

# file(GLOB CORE_SOURCES "core/*.c" "core/*/*.c")
# include_directories("core" "core/accessories" "core/datamanager" "core/interpreter" "core/libraries" "core/machine" "core/overlay")

add_library(core_plugin SHARED
	"core_plugin.c"
	"core.c"
)

set_target_properties(core_plugin PROPERTIES
  PUBLIC_HEADER core_plugin.h
  OUTPUT_NAME "core_plugin"
)

target_compile_definitions(core_plugin PUBLIC DART_SHARED_LIB)

#target_link_libraries(core_plugin m)

