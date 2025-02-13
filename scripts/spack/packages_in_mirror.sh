
if [[ -z $1 ]]; then
    echo "You need to pass this script the spack hash (with \"/\" prefix); returning..." >&2
    return 1
fi

cache_package=$1
buildcache_name="spack-local-buildcache"

if [[ -z $DBT_AREA_ROOT ]]; then
    echo "Error: need a work area to be set up for this script to work. Returning..." >&2
    return 2
fi

if [[ $PWD != $DBT_AREA_ROOT ]]; then
    echo "Error: need to be in the base of a work area for this script to work. Returning..." >&2
    return 3
fi

function remove_buildcache() {

    echo "This function is a stub until it can be made safe against \"rm -rf\""
    #rm -rf $DBT_AREA_ROOT/$buildcache_name 
    #spack mirror rm mirror-of-${buildcache_name}
}

spack find $cache_package
retval=$?

if [[ $retval != 0 ]]; then
    echo "Need to install ${cache_package}, a dependency-free package"
    return 4
fi

# The sed command below is designed to give you the Spack hashes of
# the target package and all its dependencies *which are
# installed*. This way we can use the "--only package" argument to
# "spack buildcache push" to circumvent the error that occurs if you
# try pushing an installed package whose build-only dependencies have
# been deleted

for installed_hash in $( spack spec -t -l ${cache_package} | sed -r -n '/Concretized/,$s/^\[[^-]\]\s+(\S+)\s+.*/\1/p' ); do
    cmd="spack buildcache push --unsigned --only package $DBT_AREA_ROOT/$buildcache_name /$installed_hash"
    echo $cmd
    $cmd || return 5
done

mirrorname="mirror-of-${buildcache_name}"

if [[ -z $( spack mirror list | grep $mirrorname ) ]]; then
    spack mirror add --unsigned $mirrorname $DBT_AREA_ROOT/$buildcache_name || return 6
fi

spack mirror list

# Note that you need to give an absolute directory below,
# otherwise it seems like the index isn't updated the way you'd
# expect it to be
    
spack buildcache update-index $DBT_AREA_ROOT/$buildcache_name || return 7
spack buildcache list

echo "Now try uninstalling and re-installing $cache_package and perhaps some of its dependencies as described in https://spack.readthedocs.io/en/latest/binary_caches.html"

return 0

