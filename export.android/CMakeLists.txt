cmake_minimum_required(VERSION 3.0...4.0)
project(sdlcross C)

find_package(SDL3 REQUIRED CONFIG)

if(ANDROID)
	function(add_sdl_executable TARGET)
		add_library(${TARGET} SHARED ${ARGN})
	endfunction()
endif()

add_sdl_executable(sdlcross
	src/all.c
)

target_link_libraries(sdlcross PRIVATE SDL3::SDL3)

set_target_properties(sdlcross PROPERTIES
	CXX_STANDARD 11
)
if(NOT MSVC)
    target_compile_options(sdlcross PRIVATE)
endif()
