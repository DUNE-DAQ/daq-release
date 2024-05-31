
_JCF, May-31-2024: these instructions haven't been maintained. Please contact me if you wish to build externals_

The steps below has been tested `isc01.fnal.gov` (running `Almalinux9` with `docker-ce` installed) as user `dunedaq`.


## Job setup

The build is performed inside docker containers. There are two images used, one is based on `Scientific Linux 7` and the other is based on `Almalinux 9`. 

The follow commands set up some environment variables passed to the docker container, and executes a script inside it with the usage of those environment variables. The script can be found in the `daq-release` repo (`daq-release/scritps/build-ext.sh`). Please refer to the next section for a detailed explanation of the script.

```bash=
export LOCAL_CVMFS_DUNEDAQ=/scratch/dunedaq/docker-scratch/cvmfs_dunedaq
export TARGET_CVMFS_DUNEDAQ=/cvmfs/dunedaq.opensciencegrid.org
mkdir -p $LOCAL_CVMFS_DUNEDAQ

export SPACK_VERSION=0.20.0
export GCC_VERSION=12.1.0
export DAQ_RELEASE=NB23-08-00

export DAQ_RELEASE_DIR=/scratch/dunedaq/docker-scratch/daq-release
rm -rf $DAQ_RELEASE_DIR
git clone https://github.com/DUNE-DAQ/daq-release $DAQ_RELEASE_DIR

# For Scientific Linux 7
export DOCKER_IMAGE="ghcr.io/dune-daq/sl7-spack:latest"
export EXT_VERSION="run-v1.1" # for Scientific Linux 7 build
export ARCH="linux-scientific7-x86_64"


docker run --rm --net=host \
-e EXT_VER -e ARCH -e SPACK_VERSION \
-e GCC_VERSION -e DAQ_RELEASE_DIR -e DAQ_RELEASE \
-v $LOCAL_CVMFS_DUNEDAQ:$TARGET_CVMFS_DUNEDAQ \
-v $DAQ_RELEASE_DIR:$DAQ_RELEASE_DIR \
$DOCKER_IMAGE $DAQ_RELEASE_DIR/scritps/build-ext.sh
```

Use the following settings for `Almalinux 9` build.

```bash
# For Almalinux 9
export DOCKER_IMAGE="ghcr.io/dune-daq/al9:latest"
export ARCH="linux-almalinux9-x86_64"
export EXT_VER="run-ext-v2.0"
```

## The build script to be run inside docker containers

In summary, the build script does the following things in order:

1. Obtain specified spack version;
2. Add additional spack repos;
3. Update spack configuration files;
4. Install compilers;
5. Install `dunedaq` umbrella package (subsequently install all external and DAQ packages);
6. Install additional packages like `llvm` and `qt`;
7. Remove DAQ and umbrella packages (with only external packages left).

Here is the content of the script with in-line comments.

:red_circle: Note 1: In Step 6 below, `llvm` can be skipped when building external software stack for the running environment.

:red_circle: Note 2: In Step 6 below, `qt` can be skipped if not intending to build/run `dbe` GUI.


```bash
#!/bin/bash

if [[ ! -n $EXT_VERSION || ! -n $SPACK_VERSION || ! -n $GCC_VERSION || ! -n $ARCH || ! -n $DAQ_RELEASE || ! -n $DAQ_RELEASE_DIR ]]; then
    echo "Error: at least one of the environment variables needed by this script is unset. Exiting..." >&2
    exit 1
fi

## Step 1 -- obtain and set up spack
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}/spack-$SPACK_VERSION-gcc-$GCC_VERSION
mkdir -p $SPACK_EXTERNALS
cd $SPACK_EXTERNALS
wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz
tar xf v${SPACK_VERSION}.tar.gz
ln -s spack-${SPACK_VERSION} spack-installation
rm -f v${SPACK_VERSION}.tar.gz

source spack-${SPACK_VERSION}/share/spack/setup-env.sh

## Step 2 -- add spack repos
### Step 2.1 -- add spack repos for external packages maintained by DUNE DAQ

cp -pr $DAQ_RELEASE_DIR/spack-repos/externals $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals

### Step 2.2 -- add spack repos for DUNE DAQ pacakges

pushd $DAQ_RELEASE_DIR
python3 scripts/spack/make-release-repo.py -u \
-b develop \
-i configs/dunedaq/dunedaq-develop/release.yaml \
-t spack-repos/dunedaq-repo-template \
-r ${DAQ_RELEASE} \
-o ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}
popd

mv  ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE}

### Step 2.3 -- change spack repos.yaml to include the two repos created above

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE}
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT


## Step 3 -- update spack config

\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/config.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/modules.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
\cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/concretizer.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/

## Step 4 -- install compiler

spack compiler find
spack install gcc@${GCC_VERSION} +binutils arch=${ARCH}


spack load gcc@${GCC_VERSION}
spack compiler find
mv $HOME/.spack/linux/compilers.yaml  $SPACK_EXTERNALS/spack-0.20.0/etc/spack/defaults/linux/
spack compiler list

## Step 5 -- install dunedaq (externals + DAQ packages)

spack spec --reuse dunedaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}
spack install --reuse dunedaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}

# overwrite ssh config - in the future, this part should be done in daq-release/spack-repos/externals/packages/openssh/package.py 
SSH_INSTALL_DIR=$(spack location -i openssh)
\cp $DAQ_RELEASE_DIR/spack-repos/externals/packages/openssh/ssh_config $SSH_INSTALL_DIR/etc/

## Step 6 -- install llvm and qt

spack install --reuse llvm@15.0.7~omp_as_runtime %gcc@${GCC_VERSION} build_type=MinSizeRel arch=${ARCH}
spack install --reuse qt@5.15.9%gcc@${GCC_VERSION} arch=${ARCH}

## Step 7 -- remove DAQ packages and umbrella packages

spack uninstall --dependents daq-cmake externals devtools systems
```
