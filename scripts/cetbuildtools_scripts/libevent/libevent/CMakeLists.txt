include(ExternalProject)

ExternalProject_Add (
  libevent

  PREFIX         libevent
  GIT_REPOSITORY https://github.com/libevent/libevent
  GIT_TAG        release-2.1.11-stable
  GIT_SHALLOW    ON

  BUILD_ALWAYS   OFF
  # There's one install target in the libevent CMakeLists.txt that has
  # an absolute path, which causes cmake to put absolute paths in the
  # libevents targets file, so that the resulting installation can't
  # be moved. So we patch it here
  PATCH_COMMAND  patch < ${CMAKE_CURRENT_LIST_DIR}/libevent-cmakelists-relative-dir.patch
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                 -DBUILD_SHARED_LIBS=on
                 -DCMAKE_POSITION_INDEPENDENT_CODE=on
)

# We have to use relative paths for the DESTINATION (which are
# interpreted as being relative to ${CMAKE_INSTALL_PREFIX}), so that
# CPack will work
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/lib
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
