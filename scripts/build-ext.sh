#!/bin/bash

# Example usage:

# docker run -it --name <some useful name> \
    # -e "EXT_VERSION=2.1" -e "DAQ_RELEASE=EXT2.1ADD" \
    # -e "SPACK_VERSION=0.20.0" -e "GCC_VERSION=12.1.0" -e "ARCH=linux-almalinux9-x86_64" \
    # -v <location of freshly-checked-out daq-release repo>:/daq-release \
    # -v <location of local directory for log output>:/log \
    # -v <location of local area for installation>:/cvmfs/dunedaq.opensciencegrid.org \
    # ghcr.io/dune-daq/alma9-spack:latest

# When in the container, run:
# /daq-release/scripts/build-ext.sh

if [[ ! -n $EXT_VERSION || ! -n $SPACK_VERSION || ! -n $GCC_VERSION || ! -n $ARCH || ! -n $DAQ_RELEASE ]]; then
    echo "Error: at least one of the environment variables needed by this script is unset. Exiting..." >&2
    exit 1
fi

export DAQ_RELEASE_DIR=/daq-release
set -o pipefail   # So tee doesn't swallow return values

## Step 0 -- sanity check(s)

test -d $DAQ_RELEASE_DIR                   || exit 2
test -d /cvmfs/dunedaq.opensciencegrid.org || exit 3
test -d /log                               || exit 4   

## Step 1 -- obtain and set up spack
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v${EXT_VERSION}
mkdir -p $SPACK_EXTERNALS
cd $SPACK_EXTERNALS

if [[ ! -d  spack-${SPACK_VERSION} ]]; then
   wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz || exit 5
   tar xf v${SPACK_VERSION}.tar.gz
   ln -s spack-${SPACK_VERSION} spack-installation
   rm -f v${SPACK_VERSION}.tar.gz
else
    echo "ALREADY HAVE DIRECTORY $PWD/spack-${SPACK_VERSION}; WILL SKIP INSTALLATION OF SPACK AREA"
fi

source spack-${SPACK_VERSION}/share/spack/setup-env.sh || exit 6

## Step 2 -- add spack repos

### Step 2.0 -- wipe out any pre-existing repos
rm -rf $SPACK_EXTERNALS/spack-installation/spack-repo-externals \
       $SPACK_EXTERNALS/spack-installation/spack-repo \
       $SPACK_EXTERNALS/spack-installation/spack-repo-${DAQ_RELEASE} \
       $SPACK_ROOT/etc/spack/defaults/repos.yaml

### Step 2.1 -- add spack repos for external packages maintained by DUNE DAQ

cp -pr $DAQ_RELEASE_DIR/spack-repos/externals $SPACK_EXTERNALS/spack-installation/spack-repo-externals

### Step 2.2 -- add spack repos for DUNE DAQ packages

pushd $DAQ_RELEASE_DIR

cmd="python3 scripts/spack/make-release-repo.py -u \
-b develop \
-i configs/dunedaq/dunedaq-develop/release.yaml \
-t spack-repos/dunedaq-repo-template \
-r ${DAQ_RELEASE} \
-o ${SPACK_EXTERNALS}/spack-installation"

echo $cmd
$cmd

popd

mv  ${SPACK_EXTERNALS}/spack-installation/spack-repo $SPACK_EXTERNALS/spack-installation/spack-repo-${DAQ_RELEASE}

### Step 2.3 -- change spack repos.yaml to include the two repos created above

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_EXTERNALS}/spack-installation/spack-repo-${DAQ_RELEASE}
  - ${SPACK_EXTERNALS}/spack-installation/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT

## Step 3 -- update spack config

\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/config.yaml $SPACK_EXTERNALS/spack-installation/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/modules.yaml $SPACK_EXTERNALS/spack-installation/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/concretizer.yaml $SPACK_EXTERNALS/spack-installation/etc/spack/defaults/

## Step 4 -- install compiler

spack compiler find
spack install gcc@${GCC_VERSION} +binutils arch=${ARCH} |& tee /log/spack_install_gcc.txt || exit 8


spack load gcc@${GCC_VERSION}
spack compiler find
mv $HOME/.spack/linux/compilers.yaml  $SPACK_EXTERNALS/spack-installation/etc/spack/defaults/linux/
spack compiler list

spack clean -a

# JCF, Mar-16-2024

# Quite hacky, but the situation is that we're building dunedaq,
# etc. solely so we're guaranteed that all non-DUNE-DAQ packages on
# which DUNE DAQ packages depend get installed and are compatible with
# one another. So we want the traditional CMake, and not the latest
# CMake, for consistency with older versions of externals installations

sed -i 's/cmake@3.26.3/cmake@3.23.1/' $(spack location -p devtools)/package.py

## Step 5 -- check all specs, then install

dunedaq_spec="dunedaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}"

dbe_spec="dbe%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}"

llvm_spec="llvm@15.0.7~omp_as_runtime %gcc@${GCC_VERSION} build_type=MinSizeRel arch=${ARCH}"

spack spec -l --reuse $dunedaq_spec |& tee /log/spack_spec_dunedaq.txt || exit 9
spack spec -l --reuse $dbe_spec     |& tee /log/spack_spec_dbe.txt || exit 10
spack spec -l --reuse $llvm_spec    |& tee /log/spack_spec_llvm.txt || exit 11

spack install --reuse $dunedaq_spec |& tee /log/spack_install_dunedaq.txt || exit 12

# overwrite ssh config - in the future, this part should be done in daq-release/spack-repos/externals/packages/openssh/package.py 
SSH_INSTALL_DIR=$(spack location -i openssh)
\cp $DAQ_RELEASE_DIR/spack-repos/externals/packages/openssh/ssh_config $SSH_INSTALL_DIR/etc/ || exit 7

spack install --reuse $dbe_spec  |& tee /log/spack_install_dbe.txt || exit 13
spack install --reuse $llvm_spec |& tee /log/spack_install_llvm.txt || exit 14

## Step 6 -- remove DAQ packages and umbrella packages

spack uninstall -y --all --dependents daq-cmake externals devtools systems

spack find -l | sort |& tee /log/externals_list.txt
