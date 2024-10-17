#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function disable_ci_all {
  repo_list_name=$1[@]
  workflow_file=$2
  repo_list=("${!repo_list_name}")
  for repo in "${repo_list[@]}"; do
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    gh workflow -R DUNE-DAQ/${repo_name} disable $workflow_file
  done
}


disable_ci_all dune_packages_with_ci $workflow_file
