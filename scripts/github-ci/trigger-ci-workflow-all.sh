#!/bin/bash

dune_packages_with_ci=(
 "daq-cmake"       
 "ers"             
 "erskafka"        
 "logging"         
 "opmonlib"        
 "cmdlib"          
 "rcif"            
 "restcmd"         
 "utilities"       
 "ipm"             
 "networkmanager"  
 "appfwk"          
 "listrev"         
 "serialization"   
 "nwqueueadapters" 
 "daqdataformats"  
 "detdataformats"  
 "detchannelmaps"  
 "dfmessages"      
 "trigemu"         
 "triggeralgs"     
 "timing"          
 "timinglibs"      
 "hdf5libs"        
 "trigger"         
 "readoutlibs"     
 "fdreadoutlibs"   
 "ndreadoutlibs"   
 "readoutmodules"  
 "flxlibs"         
 "dfmodules"       
 "influxopmon"     
 "kafkaopmon"      
 "daqconf"         
 "dqm"             
 "lbrulibs"        
 "wibmod"          
 "sspmodules"      
 "uhallibs"
 "dtpcontrols"
 "dtpctrllibs"
 "rawdatautils"
)

function trigger_ci_all {
  prd_list_name=$1[@]
  workflow_file=$2
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]//_/-}
    # Trigger workflow 
    #last_failed_scheudle_run=$(gh run -R DUNE-DAQ/trigemu list|grep build-develop| grep failure| head -n 1|egrep -o '[[:digit:]]{10}')
    gh workflow -R DUNE-DAQ/${prod_name} run $workflow_file
  done
}


trigger_ci_all dune_packages_with_ci dunedaq-develop-cpp-ci.yml
