#!/bin/bash

export NIGHTLY_RELEASE=NT$(date +%y-%m-%d)
export DAQ_RELEASE=$NIGHTLY_RELEASE
export SPACK_VERSION=0.18.1
export GCC_VERSION=12.1.0
export EXT_VERSION=v1.0

WORK_AREA=$1
mkdir -p $WORK_AREA
cd $WORK_AREA
export WORK_AREA=$PWD
export DAQ_RELEASE_BRANCH=$2

export SPACK_RELEASE_DIR=$WORK_AREA/spack-release/
export DAQ_RELEASE_DIR=$WORK_AREA/daq-release

export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}/spack-$SPACK_VERSION-gcc-$GCC_VERSION

export SPACK_RELEASE=$SPACK_RELEASE_DIR/spack-$SPACK_VERSION-gcc-$GCC_VERSION

mkdir -p ${SPACK_RELEASE}

cd ${SPACK_RELEASE}
wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz
tar xf v${SPACK_VERSION}.tar.gz
ln -s spack-${SPACK_VERSION} spack-installation
rm -f v${SPACK_VERSION}.tar.gz

git clone https://github.com/DUNE-DAQ/daq-release $DAQ_RELEASE_DIR -b $DAQ_RELEASE_BRANCH

cd $DAQ_RELEASE_DIR
python3 scripts/spack/make-release-repo.py -u \
	-i configs/dunedaq-develop/dunedaq-develop.yaml \
	-t spack-repos/release-repo-template \
	-r ${DAQ_RELEASE} \
	-o ${SPACK_RELEASE}/spack-${SPACK_VERSION}

cd $SPACK_RELEASE
pwd
source spack-${SPACK_VERSION}/share/spack/setup-env.sh

echo "*********** spack arch ************ "
spack arch

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_RELEASE}/spack-${SPACK_VERSION}/spack-repo
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT

spack repo list

cat <<EOT >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml
upstreams:
  spack-externals:
    install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOT


cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/compilers.yaml \
$SPACK_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
# verify the compilers have been added
spack compiler list

\cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml  $SPACK_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml

sed -i 's/host_compatible: true/host_compatible: false/g' $SPACK_ROOT/etc/spack/defaults/concretizer.yaml

spack spec --reuse dunedaq@${DAQ_RELEASE}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-scientific7-broadwell |  tee $SPACK_RELEASE/spec_dunedaq_log.txt
spack install --reuse dunedaq@${DAQ_RELEASE}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-scientific7-broadwell

# Generate pyvenv_requirements.txt
spack load dunedaq@${DAQ_RELEASE}
/usr/bin/python3 $DAQ_RELEASE_DIR/scripts/spack/make-release-repo.py -o ${SPACK_RELEASE}/../ --pyvenv-requirements -i $DAQ_RELEASE_DIR/configs/dunedaq-develop/dunedaq-develop.yaml

python -m venv --prompt dbt ${SPACK_RELEASE}/../.venv
source ${SPACK_RELEASE}/../.venv/bin/activate

python -m pip install -r ${SPACK_RELEASE}/../pyvenv_requirements.txt

spack clean -a

cp $DAQ_RELEASE_DIR/configs/dunedaq-develop/dbt-build-order.cmake ${SPACK_RELEASE}/../
