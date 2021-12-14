include(ExternalProject)

ExternalProject_Add (
  folly

  PREFIX         folly
  GIT_REPOSITORY https://github.com/facebook/folly
  GIT_TAG        v2021.12.13.00
  GIT_SHALLOW    ON

  PATCH_COMMAND  patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/folly-make-relocatable.diff

  BUILD_ALWAYS   OFF
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                 -DBUILD_SHARED_LIBS=on
                 -DCMAKE_POSITION_INDEPENDENT_CODE=on
                 -DBoost_NO_BOOST_CMAKE=on -DBOOST_ROOT=$ENV{BOOST_FQ_DIR}
                 -DBUILD_TESTS=off
)

#  We have to use relative paths for the DESTINATION (which are
# interpreted as being relative to ${CMAKE_INSTALL_PREFIX}), so that
# CPack will work
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/lib
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
