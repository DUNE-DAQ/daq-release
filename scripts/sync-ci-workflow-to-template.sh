#!/bin/bash

dune_packages_with_ci=(
  "ers"
  "logging"
  "cmdlib"
  "restcmd"
  "opmonlib"
  "rcif"
  "appfwk"
  "listrev"
  "serialization"
  "flxlibs"
  "dataformats"
  "dfmessages"
  "dfmodules"
  "trigemu"
  "readout"
  "minidaqapp"
  "ipm"
  "timing"
  "timinglibs"
  "influxopmon"
  "nwqueueadapters"
  "erses"
  "triggeralgs"
  "trigger"
)

function git_checkout_and_update_ci {
  prd_list_name=$1[@]
  workflow_file=$2
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    git clone git@github.com:DUNE-DAQ/${prod_name}.git -b develop
    pushd ${prod_name}
    cp $workflow_file .github/workflows
    git commit -am "syncing $(basename $workflow_file) to the template in DUNE-DAQ/.github repo"
    git push
    popd
  done
}


tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)

pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git

git_checkout_and_update_ci dune_packages_with_ci $tmp_dir/.github/workflow-templates/dunedaq-develop-cpp-ci.yml

popd

rm -rf $tmp_dir
