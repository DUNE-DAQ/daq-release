#!/bin/bash

repo=appfwk

. /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh || exit 1
setup_dbt latest_v5 || exit 2

newdir=$( mktemp -d )

mkdir -p $newdir
cd $newdir
dbt-create -n last_fddaq || exit 3
cd $( ls )  # Only thing in the directory will be the work area
cd sourcecode
git clone https://github.com/DUNE-DAQ/$repo || exit 4
cd ..
. env.sh || exit 5
dbt-build || exit 6
dbt-build --lint || exit 7
dbt-build --unittest || exit 8

cd sourcecode
dbt-clang-format.sh $repo --view-differences-only || exit 9

rm -rf $newdir

exit 0
