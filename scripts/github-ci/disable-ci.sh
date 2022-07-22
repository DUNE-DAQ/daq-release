#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function trigger_ci_all {
  prd_list_name=$1[@]
  workflow_file=$2
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    # Trigger workflow 
    #last_failed_scheudle_run=$(gh run -R DUNE-DAQ/trigemu list|grep build-develop| grep failure| head -n 1|egrep -o '[[:digit:]]{10}')
    gh workflow -R DUNE-DAQ/${prod_name} disable $workflow_file
  done
}


disable_ci_all dune_packages_with_ci auto_approve.yml
