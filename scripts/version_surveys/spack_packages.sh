# #!/bin/env

# Will want to "dbt-create -s -n last_fddaq" before calling the script as long as it is sourced

thisdir="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"  # if sourced

if [[ -z $DBT_AREA_ROOT ]]; then
    echo "Currently you need a work area environment set up for this script to work; returning..." >&2
    return 1
fi

if [[ $PWD != $DBT_AREA_ROOT ]]; then
    echo "Currently you need to be in the base of a work area environment for this script to work; returning..." >&2
    return 2
fi

if [[ ! -d $DBT_AREA_ROOT/.spack ]]; then
    echo "The work area needs to have been installed with the \"-s\" option (i.e., have a local Spack repo); this does not appear to be the case with $DBT_AREA_ROOT. Returning..." >&2
    return 3
fi

tmpdir=$( mktemp -d )
cd $tmpdir

echo "Running \"spack spec fddaq\", output kept internal to this script. This will take a moment..."
spack spec fddaq > $tmpdir/fddaq_spec.txt || return 3
echo "\"spack spec fddaq\" completed successfully"


external_packages=$( $thisdir/../list_packages.py develop externals )
coredaq_packages=$( $thisdir/../list_packages.py develop coredaq )
fddaq_packages=$( $thisdir/../list_packages.py develop fddaq )

# echo "EXTERNAL PACKAGES LISTED IN THE YAML FILE: "
# echo $external_packages

# echo
# echo "SPEC OF EACH OF THOSE PACKAGES"
# for pkg in $external_packages; do

#     grep $pkg $tmpdir/fddaq_spec.txt | sed -r 's/\s//g'

# done

cd $DBT_AREA_ROOT

if true; then

random_coredaq_package=$( echo $coredaq_packages | awk '{print $1}' )
coredaq_dir=$( spack location -p $random_coredaq_package )/..

random_fddaq_package=$( echo $fddaq_packages | awk '{print $1}' )
fddaq_dir=$( spack location -p $random_fddaq_package )/..

find $coredaq_dir -name package.py -not -regex ".*/externals/package.py"  | xargs sed -r -n 's/(depends_on.*)/\1/p' | sort | uniq > $tmpdir/sorted_coredaq_dependencies.txt

find $fddaq_dir -name package.py | xargs sed -r -n 's/(depends_on.*)/\1/p' | sort | uniq > $tmpdir/sorted_fddaq_dependencies.txt

cat $tmpdir/sorted_coredaq_dependencies.txt $tmpdir/sorted_fddaq_dependencies.txt | sort | uniq > $tmpdir/sorted_dependencies.txt

echo
for pkg in $external_packages; do

    if [[ -z $( grep -l $pkg $tmpdir/sorted_dependencies.txt ) ]]; then
	echo "$pkg doesn't appear to be an explicit dependency in any package.py file"
    fi
done

echo
for pkg in $( sed -r 's/^.*depends_on\(f*"([a-z\-_0-9]+).*/\1/' $tmpdir/sorted_dependencies.txt | tr "\n" " " ); do
    if [[ -n $( echo "$coredaq_packages" | grep $pkg ) || -n $( echo "$fddaq_packages" | grep $pkg ) ]]; then
	continue
    fi
    
    if [[ -z $( echo "$external_packages" | grep $pkg ) ]]; then
	echo "Unable to find $pkg in the externals from the YAML file; note this is not necessarily a problem"
    fi
done

fi

echo

# Prioritize the built in package.py over any that may have been
# copied into daq-release since available versions may have updated
# since the copy

# -> If I source this script multiple times I get "==> Error:
#  Repository is already registered with Spack" so swallow any
#  errors and just list repos, which will tell you their priorities

spack repo add $DBT_AREA_ROOT/.spack/var/spack/repos/builtin > /dev/null 2>&1
spack repo list
if [[ -z $( spack repo list | head -1 | grep "/builtin$" ) ]]; then
    echo "This script failed to prioritize the builtin repos; returning..." >&2
    return 4
fi


pwd
for pkg in $external_packages ; do

    echo
    echo
    echo $pkg

    echo -n "Current version: "
    sed -r -n 's/.*\s+\^'$pkg'@([^%]+).*/\1/p' $tmpdir/fddaq_spec.txt

    preferred_version=$( spack info $pkg | sed -n '/Preferred version/{n;p}' )
    echo -n "Preferred version is "$preferred_version

    location=$( spack location -p $pkg )
    if [[ "$location" =~ spack-repo-externals ]]; then
	echo ", according to vendored package.py"
    else
	echo ", according to builtin package.py"
    fi

    if [[ -d $thisdir/../../spack-repos/externals/packages/$pkg ]]; then
	echo "For this work area, using vendored package.py from daq-release for $pkg"

	if [[ -d $DBT_AREA_ROOT/.spack/var/spack/repos/builtin/packages/$pkg ]]; then
	    echo "package.py also found in builtin area"
	else
	    echo "No package.py supplied by builtin area"
	fi
    else
	echo "For this work area, using Spack builtin package.py for $pkg"
    fi

    archive_file_found=false
    if [[ -n $( echo $preferred_version | grep https | grep -v "\[git\]" ) ]]; then
	archive_file=$( echo $preferred_version | sed -r 's/.*(https\S+).*/\1/' )

	if [[ -n $archive_file ]]; then
	    echo "Archive file found, $archive_file"
	    archive_file_found=true
	else
	    echo "Internal script error: curiously unable to find archive file; returning..." >&2
	    return 5
	fi
    fi

    if $archive_file_found ; then
	curl -sI $archive_file | grep -i last-modified
    else
	echo "Unable to (automatically) determine date of preferred version"
    fi

    echo
    cmd="spack checksum --batch --latest $pkg"
    echo "Output of \"$cmd\":"
    $cmd
    echo
    
done

# Restore repo priority to where they'd been before
spack repo rm $DBT_AREA_ROOT/.spack/var/spack/repos/builtin || echo "Problem restoring repo priorities!"

rm -rf $tmpdir

