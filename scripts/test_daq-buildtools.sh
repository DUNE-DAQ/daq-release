#!/bin/bash

usage() {
    local prog=$(basename "$0")
    cat << EOF
Usage: $prog [OPTIONS]

Test functionality of daq-buildtools commands.

Optional arguments:
    --release <release_name>    Release in which to test daq-buildtools commands. [default: last_fddaq]
    --dbt-branch <branch_name>  Branch of daq-buildtools to run tests from. [default: develop]
    --repo <repo_name>          Name of a single repo to checkout for tests. [default: ipm]

Example:
    ./${prog} --release fddaq-v5.5.0-a9 --dbt-branch user/new_feature --repo hdf5libs
EOF
}

start_dir="$PWD"

cleanup() {
    echo "Triggering cleanup"
    cd "$start_dir"
    rm -rf "$dbt_tmpdir"
    rm -rf "$release_tmpdir"
    echo "Done"
}
trap cleanup EXIT SIGINT SIGTERM

release="last_fddaq"
repo="ipm"
dbt_branch="develop"

while [[ $# -gt 0 ]]; do
    case "$1" in
    -h|--help|-?)
        usage
        exit 1
        ;;
    --release)
        release="$2"
        shift 2
        ;;
    --repo)
        repo="$2"
        shift 2
        ;;
    --dbt-branch)
        dbt_branch="$2"
        shift 2
        ;;
    *)
        echo "ERROR: Unknown argument: $1"
        exit 1
        ;;
    esac
done

extra_args=()
if [[ "$release" == *"FD_"* || "$release" == "last_fddaq" ]]; then
    extra_args=(-n)
elif [[ "$release" == *"rc"* ]]; then
    extra_args=(-b candidate)
fi

echo -e "Running daq-buildtools commands using:\n"
echo -e "\tRelease name: $release"
echo -e "\tdbt branch:   $dbt_branch"
echo -e "\trepo:         $repo\n"

. /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh || exit 1
setup_dbt latest_v5 || exit 2

if [[ -n "dbt_branch" ]]; then
    dbt_tmpdir=$(mktemp -d)
    mkdir -p "$dbt_tmpdir" && cd "$dbt_tmpdir"
    git clone https://github.com/DUNE-DAQ/daq-buildtools.git -b "$dbt_branch"
    source daq-buildtools/env.sh
fi

release_tmpdir=$(mktemp -d)
mkdir -p "$release_tmpdir" && cd "$release_tmpdir"

echo "dbt_tmpdir: $dbt_tmpdir"
echo "release_tmpdir: $release_tmpdir"

echo "*********************************TEST dbt-setup-release *******************************"
# Check that dbt-setup-release works without altering the environment, thus the (...)
(dbt-setup-release "${extra_args[@]}" "$release"; echo $? > $release_tmpdir/dbt-setup-release_result.txt)

test -e $release_tmpdir/dbt-setup-release_result.txt || exit 3
test $( cat $release_tmpdir/dbt-setup-release_result.txt ) == 0 || exit 4
rm -f dbt-setup-release_result.txt

echo "*********************************TEST dbt-create ***************************************"
dbt-create -s ${extra_args[@]} $release || exit 5
cd $(ls)  # Only thing in the directory will be the work area
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
mv sourcecode $release_tmpdir
ln -s $release_tmpdir/sourcecode
dbt-build --clean || exit 12

echo "*****************************TEST dbt-build --codegen **********************************"
dbt-build --codegen || exit 13

echo "********************TEST local workarea Spack package installation **********************"
spack install py-wesanderson || exit 14

echo "*********************************TEST dbt-lcov.sh****************************************"
dbt-lcov.sh || exit 15

exit 0
