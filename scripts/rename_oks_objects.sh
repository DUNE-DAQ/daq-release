#!/bin/bash

# JCF, Jun-13-2024

# This script is meant to address Issue #376

# Execute the name changes recommended in
# https://docs.google.com/spreadsheets/d/1k3ScIDQJTLHPjsMgsIPm8tGYSZa6HEm5f1xvsl8M5vU/edit?gid=0#gid=0

if [[ $( basename $PWD ) != "sourcecode" ]]; then
    echo "You need to be in the sourcecode/ directory of a work area to run this script" >&2
    exit 1
fi

if [[ -n $( find . -mindepth 1 -maxdepth 1 -type d ) ]]; then
    echo "There shouldn't be any directories (repos) in the work area when you run this script" >&2
    exit 2
fi

function gitclone {
    for pkg in "$@"; do
	if [[ ! -d $pkg ]]; then
	    git clone https://github.com/DUNE-DAQ/$pkg -b develop
	    if [[ "$?" != "0" ]]; then
		echo "Unable to clone $pkg; exiting.." >&2
		exit 3
	    fi

	    cd $pkg
	    git checkout -b johnfreeman/daq-release_issue376_renames_for_v5
	    cd ..
	fi
    done
}

function replace_token() {

    local old=$1
    local new=$2
    
    if [[ ! -d .git ]]; then
	echo "Looks like $PWD isn't a git repo; exiting..." >&2
	exit 4
    fi

    local tmpdir=$( mktemp -d )
    mv .git $tmpdir
    find . -type f | xargs sed -r -i 's!'$old'!'$new'!g'
    mv $tmpdir/.git .
    rm -rf $tmpdir

}

function fully_replace_token() {
    local old=$1
    local new=$2
    local affected_repos=$3

    gitclone $affected_repos

    for repo in $affected_repos; do
	cd $repo
	replace_token $old $new
	cd ..
    done
}

fully_replace_token QueueWithId QueueWithSourceId "readoutmodules dfmodules confmodel appmodel dpdklibs flxlibs"

fully_replace_token DataFlowOrchestrator DFOModule "dfmodules appmodel"

cd dfmodules
git mv plugins/DataFlowOrchestrator.hpp plugins/DFOModule.hpp                                             
git mv plugins/DataFlowOrchestrator.cpp plugins/DFOModule.cpp                                             
git mv unittest/DataFlowOrchestrator_test.cxx unittest/DFOModule_test.cxx
cd ..

fully_replace_token DataReaderConf DataReceiverConf "readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"
fully_replace_token DataReader DataReceiverModule "readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"


fully_replace_token DataRecorder DataRecorderModule "readoutlibs readoutmodules appmodel fdreadoutmodules"
fully_replace_token DataRecorderModuleConf DataRecorderConf "readoutlibs readoutmodules appmodel fdreadoutmodules" 
cd fdreadoutmodules
git mv plugins/DataRecorder.hpp plugins/DataRecorderModule.hpp
git mv plugins/DataRecorder.cpp plugins/DataRecorderModule.cpp
cd ..

fully_replace_token DataWriter DataWriterModule "appfwk dfmodules hdf5libs appmodel fdreadoutmodules integrationtest"
fully_replace_token DataWriterModuleConf DataWriterConf "appfwk dfmodules hdf5libs appmodel fdreadoutmodules integrationtest"
cd dfmodules
git mv plugins/DataWriter.hpp plugins/DataWriterModule.hpp
git mv plugins/DataWriter.cpp plugins/DataWriterModule.cpp
cd ..
