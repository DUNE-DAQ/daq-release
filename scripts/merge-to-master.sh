#!/bin/bash

release_config_dir="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0

function git_checkout_and_tag {
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    echo "++++++++++++++++++++++++++++++++++++++++++++++"
    prod_name=${iprd_arr[0]//_/-}
    if [ "$prod_name" = "elisa-client-api" ]; then
        prod_name="elisa_client_api"
    fi
    prod_ups_version=${iprd_arr[1]//_/.}
    prod_version=${prod_ups_version//[^v.[:digit:]]/}
    git clone git@github.com:DUNE-DAQ/${prod_name}.git -b ${prod_version}
    pushd ${prod_name}
    git checkout master
    git pull
    git merge --no-ff ${prod_version} -m "Merge ${prod_version} into master"
    read -r -p "INFO: Push merge to master, are you sure? [y/N] " response
    if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]
    then
        git push
    else
        echo "INFO: Return without pushing, please manually do the merge for $repo."
        return 1
    fi
    popd
  done
}

while getopts ":f:r:h" opt; do
  case ${opt} in
    f )
       release_config_dir=$OPTARG
       ;;
    h )
      echo "Usage:"
      echo "    create-release-tag.sh  -h Display this help message."
      echo "    <-f> <release_config_dir>"
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

pushd $tmp_dir

for i in dune_packages dune_extras dune_services; do 
    git_checkout_and_tag $i
done

popd

rm -rf $tmp_dir
