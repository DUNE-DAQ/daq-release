#!/bin/bash

## This script is intended to be used by GitHub Action's Ubuntu Runner node.
## The node specs are: 2-core CPU, 7GB RAM, 14 SSD disk space.

## Before running CI jobs with SL7 container, the host OS must run this script 
## to have cvmfs installed.

wget https://ecsft.cern.ch/dist/cvmfs/cvmfs-release/cvmfs-release-latest_all.deb
sudo dpkg -i cvmfs-release-latest_all.deb
rm -f cvmfs-release-latest_all.deb
sudo apt-get update
sudo apt-get -y install cvmfs
