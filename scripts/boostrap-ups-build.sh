#!/bin/bash

WORK_DIR=/scratch
PROD_DIR=${WORK_DIR}/products
TAR_DIR=${WORK_DIR}/tarballs
CET_BUILD_DIR=${WORK_DIR}/cetbuild
CETPKG_INSTALL=$PROD_DIR
NCORE=8
mkdir -p $WORK_DIR
mkdir -p $PROD_DIR
mkdir -p $TAR_DIR
mkdir -p $CET_BUILD_DIR

timenow="date \"+%D %T\""

cd $WORK_DIR
git clone https://github.com/DUNE-DAQ/daq-release.git
git clone https://github.com/DUNE-DAQ/daq-externals.git
echo "INFO [`eval $timenow`]: Finished cloning daq-release and daq-externals repo from DUNE-DAQ@github."

cp -rT $WORK_DIR/daq-release/scripts/ups_build_scripts/ $PROD_DIR
cd $PROD_DIR
source setup
#./get_scisoft_pkgs.sh 
echo "INFO [`eval $timenow`]: Finished getting packages from SciSoft."


PROD_NAME="double-conversion"

cd $CET_BUILD_DIR
for i in double-conversion fmt glog googletest libevent; do
  ibuild_dir=build_${i}
  isrc_dir=$WORK_DIR/daq-externals-workdir/daq-externals/ups/multi-project/${i}/ups
  mkdir ${ibuild_dir}
  pushd ${ibuild_dir}
  source ${isrc_dir}/setup_for_development -p e19
  echo "INFO[`eval $timenow`]: Running \"buildtool -A -c -l -bti -j${NCORE}\""
  #buildtool -A -c -l -bti -j8
  echo "INFO[`eval $timenow`]: Finished building UPS product: ${i}"
  popd
done

cd $PROD_DIR

for i in `find . -name build*.sh`; do
  pushd `dirname $i`
  IBUILD_SH=`basename $i`
  if [[ "$i" == *"pyyaml"* ]]; then
    echo "INFO[`eval $timenow`]: Running \"./`basename $i` $PROD_DIR p383b tar\""
    #./`basename $i` $PROD_DIR p383b tar
  else
    echo "INFO[`eval $timenow`]: Running \"./`basename $i` $PROD_DIR e19 prof tar\""
    #./`basename $i` $PROD_DIR e19 prof tar
  fi
  echo "INFO[`eval $timenow`]: Finished running build script: ${i}"
  popd
done
popd

for i in `find $WORK_DIR -name "*tar.bz2"`; do
  mv $i $TAR_DIR
  echo "INFO: Moved tarball to $TAR_DIR: $i"
done

echo "INFO[`eval $timenow`]: Finished building all UPS product."
