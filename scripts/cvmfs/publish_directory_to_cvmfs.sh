#! /bin/sh

if [[ $# != 1 ]]; then
    echo "Usage: "$(basename $0)" <path to sync>" >&2
    exit 1
fi

PATH_TO_SYNC=$1

if [[ $( hostname ) != "oasiscfs01.fnal.gov" ]]; then
    echo "This script needs to be run on oasiscfs01.fnal.gov; exiting..." >&2
    exit 2
fi

if [[ $( whoami ) == "cvmfsdunedaq" ]]; then
    REPO="dunedaq.opensciencegrid.org"
elif [[ $( whoami ) == "cvmfsdunedaqdev" ]]; then
    REPO="dunedaq-development.opensciencegrid.org"
else
    echo "You need to be either cvmfsdunedaq or cvmfsdunedaqdev to sync to cvmfs; exiting..." >&2
    exit 3
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

DIR_TO_SYNC=`dirname ${PATH_TO_SYNC}`
SRCPATH=dunedaq@daq.fnal.gov:/home/dunedaq/docker-scratch/cvmfs_dunedaq/$PATH_TO_SYNC
DESTPATH=/cvmfs/$REPO/$DIR_TO_SYNC

if [ ! -r $DESTPATH ]; then
  echo Destination path not found: $DESTPATH
  exit 3
fi


ROPTS="-rvplt --stats"
DRYRUN_ROPTS="$ROPTS -n"
MYDATE=`date +%Y%m%d_%H%M%S`
TAG=${DIR_TO_SYNC//\//_}-$MYDATE
LOG=$HOME/dunedaq-sync.log
RLOG=$HOME/logs/rsync_$TAG.log

echo Source: $SRCPATH
echo Destination: $DESTPATH
echo Tag: $TAG

echo >>$LOG
echo -n Transaction $TAG: >>$LOG
cvmfs_server transaction $REPO
COM="rsync $DRYRUN_ROPTS $SRCPATH $DESTPATH"

echo "The command which will run is \"$COM\"; proceed?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) break;;
        No ) exit 0;;
    esac
done

echo "Doing a dry run of rsync:" | tee $RLOG
echo $COM | tee $RLOG
echo "-------------------" | tee $RLOG
$COM 2>&1 | tee $RLOG
echo ""
echo "Do you wish to publish the above changes (if any)?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) break;;
        No ) cvmfs_server abort $REPO; echo "rsync & publishing aborted!"|tee $RLOG; exit;;
    esac
done

COM="rsync $ROPTS $SRCPATH $DESTPATH"
echo "Running rsync:" | tee $RLOG
echo $COM | tee $RLOG
echo "-------------------" | tee $RLOG
$COM 2>&1 | tee $RLOG
RET=$?
echo "-------------------"

if [ $RET -eq 0 ]; then
  echo
  echo Rsync succeeded. Publishing change.
  echo -n " rsync succeeded" >>$LOG
  cvmfs_server publish -a $TAG $REPO
else
  echo Rsync failed with error $RET. Aborting.
  echo -n " rsync failed with error $RET" >>$LOG
  cvmfs_server abort $REPO
fi

echo >>$LOG
echo Done
