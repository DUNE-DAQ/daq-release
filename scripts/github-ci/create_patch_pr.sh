#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/patch.sh

function create_patch {
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $prod_name *****************************"
    cmd='gh pr create -R DUNE-DAQ/$prod_name -B develop -H patch/v3.2.x -t "Merge patch/v3.2.x to develop" -r wesketchum -b "Merge patch/v3.2.x (dunedaq-v3.2.2) into develop"'
    gh pr create -R DUNE-DAQ/$prod_name -B develop -H patch/v3.2.x -t "Merge patch/v3.2.x to develop" -r wesketchum -b "Merge patch/v3.2.x (dunedaq-v3.2.2) into develop"
    #echo $cmd
  done
}


create_patch patch_repo

