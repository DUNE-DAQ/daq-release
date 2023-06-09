#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function check_patch {
  tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
  cd $tmp_dir
  prd_list_name=$1[@]
  release_tag=$2
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $prod_name *****************************"
    git clone --quiet git@github.com:DUNE-DAQ/${prod_name}.git -b develop
    cd ${prod_name}
    git branch -r --contains $release_tag|grep patch
    cd ..
  done
  cd ..
  rm -rf $tmp_dir
}


check_patch dune_packages_with_ci dunedaq-v3.2.2
