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
  branch="$DEVLINE"
  if [[ "$branch" == "production_v4" ]]; then
    branch="production/v4"
  fi
  for repo in "${repo_list[@]}"; do
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $repo_name *****************************"
    git clone --quiet https://github.com/DUNE-DAQ/${repo_name}.git -b "$branch" || exit 3
    pushd "${repo_name}" > /dev/null
    mkdir -p .github/workflows
    if diff -q "$src_workflow_file" ".github/workflows/$dest_workflow_file" > /dev/null; then
      echo "The workflow "$dest_workflow_file" in "$repo_name" is already up to date; continuing..."
      popd > /dev/null
      continue
    fi
    echo "Syncing $dest_workflow_file..."
    cp "$src_workflow_file" ".github/workflows/$dest_workflow_file"
    git add .github/workflows
    git commit -am "Sync .github/workflows/$(basename $dest_workflow_file)"
    git push --quiet || exit 4
    echo "Done"
    popd > /dev/null
  done
  echo "Sync complete"
}
