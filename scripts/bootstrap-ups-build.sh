#!/bin/bash

WORK_DIR=$PWD
PROD_DIR=${WORK_DIR}/products
TAR_DIR=${WORK_DIR}/tarballs
CET_BUILD_DIR=${WORK_DIR}/build_pkgs_cet
NCORE=8

mkdir -p $WORK_DIR
mkdir -p $PROD_DIR
mkdir -p $TAR_DIR
mkdir -p $CET_BUILD_DIR

timenow="date \"+%D %T\""

###
# Get tarballs from SciSoft and unpack them under $PROD_DIR
###
cd $WORK_DIR
git clone https://github.com/DUNE-DAQ/daq-release.git
echo "INFO [`eval $timenow`]: Finished cloning daq-release repo from DUNE-DAQ@github."

cp -rT $WORK_DIR/daq-release/scripts/ups_build_scripts/ $PROD_DIR
cd $PROD_DIR
./get_scisoft_pkgs.sh 
#source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev/setup
#source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products/setup
echo "INFO [`eval $timenow`]: Finished getting packages from SciSoft."

###
# Build UPS products with cetbuildtools
###
cd $PROD_DIR
source $PROD_DIR/setup
(
export CETPKG_INSTALL=$PROD_DIR
export CETPKG_J=$NCORE
cd $CET_BUILD_DIR
for i in double_conversion fmt glog googletest libevent folly zmq cppzmq msgpack_c pybind11; do
  ibuild_dir=build_${i}
  isrc_dir=$WORK_DIR/daq-release/scripts/cetbuildtools_scripts/${i}/ups
  mkdir -p ${ibuild_dir}
  pushd ${ibuild_dir}
  source ${isrc_dir}/setup_for_development -p e19
  echo "INFO [`eval $timenow`]: Running \"buildtool -A -c -l -bti -j${NCORE}\""
  buildtool -A -c -l -bti -j${NCORE}
  echo "INFO [`eval $timenow`]: Finished building UPS product: ${i}"
  popd
done


for i in `find $CET_BUILD_DIR -name "*tar.bz2"`; do
  mv $i $TAR_DIR
  echo "INFO: Moved tarball to $TAR_DIR: $i"
done
)

###
# Build UPS products with ssibuildshims
###
cd $PROD_DIR
for i in `find . -maxdepth 3 -name build_*.sh`; do
  pushd `dirname $i`
  IBUILD_SH=`basename $i`
  if [[ "$i" == *"pyyaml"* ]]; then
    echo "INFO [`eval $timenow`]: Running \"./`basename $i` $PROD_DIR p383b tar\""
    ./`basename $i` $PROD_DIR p383b tar
  else
    echo "INFO [`eval $timenow`]: Running \"./`basename $i` $PROD_DIR e19 prof tar\""
    ./`basename $i` $PROD_DIR e19 prof tar
  fi
  echo "INFO [`eval $timenow`]: Finished running build script: ${i}"
  popd
done

for i in `find $PROD_DIR -name "*tar.bz2"`; do
  mv $i $TAR_DIR
  echo "INFO: Moved tarball to $TAR_DIR: $i"
done

echo "INFO [`eval $timenow`]: Finished building all UPS product."
