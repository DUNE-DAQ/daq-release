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
	    local branch_name=johnfreeman/daq-release_issue376_renames_for_v5_pt3
	    git checkout $branch_name || git checkout -b $branch_name
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


fully_replace_token DataReceiverConf DataReaderConf      "appmodel"
fully_replace_token DataReceiverModule DataReaderModule  "appmodel"

fully_replace_token WIBConfigurator WIBModule "appmodel wibmod"
cd wibmod
git mv plugins/WIBConfigurator.hpp plugins/WIBModule.hpp
git mv plugins/WIBConfigurator.cpp plugins/WIBModule.cpp
cd ..

########### EVERYTHING BELOW THIS LINE IS FOR THE HISTORICAL RECORD ###########

# NAME CHANGES FROM Jun-17-2024 (Round 2)

# fully_replace_token FDFakeCardReaderModule FDFakeReaderModule "appmodel oksconfgen fdreadoutmodules"
# cd fdreadoutmodules
# git mv plugins/FDFakeCardReaderModule.hpp plugins/FDFakeReaderModule.hpp
# git mv plugins/FDFakeCardReaderModule.cpp plugins/FDFakeReaderModule.cpp
# cd .. 


# NAME CHANGES FROM Jun-17-2024 (Round 1)

# fully_replace_token DataReceiverConf DataReaderConf      "readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"
# fully_replace_token DataReceiverModule DataReaderModule  "readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"

# fully_replace_token FDFakeCardReader FDFakeCardReaderModule "appmodel oksconfgen fdreadoutmodules"
# cd fdreadoutmodules
# git mv plugins/FDFakeCardReader.hpp plugins/FDFakeCardReaderModule.hpp
# git mv plugins/FDFakeCardReader.cpp plugins/FDFakeCardReaderModule.cpp
# cd ..

# # DPDK-related changes are just an automated double check on Alessandro's work, basically
# fully_replace_token DPDKReader DPDKReaderModule             "appmodel oksconfgen dpdklibs"
# fully_replace_token DPDKReaderModuleConf DPDKReaderConf     "appmodel oksconfgen dpdklibs"
# fully_replace_token DPDKReaderModuleModule DPDKReaderModule "appmodel oksconfgen dpdklibs"

######################################################################

# NAME CHANGES FROM Jun-14-2024 :

# fully_replace_token QueueWithId QueueWithSourceId "readoutmodules dfmodules confmodel appmodel dpdklibs flxlibs"

# fully_replace_token DataFlowOrchestrator DFOModule "dfmodules appmodel"

# cd dfmodules
# git mv plugins/DataFlowOrchestrator.hpp plugins/DFOModule.hpp                                             
# git mv plugins/DataFlowOrchestrator.cpp plugins/DFOModule.cpp                                             
# git mv unittest/DataFlowOrchestrator_test.cxx unittest/DFOModule_test.cxx
# cd ..

# fully_replace_token DataReaderConf DataReceiverConf "readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"
# fully_replace_token DataReader DataReceiverModule   "readoutmodules appmodel oksconfgen dpdklibs integrationtest flxlibs fdreadoutmodules"


# fully_replace_token DataRecorder DataRecorderModule         "readoutlibs readoutmodules appmodel fdreadoutmodules"
# fully_replace_token DataRecorderModuleConf DataRecorderConf "readoutlibs readoutmodules appmodel fdreadoutmodules" 
# cd fdreadoutmodules
# git mv plugins/DataRecorder.hpp plugins/DataRecorderModule.hpp
# git mv plugins/DataRecorder.cpp plugins/DataRecorderModule.cpp
# cd ..

# fully_replace_token DataWriter DataWriterModule         "appfwk dfmodules hdf5libs appmodel fdreadoutmodules integrationtest"
# fully_replace_token DataWriterModuleConf DataWriterConf "appfwk dfmodules hdf5libs appmodel fdreadoutmodules integrationtest"
# cd dfmodules
# git mv plugins/DataWriter.hpp plugins/DataWriterModule.hpp
# git mv plugins/DataWriter.cpp plugins/DataWriterModule.cpp
# cd ..

# fully_replace_token FakeDataProd FakeDataProdModule         "dfmodules timinglibs appmodel daqsystemtest"
# fully_replace_token FakeDataProdModuleConf FakeDataProdConf "dfmodules timinglibs appmodel daqsystemtest"

# cd dfmodules
# git mv plugins/FakeDataProd.hpp plugins/FakeDataProdModule.hpp
# git mv plugins/FakeDataProd.cpp plugins/FakeDataProdModule.cpp
# cd ..

# fully_replace_token FakeHSIEventGenerator FakeHSIEventGeneratorModule         "hsilibs timinglibs appmodel integrationtest"
# fully_replace_token FakeHSIEventGeneratorModuleConf FakeHSIEventGeneratorConf "hsilibs timinglibs appmodel integrationtest"

