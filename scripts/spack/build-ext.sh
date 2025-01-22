#!/bin/bash

# Example usage:

# docker run -it --name <some useful name> \
    # -e "EXT_VERSION=2.2" -e "DAQ_RELEASE=EXT2.2ADD" \
    # -e "SPACK_VERSION=0.22.0" -e "GCC_VERSION=13.2.0" -e "ARCH=linux-almalinux9-x86_64" \
    # -v <location of freshly-checked-out daq-release repo>:/daq-release \
    # -v <location of local directory for log output>:/log \
    # -v <location of local area for installation>:/cvmfs/dunedaq.opensciencegrid.org \
    # ghcr.io/dune-daq/alma9-spack:latest

# When in the container, run:
# /daq-release/scripts/spack/build-ext.sh <niceness> (false)   # Adding false means script tries to pick up where it left off

niceness=$1

if ! [[ $niceness =~ ^[0-9]+$ ]]; then
    echo 'Need to pass a first argument which is the integer passed to the Linux "nice" command for computation-intensive actions; please make this 0 or greater' >&2
    exit 12
fi

if [[ -z $2 ]]; then
    fresh_build=true
else
    fresh_build=$2
fi

if [[ ! -n $EXT_VERSION || ! -n $SPACK_VERSION || ! -n $GCC_VERSION || ! -n $ARCH || ! -n $DAQ_RELEASE ]]; then
    echo "Error: at least one of the environment variables needed by this script is unset. Exiting..." >&2
    exit 1
fi

starttime=$( date )

thisdir="$(dirname "$(realpath "$0")")"

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

source $SPACK_EXTERNALS/spack-${SPACK_VERSION}/share/spack/setup-env.sh || exit 6

## Step 2 -- add spack repos

if $fresh_build; then

### Step 2.0 -- wipe out any pre-existing repos
    rm -rf $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals \
       $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo \
       $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-${DAQ_RELEASE} \
       $SPACK_ROOT/etc/spack/defaults/repos.yaml

### Step 2.1 -- add spack repos for external packages maintained by DUNE DAQ

    cp -rp $DAQ_RELEASE_DIR/spack-repos/externals $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals
    find $SPACK_EXTERNALS/spack-${SPACK_VERSION}/spack-repo-externals | xargs chmod a+rx  # Needed to ensure users' work areas will be able to access files here

### Step 2.2 -- add spack repos for DUNE DAQ packages

    pushd $DAQ_RELEASE_DIR

    cmd="python3 scripts/spack/make-release-repo.py -u \
-b develop \
-i configs/coredaq/coredaq-develop/release.yaml \
-t spack-repos/coredaq-repo-template \
-r ${DAQ_RELEASE} \
-o ${SPACK_EXTERNALS}/spack-${SPACK_VERSION} \
-b develop
"

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

    cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/config.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
    cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/modules.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/
    cp  $DAQ_RELEASE_DIR/misc/spack-${SPACK_VERSION}-config/concretizer.yaml $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/

fi # $fresh_build

## Step 4 -- install compiler

spack compiler find

spack find gcc@${GCC_VERSION} +binutils arch=${ARCH}
retval=$?

if $fresh_build || [[ "$retval" != "0" ]]; then
    nice -n $niceness spack install gcc@${GCC_VERSION} +binutils arch=${ARCH} |& tee /log/spack_install_gcc.txt || exit 8
fi

spack load gcc@${GCC_VERSION} || exit 12
spack compiler find || exit 13

if [[ -e $HOME/.spack/linux/compilers.yaml ]]; then
    mv $HOME/.spack/linux/compilers.yaml  $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
fi
spack compiler list

gcc_hash=$( spack find -l --loaded gcc@${GCC_VERSION} | sed -r -n 's/^(\w{7}) gcc.*/\1/p' )

spack clean -a

cp -rp $DAQ_RELEASE_DIR/spack-repos/externals/packages/umbrella $(spack location -p coredaq)

## Step 5 -- check all specs, then install

coredaq_spec="coredaq@${DAQ_RELEASE}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH}"

dbe_spec="dbe%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=${ARCH} ^qt@5.15.9:~sql~ssl~tools"

boost_spec="boost@1.85.0%gcc@${GCC_VERSION}+atomic+chrono~clanglibcpp+container+context~contract~coroutine+date_time~debug+exception~fiber+filesystem+graph~graph_parallel~icu+iostreams~json+locale+log+math~mpi+multithreaded~nowide~numpy~pic+program_options~python+random+regex+serialization+shared+signals~singlethreaded~stacktrace+system~taggedlayout+test+thread+timer~type_erasure~versionedlayout+wave"

llvm_spec="llvm@18.1.3%gcc@${GCC_VERSION}~gold~libomptarget~lld~lldb~lua~polly build_type=MinSizeRel compiler-rt=none libcxx=none libunwind=none targets=none arch=${ARCH}"

# Prevent a second build of gcc@${GCC_VERSION}
gcc_spec="/${gcc_hash}"

umbrella_spec="umbrella ^$gcc_spec ^$coredaq_spec ^$dbe_spec ^$llvm_spec ^$boost_spec"

echo $umbrella_spec

if $fresh_build || [[ ! -e umbrella_build_semaphore ]]; then
    nice -n $niceness spack spec -l -t --reuse $umbrella_spec |& tee /log/spack_spec_umbrella.txt || exit 9
    nice -n $niceness spack install --reuse $umbrella_spec |& tee /log/spack_install_umbrella.txt || exit 10

    rm -f umbrella_build_semaphore
    echo "The existence of this file means the umbrella package was built" > umbrella_build_semaphore
else
    echo "Spotted a file called $PWD/umbrella_build_semaphore; will skip spack install of the umbrella package"
fi

## Step 6 -- install graphviz, useful for generating plots after
## daqconf's create_config_plot program has created a DOT file

spack install --reuse graphviz@8.0.5%gcc@${GCC_VERSION}~doc+expat~ghostscript~gtkplus~gts~java~libgd~pangocairo~poppler~qt~quartz~x build_system=autotools arch=${ARCH} || exit 11

## Step 7 -- remove DAQ packages and umbrella packages

for pkg in daq-cmake externals devtools systems; do
    echo "Uninstalling $pkg"
    spack uninstall -y --all --dependents $pkg || echo "Spack uninstall of $pkg returned nonzero"
done

# Step 8 -- remove any unneeded externals (build-only packages, and those which are dependencies of build-only packages only)

. $SPACK_EXTERNALS/spack-${SPACK_VERSION}/share/spack/setup-env.sh

build_only_packages=$( cat /log/spack_spec_umbrella.txt | sed -r -n 's/.*\[b   \] +\^([^@]+).*/\1/p' )

for pkg in $build_only_packages; do
    echo "Uninstalling $pkg"
    spack uninstall -y $pkg || echo "Spack uninstall of $pkg returned nonzero"
done

# Now packages which are dependencies of build-only packages
for pkg in py-hatch-vcs py-setuptools-scm py-typing-extensions go-bootstrap git libidn2 docbook-xsl docbook-xml go libunistring gmake diffutils sed libtool bison flex autoconf automake openssl; do
    echo "Uninstalling $pkg"
    spack uninstall -y $pkg || echo "Spack uninstall of $pkg returned nonzero; this likely means it had already been uninstalled"
done

spack find -l | sort |& tee /log/externals_list.txt

endtime=$( date )

echo "Externals build complete"
echo "Start time: $starttime"
echo "End time: $endtime"

