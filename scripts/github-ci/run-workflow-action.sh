#!/bin/bash

usage() {
    echo "Usage: $(basename "$0") <production_v4|develop> <workflow_file_name> <sync|trigger|disable> [--pymodules]" >&2
    exit 1
}

[[ $# -lt 3 || $# -gt 4 ]] && usage

DEVLINE=$1
workflow_file=$2
action=$3
run_pymodules=false

if [[ "${4:-}" == "--pymodules" ]]; then
    run_pymodules=true
elif [[ -n "${4:-}" ]]; then
    echo "Unknown flag '$4' (expected --pymodules)" >&2
    exit 2
fi

# Store list of packages from repo.sh as dune_packages_with_ci
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh || exit $?

if [[ "$run_pymodules" == "true" ]]; then
    packages_to_run=( "${dune_packages_with_ci[@]}" "${dune_pymodules[@]}" )
else
    packages_to_run=( "${dune_packages_with_ci[@]}" )
fi

if [[ $DEVLINE == "develop" && "$workflow_file" == *v4* ]]; then
    echo "ERROR: $workflow_file is not a develop-line workflow." >&2
    exit 2
elif [[ $DEVLINE == "production_v4" && "$workflow_file" != *v4* ]]; then
    echo "ERROR: $workflow_file is not a production_v4 workflow." >&2
    exit 3
fi
if [[ $action != "sync" && $action != "trigger" && $action != "disable" ]]; then
    echo "ERROR: $action is not a valid action argument. Available options are sync, trigger, or disable" >&2
    exit 4
fi

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git || exit 5
existing_workflow_templates=$(ls .github/workflow-templates/*.yml | xargs -n 1 basename)
if ! echo "$existing_workflow_templates" | grep -xq "${workflow_file}"; then
    echo "ERROR: $workflow_file is not a valid workflow file name. The available options are:" >&2
    echo $existing_workflow_templates >&2
    exit 6
fi

extra_info=""
if [[ "$run_pymodules" == "true" ]]; then
    extra_info=", including ${#dune_pymodules[@]} Python packages"
fi
read -r -p "This action will $action the workflow $workflow_file in ${#packages_to_run[@]} packages$extra_info. Proceed? (y/N): " response
[[ "$response" != "y" && "$response" != "Y" ]] && { echo "Action canceled."; exit 7; }

if [[ $action == "sync" ]]; then
    source $SCRIPT_DIR/sync-ci-workflow-to-template.sh
    git_checkout_and_update_ci packages_to_run $workflow_file
elif [[ $action == "trigger" || $action == "disable" ]]; then
    source $SCRIPT_DIR/trigger-or-disable-ci-all.sh
    trigger_or_disable_ci_all packages_to_run $workflow_file $action
else
    echo "WARNING: No action taken. If this is unexpected, check your syntax or contact software coordination."
fi

popd
rm -rf $tmp_dir
