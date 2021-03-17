#!/bin/bash


if [ "${BASH_SOURCE[0]}" -ef "$0" ]
then
	echo "This script is meant to be sourced, not executed!"
	exit 1
fi


function setup_dbt
{
	source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/tools/dbt/daq-buildtools-${1}/dbt-setup-env.sh
}
