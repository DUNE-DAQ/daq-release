#!/bin/bash

# JCF, Feb-25-2025:

# This is meant to be run in a docker container which has the following volume mounted:
# -This repo (daq-release), containing this script
# -A scratch directory, so you can write the buildcache directory to the outside world
# -An installation of the externals (*)

# So, e.g., from the dunedaq account I've done the following:

# docker run -it --name create_buildcache_file \
    #    -v $PWD/daq-release:/daq-release \
    #    -v $PWD/scratch:/scratch \
    #    -v $HOME/docker-scratch/cvmfs_dunedaq:/cvmfs/dunedaq.opensciencegrid.org \
    # ghcr.io/dune-daq/alma9-spack:latest


# (*) Note the externals need to be installed s.t. there aren't
# duplicate installations of a given package, and s.t. build-ext.sh
# hasn't uninstalled build-only dependencies and other external
# packages; this can be accomplished by simple commenting out of the
# relevant "spack uninstall ..." lines

######################################################################

# ghcr.io/dune-daq/alma9-spack:latest doesn't have "file", but Spack
# needs this

if [[ -z $( which file ) ]]; then

    yum install file -y || exit 1
fi

. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v2.2/spack-0.22.0/share/spack/setup-env.sh || exit 2

if [[ ! -d /scratch ]]; then
    echo "A /scratch directory hasn't been volume mounted in the container; exiting..." >&2
    exit 3
fi

buildcache_dir=/scratch/externals_buildcache

direct_external_installs="gcc boost cetlib trace nlohmann-json pistache highfive hdf5 libarchive libzmq cppzmq msgpack-c py-pybind11 uhal librdkafka protobuf grpc felix-software folly cli11 intel-tbb dpdk fmt py-moo py-anyconfig py-jsonnet rclone libtorrent cyrus-sasl libevent qt"

for package in $direct_external_installs ; do

    echo
    echo "Pushing $package into the buildcache"
    
    spack_hash=$( spack find -l $package | sed -n -r 's/^(\w{7}) .*/\1/p' )
    if [[ -z $spack_hash ]]; then
	echo "Unable to get hash for $package; exiting..." >&2
	exit 4
    fi

    spack buildcache push --unsigned $buildcache_dir /${spack_hash} 
    retval=$?

    if [[ "$retval" != "$?" ]]; then
	echo "Unable to push hash \"$spack_hash\" for package $package; exiting..." >&2
	exit 5
    fi
done

spack buildcache update-index $buildcache_dir || exit 6

echo "Script completed successfully; buildcache dir is $buildcache_dir in this container" >&2
exit 0

