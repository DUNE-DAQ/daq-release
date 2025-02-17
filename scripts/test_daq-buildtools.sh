#!/bin/bash

repo=ipm

. /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh || exit 1
setup_dbt latest_v5 || exit 2

newdir=$( mktemp -d )

mkdir -p $newdir
cd $newdir

echo "*********************************TEST dbt-setup-release *******************************"
# Check that dbt-setup-release works without altering the environment, thus the (...)
(dbt-setup-release -n last_fddaq; echo $? > $newdir/dbt-setup-release_result.txt)

test -e $newdir/dbt-setup-release_result.txt || exit 3
test $( cat $newdir/dbt-setup-release_result.txt ) == 0 || exit 4
rm -f dbt-setup-release_result.txt

echo "*********************************TEST dbt-create ***************************************"
dbt-create -s -n last_fddaq || exit 5
cd $( ls )  # Only thing in the directory will be the work area
cd sourcecode
git clone https://github.com/DUNE-DAQ/$repo || exit 6
cd ..
. env.sh || exit 7

echo "**********************************TEST dbt-build ****************************************"
dbt-build || exit 8

echo "******************************TEST dbt-build --lint *************************************"
dbt-build --lint || exit 9

echo "******************************TEST dbt-build --unittest *********************************"
dbt-build --unittest || exit 10

echo "******************************TEST dbt-clang-format.sh **********************************"
cd $DBT_AREA_ROOT/sourcecode
dbt-clang-format.sh $repo --view-differences-only || exit 11

# Test building against a sourcecode directory outside of the work area
echo "***********************TEST dbt-build with external sourcecode **************************"
cd ..
mv sourcecode $newdir
ln -s $newdir/sourcecode
dbt-build --clean || exit 12

echo "*****************************TEST dbt-build --codegen **********************************"
dbt-build --codegen || exit 13

echo "********************TEST local workarea Spack package installation **********************"
spack install py-wesanderson || exit 14

echo "*********************************TEST dbt-lcov.sh****************************************"
dbt-lcov.sh || exit 15

rm -rf $newdir

exit 0
