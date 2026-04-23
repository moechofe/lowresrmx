cmake_minimum_required(VERSION 3.21.0)
project(LowResRMX LANGUAGES C)

IF(CMAKE_HOST_SYSTEM_VERSION MATCHES "microsoft-standard-WSL2$")
  SET(WSL 2)
ELSEIF(CMAKE_HOST_SYSTEM_VERSION MATCHES "-Microsoft$")
  SET(WSL 1)
ELSE()
  SET(WSL 0)
ENDIF()

# Sauce: https://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
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

file(GLOB ENGINE_SOURCES "${BACKEND_SOURCE_DIR}/*.c" "${BACKEND_SOURCE_DIR}/*/*.c" "${FRONDEND_SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

# target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
# target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

target_include_directories(${PROJECT_NAME} PUBLIC "${BACKEND_SOURCE_DIR}" "${BACKEND_SOURCE_DIR}/accessories" "${BACKEND_SOURCE_DIR}/datamanager" "${BACKEND_SOURCE_DIR}/interpreter" "${BACKEND_SOURCE_DIR}/libraries" "${BACKEND_SOURCE_DIR}/machine" "${BACKEND_SOURCE_DIR}/overlay" "${FRONDEND_SOURCE_DIR}")

# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

# if(SDL2_PATH)
# 	message("Using SDL2_PATH= ${SDL2_PATH}")
# 	find_package(SDL2 REQUIRED)
# 	target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
# else()
# 	add_subdirectory(${SDL} EXCLUDE_FROM_ALL)
# endif()

# set(SDL_DBUS OFF)
# set(SDL_DUMMYCAMERA OFF)
# set(SDL_X11_XSCRNSAVER OFF)
# set(SDL_HIDAPI_JOYSTICK OFF)
# set(SDL_VIRTUAL_JOYSTICK OFF)
# set(SDL_X11_XTEST OFF)
# set(SDL_HIDAPI_LIBUSB OFF)
# if(WSL GREATER 0)
# 	set(SDL_X11_XTEST OFF)
# endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/SDL3 EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
if(WIN32)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		VERBATIM
	)
endif()

# if(APPLE)
# 	target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/Cellar/sdl2/2.28.1/include/)
# endif()

if(NOT MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()
