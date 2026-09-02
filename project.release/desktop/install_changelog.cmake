# Run by cpack during its staging install (CPACK_INSTALL_SCRIPTS), for the ZIP generator only.
# CMAKE_INSTALL_PREFIX is the staging directory that becomes the archive's top-level folder.
# The repo root is resolved from this script's own location: no CPACK_* variable is relied upon,
# because only CMAKE_CURRENT_{SOURCE,BINARY}_DIR and CMAKE_INSTALL_PREFIX are guaranteed here.
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/../../CHANGELOG.md" DESTINATION "${CMAKE_INSTALL_PREFIX}")
