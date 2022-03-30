#!/bin/bash

function update_hash {
    iprd_arr="$@"
    iprd_arr="${iprd_arr#\"}"
    iprd_arr="${iprd_arr%\"}"
    iprd_arr=($iprd_arr)
    orig_prod_name=${iprd_arr[0]}
    prod_name=${iprd_arr[0]//_/-}
    prod_branch=${iprd_arr[1]}
    if [ "$branch_name" != "not_set" ]; then
        prod_branch=$branch_name
	slack_ver=$release
    fi
    if [[ $prod_branch == v* ]]; then
        prod_branch=$(echo "$prod_branch" | tr [a-g] ' '|tr '_' '.')
	prod_branch="${prod_branch%"${prod_branch##*[![:space:]]}"}"
	slack_ver=$(echo "$prod_branch" | tr [a-z] ' ')
    fi
    git clone --quiet git@github.com:DUNE-DAQ/${prod_name}.git
    cd ${prod_name}
    git checkout --quiet ${prod_branch}
    commit_hash=`git rev-parse --short HEAD`
    echo "        - name: ${prod_name}"
    echo "          version: ${slack_ver}"
    echo "          commit: ${commit_hash}"
    cd ..
}

update_all=0
branch_name="not_set"
release="dunedaq-develop"
release_manifest_file="./release_manifest.sh"
while getopts ":f:p:b:r:ah" opt; do
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
    r )
       release=$OPTARG
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
      echo "    [-r] <release name>"
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
    if grep -q \"$package_name\  "$release_manifest_file"; then
	prd_array=$(grep \"$package_name\  "$release_manifest_file")
	update_hash $prd_array
    else
        echo "Error: $package_name is not included in $release_manifest_file, exit now"
	exit 3
    fi
fi
popd

rm -rf $tmp_dir
