#!/bin/bash

# JCF, Jun-13-2024

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

# QueueWithId -> QueueWithSourceId

affected_repos="readoutmodules dfmodules confmodel appmodel dpdklibs flxlibs"

gitclone $affected_repos

for repo in $affected_repos; do
    cd $repo
    replace_token QueueWithId QueueWithSourceId 
    cd ..
done

# DataFlowOrchestrator -> DFOModule

affected_repos="dfmodules appmodel"

gitclone $affected_repos

for repo in $affected_repos; do                                                                             
    cd $repo                                                                                                
    replace_token DataFlowOrchestrator DFOModule

    if [[ $repo == dfmodules ]]; then
	git mv plugins/DataFlowOrchestrator.hpp plugins/DFOModule.hpp
	git mv plugins/DataFlowOrchestrator.cpp plugins/DFOModule.cpp
	git mv unittest/DataFlowOrchestrator_test.cxx unittest/DFOModule_test.cxx
    fi
    
    cd ..                                                                                                   
done  

# DataReader -> DataReceiverModule, DataReaderConf -> DataReceiverConf

affected_repos="readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"

gitclone $affected_repos

for repo in $affected_repos; do
    cd $repo
    replace_token DataReaderConf DataReceiverConf
    replace_token DataReader DataReceiverModule
    cd ..
done

# DataRecorder -> DataRecorderModule, DataRecorderConf is unchanged

affected_repos="readoutlibs readoutmodules appmodel fdreadoutmodules"

gitclone $affected_repos

for repo in $affected_repos; do
    cd $repo
    replace_token DataRecorder DataRecorderModule
    replace_token DataRecorderModuleConf DataRecorderConf  # partial reversion

    if [[ $repo == fdreadoutmodules ]]; then
	git mv plugins/DataRecorder.hpp plugins/DataRecorderModule.hpp
	git mv plugins/DataRecorder.cpp plugins/DataRecorderModule.cpp
    fi
    
    cd ..
done

# DataWriter -> DataWriterModule, DataWriteConf unchanged

affected_repos="appfwk dfmodules hdf5libs appmodel fdreadoutmodules integrationtest"

gitclone $affected_repos

for repo in $affected_repos; do
    cd $repo
    replace_token DataWriter DataWriterModule
    replace_token DataWriterModuleConf DataWriterConf # partial reversion

    if [[ $repo == dfmodules ]]; then
	git mv plugins/DataWriter.hpp plugins/DataWriterModule.hpp
	git mv plugins/DataWriter.cpp plugins/DataWriterModule.cpp
    fi

    cd ..
done
    
    
