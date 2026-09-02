# Included by cpack once per generator, after CPACK_GENERATOR has been set to the single generator
# being run (see CPACK_PROJECT_CONFIG_FILE in packaging.cmake). Per-generator payload differences
# belong here; everything common stays in packaging.cmake.
#
# The full CHANGELOG.md ships in the ZIP packages only. The NSIS setup and the DMG install the app
# into Program Files / Applications, where a changelog file is noise.
if(CPACK_GENERATOR STREQUAL "ZIP")
	set(CPACK_INSTALL_SCRIPTS "${CMAKE_CURRENT_LIST_DIR}/install_changelog.cmake")
endif()
