
# Start with externals

. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v1.1/spack-0.20.0-gcc-12.1.0/spack-installation/share/spack/setup-env.sh

# cetmodules a build-only dependency
# cli11 is header-only
# cmake, ninja, gdb not needed for a run time environment
externals_to_remove="cetmodules cli11 cmake ninja gdb"

# Other packages which are removed here are at least one of the following
# (1) build-only dependencies
# (2) clearly used for development purposes only
# (3) header-only (e.g. czmq)

for package in ca-certificates-mozilla texinfo meson autoconf-archive autoconf \
	       automake libtool czmq docbook-xsl docbook-xml cppzmq yaml-cpp gawk \
	       bison diffutils re2c \
	       bdftopcf mkfontdir gperf which flex findutils nasm xkbcomp \
	       py-pip py-wheel py-hatch-fancy-pypi-readme py-flit-core py-hatch-vcs py-calver py-cython \
               $externals_to_remove ; do
    spack uninstall -y --all $package
done

# Although a link dependency of cetlib, catch2 is only a
# "multi-paradigm test framework" for C++. Not needed in a run
# environment.

for package in catch2; do
    spack uninstall -y --all --force $package
done
