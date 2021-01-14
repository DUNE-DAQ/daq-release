#!/bin/bash

## This script is to validate the release setup.

source /cvmfs/dune.opensciencegrid.org/dune/dunedaq/DUNE/products/setup
ups list -aK+
ups active
