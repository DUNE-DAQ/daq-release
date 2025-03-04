#!/bin/bash

if (( $# != 2 )); then
    echo "Usage: "$( basename $0 )" <new installation directory> <path-qualified name of buildcache with externals>" >&2
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

. "$(dirname "$(realpath "$0")")"/caching_tools.sh || exit 5

mkdir $installation_dir
cd $installation_dir

download_and_setup_spack

mirrorname=externals-mirror

if [[ -n $( spack mirror list | awk '{print $1}' | grep $mirrorname ) ]] ; then
    echo "Found mirror with name $mirrorname already existing; will remove"
    spack mirror rm $mirrorname 
fi

spack mirror add --unsigned $mirrorname file://$buildcache_dir || exit 3

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

spack buildcache list -l --allarch

function install_package() {
    local pkg=$1

    echo "Searching for hash of package $pkg"
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
cp ~/.spack/linux/compilers.yaml $PWD/spack-0.22.0/etc/spack/defaults/

for package in $EXTERNALS_PACKAGES ; do
    install_package $package
done

echo "Script completed successfully"
exit 0
