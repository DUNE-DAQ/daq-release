#!/bin/bash

if (( $# != 2 )); then
    echo "Usage: $( basename $0 ) <production_v4 or develop> <workflow_file_name>" >&2
    exit 1
fi

export DEVLINE=$1
workflow_file=$2

if [[ $DEVLINE != "develop" && $DEVLINE != "production_v4" ]]; then
    echo "Invalid argument: you must pass either develop or production_v4 as the branch name" >&2
    exit 2
fi
if [[ $DEVLINE == "develop" && "$workflow_file" == *v4* ]]; then
    echo "You must use the production_v4 branch for v4 workflows such as $workflow_file" >&2
    exit 3
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
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $repo_name *****************************"
    git clone --quiet https://github.com/DUNE-DAQ/${repo_name}.git -b $common_branch || exit 4
    pushd ${repo_name}
    mkdir -p .github/workflows
    cp $src_workflow_file .github/workflows/$dest_workflow_file
    git add .github/workflows
    git commit -am "Syncing .github/workflows/$(basename $dest_workflow_file)"
    git push --quiet || exit 5
    popd
  done
}

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)

pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git || exit 6

existing_workflow_templates=$(ls .github/workflow-templates/*.yml | xargs -n 1 basename)
if ! echo "$existing_workflow_templates" | grep -xq "${workflow_file}"; then
    echo "Invalid workflow file name provided. The available options are:" >&2
    echo $existing_workflow_templates >&2
    exit 7
fi

git_checkout_and_update_ci dune_packages_with_ci $tmp_dir/.github/workflow-templates/$workflow_file $workflow_file $DEVLINE

popd

rm -rf $tmp_dir
