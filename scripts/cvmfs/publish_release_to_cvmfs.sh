#!/bin/bash

if (( $# != 3 )); then
    echo "Usage: "$( basename $0 )" [build type (candidate or stable)] [target name (fddaq, etc.)] [OS for build (alma9, alma10, etc. )]" >&2
    exit 1
fi

build=$1
target=$2
os=$3

if [[ $build != "candidate" && $build != "stable" ]]; then
    echo "Build type needs to be \"candidate\" or \"stable\"; exiting..." >&2
    exit 1
fi

if [[ $os == "alma9" ]]; then
    oslabel=Alma9
elif [[ $os == "alma10" ]]; then
    oslabel=Alma10
else
    echo "OS of build needs to be \"alma9\" or \"alma10\"; exiting..." >&2
    exit 3
fi

workflow_name="${oslabel} build v5 ${build} release"

REPO=
SOURCE_DIR=
DEST_DIR=

if [[ $build == "candidate" ]]; then
    REPO="dunedaq-development.opensciencegrid.org"
    SOURCE_DIR="candidates"
    DEST_DIR=/cvmfs/$REPO/candidates

elif [[ $build == "stable" ]]; then
    REPO="dunedaq.opensciencegrid.org"
    SOURCE_DIR="releases"
    DEST_DIR=/cvmfs/$REPO/spack/releases

fi

BASE_WILDCARD='coredaq-v*'
TARGET_WILDCARD=$target'-v*'

tmp_dir=$(mktemp --tmpdir=/dev/shm -d -t release_XXXXXXXXXX)

#/home/cvmfsdunedaq/bin/gh auth login --with-token < /home/cvmfsdunedaq/.git-token-readonly

which gh >& /dev/null 2>&1
retval=$?

if [[ $retval != 0 ]]; then
    echo "You need the GitHub command line utility \"gh\" for this script to work; exiting..." >&2
    exit 4
fi

# Note that among other things you need to have successfully run "gh auth login" for this to work
run_id=$( gh run -R DUNE-DAQ/daq-release list | grep "${workflow_name}" | grep completed |head -n 1 | egrep -o '[[:digit:]]{11}' )

if [[ -z $run_id || ! $run_id =~ [0-9]+ ]]; then
     echo "Unable to obtain a relevant GitHub Action run ID; exiting..." >&2
     exit 5
fi

read -p "Will publish the results of the GitHub Action https://github.com/DUNE-DAQ/daq-release/actions/runs/$run_id; confirm? (y/n): " answer

test "$answer" != "y" && exit 0

artifacts="${build}s_coredaq ${build}s_${target} ${target}-dbt_setup_release_env ${target}_app_rte"

for artifact in $artifacts; do
    echo "Downloading $artifact..."
    gh -R DUNE-DAQ/daq-release run download $run_id -D $tmp_dir -n $artifact || exit 10
done

mkdir $tmp_dir/$SOURCE_DIR || exit 44
cd $tmp_dir/$SOURCE_DIR

for tarfile in ../*.tar.gz ; do
    tar xf $tarfile 
    rm -f $tarfile
done

full_target_release_name=$( ls | grep "${target}-v.*" )
shorthand_target_release_name=$( echo $full_target_release_name | sed -r 's/(.*)-[0-9]+$/\1/' )
ln -s $full_target_release_name $shorthand_target_release_name

cd $full_target_release_name || exit 45
cp -p $tmp_dir/${target}-dbt-setup-release-env.sh dbt-setup-release-env.sh
cp -p $tmp_dir/${target}_app_rte.sh daq_app_rte.sh

cd $tmp_dir

TAG=release_build_$( date +%Y%m%d_%H%M%S )
LOG=$HOME/dunedaq-sync.log

cvmfs_server transaction $REPO

echo >> $LOG
echo -n Transaction $TAG: >>$LOG
find $SOURCE_DIR/coredaq-* -name .cvmfscatalog -delete
rsync -rlpvt --delete-after --stats $SOURCE_DIR/$BASE_WILDCARD $DEST_DIR
rsync -rlpvt --delete-after --stats $SOURCE_DIR/$TARGET_WILDCARD $DEST_DIR

RET=$?

if [ $RET -eq 0 ]; then
  echo -n " rsync succeeded" >>$LOG
  cvmfs_server publish -a $TAG $REPO
  rm -rf $tmp_dir
else
  echo -n " rsync failed with error $RET" >>$LOG
  cvmfs_server abort $REPO
  echo "Downloaded artifacts in $tmp_dir will not be deleted"
fi
echo >>$LOG

