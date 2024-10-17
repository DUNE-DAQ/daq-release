#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function trigger_ci_all {
  repo_list_name=$1[@]
  workflow_file=$2
  repo_list=("${!repo_list_name}")
  for prod in "${prd_list[@]}"; do
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    # Trigger workflow 
    #last_failed_scheudle_run=$(gh run -R DUNE-DAQ/trigemu list|grep build-develop| grep failure| head -n 1|egrep -o '[[:digit:]]{10}')
    gh workflow -R DUNE-DAQ/${repo_name} run $workflow_file
  done
}


trigger_ci_all dune_packages_with_ci $workflow_file
