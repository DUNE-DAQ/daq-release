#!/bin/bash

if [[ ! -n $EXT_VERSION || ! -n $SPACK_VERSION || ! -n $GCC_VERSION || ! -n $ARCH || ! -n $DAQ_RELEASE || ! -n $DAQ_RELEASE_DIR ]]; then
    echo "Error: at least one of the environment variables needed by this script is unset. Exiting..." >&2
    exit 1
fi

## Step 0 -- sanity check(s)

test -d $DAQ_RELEASE_DIR ||                   exit 1
test -d /cvmfs/dunedaq.opensciencegrid.org || exit 2

## Step 1 -- obtain and set up spack
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}
mkdir -p $SPACK_EXTERNALS
cd $SPACK_EXTERNALS
wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz || exit 3
tar xf v${SPACK_VERSION}.tar.gz
ln -s spack-${SPACK_VERSION} spack-installation
rm -f v${SPACK_VERSION}.tar.gz

source spack-${SPACK_VERSION}/share/spack/setup-env.sh || exit 4

## Step 2 -- add spack repos
### Step 2.1 -- add spack repos for external packages maintained by DUNE DAQ

cp -pr $DAQ_RELEASE_DIR/spack-repos/externals $SPACK_EXTERNALS/spack-installation/spack-repo-externals

### Step 2.2 -- add spack repos for DUNE DAQ pacakges

pushd $DAQ_RELEASE_DIR
python3 scripts/spack/make-release-repo.py -u \
-b develop \
-i configs/dunedaq/dunedaq-develop/release.yaml \
-t spack-repos/dunedaq-repo-template \
-r ${DAQ_RELEASE} \
-o ${SPACK_EXTERNALS}/spack-installation
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

# overwrite ssh config - in the future, this part should be done in daq-release/spack-repos/externals/packages/openssh/package.py 
SSH_INSTALL_DIR=$(spack location -i openssh)
\cp $DAQ_RELEASE_DIR/spack-repos/externals/packages/openssh/ssh_config $SSH_INSTALL_DIR/etc/ || exit 5

## Step 4 -- install compiler

spack compiler find
spack install gcc@${GCC_VERSION} +binutils arch=${ARCH} || exit 6


spack load gcc@${GCC_VERSION}
spack compiler find
mv $HOME/.spack/linux/compilers.yaml  $SPACK_EXTERNALS/spack-installation/etc/spack/defaults/linux/
spack compiler list


## Step 5 -- check all specs, then install

for action in spec install ; do
      spack $action --reuse dunedaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}  || exit 7
      spack $action --reuse dbe%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}                     || exit 8
      spack $action --reuse llvm@15.0.7~omp_as_runtime %gcc@${GCC_VERSION} build_type=MinSizeRel arch=${ARCH} || exit 9
done

## Step 6 -- remove DAQ packages and umbrella packages

spack uninstall --dependents daq-cmake externals devtools systems

spack find -l | sort
