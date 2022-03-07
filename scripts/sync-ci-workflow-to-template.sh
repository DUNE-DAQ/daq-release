#!/bin/bash

dune_packages_with_ci=(
  "daq-cmake"
  "ers"
  "erskafka"
  "logging"
  "opmonlib"
  "cmdlib"
  "rcif"
  "restcmd"
  "appfwk"
  "listrev"
  "daqdemos"
  "ipm"
  "serialization"
  "nwqueueadapters"
  "daqdataformats"
  "detdataformats"
  "detchannelmaps"
  "dfmessages"
  "trigemu"
  "triggeralgs"
  "timing"
  "timinglibs"
  "trigger"
  "flxlibs"
  "dfmodules"
  "influxopmon"
  "kafkaopmon"
  "daqconf"
  "dqm"
  "lbrulibs"
  "wibmod"
  "sspmodules"
  "utilities"
  "readoutlibs"
  "readoutmodules"
  "fdreadoutlibs"
  "ndreadoutlibs"
  "networkmanager"
  "hdf5libs"
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
    git add .github/workflows
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
