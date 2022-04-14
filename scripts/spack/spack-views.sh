#!/bin/bash
if [ $# -ge 1 ]; then
  LOCATION=$1; shift
else
  echo "$0: expecting 1 argument: [absolute path] "
  return
fi

source $HOME/spack/share/spack/setup-env.sh

if [[ -d "$LOCATION" ]]
then
    echo "using $LOCATION to set the views"
else
    mkdir -p $LOCATION
   echo "$LOCATION  path not existing, creating it"
fi
echo "starting with the compiler gcc"
spack view --dependencies no  symlink -i $LOCATION gcc@8.2.0
echo "copy of cmake"
spack view --dependencies no symlink $LOCATION cmake@3.20.5
echo "copy of folly"
spack view --verbose symlink $LOCATION folly@2021.05.24.00
echo "copy of trace"
spack view --verbose symlink $LOCATION trace
echo "copy of nlohmann-json"
spack view --verbose symlink $LOCATION nlohmann-json@3.9.1
echo "copy of pistache"
spack view --exclude openssl --verbose symlink $LOCATION pistache
echo "copy of highfive"
spack view --exclude openssl --exclude ncurses --verbose symlink $LOCATION highfive@2.2.2
echo "copy of msgpack-c"
spack view --verbose symlink $LOCATION msgpack-c@3.3.0
echo "copy of pybind11"
spack view --exclude openssl --exclude bzip2 --exclude libxml2 --exclude ncurses --verbose symlink -i $LOCATION py-pybind11@2.6.2
echo "copy of ipbus-sfotware"
spack view --exclude openssl --exclude libxml2 --exclude bzip2 --exclude ncurses --exclude python --exclude py-pybind11 --exclude util-linux-uuid --verbose symlink $LOCATION ipbus-software@2.8.0
echo "copy of libzmq"
spack view --verbose symlink $LOCATION libzmq@4.3.4
echo "copy of felix-software"
spack view --verbose symlink $LOCATION felix-software
echo "copy of cetmodules"
spack view --verbose symlink $LOCATION cetmodules@2.25.05
echo "copy of hep-concurrency"
spack view --exclude openssl --exclude ncurses --verbose symlink -i $LOCATION hep-concurrency
echo "copy of cetlib-except"
spack view --exclude ncurses --exclude catch2 --exclude openssl --verbose symlink $LOCATION cetlib-except
echo "copy of cetlib"
spack view --exclude openssl --exclude boost --exclude bzip2 --exclude ncurses --exclude catch2 --exclude hep-concurrency --exclude cetlib-except --exclude intel-tbb --verbose symlink -i $LOCATION cetlib
