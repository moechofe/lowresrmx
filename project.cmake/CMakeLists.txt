cmake_minimum_required(VERSION 3.21.0)
project(LowResRMX LANGUAGES C)

IF(CMAKE_HOST_SYSTEM_VERSION MATCHES "microsoft-standard-WSL2$")
  SET(WSL 2)
ELSEIF(CMAKE_HOST_SYSTEM_VERSION MATCHES "-Microsoft$")
  SET(WSL 1)
ELSE()
  SET(WSL 0)
ENDIF()

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH PROJECT_ROOT_DIR)
message("PROJECT_ROOT_DIR= ${PROJECT_ROOT_DIR}")
message("SDL= ${SDL}")

set(BACKEND_SOURCE_DIR "${PROJECT_ROOT_DIR}/backend.core")
set(FRONDEND_SOURCE_DIR "${PROJECT_ROOT_DIR}/frontend.sdl")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LOWRESRMX_SDL_VERSION "3.4.12" CACHE STRING "SDL3 release tag used when project.cmake/SDL3 is absent")

file(GLOB ENGINE_SOURCES "${BACKEND_SOURCE_DIR}/*.c" "${BACKEND_SOURCE_DIR}/*/*.c")

add_library(lowresrmx_core STATIC ${ENGINE_SOURCES})
set_property(TARGET lowresrmx_core PROPERTY C_STANDARD 11)
target_include_directories(lowresrmx_core PUBLIC "${BACKEND_SOURCE_DIR}" "${BACKEND_SOURCE_DIR}/accessories" "${BACKEND_SOURCE_DIR}/datamanager" "${BACKEND_SOURCE_DIR}/interpreter" "${BACKEND_SOURCE_DIR}/libraries" "${BACKEND_SOURCE_DIR}/machine" "${BACKEND_SOURCE_DIR}/overlay" "${FRONDEND_SOURCE_DIR}")

set(HARNESS_SOURCE_DIR "${PROJECT_ROOT_DIR}/test.harness")
add_library(lowresrmx_harness STATIC "${HARNESS_SOURCE_DIR}/harness.c" "${HARNESS_SOURCE_DIR}/scenario.c" "${HARNESS_SOURCE_DIR}/golden.c")
set_property(TARGET lowresrmx_harness PROPERTY C_STANDARD 11)
target_include_directories(lowresrmx_harness PUBLIC "${HARNESS_SOURCE_DIR}")
target_link_libraries(lowresrmx_harness PUBLIC lowresrmx_core)

file(GLOB FRONTEND_SOURCES "${FRONDEND_SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME} ${FRONTEND_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)

# target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
# target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

target_include_directories(${PROJECT_NAME} PUBLIC "${FRONDEND_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE lowresrmx_core lowresrmx_harness)

add_executable(LowResRMXTest "${HARNESS_SOURCE_DIR}/main.c")
set_property(TARGET LowResRMXTest PROPERTY C_STANDARD 11)
target_link_libraries(LowResRMXTest PRIVATE lowresrmx_harness)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

if(EXISTS "${PROJECT_SOURCE_DIR}/SDL3/CMakeLists.txt")
	message(STATUS "SDL3: local checkout ${PROJECT_SOURCE_DIR}/SDL3")
	add_subdirectory(${PROJECT_SOURCE_DIR}/SDL3 EXCLUDE_FROM_ALL)
else()
	message(STATUS "SDL3: fetching release-${LOWRESRMX_SDL_VERSION}")
	include(FetchContent)
	FetchContent_Declare(SDL3
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG "release-${LOWRESRMX_SDL_VERSION}"
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
	)
	FetchContent_MakeAvailable(SDL3)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

if(NOT MSVC)
	target_link_libraries(lowresrmx_core PUBLIC m)
endif()

include("${PROJECT_ROOT_DIR}/project.release/desktop/packaging.cmake")
