# ======================================================================
#  double_conversion main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/double_conversion/ups/setup_for_development <-d|-o|-p>
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package
# ======================================================================


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

project(folly)

# cetbuildtools contains our cmake modules
set( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
if( NOT CETBUILDTOOLS_VERSION )
  message( FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules" )
endif()
set(CMAKE_MODULE_PATH $ENV{CETBUILDTOOLS_DIR}/Modules 
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -pedantic)
cet_report_compiler_flags()

find_ups_boost(v1_70_00)
find_ups_product(cetbuildtools v2_03_00)
find_ups_product(double_conversion v3_1_5)
find_ups_product(fmt v6_2_1)
find_ups_product(glog v0_4_0)
find_ups_product(libevent v2_1_8)

# source
add_subdirectory(folly)

# ups - table and config files
add_subdirectory(ups)

# packaging utility - enable building a package tarball
include(UseCPack)
