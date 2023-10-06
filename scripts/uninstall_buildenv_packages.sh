

if [[ ! -n $EXT_VERSION || ! -n $SPACK_VERSION || ! -n $GCC_VERSION ]]; then
    echo "Error: at least one of the environment variables needed by this script is unset. Exiting..." >&2
    exit 1
fi


. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}/spack-${SPACK_VERSION}-gcc-${GCC_VERSION}/spack-installation/share/spack/setup-env.sh

# For externals:
# cetmodules a build-only dependency
# cli11 and cppzmq are header-only
# cmake and gdb not needed for a run time environment
# ninja appears to be needed by "spack load". Therefore not in this list...
externals_to_remove="cetmodules cli11 cmake gdb cppzmq grpc"

python_packages_to_remove="py-pip py-wheel py-hatch-fancy-pypi-readme py-flit-core py-hatch-vcs py-calver py-cython"

# Packages dbe depends on which aren't part of fddaq
dbe_packages_to_remove="bdftopcf mkfontdir gperf which flex findutils nasm xkbcomp"

# The go language is a build-only dependency of rclone. Not needed after rclone has been built.
go_packages_to_remove="go go-bootstrap git libidn2 libunistring"

llvm_packages_to_remove="llvm libsigsegv perl-data-dumper"

# Other packages which are removed here are at least one of the following
# (1) build-only dependencies (e.g. czmq)
# (2) clearly used for development purposes only (e.g., autoconf)
# (3) header-only (e.g. cppzmq)

for package in ca-certificates-mozilla texinfo meson autoconf-archive autoconf \
	       automake gmake libtool docbook-xsl docbook-xml czmq yaml-cpp gawk \
	       bison diffutils re2c m4 \
	       $go_packages_to_remove \
	       $dbe_packages_to_remove \
	       $python_packages_to_remove \
	       $llvm_packages_to_remove \
               $externals_to_remove ; do
    spack uninstall -y --all $package
done

# Although a link dependency of cetlib, catch2 is only a
# "multi-paradigm test framework" for C++. Not needed in a run
# environment.

for package in catch2; do
    spack uninstall -y --all --force $package
done
