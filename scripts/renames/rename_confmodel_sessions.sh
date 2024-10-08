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

