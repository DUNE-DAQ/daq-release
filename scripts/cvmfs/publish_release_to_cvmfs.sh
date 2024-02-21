#!/bin/bash

if (( $# != 3 )); then
    echo "Usage: "$( basename $0 )" [build type (candidate or frozen)] [detector type (nd or fd)] [OS for build (sl7 or alma9)] " >&2
    exit 1
fi

build=$1
det=$2
os=$3

if [[ $build != "candidate" && $build != "frozen" ]]; then
    echo "Build type needs to be \"candidate\" or \"frozen\"; exiting..." >&2
    exit 1
fi

if [[ $det != "nd" && $det != "fd" ]]; then
    echo "Detector type needs to be \"nd\" or \"fd\"; exiting..." >&2
    exit 2
fi

if [[ $os == "sl7" ]]; then
    oslabel=SL7
elif [[ $os == "alma9" ]]; then
    oslabel=Alma9
else
    echo "OS of build needs to be \"sl7\" or \"alma9\"; exiting..." >&2
    exit 3
fi


REPO=
SOURCE_DIR=
DEST_DIR=

if [[ $build == "candidate" ]]; then
    REPO="dunedaq-development.opensciencegrid.org"
    SOURCE_DIR="candidates"
    DEST_DIR=/cvmfs/$REPO/candidates

elif [[ $build == "frozen" ]]; then
    REPO="dunedaq.opensciencegrid.org"
    SOURCE_DIR="releases"
    DEST_DIR=/cvmfs/$REPO/spack/releases

fi

BASE_WILDCARD='dunedaq-v*'
DET_WILDCARD=$det'daq-v*'

tmp_dir=$(mktemp --tmpdir=/dev/shm -d -t release_XXXXXXXXXX)

#/home/cvmfsdunedaq/bin/gh auth login --with-token < /home/cvmfsdunedaq/.git-token-readonly

which gh >& /dev/null 2>&1
retval=$?

if [[ $retval != 0 ]]; then
    echo "You need the GitHub command line utility \"gh\" for this script to work; exiting..." >&2
    exit 4
fi

# Note that among other things you need to have successfully run "gh auth login" for this to work
run_id=$( gh run -R DUNE-DAQ/daq-release list | grep "${oslabel} build ${build} release" | grep completed |head -n 1 | egrep -o '[[:digit:]]{10}' )

if [[ -z $run_id || ! $run_id =~ [0-9]+ ]]; then
     echo "Unable to obtain a relevant GitHub Action run ID; exiting..." >&2
     exit 5
fi

read -p "Will publish the results of the GitHub Action https://github.com/DUNE-DAQ/daq-release/actions/runs/$run_id; confirm? (y/n): " answer

test "$answer" != "y" && exit 0

artifacts="${build}s_dunedaq ${build}s_${det}daq ${det}daq-dbt_setup_release_env ${det}daq_app_rte"

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

cd ${det}*-v* || exit 45
cp -p $tmp_dir/${det}daq-dbt-setup-release-env.sh dbt-setup-release-env.sh
cp -p $tmp_dir/${det}daq_app_rte.sh daq_app_rte.sh

cd $tmp_dir/$SOURCE_DIR

res=$( ls | grep "${det}.*-v.*" | wc -l )

if (( res != 1 )); then
    echo "There's a problem with the logic in this script; exiting..." >&2
    exit 1
fi

softlink_name=$( ls | grep "${det}.*-v.*" | sed -r 's/(.*)-[0-9]/\1/' )
ln -s ${det}*-v* $softlink_name
cd $tmp_dir

TAG=release_build_$( date +%Y%m%d_%H%M%S )
LOG=$HOME/dunedaq-sync.log

cvmfs_server transaction $REPO

echo >> $LOG
echo -n Transaction $TAG: >>$LOG
find $SOURCE_DIR/dunedaq-* -name .cvmfscatalog -delete
rsync -rlpvt --delete-after --stats $SOURCE_DIR/$BASE_WILDCARD $DEST_DIR
rsync -rlpvt --delete-after --stats $SOURCE_DIR/$DET_WILDCARD $DEST_DIR

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

