
if [[ -z $1 ]]; then
    echo "You need to pass this script the spack hash (with \"/\" prefix); returning..." >&2
    return 1
fi

cache_package=$1
buildcache_name="spack-local-buildcache"
mirrorname="mirror-of-${buildcache_name}"

if [[ -z $DBT_AREA_ROOT ]]; then
    echo "Error: need a work area to be set up for this script to work. Returning..." >&2
    return 2
fi

if [[ $PWD != $DBT_AREA_ROOT ]]; then
    echo "Error: need to be in the base of a work area for this script to work. Returning..." >&2
    return 3
fi

function remove_buildcache() {

    local buildcache_name=$1
    
    if [[ -n $buildcache_name && $buildcache_name != "" ]]; then
	rm -rf $DBT_AREA_ROOT/$buildcache_name
	spack mirror rm $mirrorname
    else
	echo "ERROR: remove_buildcache called but \$buildcache_name not set; no action will be taken..." >&2
    fi	
}

spack find $cache_package
retval=$?

if [[ $retval != 0 ]]; then
    echo "Need to install ${cache_package}, a dependency-free package"
    return 4
fi

# The sed command below is designed to give you the Spack hashes of
# the target package and all its dependencies (1) which are installed
# and (2) which aren't part of our externals. This way we can use the
# "--only package" argument to "spack buildcache push" to circumvent
# the error that occurs if you try pushing an installed package whose
# build-only dependencies have been deleted

hashes_to_install=$( spack spec -t -l -N ${cache_package} |  sed -r -n '/Concretized/,${/\^builtin|\^dunedaq-externals/d;s/^\[[^-]\]\s+(\S+)\s+.*/\1/p}' )

num_packages=$( echo $hashes_to_install | wc -w )

if ! (( num_packages > 0 )); then
    echo "Problem determining which packages to install; returning..." >&2
    return 5
fi

package_counter=1

for installed_hash in $hashes_to_install; do

    echo "Installing package $package_counter of $num_packages"
    cmd="spack buildcache push --unsigned --only package $DBT_AREA_ROOT/$buildcache_name /$installed_hash"
    echo $cmd
    $cmd || return 5
    package_counter=$(( package_counter + 1 ))
done

if [[ -z $( spack mirror list | grep $mirrorname ) ]]; then
    spack mirror add --unsigned $mirrorname $DBT_AREA_ROOT/$buildcache_name || return 6
fi

echo
echo
spack mirror list
echo

# Note that you need to give an absolute directory below,
# otherwise it seems like the index isn't updated the way you'd
# expect it to be
    
spack buildcache update-index $DBT_AREA_ROOT/$buildcache_name || return 7

cmd="spack buildcache list --allarch"
echo "Running $cmd ..."
$cmd

echo "Script completed successfully"

return 0

