#!/bin/bash

if (( $# != 2 )); then
    echo "Usage: "$( basename $0 )" <nightly tag (NFD...)> <path-qualified name of buildcache for dune daq packages>" >&2
    exit 1
fi

nightly_tag=$1
buildcache_dir=$2

tmpdir=$( mktemp -d )
cd $tmpdir

. /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh
setup_dbt latest_v5

dbt-setup-release -n $nightly_tag || exit 1

# Leaving out fddaq~dev since it's a subset of fddaq+dev
top_level_specs="fddaq+dev dbe"

for top_level_spec in $top_level_specs ; do

    top_level_hash=$( spack find -l $top_level_spec | sed -r -n 's/(^\w{7}) .*/\1/p' )

    if [[ -z $top_level_hash ]]; then
	echo "There was a problem determining the Spack hash of the spec $top_level_spec for the $nightly_tag nightly; exiting..." >&2
	exit 2
    fi

    echo $top_level_hash

    hashes_to_install=$( spack spec -t -l -N /$top_level_hash |  sed -r -n '/Concretized/,${s/^\[[^-]\]\s+(\S+)\s+.*/\1/p}' )

    num_packages=$( echo $hashes_to_install | wc -w )

    if ! (( num_packages > 0 )); then
	echo "Problem determining which packages to install for $top_level_spec; exiting..." >&2
	exit 3
    fi
    echo

    package_counter=1
    for installed_hash in $hashes_to_install; do

	echo "Caching package $package_counter of $num_packages for $top_level_spec"
	cmd="spack buildcache push --only package --unsigned $buildcache_dir /$installed_hash"
	echo $cmd
	eval "$cmd" || exit 4
	package_counter=$(( package_counter + 1 ))
    done ;
done
    
spack buildcache update-index $buildcache_dir || exit 5

cmd="spack buildcache list --allarch"
echo "Running $cmd ..."
$cmd

echo "Script completed successfully"
rm -rf $tmpdir

exit 0

