#!/bin/bash

if (( $# != 1 )); then
    echo "Usage: $( basename $0 ) <production_v4 or develop)>" >&2
    exit 1
fi

export DEVLINE=$1

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function git_checkout_and_update_ci {
  prd_list_name=$1[@]
  src_workflow_file=$2
  dest_workflow_file=$3
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $prod_name *****************************"

    if [[ "$DEVLINE" == "develop" ]]; then
	common_branch="develop"
    elif [[ "$DEVLINE" == "production_v4" ]]; then
	common_branch="production/v4"
    fi

    git clone --quiet https://github.com/DUNE-DAQ/${prod_name}.git -b $common_branch || exit 1
    pushd ${prod_name}
    mkdir -p .github/workflows
    cp $src_workflow_file .github/workflows/$dest_workflow_file
    git add .github/workflows
    git commit -am "Syncing .github/workflows/$(basename $dest_workflow_file)"
    git push --quiet || exit 2
    popd
  done
}

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)

pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git || exit 3

if [[ "$DEVLINE" == "develop" ]]; then
    workflow_file=dunedaq-develop-cpp-ci.yml
else
    workflow_file=dunedaq-v4-cpp-ci.yml
fi

git_checkout_and_update_ci dune_packages_with_ci $tmp_dir/.github/workflow-templates/$workflow_file $workflow_file

popd

rm -rf $tmp_dir
