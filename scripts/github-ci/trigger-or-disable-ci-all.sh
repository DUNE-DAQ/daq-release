#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "This script is intended to be sourced, not executed directly." >&2
    echo "To execute a workflow action, use run-workflow-action.sh" >&2
    exit 1
fi

function trigger_or_disable_ci_all {
  repo_list_name=$1[@]
  workflow_file=$2
  action=$3
  repo_list=("${!repo_list_name}")
  for repo in "${repo_list[@]}"; do
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    # Trigger workflow 
    #last_failed_scheudle_run=$(gh run -R DUNE-DAQ/trigemu list|grep build-develop| grep failure| head -n 1|egrep -o '[[:digit:]]{10}')
    if [[ $action == "trigger" ]]; then
        gh workflow -R DUNE-DAQ/${repo_name} run $workflow_file
    elif [[ $action == "disable" ]]; then
        gh workflow -R DUNE-DAQ/${repo_name} disable $workflow_file
    else 
        echo "No action taken for ${repo_name}"
    fi
  done
}
