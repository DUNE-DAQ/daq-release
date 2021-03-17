#!/bin/bash


if [ "${BASH_SOURCE[0]}" -ef "$0" ]
then
	echo "This script is meant to be sourced, not executed!"
	exit 1
fi


function setup_dbt
{
	cvmfs_path="/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/tools/dbt"
	if [ ! -d ${cvmfs_path}/daq-buildtools-${1} ]; then
		echo -e "\033[31mError: Tagged version ${1} of daq-buildtools not found under ${cvmfs_path}\e[0m"
		echo "Available versions are: "
		for i in `ls ${cvmfs_path}`; do
			echo ${i/daq-buildtools-/};
		done
		return
	fi

	source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/tools/dbt/daq-buildtools-${1}/dbt-setup-env.sh
}
