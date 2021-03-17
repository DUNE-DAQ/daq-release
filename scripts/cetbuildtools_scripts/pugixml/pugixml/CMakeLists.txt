include(ExternalProject)

ExternalProject_Add (
  pugixml

  PREFIX         pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG        v1.11
  GIT_SHALLOW    ON

  BUILD_ALWAYS   OFF
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
		 -DBUILD_SHARED_LIBS=ON
		 -DBUILD_STATIC_LIBS=OFF
)

# We have to use relative paths for the DESTINATION (which are
# interpreted as being relative to ${CMAKE_INSTALL_PREFIX}), so that
# CPack will work
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/lib64
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
