include(ExternalProject)

# We could just set the CMAKE_INSTALL_*DIR directories directly to
# ${CMAKE_INSTALL_PREFIX}/${ers_*_dir}, but we want to
# use CPack, which only knows about files/directories that are
# install()ed. So we install into a staging area and then install()
# from the staging area into the final destination
ExternalProject_Add (
  ers

  PREFIX         ers
  GIT_REPOSITORY https://github.com/philiprodrigues/ers
  GIT_TAG        dune/ers-00-26-00-cmake
  GIT_SHALLOW    ON

  BUILD_ALWAYS   OFF
  INSTALL_DIR    ${PROJECT_BINARY_DIR}/external_staging
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                 -DBUILD_SHARED_LIBS=on
                 -DBoost_NO_BOOST_CMAKE=on -DBOOST_ROOT=$ENV{BOOST_FQ_DIR}
)

install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/lib64
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/include
       DESTINATION ${flavorqual_dir})
install(DIRECTORY ${PROJECT_BINARY_DIR}/external_staging/bin
       DESTINATION ${flavorqual_dir})
