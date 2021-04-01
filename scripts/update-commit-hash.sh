#!/bin/bash

function update_hash {
    iprd_arr="$@"
    iprd_arr="${iprd_arr#\"}"
    iprd_arr="${iprd_arr%\"}"
    iprd_arr=($iprd_arr)
    echo ${iprd_arr[1]}
    prod_name=${iprd_arr[0]//_/-}
    prod_branch=${iprd_arr[1]}
    if [ "$branch_name" != "not_set" ]; then
        prod_branch=$branch_name
    fi
    echo "$prod_name -- $prod_branch"
    git clone git@github.com:DUNE-DAQ/${prod_name}.git -b ${prod_branch}
    cd ${prod_name}
    commit_hash=`git rev-parse --short HEAD`
    echo "${prod_name} ${commit_hash}"
    sed -i "/.*${prod_name}.*/c\  \"${prod_name} ${commit_hash}\"" $release_manifest_file
    cd ..
}

update_all=0
branch_name="not_set"
release_manifest_file="./release_manifest.sh"
while getopts ":f:p:b:ah" opt; do
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
    a )
       update_all=1
       ;;
    h )
      echo "Usage:"
      echo "    update-package-hash.sh  -h Display this help message."
      echo "    <-f> <release manifest file>"
      echo "    [-p] <package name>"
      echo "    [-b] <branch name>"
      echo "    [-a] <update all packages' hash>"
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

tmp_dir=$(mktemp -d -t dunedaq_src_repo_XXXXXXXXXX)
release_manifest_file=`realpath $release_manifest_file`

pushd $tmp_dir

if [ "$update_all" -eq "1" ]; then
    for ((i=0; i < ${#dune_packages[@]}; i++)); do
        update_hash ${dune_packages[i]}
    done
else
    if grep -q $package_name "$release_manifest_file"; then
	prd_array=$(grep $package_name "$release_manifest_file")
	update_hash $prd_array
    else
        echo "Error: package is not included in $release_manifest_file, exit now"
	exit 3
    fi
fi
popd

rm -rf $tmp_dir
