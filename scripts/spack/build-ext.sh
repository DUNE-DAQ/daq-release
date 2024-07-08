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
# /daq-release/scripts/spack/build-ext.sh

if [[ ! -n $EXT_VERSION || ! -n $SPACK_VERSION || ! -n $GCC_VERSION || ! -n $ARCH || ! -n $DAQ_RELEASE ]]; then
    echo "Error: at least one of the environment variables needed by this script is unset. Exiting..." >&2
    exit 1
fi

starttime=$( date )

export DAQ_RELEASE_DIR=/daq-release
export STANDARD_SPACK_VERSION=0.20.0

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

   if [[ "$SPACK_VERSION" == "$STANDARD_SPACK_VERSION" ]]; then
       ln -s spack-${SPACK_VERSION} spack-installation
   fi
   
   rm -f v${SPACK_VERSION}.tar.gz
else
    echo "ALREADY HAVE DIRECTORY $PWD/spack-${SPACK_VERSION}; WILL SKIP INSTALLATION OF SPACK AREA"
fi

source spack-${SPACK_VERSION}/share/spack/setup-env.sh || exit 6

## Step 2 -- add spack repos

### Step 2.0 -- wipe out any pre-existing repos
rm -rf $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals \
       $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo \
       $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE} \
       $SPACK_ROOT/etc/spack/defaults/repos.yaml

### Step 2.1 -- add spack repos for external packages maintained by DUNE DAQ

cp -pr $DAQ_RELEASE_DIR/spack-repos/externals $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals

### Step 2.2 -- add spack repos for DUNE DAQ packages

pushd $DAQ_RELEASE_DIR

cmd="python3 scripts/spack/make-release-repo.py -u \
-b develop \
-i configs/coredaq/coredaq-develop/release.yaml \
-t spack-repos/coredaq-repo-template \
-r ${DAQ_RELEASE} \
-o ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}"

echo $cmd
$cmd

popd

mv ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE}

### Step 2.3 -- change spack repos.yaml to include the two repos created above

if [[ "$SPACK_VERSION" == "$STANDARD_SPACK_VERSION" ]]; then

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_EXTERNALS}/spack-installation/spack-repo-${DAQ_RELEASE}
  - ${SPACK_EXTERNALS}/spack-installation/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT

else

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE}
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT
    
fi

## Step 3 -- update spack config

\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/config.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/modules.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/concretizer.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/

## Step 4 -- install compiler

spack compiler find
spack install gcc@${GCC_VERSION} +binutils arch=${ARCH} |& tee /log/spack_install_gcc.txt || exit 8


spack load gcc@${GCC_VERSION}
spack compiler find
mv $HOME/.spack/linux/compilers.yaml  $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
spack compiler list

spack clean -a

# JCF, Mar-16-2024

# Quite hacky, but the situation is that we're building coredaq,
# etc. solely so we're guaranteed that all non-DUNE-DAQ packages on
# which DUNE DAQ packages depend get installed and are compatible with
# one another. So we want the traditional CMake, and not the latest
# CMake, for consistency with older versions of externals installations

sed -i 's/cmake@3.26.3/cmake@3.23.1/' $(spack location -p devtools)/package.py

cp -rp $DAQ_RELEASE_DIR/spack-repos/externals/packages/umbrella $(spack location -p coredaq)

## Step 5 -- check all specs, then install

coredaq_spec="coredaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH} ^glog@0.4.0"

dbe_spec="dbe%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH} ^qt@5.15.9:"

llvm_spec="llvm@15.0.7%gcc@${GCC_VERSION}~libomptarget~lua build_type=MinSizeRel arch=${ARCH}"

umbrella_spec="umbrella ^$coredaq_spec ^$dbe_spec ^$llvm_spec"

echo spack spec -l --reuse $umbrella_spec
spack spec -l --reuse $umbrella_spec |& tee /log/spack_spec_umbrella.txt || exit 9
spack install --reuse $umbrella_spec |& tee /log/spack_install_umbrella.txt || exit 10

# Now get the CMake we want for DUNE DAQ package building
spack install --reuse cmake@3.26.3%gcc@12.1.0~doc+ncurses+ownlibs~qt build_system=generic build_type=Release patches=4759c83 arch=linux-almalinux9-x86_64 |& tee /log/spack_install_cmake.txt || exit 11

# overwrite ssh config - in the future, this part should be done in daq-release/spack-repos/externals/packages/openssh/package.py 
SSH_INSTALL_DIR=$(spack location -i openssh)
\cp $DAQ_RELEASE_DIR/spack-repos/externals/packages/openssh/ssh_config $SSH_INSTALL_DIR/etc/ || exit 7

## Step 6 -- remove DAQ packages and umbrella packages

spack uninstall -y --all --dependents daq-cmake externals devtools systems

spack find -l | sort |& tee /log/externals_list.txt

endtime=$( date )

echo "Externals build complete"
echo "Start time: $starttime"
echo "End time: $endtime"

