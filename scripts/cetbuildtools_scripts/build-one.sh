#!/bin/bash

set -eu

usage()
{
    echo Usage: $(basename $0) product src_dir build_dir install_dir
}

if [ "$#" != 4 ]; then
    usage
    exit 1
fi

product=$1
SRC_DIR=$2
BUILD_DIR=$3
INSTALL_DIR=$4


if [ ! -e "$SRC_DIR" ]; then
    echo Source directory $SRC_DIR does not exist
    exit 1
fi

set +eu
source  /cvmfs/larsoft.opensciencegrid.org/products/setup
if [ "$?" != "0" ]; then
    exit $?
fi
export PRODUCTS=$INSTALL_DIR:$PRODUCTS
set -eu

mkdir -p ${BUILD_DIR}/$product
cd       ${BUILD_DIR}/$product

echo
echo '========================================================================'
echo Sourcing setup_for_development
# not really clear to me what this line does, but it's certainly
# necessary: it's creating some weird cet files that the cmake step
# depends on. I think '-p' is 'prof' qualifier, and the e19 has to
# match a qualifier in the ups/product_deps file
set +eu
source ${SRC_DIR}/${product}/ups/setup_for_development -p e19
if [ "$?" != "0" ]; then
    exit $?
fi
set -eu

echo
echo '========================================================================'
echo Running top-level cmake
# this command line is printed out by the previous step. the manual
# setting of CC and CXX appears to be necessary because of some checks
# done by the cet build scripts. $CETPKG_TYPE was setup by setup_for_development
env CC=gcc CXX=g++ FC=gfortran cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_BUILD_TYPE=$CETPKG_TYPE ${SRC_DIR}/${product}

echo
echo '========================================================================'
echo Running make

make -j5

echo
echo '========================================================================'
echo Running make install

make install

echo
echo '========================================================================'
echo Running make package

make package
