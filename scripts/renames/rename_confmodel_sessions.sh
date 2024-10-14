#!/bin/bash

# JCF, Oct-8-2024

# This script is meant to address daq-deliverables Issue #149

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

fully_replace_token Session System confmodel
fully_replace_token session system confmodel
fully_replace_token DUNEDAQ_SESSION DUNEDAQ_SYSTEM confmodel
fully_replace_token TDAQ_SESSION DUNEDAQ_SYSTEM confmodel

for pkg in appmodel iomanager listrev appfwk dfmodules timinglibs hdf5libs daqconf dpdklibs hermesmodules; do 

    fully_replace_token Session System $pkg
    fully_replace_token session system $pkg
    fully_replace_token SESSION SYSTEM $pkg
done

cd appfwk
git mv ./test/config/appSession.data.xml ./test/config/appSystem.data.xml
cd ..

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
cd ..

