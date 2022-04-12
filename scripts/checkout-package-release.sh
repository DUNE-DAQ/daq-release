#!/bin/bash

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

function checkout_package {
    iprd_arr="$@"
    iprd_arr="${iprd_arr#\"}"
    iprd_arr="${iprd_arr%\"}"
    iprd_arr=($iprd_arr)
    #echo ${iprd_arr[1]}
    prod_name=${iprd_arr[0]//_/-}
    prod_branch=${iprd_arr[1]}
    prod_ups_version=${iprd_arr[1]}

    echo "INFO: -------- $prod_name ---------- $prod_branch"

    git clone --quiet https://github.com/DUNE-DAQ/${prod_name}.git
    cd ${prod_name}

    if [[ $prod_branch == v* ]]; then
	prod_ups_version=$(echo "$prod_ups_version" | tr '_' '.')
        prod_branch=$(echo "$prod_branch" | tr [a-k] ' '|tr '_' '.')
	prod_branch="${prod_branch%"${prod_branch##*[![:space:]]}"}"

    fi

    if [ "$branch_name" != "not_set" ]; then
	if git ls-remote --exit-code --heads origin $branch_name; then
	    prod_branch=$branch_name
        else
	    echo "INFO: remote branch $branch_name does not exist"
            if $NO_DEV_CHECKOUT; then 
		echo "INFO: skipping package checkout for $prod_name"
		cd ..; rm -rf ${prod_name}
                return 1;
	    else
	        echo "INFO: using branch name from release_manifest.sh."
            fi
	fi
    fi

    #echo "INFO: checking out $prod_name, uisng branch/tag $prod_branch"
    git checkout --quiet ${prod_branch}

    if [[ $prod_branch == v* ]]; then
        if [ "$prod_branch" != "$prod_ups_version" ]; then
            git tag ${prod_ups_version}
	    echo "INFO: creating local tag ${prod_ups_version} for ${prod_name}"
        fi
	# Check version number in CMakeLists.txt
	cmake_version=`grep "^project" CMakeLists.txt |grep ")$"|grep -oP "(([[:digit:]]+\.)([[:digit:]]+\.)([[:digit:]]+))"`
	if [[ $prod_branch != "v$cmake_version" ]]; then
	    echo -e '\033[0;31m' "ERROR: cmake package version does not match git tag. Pkg: ${prod_name} Tag: ${prod_branch} VERSION ${cmake_version}"
	    echo -e '\033[0m'
        else
	    echo -e '\033[0;32m'"INFO: cmake package version matches git tag. Pkg: ${prod_name} Tag: ${prod_branch} VERSION ${cmake_version}"
	    echo -e '\033[0m'

	fi
    fi

    cd ..
}

update_all=0
branch_name="not_set"
release_manifest_file="./release_manifest.sh"
outdir="sourcecode"
NO_DEV_CHECKOUT=false
while getopts ":f:p:b:no:ah" opt; do
  case ${opt} in
    f )
       release_manifest_file=$OPTARG
       ;;
    p )
       package_name=$OPTARG
       ;;
    b )
       branch_name=$OPTARG
       ;;
    n )
       NO_DEV_CHECKOUT=true
       ;;
    o )
       outdir=$OPTARG
       ;;
    a )
       update_all=1
       ;;
    h )
      echo "Usage:"
      echo "    checkout-package.sh  -h Display this help message."
      echo "    <-f> <release manifest file>"
      echo "    [-o] <output sourcecode directory>"
      echo "    [-p] <package name>"
      echo "    [-b] <branch name>"
      echo "    [-a] <checkout all DAQ packages>"
      echo "Examples:"
      echo "    checkout-package-release.sh -f <path_to_release_manifest.sh> -a"
      echo "    checkout-package-release.sh -f <path_to_release_manifest.sh> -p daq-cmake"
      exit 0
      ;;
   \? )
     echo "Invalid Option: -$OPTARG" 1>&2
     exit 1
     ;;
  esac
done


shift $((OPTIND -1))

if [ ! -f "$release_manifest_file" ]; then
    echo "[Error]: $release_manifest_file is not found, exit now."
    exit 2
else
    source $release_manifest_file
fi

release_manifest_file=`realpath $release_manifest_file`

mkdir -p $outdir
pushd $outdir

if [ "$update_all" -eq "1" ]; then
    for ((i=0; i < ${#dune_packages[@]}; i++)); do
        checkout_package ${dune_packages[i]}
    done
else
    if grep -q \"$package_name\  "$release_manifest_file"; then
	prd_array=$(grep \"$package_name\  "$release_manifest_file")
	checkout_package $prd_array
    else
	echo "Error: package $package_name is not included in $release_manifest_file, exit now"
	exit 3
    fi
fi
popd

