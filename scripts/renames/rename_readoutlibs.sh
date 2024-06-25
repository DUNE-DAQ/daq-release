#!/bin/bash

# JCF, Jun-24-2024

# This script is meant to address Issue #379

if [[ $( basename $PWD ) != "sourcecode" ]]; then
    echo "You need to be in the sourcecode/ directory of a work area to run this script" >&2
    exit 1
fi

if [[ -n $( find . -mindepth 1 -maxdepth 1 -type d ) ]]; then
    echo "There shouldn't be any directories (repos) in the work area when you run this script" >&2
    exit 2
fi

sourcedir=$PWD

scriptdir=$(dirname "${BASH_SOURCE[0]}")
. $scriptdir/renaming_tools.sh


gitclone datahandlinglibs

tmpdir=$( mktemp -d )
cd $tmpdir
gitclone readoutlibs
gitclone readoutmodules
cd $sourcedir

res=$( diff -r -x .git datahandlinglibs $tmpdir/readoutlibs  )

if [[ -z $res ]]; then
    rm -rf $tmpdir/readoutlibs
else
    echo "datahandlinglibs is not up to date with $tmpdir/readoutlibs ; exiting..." >&2
    exit 1
fi

cd datahandlinglibs

git mv include/readoutlibs include/datahandlinglibs
git mv schema/readoutlibs schema/datahandlinglibs
git mv cmake/readoutlibsConfig.cmake.in cmake/datahandlinglibsConfig.cmake.in
git mv unittest/readoutlibs_BufferedReadWrite_test.cxx unittest/datahandlinglibs_BufferedReadWrite_test.cxx
git mv unittest/readoutlibs_VariableSizeElementQueue_test.cxx unittest/datahandlinglibs_VariableSizeElementQueue_test.cxx

cp $tmpdir/readoutmodules/scripts/* scripts/
cp $tmpdir/readoutmodules/schema/readoutmodules/* schema/datahandlinglibs
cp -rp $tmpdir/readoutmodules/include/readoutmodules/* include/datahandlinglibs
rm include/datahandlinglibs/ReadoutModulesIssues.hpp  # readoutmodules has just a subset of exactly the same set of ERS Issues as readoutlibs
git add -A
cd ..

# header guard replacement
fully_replace_token READOUTLIBS DATAHANDLINGLIBS "datahandlinglibs"
fully_replace_token READOUTMODULES DATAHANDLINGLIBS "datahandlinglibs"

fully_replace_token readoutlibs datahandlinglibs "datahandlinglibs dfmodules hsilibs trigger dpdklibs fdreadoutlibs fdreadoutmodules flxlibs"
fully_replace_token fddatahandlinglibs fdreadoutlibs "trigger dpdklibs fdreadoutlibs fdreadoutmodules flxlibs"

fully_replace_token readoutmodules datahandlinglibs "datahandlinglibs dfmodules hsilibs trigger dpdklibs fdreadoutlibs fdreadoutmodules flxlibs"
fully_replace_token fddatahandlinglibs fdreadoutmodules "fdreadoutmodules"

fully_replace_token fddatahandlinglibs fdreadoutlibs "trigger dpdklibs fdreadoutlibs fdreadoutmodules flxlibs"

fully_replace_token ReadoutIssues DataHandlingIssues "datahandlinglibs hsilibs trigger dpdklibs fdreadoutlibs fdreadoutmodules flxlibs"
fully_replace_token ReadoutModulesIssues DataHandlingIssues "datahandlinglibs"
fully_replace_token FDDataHandlingIssues FDReadoutIssues "fdreadoutlibs"
cd datahandlinglibs
git mv include/datahandlinglibs/ReadoutIssues.hpp include/datahandlinglibs/DataHandlingIssues.hpp
cd ..

fully_replace_token ReadoutConcept DataHandlingConcept "datahandlinglibs hsilibs trigger fdreadoutmodules"
cd datahandlinglibs
git mv include/datahandlinglibs/concepts/ReadoutConcept.hpp include/datahandlinglibs/concepts/DataHandlingConcept.hpp
cd ..

fully_replace_token ReadoutModel DataHandlingModel "datahandlinglibs hsilibs trigger dpdklibs fdreadoutmodules"
cd datahandlinglibs
git mv include/datahandlinglibs/models/ReadoutModel.hpp include/datahandlinglibs/models/DataHandlingModel.hpp
git mv include/datahandlinglibs/models/detail/ReadoutModel.hxx include/datahandlinglibs/models/detail/DataHandlingModel.hxx
cd ..

