#!/bin/bash

set -eu

usage()
{
    echo Usage: $(basename $0) src_dir build_dir install_dir
}

if [ "$#" != 3 ]; then
    usage
    exit 1
fi

SRC_DIR=$1
BUILD_DIR=$2
INSTALL_DIR=$3


if [ ! -e "$SRC_DIR" ]; then
    echo Source directory $SRC_DIR does not exist
    exit 1
fi

for product in ers fmt glog googletest double_conversion libevent folly; do
    ./build-one.sh $product $SRC_DIR $BUILD_DIR $INSTALL_DIR 2>&1 | tee ${product}-build.log 2>&1 || exit 1
done