# cd hsilibs
# git mv plugins/FakeHSIEventGenerator.hpp plugins/FakeHSIEventGeneratorModule.hpp
# git mv plugins/FakeHSIEventGenerator.cpp plugins/FakeHSIEventGeneratorModule.cpp
# cd ..

# fully_replace_token FragmentAggregator FragmentAggregatorModule "dfmodules appmodel oksconfgen"

# cd dfmodules
# git mv plugins/FragmentAggregator.hpp plugins/FragmentAggregatorModule.hpp
# git mv plugins/FragmentAggregator.cpp plugins/FragmentAggregatorModule.cpp
# cd ..

# fully_replace_token HSIDataLinkHandler HSIDataHandlerModule "hsilibs appmodel integrationtest"
# cd hsilibs
# git mv plugins/HSIDataLinkHandler.hpp plugins/HSIDataHandlerModule.hpp
# git mv plugins/HSIDataLinkHandler.cpp plugins/HSIDataHandlerModule.cpp
# cd ..

# fully_replace_token ReadoutModuleConf DataHandlerConf           "readoutlibs readoutmodules hsilibs trigger appmodel oksconfgen fdreadoutlibs fdreadoutmodules integrationtest"
# fully_replace_token ReadoutModule DataHandlerModule             "readoutlibs readoutmodules hsilibs trigger appmodel oksconfgen fdreadoutlibs fdreadoutmodules integrationtest"
# fully_replace_token DataHandlerModulesIssues ReadoutModulesIssues "readoutlibs readoutmodules hsilibs trigger appmodel oksconfgen fdreadoutlibs fdreadoutmodules integrationtest"



# fully_replace_token TPStreamWriter TPStreamWriterModule                       "dfmodules appmodel daqsystemtest"
# fully_replace_token TPStreamWriterModuleApplication TPStreamWriterApplication "dfmodules appmodel daqsystemtest"
# fully_replace_token TPStreamWriterModuleConf TPStreamWriterConf               "dfmodules appmodel daqsystemtest"
# cd dfmodules
# git mv plugins/TPStreamWriter.hpp plugins/TPStreamWriterModule.hpp
# git mv plugins/TPStreamWriter.cpp plugins/TPStreamWriterModule.cpp
# cd ..

# fully_replace_token TriggerRecordBuilder TRBModule         "dfmodules appmodel"
# fully_replace_token TRBModuleData TriggerRecordBuilderData "dfmodules appmodel"
# cd dfmodules
# git mv plugins/TriggerRecordBuilder.hpp plugins/TRBModule.hpp
# git mv plugins/TriggerRecordBuilder.cpp plugins/TRBModule.cpp
# cd ..

# fully_replace_token FDDataLinkHandler FDDataHandlerModule "appmodel oksconfgen fdreadoutmodules"
# cd fdreadoutmodules
# git mv plugins/FDDataLinkHandler.hpp plugins/FDDataHandlerModule.hpp
# git mv plugins/FDDataLinkHandler.cpp plugins/FDDataHandlerModule.cpp
# cd ..

# fully_replace_token FelixCardReader FelixReaderModule "appmodel oksconfgen flxlibs"
# cd flxlibs
# git mv plugins/FelixCardReader.hpp plugins/FelixReaderModule.hpp
# git mv plugins/FelixCardReader.cpp plugins/FelixReaderModule.cpp
# cd ..

# fully_replace_token HermesController HermesModule "appmodel oksconfgen hermesmodules"
# cd hermesmodules
# git mv plugins/HermesController.hpp plugins/HermesModule.hpp
# git mv plugins/HermesController.cpp plugins/HermesModule.cpp
# cd ..

# fully_replace_token NICReceiverConf DPDKReaderConf "dpdklibs oksconfgen appmodel"
# fully_replace_token NICReceiver DPDKReader         "dpdklibs oksconfgen appmodel"
# cd dpdklibs
# git mv plugins/NICReceiver.hpp plugins/DPDKReader.hpp
# git mv plugins/NICReceiver.cpp plugins/DPDKReader.cpp
# cd ..

# fully_replace_token CustomTriggerCandidateMakerConf CustomTCMakerConf "trigger appmodel integrationtest"
# fully_replace_token CustomTriggerCandidateMaker CustomTCMaker         "trigger appmodel integrationtest"
# cd trigger
# git mv plugins/CustomTriggerCandidateMaker.hpp plugins/CustomTCMaker.hpp
# git mv plugins/CustomTriggerCandidateMaker.cpp plugins/CustomTCMaker.cpp
# cd ..

