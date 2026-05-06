cmake_minimum_required(VERSION 3.0...4.0)
project(lowresrmx C)

find_package(SDL3 REQUIRED CONFIG)

if(ANDROID)
	function(add_sdl_executable TARGET)
		add_library(${TARGET} SHARED ${ARGN})
	endfunction()
endif()

add_sdl_executable(${PROJECT_NAME}
	all.c
	core_wrapper.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
)
if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE)
endif()
