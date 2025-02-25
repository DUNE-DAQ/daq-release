#!/bin/bash

if (( $# != 2 )); then
    echo "Usage: "$( basename $0 )" <relative installation directory> <path-qualified name of buildcache with externals>" >&2
    exit 1
fi

installation_dir=$1
buildcache_dir=$2

if [[ ! -e $buildcache_dir ]]; then
    echo "Unable to find requested buildcache directory $buildcache_dir; exiting..." >&2
    exit 1
fi


if [[ -d $installation_dir ]]; then
    echo "Error: directory \"$installation_dir\" can't exist if you want to run this script. Exiting..." >&2
    exit 1
fi

if [[ -e $HOME/.spack ]]; then
    echo "An account-wide Spack directory \"$HOME/.spack\" has been found which could interfere with the running of this script. Exiting..." >&2
    exit 2
fi

export DAQ_RELEASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"/../..

mkdir $installation_dir
cd $installation_dir
git clone https://github.com/spack/spack.git -b v0.22.0 spack-0.22.0

. $PWD/spack-0.22.0/share/spack/setup-env.sh

spack mirror add --unsigned externals-mirror file://$buildcache_dir || exit 3

cp -rp $DAQ_RELEASE_DIR/spack-repos/externals spack-0.22.0/spack-repo-externals
find spack-0.22.0/spack-repo-externals | xargs chmod a+rx  # Needed to ensure users' work areas will be able to access files here

cat <<EOT > $SPACK_ROOT/etc/spack/defaults/repos.yaml
repos:
  - ${PWD}/spack-0.22.0/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOT


cp $DAQ_RELEASE_DIR/misc/spack-0.22.0-config/config.yaml $PWD/spack-0.22.0/etc/spack/defaults/
cp $DAQ_RELEASE_DIR/misc/spack-0.22.0-config/modules.yaml $PWD/spack-0.22.0/etc/spack/defaults/
cp $DAQ_RELEASE_DIR/misc/spack-0.22.0-config/concretizer.yaml $PWD/spack-0.22.0/etc/spack/defaults/

# This function relies on there being one, and only one, version of the package in the buildcache(s)

function install_package() {
    local pkg=$1

    local pkg_hash=$(spack buildcache list -l --allarch | sed -r -n 's/^(\w{7}) '$pkg'@.*/\1/p')

    cmd="spack install /$pkg_hash"
    $cmd

    if [[ $? != 0 ]]; then
	echo "Problem running \"$cmd\"; exiting..." >&2
	exit 5
    fi
}

install_package gcc

spack load gcc@13.2.0 || exit 6
spack compiler find

direct_external_installs="boost cetlib trace nlohmann-json pistache highfive hdf5 libarchive libzmq cppzmq msgpack-c py-pybind11 uhal librdkafka protobuf grpc felix-software folly cli11 intel-tbb dpdk fmt py-moo py-anyconfig py-jsonnet rclone libtorrent cyrus-sasl libevent"

for package in $direct_external_installs ; do
    install_package $package
done

echo "Script completed successfully"
exit 0
