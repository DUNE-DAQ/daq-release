#!/bin/bash

if (( $# != 4 )); then
    echo "Usage: $( basename $0 ) <full path of target installation directory> <full path of cache directory> <tag of nightly in cache directory (NFD...) > <full path of externals directory>" >&2
    exit 1
fi

installation_dir=$1
buildcache_dir=$2
nightly_tag=$3
externals_dir=$4

tag_no_prefix=$( echo $nightly_tag | sed -r 's/^[^_]+(.*)/\1/' )

if [[ -e $HOME/.spack ]]; then
    echo "An account-wide Spack directory \"$HOME/.spack\" has been found which could interfere with the running of this script. Exiting..." >&2
    exit 2
fi

mkdir -p $installation_dir

script_dir="$(dirname "$(realpath "$0")")"  # This script is expected to share a directory with build-release.sh

# SPACK_EXTERNALS is used in build-release.sh's plumbing to find the externals packages
export SPACK_EXTERNALS=$externals_dir

cmd="$script_dir/build-release.sh $installation_dir/NB${tag_no_prefix} $installation_dir/NFD${tag_no_prefix} core almalinux9 develop $buildcache_dir"

echo $cmd
$cmd || exit 3

cmd="$script_dir/build-release.sh $installation_dir/NB${tag_no_prefix} $installation_dir/NFD${tag_no_prefix} fd almalinux9 develop $buildcache_dir"

echo $cmd
$cmd || exit 4

exit 0
