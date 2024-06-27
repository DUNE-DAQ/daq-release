#!/bin/bash

# JCF, Jun-13-2024

# This script is meant to address Issue #376

# Execute the name changes recommended in
# https://docs.google.com/spreadsheets/d/1k3ScIDQJTLHPjsMgsIPm8tGYSZa6HEm5f1xvsl8M5vU/edit?gid=0#gid=0

# JCF, Jun-27-2024

# Update the script to
# (1) apply the successive waves of name changes to previously-untouched daqconf, and
# (2) make the change DataLinkHandler -> RawDataHandler

if [[ $( basename $PWD ) != "sourcecode" ]]; then
    echo "You need to be in the sourcecode/ directory of a work area to run this script" >&2
    exit 1
fi

if [[ -n $( find . -mindepth 1 -maxdepth 1 -type d ) ]]; then
    echo "There shouldn't be any directories (repos) in the work area when you run this script" >&2
    exit 2
fi

scriptdir=$(dirname "${BASH_SOURCE[0]}")
. $scriptdir/renaming_tools.sh


# Run all the rounds of substitutions shown below the fold on previously-untouched daqconf
fully_replace_token QueueWithId QueueWithSourceId daqconf
fully_replace_token DataFlowOrchestrator DFOModule daqconf

fully_replace_token DataReaderConf DataReceiverConf daqconf
fully_replace_token DataReader DataReceiverModule daqconf

fully_replace_token DataRecorder DataRecorderModule daqconf
fully_replace_token DataRecorderModuleConf DataRecorderConf daqconf

fully_replace_token DataWriter DataWriterModule daqconf
fully_replace_token DataWriterModuleConf DataWriterConf daqconf

fully_replace_token FakeDataProd FakeDataProdModule daqconf
fully_replace_token FakeDataProdModuleConf FakeDataProdConf daqconf

fully_replace_token FakeHSIEventGenerator FakeHSIEventGeneratorModule daqconf
fully_replace_token FakeHSIEventGeneratorModuleConf FakeHSIEventGeneratorConf daqconf

fully_replace_token FragmentAggregator FragmentAggregatorModule daqconf
fully_replace_token HSIDataLinkHandler HSIDataHandlerModule daqconf

fully_replace_token ReadoutModuleConf DataHandlerConf daqconf
fully_replace_token ReadoutModule DataHandlerModule daqconf
fully_replace_token DataHandlerModulesIssues ReadoutModulesIssues daqconf

fully_replace_token TPStreamWriter TPStreamWriterModule daqconf
fully_replace_token TPStreamWriterModuleApplication TPStreamWriterApplication daqconf
fully_replace_token TPStreamWriterModuleConf TPStreamWriterConf daqconf

fully_replace_token TriggerRecordBuilder TRBModule daqconf
fully_replace_token TRBModuleData TriggerRecordBuilderData daqconf

fully_replace_token FDDataLinkHandler FDDataHandlerModule daqconf
fully_replace_token FelixCardReader FelixReaderModule daqconf

fully_replace_token HermesController HermesModule daqconf
fully_replace_token NICReceiverConf DPDKReaderConf daqconf
fully_replace_token NICReceiver DPDKReader daqconf

fully_replace_token CustomTriggerCandidateMakerConf CustomTCMakerConf daqconf
fully_replace_token CustomTriggerCandidateMaker CustomTCMaker daqconf

fully_replace_token DataSubscriber DataSubscriberModule daqconf
fully_replace_token DataSubscriberModuleBase DataSubscriberBase daqconf
fully_replace_token DataSubscriberModuleModel DataSubscriberModel daqconf

fully_replace_token ModuleLevelTriggerConf MLTConf daqconf
fully_replace_token ModuleLevelTrigger MLTModule daqconf

fully_replace_token RandomTriggerCandidateMakerConf RandomTCMakerConf daqconf
fully_replace_token RandomTriggerCandidateMaker RandomTCMakerModule daqconf

fully_replace_token StandaloneCandidateMakerConf StandaloneTCMakerConf daqconf
fully_replace_token StandaloneCandidateMaker StandaloneTCMakerModule daqconf

fully_replace_token TCCustom TCCustomAlgorithm daqconf

fully_replace_token TriggerActivityMakerHorizontalMuonPlugin TAMakerHorizontalMuonAlgorithm daqconf
fully_replace_token TriggerActivityMakerHorizontalMuon TAMakerHorizontalMuonAlgorithm daqconf
fully_replace_token TriggerActivityMakerPrescalePlugin TAMakerPrescaleAlgorithm daqconf
fully_replace_token TriggerActivityMakerPrescale TAMakerPrescaleAlgorithm daqconf

fully_replace_token TriggerCandidateMakerHorizontalMuonPlugin TCMakerHorizontalMuonAlgorithm daqconf
fully_replace_token TriggerCandidateMakerHorizontalMuon TCMakerHorizontalMuonAlgorithm daqconf

fully_replace_token TriggerCandidateMakerPrescalePlugin TCMakerPrescaleAlgorithm daqconf
fully_replace_token TriggerCandidateMakerPrescale TCMakerPrescaleAlgorithm daqconf
fully_replace_token TriggerDataHandler TriggerDataHandlerModule daqconf

fully_replace_token DataReceiverConf DataReaderConf daqconf
fully_replace_token DataReceiverModule DataReaderModule daqconf

fully_replace_token FDFakeCardReader FDFakeCardReaderModule daqconf
fully_replace_token DPDKReader DPDKReaderModule daqconf
fully_replace_token DPDKReaderModuleConf DPDKReaderConf daqconf
fully_replace_token DPDKReaderModuleModule DPDKReaderModule daqconf

fully_replace_token FDFakeCardReaderModule FDFakeReaderModule daqconf

fully_replace_token DataReceiverConf DataReaderConf  daqconf
fully_replace_token DataReceiverModule DataReaderModule daqconf
fully_replace_token WIBConfigurator WIBModule daqconf
fully_replace_token ProtoWIBModule ProtoWIBConfigurator daqconf

# And now make the change described in bullet point 6 of Alessandro's comment for daq-release Issue #379

fully_replace_token DataLinkHandler RawDataHandler "datahandlinglibs hsilibs trigger daqconf dpdklibs fdreadoutmodules"
cd datahandlinglibs
git mv include/datahandlinglibs/DataLinkHandlerBase.hpp include/datahandlinglibs/RawDataHandlerBase.hpp
git mv include/datahandlinglibs/detail/DataLinkHandlerBase.hxx include/datahandlinglibs/detail/RawDataHandlerBase.hxx
cd ..

########### EVERYTHING BELOW THIS LINE IS FOR THE HISTORICAL RECORD ###########

# Name changes from June-18-2024

# fully_replace_token DataReceiverConf DataReaderConf      "appmodel"
# fully_replace_token DataReceiverModule DataReaderModule  "appmodel"

# fully_replace_token WIBConfigurator WIBModule           "appmodel wibmod"
# fully_replace_token ProtoWIBModule ProtoWIBConfigurator "appmodel wibmod"
# cd wibmod
# git mv plugins/WIBConfigurator.hpp plugins/WIBModule.hpp
# git mv plugins/WIBConfigurator.cpp plugins/WIBModule.cpp
# cd ..

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
