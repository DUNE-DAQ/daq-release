#!/bin/bash

if (( $# != 4 )); then
    echo "Usage: $( basename $0 ) <full path of externals installation> <full path of target installation> <full path of cache directory> <tag of nightly stored in cache directory (NFD...) >" >&2
    exit 1
fi

externals_dir=$1
installation_dir=$2
buildcache_dir=$3
nightly_tag=$4

tag_no_prefix=$( echo $nightly_tag | sed -r 's/^[^_]+(.*)/\1/' )

. "$(dirname "$(realpath "$0")")"/caching_tools.sh || exit 5

mkdir -p $installation_dir

# SPACK_EXTERNALS is used in build-release.sh's plumbing to find the externals packages
export SPACK_EXTERNALS=$externals_dir

cmd="$DAQ_RELEASE_DIR/scripts/spack/build-release.sh $installation_dir/NB${tag_no_prefix} $installation_dir/NFD${tag_no_prefix} core almalinux9 develop $buildcache_dir"

echo $cmd
eval "$cmd" || exit 3

cmd="$DAQ_RELEASE_DIR/scripts/spack/build-release.sh $installation_dir/NB${tag_no_prefix} $installation_dir/NFD${tag_no_prefix} fd almalinux9 develop $buildcache_dir"

echo $cmd
eval "$cmd" || exit 4

exit 0
