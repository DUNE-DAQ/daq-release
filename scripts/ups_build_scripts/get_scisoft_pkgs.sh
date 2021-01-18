#!/bin/bash

if [ $# -ge 2 ]; then
  MODE=$1; shift
  COMP=$1; shift
else
  echo "$0: expecting 2 argument: [buildmode] [compiler]"
  exit
fi
function get_scisoft_pkg(){
	pkgs=( $@ )
	for ((i=0; i<${#pkgs[@]}; i+=2)); do
		echo "Getting ${pkgs[i]} from: ${pkgs[i+1]}"
		curl "${pkgs[i+1]}" 2>/dev/null| tar xj 2>/dev/null
	  	RESULT=$?
	  	if [ $RESULT -eq 0 ]; then
		  	echo "Successfully obtained ${pkgs[i]} from SciSoft."
	  	else
		  	echo "Failed to obtain ${pkgs[i]} from SciSoft, exit now!"
		  	exit 10
	  	fi
	done
}

arrCOMP=(${COMP//./ })
COMPVERSION=${arrCOMP[0]}_${arrCOMP[1]}_${arrCOMP[2]}       


PKGS_NEWER_VERSIONS=(
boost https://scisoft.fnal.gov/scisoft/packages/boost/v1_73_0/boost-1.73.0-sl7-x86_64-e19-debug.tar.bz2
boost https://scisoft.fnal.gov/scisoft/packages/boost/v1_73_0/boost-1.73.0-sl7-x86_64-e19-prof.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_17_3/cmake-3.17.3-sl7-x86_64.tar.bz2
sqlite https://scisoft.fnal.gov/scisoft/packages/sqlite/v3_32_03_00/sqlite-3.32.03.00-sl7-x86_64.tar.bz2
TRACE https://scisoft.fnal.gov/scisoft/packages/TRACE/v3_16_02/TRACE-3.16.02-sl7-x86_64.tar.bz2
)

PKGS_OLDER_VERSIONS=(
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_9_2/cmake-3.9.2-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_9_5/cmake-3.9.5-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_10_1/cmake-3.10.1-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_11_4/cmake-3.11.4-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_12_1/cmake-3.12.1-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_12_2/cmake-3.12.2-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_13_1/cmake-3.13.1-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_13_2/cmake-3.13.2-sl7-x86_64.tar.bz2
ninja https://scisoft.fnal.gov/scisoft/packages/ninja/v1_8_2/ninja-1.8.2-sl7-x86_64.tar.bz2
python https://scisoft.fnal.gov/scisoft/packages/python/v3_7_2/python-3.7.2-sl7-x86_64.tar.bz2
)

COMPILER=(
gcc https://scisoft.fnal.gov/scisoft/packages/gcc/v$COMPVERSION/gcc-$COMP-sl7-x86_64.tar.bz2
)

PKGS_COMMON=(
ssibuildshims https://scisoft.fnal.gov/scisoft/packages/ssibuildshims/v1_04_13/ssibuildshims-1.04.13-noarch.tar.bz2
cetbuildtools https://scisoft.fnal.gov/scisoft/packages/cetbuildtools/v7_15_01/cetbuildtools-7.15.01-noarch.tar.bz2
cetpkgsupport https://scisoft.fnal.gov/scisoft/packages/cetpkgsupport/v1_14_01/cetpkgsupport-1.14.01-noarch.tar.bz2
clang https://scisoft.fnal.gov/scisoft/packages/clang/v7_0_0rc3/clang-7.0.0rc3-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_14_3/cmake-3.14.3-sl7-x86_64.tar.bz2
cmake https://scisoft.fnal.gov/scisoft/packages/cmake/v3_17_2/cmake-3.17.2-sl7-x86_64.tar.bz2
git https://scisoft.fnal.gov/scisoft/packages/git/v2_20_1/git-2.20.1-sl7-x86_64.tar.bz2
ninja https://scisoft.fnal.gov/scisoft/packages/ninja/v1_10_0/ninja-1.10.0-sl7-x86_64.tar.bz2
python https://scisoft.fnal.gov/scisoft/packages/python/v3_8_3b/python-3.8.3b-sl7-x86_64.tar.bz2
sqlite https://scisoft.fnal.gov/scisoft/packages/sqlite/v3_26_00_00/sqlite-3.26.00.00-sl7-x86_64.tar.bz2
sqlite https://scisoft.fnal.gov/scisoft/packages/sqlite/v3_32_03_00/sqlite-3.32.03.00-sl7-x86_64.tar.bz2
TRACE https://scisoft.fnal.gov/scisoft/packages/TRACE/v3_15_09/TRACE-3.15.09-sl7-x86_64.tar.bz2
gdb https://scisoft.fnal.gov/scisoft/packages/gdb/v9_2/gdb-9.2-sl7-x86_64.tar.bz2
zmq https://scisoft.fnal.gov/scisoft/packages/zmq/v4_3_1/zmq-4.3.1-sl7-x86_64-e19.tar.bz2
)
if   [[ $COMP == 8.2.0 ]]; then
PKGS_DEBUG=(
boost https://scisoft.fnal.gov/scisoft/packages/boost/v1_70_0/boost-1.70.0-sl7-x86_64-e19-debug.tar.bz2
cetlib https://scisoft.fnal.gov/scisoft/packages/cetlib/v3_10_00/cetlib-3.10.00-slf7-x86_64-e19-debug.tar.bz2
cetlib_except https://scisoft.fnal.gov/scisoft/packages/cetlib_except/v1_04_01/cetlib_except-1.04.01-slf7-x86_64-e19-debug.tar.bz2
cppunit https://scisoft.fnal.gov/scisoft/packages/cppunit/v1_14_0/cppunit-1.14.0-sl7-x86_64-e19-debug.tar.bz2
hdf5 https://scisoft.fnal.gov/scisoft/packages/hdf5/v1_12_0a/hdf5-1.12.0a-sl7-x86_64-e19-debug.tar.bz2
hep_concurrency https://scisoft.fnal.gov/scisoft/packages/hep_concurrency/v1_04_01/hep_concurrency-1.04.01-slf7-x86_64-e19-debug.tar.bz2
tbb https://scisoft.fnal.gov/scisoft/packages/tbb/v2019_3/tbb-2019.3-sl7-x86_64-e19-debug.tar.bz2
)

PKGS_PROD=(
boost https://scisoft.fnal.gov/scisoft/packages/boost/v1_70_0/boost-1.70.0-sl7-x86_64-e19-prof.tar.bz2
cetlib https://scisoft.fnal.gov/scisoft/packages/cetlib/v3_10_00/cetlib-3.10.00-slf7-x86_64-e19-prof.tar.bz2
cetlib_except https://scisoft.fnal.gov/scisoft/packages/cetlib_except/v1_04_01/cetlib_except-1.04.01-slf7-x86_64-e19-prof.tar.bz2
cppunit https://scisoft.fnal.gov/scisoft/packages/cppunit/v1_14_0/cppunit-1.14.0-sl7-x86_64-e19-prof.tar.bz2
hdf5 https://scisoft.fnal.gov/scisoft/packages/hdf5/v1_12_0a/hdf5-1.12.0a-sl7-x86_64-e19-prof.tar.bz2
hep_concurrency https://scisoft.fnal.gov/scisoft/packages/hep_concurrency/v1_04_01/hep_concurrency-1.04.01-slf7-x86_64-e19-prof.tar.bz2
tbb https://scisoft.fnal.gov/scisoft/packages/tbb/v2019_3/tbb-2019.3-sl7-x86_64-e19-prof.tar.bz2
)
fi

if   [[ $COMP == 9.3.0 ]]; then
PKGS_DEBUG=(
boost https://scisoft.fnal.gov/scisoft/packages/boost/v1_73_0/boost-1.73.0-sl7-x86_64-e20-debug.tar.bz2
cetlib https://scisoft.fnal.gov/scisoft/packages/cetlib/v3_11_01/cetlib-3.11.01-slf7-x86_64-e20-debug.tar.bz2
cetlib_except https://scisoft.fnal.gov/scisoft/packages/cetlib_except/v1_05_00/cetlib_except-1.05.00-slf7-x86_64-e20-debug.tar.bz2
cppunit https://scisoft.fnal.gov/scisoft/packages/cppunit/v1_15_1/cppunit-1.15.1-sl7-x86_64-e20-debug.tar.bz2
hdf5 https://scisoft.fnal.gov/scisoft/packages/hdf5/v1_12_0a/hdf5-1.12.0a-sl7-x86_64-e20-debug.tar.bz2
hep_concurrency https://scisoft.fnal.gov/scisoft/packages/hep_concurrency/v1_05_00/hep_concurrency-1.05.00-slf7-x86_64-e20-debug.tar.bz2
tbb https://scisoft.fnal.gov/scisoft/packages/tbb/v2020_2a/tbb-2020.2a-sl7-x86_64-e20-debug.tar.bz2
)

PKGS_PROD=(
boost https://scisoft.fnal.gov/scisoft/packages/boost/v1_73_0/boost-1.73.0-sl7-x86_64-e20-prof.tar.bz2
cetlib https://scisoft.fnal.gov/scisoft/packages/cetlib/v3_11_01/cetlib-3.11.01-slf7-x86_64-e20-prof.tar.bz2
cetlib_except https://scisoft.fnal.gov/scisoft/packages/cetlib_except/v1_05_00/cetlib_except-1.05.00-slf7-x86_64-e20-prof.tar.bz2
cppunit https://scisoft.fnal.gov/scisoft/packages/cppunit/v1_15_1/cppunit-1.15.1-sl7-x86_64-e20-prof.tar.bz2
hdf5 https://scisoft.fnal.gov/scisoft/packages/hdf5/v1_12_0a/hdf5-1.12.0a-sl7-x86_64-e20-prof.tar.bz2
hep_concurrency https://scisoft.fnal.gov/scisoft/packages/hep_concurrency/v1_05_00/hep_concurrency-1.05.00-slf7-x86_64-e20-prof.tar.bz2
tbb https://scisoft.fnal.gov/scisoft/packages/tbb/v2020_2a/tbb-2020.2a-sl7-x86_64-e20-prof.tar.bz2
)
fi

PKGS_ALL=(
${PKGS_MINIMAL[@]}
${PKGS_DEBUG[@]}
${PKGS_NEWER_VERSIONS[@]}
${PKGS_OLDER_VERSIONS[@]}
)

PKGS_OPT=(
${COMPILER[@]}
${PKGS_COMMON[@]}
${PKGS_PROD[@]}
)

PKGS_DBG=(
${COMPILER[@]}
${PKGS_COMMON[@]}
${PKGS_DEBUG[@]}
)


if   [[ $MODE == opt ]]; then get_scisoft_pkg ${PKGS_OPT[@]}
elif [[ $MODE == dbg ]]; then get_scisoft_pkg ${PKGS_DBG[@]}
else
    echo "$0: expecting mode option opt or dbg"
    exit
fi
