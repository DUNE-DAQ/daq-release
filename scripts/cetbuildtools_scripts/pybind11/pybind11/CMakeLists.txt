include(ExternalProject)

ExternalProject_Add (
  pybind11

  PREFIX         pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.6.2
  GIT_SHALLOW    ON

  BUILD_ALWAYS   OFF
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)

# We have to use relative paths for the DESTINATION (which are
# interpreted as being relative to ${CMAKE_INSTALL_PREFIX}), so that
# CPack will work
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/share
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
