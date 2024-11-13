#!/bin/bash

if (( $# != 2 )); then
    echo "Usage: $( basename $0 ) <production_v4 or develop> <branch_name>" >&2
    echo "Example usage: ./check_if_release_prep_branch_exists.sh develop prep-release/fddaq-v5.2.0" >&2
    exit 1
fi

export DEVLINE=$1
branch_name=$2

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

function check_patch {
  tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
  cd $tmp_dir
  repo_list_name=$1[@]
  branch_name=$2
  repo_list=("${!repo_list_name}")
  for repo in "${repo_list[@]}"; do
    irepo_arr=(${repo})
    repo_name=${irepo_arr[0]//_/-}
    echo "--------------------------------------------------------------"
    echo "********************* $repo_name *****************************"
    git clone --quiet https://github.com/DUNE-DAQ/${repo_name}.git 
    cd ${repo_name}
    git show-ref | awk '{print $2}' | grep "${branch_name}"
    cd ..
  done
  cd ..
  rm -rf $tmp_dir
}

check_patch dune_packages_with_ci $branch_name
