include(ExternalProject)

ExternalProject_Add (
  msgpack_c

  PREFIX         msgpack_c
  GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
  GIT_TAG        cpp-3.3.0
  GIT_SHALLOW    ON

  BUILD_ALWAYS   OFF
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)

# We have to use relative paths for the DESTINATION (which are
# interpreted as being relative to ${CMAKE_INSTALL_PREFIX}), so that
# CPack will work
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/lib
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
