#!/bin/bash

export DEVLINE="develop"

# Store list of packages from repo.sh as dune_packages_with_ci
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $SCRIPT_DIR/../repo.sh || exit $?

ORG="DUNE-DAQ"
REPOS=$(gh repo list "$ORG" --limit 100 --json name -q '.[].name')
OUTFILE="ci_summary.json"

echo "[" > "$OUTFILE"
FIRST=true

for REPO in "${dune_packages_with_ci[@]}"; do
  FULL_NAME="$ORG/$REPO"
  echo "Collecting metrics for $FULL_NAME..."

  OPEN_ISSUES=$(gh issue list -R "$FULL_NAME" --state open --limit 1000 --json number --jq 'length' || echo 0)
  ISSUES_URL=$(echo "https://github.com/DUNE-DAQ/$REPO/issues")
  OPEN_PRS=$(gh pr list -R "$FULL_NAME" --state open --limit 1000 --json number --jq 'length' || echo 0)
  PRS_URL=$(echo "https://github.com/DUNE-DAQ/$REPO/pulls")

  # Get most recent single-repo CI build status
  BUILD_DEVELOP_STATUS=$(gh run list -R "$FULL_NAME" --limit 1 \
                         --json event,status,conclusion,name,url,createdAt,updatedAt \
                         --workflow dunedaq-develop-cpp-ci.yml -q '.[0]')

  now=$(date +%s)
  last_commit_time=$(date -d "$(gh api repos/$ORG/$REPO/commits --jq '.[0].commit.author.date')" +%s)
  TIME_SINCE_LAST_COMMIT=$(( now - last_commit_time ))

  # Prepare JSON fragment
  JSON_ENTRY=$(jq -n \
    --arg repo "$REPO" \
    --argjson issues "$OPEN_ISSUES" \
    --arg issues_url "$ISSUES_URL" \
    --argjson prs "$OPEN_PRS" \
    --arg prs_url "$PRS_URL" \
    --argjson time_since_last_commit $TIME_SINCE_LAST_COMMIT \
    --argjson build_develop "$BUILD_DEVELOP_STATUS" \
    '{
      repo: $repo,
      open_issues: $issues,
      issues_url: $issues_url,
      open_prs: $prs,
      prs_url: $prs_url,
      time_since_last_commit: $time_since_last_commit,
      build_develop: $build_develop,
    }')
  retval=$?

  if [[ $retval == 0 ]]; then
    echo "Success"
    if [[ "$FIRST" = true ]]; then
      FIRST=false
    else 
      echo "," >> "$OUTFILE"
    fi
  else
    echo "Non-zero return value in $REPO. Skipping..."
    continue
  fi

  echo "$JSON_ENTRY" >> "$OUTFILE"

  # Reset workflow inactivity timer
  gh api -X PUT "repos/$FULL_NAME/actions/workflows/dunedaq-develop-cpp-ci.yml/enable"
done

echo "]" >> "$OUTFILE"

echo "Results saved to $OUTFILE"
