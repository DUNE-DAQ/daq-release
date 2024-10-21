#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "This script is intended to be sourced, not executed directly." >&2
    echo "To execute a workflow action, use run-workflow-action.sh" >&2
    exit 1
fi

function git_checkout_and_update_ci {
  repo_list_name=$1[@]
  dest_workflow_file=$2
  src_workflow_file=$tmp_dir/.github/workflow-templates/$workflow_file
  repo_list=("${!repo_list_name}")
  for repo in "${repo_list[@]}"; do
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $repo_name *****************************"
    git clone --quiet https://github.com/DUNE-DAQ/${repo_name}.git -b $DEVLINE || exit 3
    pushd ${repo_name}
    mkdir -p .github/workflows
    cp $src_workflow_file .github/workflows/$dest_workflow_file
    git add .github/workflows
    git commit -am "Syncing .github/workflows/$(basename $dest_workflow_file)"
    git push --quiet || exit 4
    popd
  done
}