# fully_replace_token DataSubscriber DataSubscriberModule           "readoutlibs trigger appmodel oksconfgen"
# fully_replace_token DataSubscriberModuleBase DataSubscriberBase   "readoutlibs trigger appmodel oksconfgen"
# fully_replace_token DataSubscriberModuleModel DataSubscriberModel "readoutlibs trigger appmodel oksconfgen"
# cd trigger
# git mv plugins/DataSubscriber.hpp plugins/DataSubscriberModule.hpp
# git mv plugins/DataSubscriber.cpp plugins/DataSubscriberModule.cpp
# cd ..

# fully_replace_token ModuleLevelTriggerConf MLTConf "trigger appmodel integrationtest"
# fully_replace_token ModuleLevelTrigger MLTModule   "trigger appmodel integrationtest"
# cd trigger
# git mv plugins/ModuleLevelTrigger.hpp plugins/MLTModule.hpp
# git mv plugins/ModuleLevelTrigger.cpp plugins/MLTModule.cpp
# cd ..

# fully_replace_token RandomTriggerCandidateMakerConf RandomTCMakerConf "trigger appmodel integrationtest"
# fully_replace_token RandomTriggerCandidateMaker RandomTCMakerModule   "trigger appmodel integrationtest"
# cd trigger
# git mv plugins/RandomTriggerCandidateMaker.hpp plugins/RandomTCMakerModule.hpp
# git mv plugins/RandomTriggerCandidateMaker.cpp plugins/RandomTCMakerModule.cpp
# cd ..

# fully_replace_token StandaloneCandidateMakerConf StandaloneTCMakerConf "appmodel"
# fully_replace_token StandaloneCandidateMaker StandaloneTCMakerModule   "appmodel"

# fully_replace_token TCCustom TCCustomAlgorithm "appmodel"

# fully_replace_token TriggerActivityMakerHorizontalMuonPlugin TAMakerHorizontalMuonAlgorithm "trigger triggeralgs trgtools appmodel"
# fully_replace_token TriggerActivityMakerHorizontalMuon TAMakerHorizontalMuonAlgorithm       "trigger triggeralgs trgtools appmodel"
# cd triggeralgs
# git mv include/triggeralgs/HorizontalMuon/TriggerActivityMakerHorizontalMuon.hpp include/triggeralgs/HorizontalMuon/TAMakerHorizontalMuonAlgorithm.hpp 
# git mv src/TriggerActivityMakerHorizontalMuon.cpp src/TAMakerHorizontalMuonAlgorithm.cpp
# cd ..

# fully_replace_token TriggerActivityMakerPrescalePlugin TAMakerPrescaleAlgorithm "dfmodules trigger triggeralgs appmodel daqsystemtest"
# fully_replace_token TriggerActivityMakerPrescale TAMakerPrescaleAlgorithm       "dfmodules trigger triggeralgs appmodel daqsystemtest"
# cd triggeralgs
# git mv include/triggeralgs/Prescale/TriggerActivityMakerPrescale.hpp include/triggeralgs/Prescale/TAMakerPrescaleAlgorithm.hpp
# git mv src/TriggerActivityMakerPrescale.cpp src/TAMakerPrescaleAlgorithm.cpp
# cd ..

# fully_replace_token TriggerCandidateMakerHorizontalMuonPlugin TCMakerHorizontalMuonAlgorithm "trigger triggeralgs appmodel"
# fully_replace_token TriggerCandidateMakerHorizontalMuon TCMakerHorizontalMuonAlgorithm       "trigger triggeralgs appmodel"
# cd triggeralgs
# git mv include/triggeralgs/HorizontalMuon/TriggerCandidateMakerHorizontalMuon.hpp include/triggeralgs/HorizontalMuon/TCMakerHorizontalMuonAlgorithm.hpp
# git mv src/TriggerCandidateMakerHorizontalMuon.cpp src/TCMakerHorizontalMuonAlgorithm.cpp
# cd ..

# fully_replace_token TriggerCandidateMakerPrescalePlugin TCMakerPrescaleAlgorithm "dfmodules trigger triggeralgs appmodel daqsystemtest integrationtest"
# fully_replace_token TriggerCandidateMakerPrescale TCMakerPrescaleAlgorithm       "dfmodules trigger triggeralgs appmodel daqsystemtest integrationtest"
# cd triggeralgs
# git mv include/triggeralgs/Prescale/TriggerCandidateMakerPrescale.hpp include/triggeralgs/Prescale/TCMakerPrescaleAlgorithm.hpp
# git mv src/TriggerCandidateMakerPrescale.cpp src/TCMakerPrescaleAlgorithm.cpp
# cd ..

# fully_replace_token TriggerDataHandler TriggerDataHandlerModule "trigger appmodel oksconfgen integrationtest"
# cd trigger
# git mv plugins/TriggerDataHandler.hpp plugins/TriggerDataHandlerModule.hpp
# git mv plugins/TriggerDataHandler.cpp plugins/TriggerDataHandlerModule.cpp
# cd ..
