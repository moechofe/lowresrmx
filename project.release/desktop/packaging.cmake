# Release packaging for the desktop app: version stamping, per-platform resources, the package
# payload and the CPack generators (ZIP everywhere, NSIS on Windows, DragNDrop on macOS).
#
# This file is include()d at the end of project.cmake/CMakeLists.txt, NOT added as a
# subdirectory: ${PROJECT_NAME}, ${CMAKE_CURRENT_BINARY_DIR} and ${PROJECT_ROOT_DIR} are the
# ones of that project, and only ${CMAKE_CURRENT_LIST_DIR} points here.

set(LOWRESRMX_PACKAGING_DIR "${CMAKE_CURRENT_LIST_DIR}")

# Version ---------------------------------------------------------------------------------------
# 1.0_68 -> numeric 1.0.68 (CPack, Info.plist) -> comma 1,0,68,0 (Windows VERSIONINFO).

set(LOWRESRMX_VERSION "0.0_0" CACHE STRING "Release version, e.g. 1.0_68")
string(REPLACE "_" "." LOWRESRMX_VERSION_NUMERIC "${LOWRESRMX_VERSION}")
string(REPLACE "." "," LOWRESRMX_VERSION_COMMA "${LOWRESRMX_VERSION_NUMERIC}")
set(LOWRESRMX_VERSION_COMMA "${LOWRESRMX_VERSION_COMMA},0")

# Windows: exe icon and version resource --------------------------------------------------------

if(WIN32)
	set(LOWRESRMX_ICO_PATH "${LOWRESRMX_PACKAGING_DIR}/LowResRMX.ico")
	configure_file("${LOWRESRMX_PACKAGING_DIR}/LowResRMX.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/LowResRMX.rc" @ONLY)
	target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/LowResRMX.rc")
	# Windowed subsystem: SDL_main.h provides WinMain and UTF-8 argv (frontend.sdl/sdl_include.h).
	set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()

# macOS: real .app bundle with a generated .icns ------------------------------------------------

if(APPLE)
	set(LOWRESRMX_ICNS "${CMAKE_CURRENT_BINARY_DIR}/LowResRMX.icns")
	add_custom_command(OUTPUT "${LOWRESRMX_ICNS}"
		COMMAND bash "${LOWRESRMX_PACKAGING_DIR}/make_macos_icns.bash"
			"${PROJECT_ROOT_DIR}/asset.dev/icon-512.png" "${LOWRESRMX_ICNS}"
		DEPENDS "${PROJECT_ROOT_DIR}/asset.dev/icon-512.png"
			"${LOWRESRMX_PACKAGING_DIR}/make_macos_icns.bash"
		COMMENT "Generating LowResRMX.icns"
		VERBATIM
	)
	target_sources(${PROJECT_NAME} PRIVATE "${LOWRESRMX_ICNS}")
	set_source_files_properties("${LOWRESRMX_ICNS}" PROPERTIES GENERATED TRUE MACOSX_PACKAGE_LOCATION Resources)

	set(MACOSX_BUNDLE_EXECUTABLE_NAME "LowResRMX")
	set(MACOSX_BUNDLE_BUNDLE_NAME "LowResRMX")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "it.ro.ret.desktop.LowResRMX")
	set(MACOSX_BUNDLE_ICON_FILE "LowResRMX.icns")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${LOWRESRMX_VERSION_NUMERIC}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${LOWRESRMX_VERSION_NUMERIC}")
	set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2016-2024 Timo Kloss, 2021-2026 Martin Mauchauffee")
	set_target_properties(${PROJECT_NAME} PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST "${LOWRESRMX_PACKAGING_DIR}/Info.plist.in"
	)
endif()

# Package payload -------------------------------------------------------------------------------
# The app needs no data files at runtime: settings live in the pref path, programs come from
# argv or drag & drop. A package is the binary plus documentation and demo programs.
# LICENSE is shipped as LICENSE.txt because NSIS's license page wants a .txt, and Windows users
# need an extension to open it.

configure_file("${LOWRESRMX_PACKAGING_DIR}/README.txt.in" "${CMAKE_CURRENT_BINARY_DIR}/README.txt" @ONLY)
configure_file("${PROJECT_ROOT_DIR}/LICENSE" "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt" COPYONLY)

if(APPLE)
	install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)
else()
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
endif()
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/README.txt"
	"${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt"
	"${PROJECT_ROOT_DIR}/asset.dev/manual.md"
	"${PROJECT_ROOT_DIR}/asset.manual/manual.html"
	DESTINATION .
)
install(DIRECTORY "${PROJECT_ROOT_DIR}/asset.programs/" DESTINATION programs FILES_MATCHING PATTERN "*.rmx")

# CPack ------------------------------------------------------------------------------------------
# One cpack run per generator, with the artifact name passed on the command line:
#   cpack --config build/CPackConfig.cmake -G ZIP -B dist \
#     -D CPACK_PACKAGE_FILE_NAME=LowResRMX-1.0_68-linux-x86_64

# Per-generator overrides (ZIP gets CHANGELOG.md, the installers do not) live in a cpack-time
# config file, because install() rules cannot see which generator is running.
set(CPACK_PROJECT_CONFIG_FILE "${LOWRESRMX_PACKAGING_DIR}/cpack_generator_config.cmake")

set(CPACK_PACKAGE_NAME "LowResRMX")
set(CPACK_PACKAGE_VENDOR "Martin Mauchauffee")
set(CPACK_PACKAGE_VERSION "${LOWRESRMX_VERSION_NUMERIC}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Retro Game Creator - a fantasy console")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://ret.ro.it/about.html")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LowResRMX")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_EXECUTABLES "LowResRMX;LowResRMX")
set(CPACK_PACKAGE_FILE_NAME "LowResRMX-${LOWRESRMX_VERSION}")
set(CPACK_VERBATIM_VARIABLES TRUE)
if(APPLE)
	# Do not strip: stripping invalidates the linker's ad-hoc signature and macOS then refuses
	# to launch the arm64 slice.
	set(CPACK_STRIP_FILES FALSE)
else()
	set(CPACK_STRIP_FILES TRUE)
endif()

# Windows installer (NSIS)
set(CPACK_NSIS_PACKAGE_NAME "LowResRMX ${LOWRESRMX_VERSION}")
set(CPACK_NSIS_DISPLAY_NAME "LowResRMX ${LOWRESRMX_VERSION}")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
set(CPACK_NSIS_INSTALLED_ICON_NAME "LowResRMX.exe")
set(CPACK_NSIS_MUI_ICON "${LOWRESRMX_PACKAGING_DIR}/LowResRMX.ico")
set(CPACK_NSIS_MUI_UNIICON "${LOWRESRMX_PACKAGING_DIR}/LowResRMX.ico")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_URL_INFO_ABOUT "https://ret.ro.it/about.html")
set(CPACK_CREATE_DESKTOP_LINKS "LowResRMX")

# macOS installer (DMG) - the /Applications symlink is created by default
set(CPACK_DMG_VOLUME_NAME "LowResRMX ${LOWRESRMX_VERSION}")
set(CPACK_DMG_FORMAT "UDZO")

include(CPack)
