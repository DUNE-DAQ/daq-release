#!/bin/bash

if (( $# != 2 )); then
    echo "Usage: $( basename $0 ) <path-qualified cache directory name> <tag of nightly in cache directory (NFD...) >" >&2
    exit 1
fi

BUILDCACHE_DIR=$1
NIGHTLY_TAG=$2

INSTALL_DIRECTORY=$HOME/buildcache_installed_packages

TAG_NO_PREFIX=$( echo $NIGHTLY_TAG | sed -r 's/^[^_]+(.*)/\1/' )

# Make the buildcache available to build-release.sh for rapid binary installation
. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v2.2/spack-0.22.0/share/spack/setup-env.sh || exit 1

spack mirror rm script-created-mirror
spack mirror add script-created-mirror file://$BUILDCACHE_DIR || exit 2

spack mirror set script-created-mirror --unsigned || exit 3  # REMOVE THIS LINE unless you're doing local testing

mkdir -p $INSTALL_DIRECTORY

SCRIPT_DIR="$(dirname "$(realpath "$0")")"

$SCRIPT_DIR/build-release.sh $INSTALL_DIRECTORY/NB${TAG_NO_PREFIX} $INSTALL_DIRECTORY/NFD${TAG_NO_PREFIX} core almalinux9 develop || exit 3

$SCRIPT_DIR/build-release.sh $INSTALL_DIRECTORY/NB${TAG_NO_PREFIX} $INSTALL_DIRECTORY/NFD${TAG_NO_PREFIX} fd almalinux9 develop || exit 4

exit 0
