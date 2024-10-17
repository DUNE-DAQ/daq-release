#!/bin/bash

if (( $# != 3 )); then
    echo "Usage: $( basename $0 ) <production_v4 or develop> <workflow_file_name> <sync, trigger, or disable>" >&2
    exit 1
fi

export DEVLINE=$1
workflow_file=$2
action=$3

# Store list of packages from repo.sh as dune_packages_with_ci
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

if [[ $DEVLINE == "develop" && "$workflow_file" == *v4* ]]; then
    echo "ERROR: $workflow_file is a production_v4 workflow." >&2
    exit 2
elif [[ $DEVLINE == "production_v4" && "$workflow_file" != *v4* ]]; then
    echo "ERROR: $workflow_file is not a production_v4 workflow. Did you mean to use develop?" >&2
    exit 3
fi
if [[ $action != "sync" && $action != "trigger" && $action != "disable" ]]; then
    echo "ERROR: $action is not a valid action argument. Available options are sync, trigger, or disable" >&2
    exit 4
fi

echo "Starting directory: $(pwd)"
tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git || exit 5
existing_workflow_templates=$(ls .github/workflow-templates/*.yml | xargs -n 1 basename)
if ! echo "$existing_workflow_templates" | grep -xq "${workflow_file}"; then
    echo "ERROR: $workflow_file is not a valid workflow file name. The available options are:" >&2
    echo $existing_workflow_templates >&2
    exit 6
fi

read -p "This action will $action the workflow $workflow_file in all ${#dune_packages_with_ci[@]} packages in the $DEVLINE line. Proceed? (y/N): " response
if [[ "$response" != "y" && "$response" != "Y" ]]; then
    echo "Action canceled."
    exit 7
fi

if [[ $action == "sync" ]]; then
    source $SCRIPT_DIR/sync-ci-workflow-to-template.sh
    git_checkout_and_update_ci dune_packages_with_ci $workflow_file
elif [[ $action == "trigger" || $action == "disable" ]]; then
    source $SCRIPT_DIR/trigger-or-disable-ci-all.sh
    trigger_or_disable_ci_all dune_packages_with_ci $workflow_file $action
else
    echo "WARNING: No action taken. If this is unexpected, check your syntax or contact software coordination."
fi

popd
rm -rf $tmp_dir
