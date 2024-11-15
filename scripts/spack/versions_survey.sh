# #!/bin/env

# Will want to "dbt-setup-release -n last_fddaq" before calling the script as long as it is sourced

thisdir="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"  # if sourced
#thisdir="$(dirname "$(realpath "$0")")"                # if executed

tmpdir=$( mktemp -d )
cd $tmpdir
spack spec fddaq > $tmpdir/fddaq_spec.txt || return 1

external_packages=$( $thisdir/../list_packages.py develop externals )
coredaq_packages=$( $thisdir/../list_packages.py develop coredaq )
fddaq_packages=$( $thisdir/../list_packages.py develop fddaq )

echo "EXTERNAL PACKAGES LISTED IN THE YAML FILE: "
echo $external_packages

echo
echo "SPEC OF EACH OF THOSE PACKAGES"
for pkg in $external_packages; do

    grep $pkg $tmpdir/fddaq_spec.txt | sed -r 's/\s//g'
done

cd -

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

rm -rf $tmpdir

