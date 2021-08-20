#!/bin/bash

release_name="dunedaq-v2.8.0"
release_config_dir="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0

function git_checkout_and_tag {
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    prod_ups_version=${iprd_arr[1]//_/.}
    prod_version=${prod_ups_version//[^v.[:digit:]]/}
    git clone git@github.com:DUNE-DAQ/${prod_name}.git -b ${prod_version}
    pushd ${prod_name}
    git tag ${release_name}
    git push origin ${release_name}
    popd
  done
}

while getopts ":f:r:h" opt; do
  case ${opt} in
    f )
       release_config_dir=$OPTARG
       ;;
    r )
       release_name=$OPTARG
       ;;
    h )
      echo "Usage:"
      echo "    create-release-tag.sh  -h Display this help message."
      echo "    <-f> <release_config_dir>"
      echo "    [-r] <release_name>"
      exit 0
      ;;
   \? )
     echo "Invalid Option: -$OPTARG" 1>&2
     exit 1
     ;;
  esac
done


shift $((OPTIND -1))

if [[ "$release_config_dir" == "NOTSET" ]]; then
  echo "[Error]: UPS list file must be speficied with option '-f'."
  echo "[Error]: Example can be found in the 'daq-release' repo."
  echo "[Error]: E.g. daq-release/configs/dunedaq-v2.0.0.release"
  echo "Exit now..."
  exit 2
fi

if [ ! -f "$release_config_dir/release_manifest.sh" ]; then
  echo "[Error]: UPS list file must exist in the release config directory."
  echo "[Error]: Example can be found in the 'daq-release' repo."
  echo "[Error]: E.g. daq-release/configs/dunedaq-v2.0.0/release_manifest"
  echo "Exit now..."
  exit 3
else
  source $release_config_dir/release_manifest.sh
fi

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)

echo "[Info]: Release name: ${release_name}"


pushd $tmp_dir

git_checkout_and_tag dune_packages
git_checkout_and_tag dune_extras

popd

rm -rf $tmp_dir
