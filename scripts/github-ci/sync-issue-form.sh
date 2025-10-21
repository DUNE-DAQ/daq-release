#!/bin/bash

set -euo pipefail

usage() {
  local prog=$(basename $0)
  cat <<EOF
Usage: $prog <repo_list>

Arguments:
  repo_list: Repository or repositories in which to sync the issue form. 
             Use 'all' to sync issue forms in all repos, 'coredaq' for all
             coredaq packages, 'fddaq' for all fddaq packages, 'pymodules' 
             for python packages, or a single repo name (e.g., 'trigger').
EOF
  exit 1
}

if [[ $# -eq 0 || $# -gt 1 ]]; then
    usage
fi

# Need to set DEVLINE before sourcing repo.sh, even though it's not used here
export DEVLINE="develop"

# Get list of repos in coredaq, fddaq, and pymodules
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh

arg="$1"
case "$arg" in
  all)
    repo_list=( "${coredaq_packages[@]}" "${fddaq_packages[@]}" "${python_packages[@]}" )
    ;;
  coredaq)
    repo_list=( "${coredaq_packages[@]}" )
    ;;
  fddaq)
    repo_list=( "${fddaq_packages[@]}" )
    ;;
  pymodules)
    repo_list=( "${dune_pymodules[@]}" )
    ;;
  *)
    echo "INFO: interpreting $arg as the name of a single repo name since it doesn't match an umbrella name."
    repo_list=( "$arg" )
    ;;
esac

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git dunedaq_github || exit 2

ORG="DUNE-DAQ"
for REPO in "${repo_list[@]}"; do
    if [[ "$REPO" == "elisa-client-api" ]]; then
      REPO="elisa_client_api"
    fi
    echo "--------------------------------------------------------------"
    echo "********************* $REPO *****************************"
    git clone --quiet https://github.com/${ORG}/${REPO}.git || exit 3
    pushd "${REPO}" > /dev/null

    src_issue_dir=$(readlink -f ../dunedaq_github/issue-templates)
    if [[ -d "${src_issue_dir}/${REPO}" ]]; then
      src_issue_dir="$(readlink -f ../dunedaq_github/issue-templates/$REPO)"
      echo "Syncing custom issue forms for $REPO..."
    fi

    dest_issue_dir=".github/ISSUE_TEMPLATE"
    mkdir -p "$dest_issue_dir"
    if diff -rq "$src_issue_dir" "$dest_issue_dir" > /dev/null; then
      echo "The issue forms in "$REPO" are already up to date; continuing..."
      popd > /dev/null
      continue
    fi
    cp "$src_issue_dir"/*.yml "$dest_issue_dir"
    git add "$dest_issue_dir"
    git commit -am "Sync issue form templates"
    git push --quiet || exit 4
    echo "Done with $REPO"
    popd > /dev/null
done

popd > /dev/null
rm -rf $tmp_dir
echo "Done"