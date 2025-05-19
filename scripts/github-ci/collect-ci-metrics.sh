#!/bin/bash

# Usage?
#if (( $# != 3 )); then
#    echo "Usage: $( basename $0 ) <production_v4 or develop> <workflow_file_name> <sync, trigger, or disable>" >&2
#    exit 1
#fi

export DEVLINE="develop"

# Store list of packages from repo.sh as dune_packages_with_ci
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/repo.sh || exit $?

#echo "${dune_packages_with_ci[@]}"

ORG="DUNE-DAQ"
REPOS=$(gh repo list "$ORG" --limit 100 --json name -q '.[].name')
OUTFILE="ci_summary.json"

echo "[" > "$OUTFILE"
FIRST=true

#for REPO in $REPOS; do
for REPO in "${dune_packages_with_ci[@]}"; do
#for REPO in "dfmodules"; do
  FULL_NAME="$ORG/$REPO"
  echo "This repo: $FULL_NAME"

  # Count open issues and PRs
  OPEN_ISSUES=$(gh issue list -R "$FULL_NAME" --state open --limit 1000 --json number --jq 'length' || echo 0)
  OPEN_PRS=$(gh pr list -R "$FULL_NAME" --state open --limit 1000 --json number --jq 'length' || echo 0)

  # Get most recent workflow run
  BUILD_DEVELOP_STATUS=$(gh run list -R "$FULL_NAME" --limit 1 --json status,conclusion,name,url --workflow "build-develop" -q '.[0]')
  echo $?

  # Prepare JSON fragment
  JSON_ENTRY=$(jq -n \
    --arg repo "$REPO" \
    --argjson issues "$OPEN_ISSUES" \
    --argjson prs "$OPEN_PRS" \
    --argjson build_develop "$BUILD_DEVELOP_STATUS" \
    '{
      repo: $repo,
      open_issues: $issues,
      open_prs: $prs,
      build_develop: $build_develop
    }')
  retval=$?

  if [[ $retval == 0 ]]; then
    if [[ "$FIRST" = true ]]; then
      FIRST=false
    else 
      echo "," >> "$OUTFILE"
    fi
  #if [[ "$FIRST" = true && $retval == 0 ]]; then
  #  FIRST=false
  #elif [[ "$FIRST" != true && $retval == 0 ]]; then
  #  echo "," >> "$OUTFILE"
  else
    echo "Non-zero return value. Skipping..."
    continue
  fi

  echo "$JSON_ENTRY" >> "$OUTFILE"
done

echo "]" >> "$OUTFILE"
