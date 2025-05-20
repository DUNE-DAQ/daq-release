#!/bin/bash

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

for REPO in "${dune_packages_with_ci[@]}"; do
  FULL_NAME="$ORG/$REPO"
  echo "This repo: $FULL_NAME"

  OPEN_ISSUES=$(gh issue list -R "$FULL_NAME" --state open --limit 1000 --json number --jq 'length' || echo 0)
  ISSUES_URL=$(echo "https://github.com/DUNE-DAQ/$REPO/issues")
  OPEN_PRS=$(gh pr list -R "$FULL_NAME" --state open --limit 1000 --json number --jq 'length' || echo 0)
  PRS_URL=$(echo "https://github.com/DUNE-DAQ/$REPO/pulls")

  # Get most recent single-repo CI build status
  BUILD_DEVELOP_STATUS=$(gh run list -R "$FULL_NAME" --limit 1 --json status,conclusion,name,url --workflow "build-develop" -q '.[0]')
  echo $?

  # Prepare JSON fragment
  JSON_ENTRY=$(jq -n \
    --arg repo "$REPO" \
    --argjson issues "$OPEN_ISSUES" \
    --arg issues_url "$ISSUES_URL" \
    --argjson prs "$OPEN_PRS" \
    --arg prs_url "$PRS_URL" \
    --argjson build_develop "$BUILD_DEVELOP_STATUS" \
    '{
      repo: $repo,
      open_issues: $issues,
      issues_url: $issues_url,
      open_prs: $prs,
      prs_url: $prs_url,
      build_develop: $build_develop,
    }')
  retval=$?

  if [[ $retval == 0 ]]; then
    if [[ "$FIRST" = true ]]; then
      FIRST=false
    else 
      echo "," >> "$OUTFILE"
    fi
  else
    echo "Non-zero return value. Skipping..."
    continue
  fi

  echo "$JSON_ENTRY" >> "$OUTFILE"
done

echo "]" >> "$OUTFILE"

echo "Results saved to $OUTFILE"
