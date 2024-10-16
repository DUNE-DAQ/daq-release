#!/bin/bash

if (( $# != 1 )); then
    echo "Usage: $( basename $0 ) <workflow_file_name>" >&2
    exit 1
fi

workflow_file=$1
common_branch="develop"

if [[ "$workflow_file" == *v4*]]; then
    echo "Using branch production_v4 for v4 workflow $workflow_file"
    common_branch="production_v4"
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function git_checkout_and_update_ci {
  repo_list_name=$1[@]
  src_workflow_file=$2
  dest_workflow_file=$3
  common_branch=$4
  repo_list=("${!repo_list_name}")
  for repo in "${repo_list[@]}"; do
    irepo_arr=(${prod})
    repo_name=${irepo_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $repo_name *****************************"
    git clone --quiet https://github.com/DUNE-DAQ/${repo_name}.git -b $common_branch || exit 2
    pushd ${repo_name}
    mkdir -p .github/workflows
    cp $src_workflow_file .github/workflows/$dest_workflow_file
    git add .github/workflows
    git commit -am "Syncing .github/workflows/$(basename $dest_workflow_file)"
    git push --quiet || exit 3
    popd
  done
}

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)

pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git || exit 4

existing_workflow_templates=$(ls .github/workflow-templates/*.yml)
if [[ ! ${existing_workflow_templates[@]} =~ ${workflow_file} ]]; then 
    echo "Invalid workflow file name provided. The available options are:" >&2
    ls .github/workflow-templates/*.yml >&2
    exit 5
fi

git_checkout_and_update_ci dune_packages_with_ci $tmp_dir/.github/workflow-templates/$workflow_file $workflow_file $common_branch

popd

rm -rf $tmp_dir
