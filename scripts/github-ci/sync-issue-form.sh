#!/bin/bash

set -euo pipefail

usage() {
    echo "Usage: $0 <repo_list>"
    echo
    echo "Arguments:"
    echo "  repo_list    : Repository or repositories in which to sync the issue form. Use 'all' to sync issue forms in all repos, or a single repo name (e.g., 'trigger')."
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
  *)
    echo "INFO: interpreting $arg as the name of a single repo name since it doesn't match an umbrella name."
    repo_list=( "$arg" )
    ;;
esac

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
pushd $tmp_dir

git clone https://github.com/DUNE-DAQ/.github.git -b amogan/new_templates dunedaq_github || exit 6
src_issue_dir=$(readlink -f dunedaq_github/issue-templates)
# Allow for individual repos to have their own set of forms to sync, separate from the generic ones
special_cases=($(ls -d $src_issue_dir/*/ | tr "/" " "))
echo "ls of source dir: $(ls $src_issue_dir)"
echo "Found special cases: ${special_cases[@]}"

ORG="andrewmogan"
for REPO in "${repo_list[@]}"; do
    echo "REPO: $REPO..."
    if [[ "$REPO" == "elisa-client-api" ]]; then
      REPO="elisa_client_api"
    fi
    echo "--------------------------------------------------------------"
    echo "********************* $REPO *****************************"
    git clone --quiet https://github.com/${ORG}/${REPO}.git || exit 3
    pushd "${REPO}" > /dev/null
    if [[ -d ${src_issue_dir}/${REPO} ]];
      src_issue_dir=$(readlink -f dunedaq_github/issue-templates/$REPO)
      echo "$REPO has its own issue forms, so will sync from"
    fi
    dest_issue_dir=".github/ISSUE_TEMPLATE"
    mkdir -p .github/ISSUE_TEMPLATE
    if diff -rq "$src_issue_dir" "$dest_issue_dir" > /dev/null; then
      echo "The issue forms in "$repo_name" are already up to date; continuing..."
      popd > /dev/null
      continue
    fi
    echo "Copying contents of $src_issue_dir into $(pwd)/${dest_issue_dir}"
    cp "$src_issue_dir"/* "$dest_issue_dir"
    git status
    git add "$dest_issue_dir"
    git status
    #git commit -am "Sync issue form templates"
    #git push --quiet || exit 4
    echo "Done with $REPO"
    popd > /dev/null
done

popd > /dev/null
echo "Done"