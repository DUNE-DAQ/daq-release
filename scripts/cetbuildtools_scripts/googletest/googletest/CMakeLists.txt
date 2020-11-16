include(ExternalProject)

ExternalProject_Add (
  googletest

  PREFIX         googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        release-1.8.1
  GIT_SHALLOW    ON

  BUILD_ALWAYS   OFF
  BUILD_COMMAND  make -j10
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                 -DBUILD_SHARED_LIBS=on
)

# We have to use relative paths for the DESTINATION (which are
# interpreted as being relative to ${CMAKE_INSTALL_PREFIX}), so that
# CPack will work
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/lib64
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
