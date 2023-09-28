cmake_minimum_required(VERSION 3.20.0)
project(LowResRMX LANGUAGES C)

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

set(BACKEND_SOURCE_DIR "${PROJECT_ROOT_DIR}/engine-backend")
SET(FRONDEND_SOURCE_DIR "${PROJECT_ROOT_DIR}/sdl-frontend")

file(READ "${PROJECT_ROOT_DIR}/version" CMAKE_PROJECT_VERSION)

file(GLOB ENGINE_SOURCES "${BACKEND_SOURCE_DIR}/*.c" "${BACKEND_SOURCE_DIR}/*/*.c" "${FRONDEND_SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${BACKEND_SOURCE_DIR}" "${BACKEND_SOURCE_DIR}/accessories" "${BACKEND_SOURCE_DIR}/datamanager" "${BACKEND_SOURCE_DIR}/interpreter" "${BACKEND_SOURCE_DIR}/libraries" "${BACKEND_SOURCE_DIR}/machine" "${BACKEND_SOURCE_DIR}/overlay" "${FRONDEND_SOURCE_DIR}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} SDL2::Main)

if(APPLE)
	target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/Cellar/sdl2/2.28.1/include/)
endif()

if(NOT MSVC)
	target_link_libraries(${PROJECT_NAME} m)
endif()
