#!/bin/bash

# You'll want to run this in a container which is set up, e.g., in the following manner:

# docker run -it --name build-from-cache \
    #       -v <daq release repo>:/daq-release \
    #       -v <directory with needed spack buildcache>:/cachedir \
    #        ghcr.io/dune-daq/alma9-slim-externals:v2.2

# ...where cachedir is a local directory that contains a XXXXXXXXX

# Then run
#
# /daq-release/scripts/spack/build-release-from-cache.sh
#

NIGHTLY_TAG="_DEV_250217_A9"  # Could (should) make this a passable environment
			      # variable. Likewise the name of the
			      # buildcache file.

# Make the buildcache available to build-release.sh for rapid binary installation
. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v2.2/spack-0.22.0/share/spack/setup-env.sh
spack mirror add local-buildcache file:///cachedir/spack-local-buildcache1
spack mirror set local-buildcache --unsigned   # REMOVE THIS LINE unless you're doing local testing

random_directory=/my/new/area
mkdir -p $random_directory

/daq-release/scripts/spack/build-release.sh $random_directory/NB${NIGHTLY_TAG} $random_directory/NFD${NIGHTLY_TAG} core almalinux9 develop

