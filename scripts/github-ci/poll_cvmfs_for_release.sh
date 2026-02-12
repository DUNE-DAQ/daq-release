#!/bin/bash

set -euo pipefail

usage() {
  local prog=$(basename $0)
  cat <<EOF
Usage: $prog <release_name> <interval> <max_tries>
EOF
}

if [[ $# -ne 3 ]]; then
  usage
  exit 1
fi

parse_args() {
  RELEASE=$1
  INTERVAL=$2
  MAX_TRIES=$3
}

get_target_dir() {
  TARGET_DIR="/cvmfs/dunedaq.opensciencegrid.org/spack/releases/"
  if [[ "$1" == *FD_* ]]; then
    TARGET_DIR="/cvmfs/dunedaq-development.opensciencegrid.org/nightly/"
  elif [[ "$1" == *rc* ]]; then
    TARGET_DIR="/cvmfs/dunedaq-development.opensciencegrid.org/candidates/"
  fi
  echo "Target: $TARGET_DIR"
}

poll_for_release() {
  local attempt=1

  while (( attempt <= MAX_TRIES )); do
    echo "Checking for ${RELEASE} in ${TARGET_DIR} (attempt ${attempt}/${MAX_TRIES})"

    if [[ -d "${TARGET_DIR}/${RELEASE}" ]]; then
      echo "$RELEASE appeared on CVMFS."
      return 0
    fi

    sleep "$INTERVAL"
    ((attempt++))
  done

  echo "Release $RELEASE not found after $MAX_TRIES attempts."
  return 1
}

main() {
  parse_args "$@"
  get_target_dir "$RELEASE"
  poll_for_release "$RELEASE" "$INTERVAL" "$MAX_TRIES"
  return $?
}

main "$@"
exit $?