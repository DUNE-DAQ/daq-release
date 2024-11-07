#!/bin/bash

# JCF, Oct-8-2024

# This script is meant to address daq-deliverables Issue #149

# JCF, Nov-6-2024

# Overhauling the script thanks to Pierre's spreadsheet

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

echo "JCF, Nov-6-2024: THIS SCRIPT IS UNDER DEVELOPMENT, PLEASE CONTACT HIM FOR MORE INFO"
sleep 5

fully_replace_token partition session erskafka
fully_replace_token Partition Session erskafka
fully_replace_token PARTITION SESSION erskafka
fully_replace_token SESSION_UA PARTITION_UA erskafka # Otherwise "error: 'SESSION_UA' is not a member of 'RdKafka::Topic'"

fully_replace_token "modified in this DB session" "modified in this DB system" conffwk

fully_replace_token Session System confmodel
fully_replace_token session system confmodel
fully_replace_token DUNEDAQ_SESSION DUNEDAQ_SYSTEM confmodel
fully_replace_token TDAQ_SESSION DUNEDAQ_SYSTEM confmodel

fully_replace_token partition session iomanager 

# hermesmodules, dpdklibs, and other packages are left out of this
# loop for now since they depend on on appfwk, which we plan to
# manually edit

for pkg in appmodel daqconf listrev hdf5libs daqsystemtest; do

     fully_replace_token Session System $pkg
     fully_replace_token session system $pkg
     fully_replace_token SESSION SYSTEM $pkg

done

cd listrev
git mv config/lrSession-g.data.xml         config/lrSystem-g.data.xml
git mv config/lrSession-r.data.xml         config/lrSystem-r.data.xml
git mv config/lrSession-separate.data.xml  config/lrSystem-separate.data.xml
git mv config/lrSession-singleapp.data.xml config/lrSystem-singleapp.data.xml
git mv config/lrSession-v.data.xml         config/lrSystem-v.data.xml
git mv config/lrSession.data.xml           config/lrSystem.data.xml
cd ..

cd daqconf
git mv python/daqconf/get_session_apps.py    python/daqconf/get_system_apps.py
git mv python/daqconf/session.py             python/daqconf/system.py
git mv python/daqconf/set_session_env_var.py python/daqconf/set_system_env_var.py
git mv scripts/daqconf_set_session_env_var   scripts/daqconf_set_system_env_var
git checkout HEAD -- docs/Inspector.md
cd ..

echo
echo
echo "Don't forget to manually make changes for appfwk"


# cd appfwk
# git mv ./test/config/appSession.data.xml ./test/config/appSystem.data.xml
# cd ..

