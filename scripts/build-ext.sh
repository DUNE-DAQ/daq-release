#!/bin/bash


## Step 1 -- obtain spack
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}/spack-$SPACK_VERSION-gcc-$GCC_VERSION
mkdir -p ${SPACK_RELEASE}
cd ${SPACK_RELEASE}
wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz
tar xf v${SPACK_VERSION}.tar.gz
ln -s spack-${SPACK_VERSION} spack-installation
rm -f v${SPACK_VERSION}.tar.gz

## Step 2 -- add spack repos
### Step 2.1 -- add spack repos for external pacakges maintianed by DUNE DAQ

cp -pr $DAQ_RELEASE_DIR/spack-repos/externals $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals

### Step 2.2 -- add spack repos for DUNE DAQ pacakges

export DAQ_RELEASE="NB23-08-00"
python3 $DAQ_RELEASE_DIR/scripts/spack/make-release-repo.py -u \
-b develop \
-i configs/dunedaq/dunedaq-develop/release.yaml \
-t spack-repos/dunedaq-repo-template \
-r ${DAQ_RELEASE} \
-o ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}
mv  ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE}

### Step 2.3 -- change spack repos.yaml to include the two repos created above

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE}
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT

## Step 3 -- update spack config

\cp  $DAQ_RELEASE_DIR/misc/spack-$SPACK_VERSION}-config/config.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-$SPACK_VERSION}-config/modules.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-$SPACK_VERSION}-config/concretizer.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/

## Step 4 -- install compiler

source ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/share/spack/setup-env.sh
spack compiler find
spack install gcc@${GCC_VERSION} +binutils arch=${ARCH}
spack load gcc@${GCC_VERSION}
spack compiler find
mv $HOME/.spack/linux/compilers.yaml  $SPACK_EXTERNALS/spack-0.20.0/etc/spack/defaults/linux/
spack compiler list

## Step 5 -- install dunedaq (extenrals + DAQ packages)

spack spec --reuse dunedaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}
spack install --reuse dunedaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}
# overwrite ssh config - in the future, this part should be done in daq-release/spack-repos/externals/packages/openssh/package.py 
SSH_INSTALL_DIR=$(spack location -i openssh)
\cp $DAQ_RELEASE_DIR/spack-repos/externals/packages/openssh/ssh_config $SSH_INSTALL_DIR/etc/

## Step 6 -- install llvm and qt

spack install --reuse llvm@15.0.7~omp_as_runtime %gcc@${GCC_VERSION} build_type=MinSizeRel arch=${ARCH}
spack install --reuse qt@5.15.9%gcc@${GCC_VERSION} arch=${ARCH}

## Step 6 -- remove DAQ packages and umbrella packages

spack uninstall --dependents daq-cmake externals devtools systems
