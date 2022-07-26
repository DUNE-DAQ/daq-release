#!/bin/bash

# export DAQ_RELEASE=N$(date +%y-%m-%d)
# export SPACK_VERSION="0.17.1"
export SPACK_RELEASE=/cvmfs/dunedaq-development.opensciencegrid.org/spack-nightly
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack-externals
export SPACK_TAR="v${SPACK_VERSION}.tar.gz"
rm -rf ${SPACK_RELEASE}/${DAQ_RELEASE}
mkdir -p ${SPACK_RELEASE}/${DAQ_RELEASE}
cd ${SPACK_RELEASE}/${DAQ_RELEASE}
wget https://github.com/spack/spack/archive/refs/tags/${SPACK_TAR}
tar xf $SPACK_TAR
ln -s spack-${SPACK_VERSION} spack-installation
rm -f $SPACK_TAR

source $SPACK_EXTERNALS/spack-0.17.1/share/spack/setup-env.sh
spack load py-pyyaml@5.3.1

export DAQ_RELEASE_DIR=$HOME/daq-release

git clone https://github.com/DUNE-DAQ/daq-release $DAQ_RELEASE_DIR

cd $DAQ_RELEASE_DIR

python3 scripts/spack/make-release-repo.py -u \
	-i configs/dunedaq-develop/dunedaq-develop.yaml \
	-t spack-repos/release-repo-template \
	-r ${DAQ_RELEASE} \
	-o ${SPACK_RELEASE}/${DAQ_RELEASE}

cd $SPACK_RELEASE/$DAQ_RELEASE
source spack-0.17.1/share/spack/setup-env.sh
spack repo add $SPACK_EXTERNALS/spack-repo-externals

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_EXTERNALS}/spack-repo-externals
  - ${SPACK_RELEASE}/${DAQ_RELEASE}/spack-repo
  - \$spack/var/spack/repos/builtin
EOT

spack repo list

# adding externals spack instance as an upstream

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/upstreams.yaml
upstreams:
  spack-externals:
    install_tree: ${SPACK_EXTERNALS}/spack-0.17.1/opt/spack
EOT

cp $SPACK_EXTERNALS/spack-0.17.1/etc/spack/defaults/linux/compilers.yaml \
	$SPACK_RELEASE/$DAQ_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
# verify the compilers have been added
spack compiler list

# modify the installation prefix for spack packages
# here we simply copy over the same settings as the externals instance
# use "\cp" as in RHEL, "cp" is an alias of "cp -i"; "\cp" will do "unalias".
\cp $SPACK_EXTERNALS/spack-0.17.1/etc/spack/defaults/config.yaml  $SPACK_RELEASE/$DAQ_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml
sed -i 's/rpath/runpath/g' $SPACK_RELEASE/$DAQ_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml

spack spec --reuse  dunedaq@${DAQ_RELEASE}%gcc@8.2.0^librdkafka/wvvgipg | tee $SPACK_RELEASE/$DAQ_RELEASE/spec_dunedaq_log.txt
spack install --reuse -j 16  dunedaq@${DAQ_RELEASE}%gcc@8.2.0^librdkafka/wvvgipg

# spack does not allow env names contain any dots 
export DUNEDAQ_SPACK_ENV=${DAQ_RELEASE//./-}
spack env create $DUNEDAQ_SPACK_ENV
spack env activate $DUNEDAQ_SPACK_ENV -p

# add the release umbrella package to the spack environment
spack add systems@${DAQ_RELEASE}%gcc@8.2.0
spack add dunedaq@${DAQ_RELEASE}%gcc@8.2.0
spack add llvm@13.0.0%gcc@8.2.0
spack add gcc@9.4.0%gcc@8.2.0
spack add cppcheck@2.1%gcc@8.2.0

# run spack concretize to ensure the concretizer gets a successfully result before installation
spack concretize --reuse | tee  $SPACK_RELEASE/$DAQ_RELEASE/concretizer_log.txt

# install the release
spack install

spack load dunedaq@${DAQ_RELEASE}
python3.8 -m venv $SPACK_RELEASE/$DAQ_RELEASE/dbt-pyvenv 
source $SPACK_RELEASE/$DAQ_RELEASE/dbt-pyvenv/bin/activate
python3.8 -m pip install -r $DAQ_RELEASE_DIR/configs/dunedaq-develop/pyvenv_requirements.txt
spack clean -a
spack env deactivate
spack clean -a
cp $DAQ_RELEASE_DIR/configs/dunedaq-develop/pyvenv_requirements.txt $SPACK_RELEASE/$DAQ_RELEASE
cp $DAQ_RELEASE_DIR/configs/dunedaq-develop/dbt-build-order.cmake $SPACK_RELEASE/$DAQ_RELEASE

rsync -lprt $SPACK_RELEASE/$DAQ_RELEASE /scratch
