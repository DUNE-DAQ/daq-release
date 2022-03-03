#!/bin/bash

function check_package_ci {
    iprd_arr="$@"
    iprd_arr="${iprd_arr#\"}"
    iprd_arr="${iprd_arr%\"}"
    iprd_arr=($iprd_arr)
    prod_name=${iprd_arr[0]//_/-}
    ci_status=$(gh run list -R DUNE-DAQ/$prod_name|grep schedule |head -n 1)
    echo "$prod_name $ci_status"
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
    a )
       update_all=1
       ;;
    h )
      echo "Usage:"
      echo "    checkout-package.sh  -h Display this help message."
      echo "    <-f> <release manifest file>"
      echo "    [-p] <package name>"
      echo "    [-b] <branch name>"
      echo "    [-a] <checkout all DAQ packages>"
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

tmp_dir=$(mktemp -d -t ci_status_dunedaq_XXXXXXXXXX)
pushd $tmp_dir
git clone --quiet https://github.com/DUNE-DAQ/daq-release > /dev/null
cd daq-release

if [ "$update_all" -eq "1" ]; then
    for ((i=0; i < ${#dune_packages[@]}; i++)); do
        check_package_ci ${dune_packages[i]}
    done
else
    prd_array="$package_name $branch_name"
    check_package_ci $prd_array
fi
popd

rm -rf $tmp_dir
