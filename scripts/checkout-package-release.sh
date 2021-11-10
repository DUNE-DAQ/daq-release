#!/bin/bash

function checkout_package {
    iprd_arr="$@"
    iprd_arr="${iprd_arr#\"}"
    iprd_arr="${iprd_arr%\"}"
    iprd_arr=($iprd_arr)
    echo ${iprd_arr[1]}
    prod_name=${iprd_arr[0]//_/-}
    prod_branch=${iprd_arr[1]}
    prod_ups_version=$(echo "$prod_branch" | tr '_' '.')
    if [[ $prod_branch == v* ]]; then
        prod_branch=$(echo "$prod_branch" | tr [a-g] ' '|tr '_' '.')
    fi
    if [ "$branch_name" != "not_set" ]; then
        prod_branch=$branch_name
    fi
    echo "$prod_name -- $prod_branch"
    git clone https://github.com/DUNE-DAQ/${prod_name}.git
    cd ${prod_name}
    git checkout ${prod_branch}
    if [ "$prod_branch" != "$prod_ups_version" ]; then
        git tag ${prod_ups_version}
	echo "INFO: creating local tag ${prod_ups_version} for ${prod_name}"
    fi
    cd ..
}

update_all=0
branch_name="not_set"
release_manifest_file="./release_manifest.sh"
outdir="sourcecode"
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
       NO_RELEASE_CHECK=true
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
        if $NO_RELEASE_CHECK; then
            prd_array="$package_name $branch_name e19:prof"
	    checkout_package $prd_array
	else
            echo "Error:  "NO_RELEASE_CHECK" is not turned on and package is not included in $release_manifest_file, exit now"
	    exit 3
	fi
    fi
fi
popd

